#include <pebble.h>

#define MARGIN	5
#define SPACING	5
#define DIGITSH	70
#define FONT7W	5
#define FONT7H	7
#define FONT10W	10
#define FONT10H	10

enum vibe {
	VIBE_SILENT,
	VIBE_SHORT,
	VIBE_LONG,
	VIBE_DOUBLE,
};

typedef uint8_t		u8;
typedef int8_t		i8;
typedef int16_t		i16;
typedef enum vibe	Vibe;

static void	vibe		(Vibe);
static void	drawpixel	(GBitmapDataRowInfo*, i16, GColor);
static void	dither		(Layer*, GContext*);
static void	drawdigits	(char*, GPoint offset, GContext*);
static void	drawshadow	(char*, GPoint offset, GContext*);
static void	onwinload	(Window*);
static void	onwinunload	(Window*);
static void	onbody		(Layer*, GContext*);
static void	onhour		(Layer*, GContext*);
static void	onminute	(Layer*, GContext*);
static void	onbottom	(Layer*, GContext*);
static void	onside		(Layer*, GContext*);
static void	ontick		(struct tm*, TimeUnits);

static struct {
	GColor	bg;
	GColor	fg;
	char	side[64];
	char	bottom[64];
	Vibe	bton;
	Vibe	btoff;
	Vibe	onhour;
	bool	padh;
	u8	shadow;
	i8	seconds;
} conf;

static struct {
	GDrawCommandImage*	digits;
	GFont	small;
	GFont	tiny;
} asset;

static struct {
	Layer*	body;
	Layer*	hour;
	Layer*	minute;
	Layer*	bottom;
	Layer*	side;
} layout;

static const i16 digitswidth[10] = {60, 30, 60, 60, 45, 60, 60, 45, 60, 60};

void
vibe(Vibe type)
{
	switch (type) {
	case VIBE_SILENT: break;
	case VIBE_SHORT:  vibes_short_pulse(); break;
	case VIBE_LONG:   vibes_long_pulse(); break;
	case VIBE_DOUBLE: vibes_double_pulse(); break;
	}
}

void
drawpixel(GBitmapDataRowInfo *info, i16 x, GColor color)
{
#if defined(PBL_COLOR)
	memset(info->data + x, color.argb, 1);
#elif defined(PBL_BW)
	u8 byte  = x / 8;
	u8 bit   = x % 8;
	u8 value = gcolor_equal(color, GColorWhite) ? 1 : 0;
	u8 *bp   = info->data + byte;
	*bp ^= (-value ^ *bp) & (1 << bit);
#endif
}

void
dither(Layer *layer, GContext *ctx)
{
	static const uint8_t map[8][8] = {
		{   0, 128,  32, 160,   8, 136,  40, 168 },
		{ 192,  64, 224,  96, 200,  72, 232, 104 },
		{  48, 176,  16, 144,  56, 184,  24, 152 },
		{ 240, 112, 208,  80, 248, 120, 216,  88 },
		{  12, 140,  44, 172,   4, 132,  36, 164 },
		{ 204,  76, 236, 108, 196,  68, 228, 100 },
		{  60, 188,  28, 156,  52, 180,  20, 148 },
		{ 252, 124, 220,  92, 244, 116, 212,  84 }
	};
	GRect rect;
	GBitmap *fb;
	GBitmapDataRowInfo info;
	i16 x, y, maxx, maxy;
	u8 amount;

	amount = 255 - conf.shadow;
	rect = layer_get_frame(layer);
	maxy = rect.origin.y + rect.size.h;
	fb = graphics_capture_frame_buffer(ctx);

	for (y = rect.origin.y; y < maxy; y++) {
		info = gbitmap_get_data_row_info(fb, y);
		maxx = rect.origin.x + rect.size.w;

		if (info.max_x < maxx)
			maxx = info.max_x;

		for (x = rect.origin.x; x < maxx; x++)
			if (amount > map[y%8][x%8])
				drawpixel(&info, x, conf.bg);
	}

	graphics_release_frame_buffer(ctx, fb);
}

void
drawdigits(char *str, GPoint offset, GContext *ctx)
{
	GDrawCommandList *cmds;
	GDrawCommand *cmd;
	i16 i, digit;

	cmds = gdraw_command_image_get_command_list(asset.digits);

	for (i = strlen(str); i; i--) {
		digit = str[i-1] - '0';
		cmd = gdraw_command_list_get_command(cmds, digit);
		gdraw_command_set_hidden(cmd, false);
		gdraw_command_set_stroke_color(cmd, conf.fg);
		gdraw_command_set_fill_color(cmd, conf.fg);
		offset.x -= digitswidth[digit];
		gdraw_command_image_draw(ctx, asset.digits, offset);
		offset.x -= SPACING;
		gdraw_command_set_hidden(cmd, true);
	}
}

void
drawshadow(char *str, GPoint offset, GContext *ctx)
{
	GDrawCommandList *cmds;
	GDrawCommand *cmd;
	i16 i, digit;

	cmds = gdraw_command_image_get_command_list(asset.digits);

	for (i=0; str[i]; i++) {
		digit = str[i] - '0';
		cmd = gdraw_command_list_get_command(cmds, digit);
		gdraw_command_set_hidden(cmd, false);
		gdraw_command_set_stroke_color(cmd, conf.fg);
		gdraw_command_set_fill_color(cmd, conf.fg);
		gdraw_command_image_draw(ctx, asset.digits, offset);
		offset.x += digitswidth[digit] + SPACING;
		gdraw_command_set_hidden(cmd, true);
	}
}

void
onwinload(Window *win)
{
	Layer *root;
	GRect rect;

	root = window_get_root_layer(win);
	rect = layer_get_bounds(root);

	layout.body = layer_create(rect);
	layer_set_update_proc(layout.body, onbody);
	layer_add_child(root, layout.body);

	rect.origin.x = MARGIN + FONT7W + SPACING;
	rect.origin.y = MARGIN;
	rect.size.w = PBL_DISPLAY_WIDTH - MARGIN*2 - SPACING - FONT7W;
	rect.size.h = DIGITSH;

	layout.hour = layer_create(rect);
	layer_set_update_proc(layout.hour, onhour);
	layer_add_child(layout.body, layout.hour);

	rect.origin.y += SPACING + DIGITSH;

	layout.minute = layer_create(rect);
	layer_set_update_proc(layout.minute, onminute);
	layer_add_child(layout.body, layout.minute);

	rect.origin.x = MARGIN;
	rect.origin.y += SPACING + DIGITSH;
	rect.size.w = PBL_DISPLAY_WIDTH - MARGIN*2;
	rect.size.h = FONT10H;

	layout.bottom = layer_create(rect);
	layer_set_update_proc(layout.bottom, onbottom);
	layer_add_child(layout.body, layout.bottom);

	rect.origin.x = MARGIN;
	rect.origin.y = MARGIN;
	rect.size.w = FONT7W;
	rect.size.h = PBL_DISPLAY_HEIGHT - MARGIN*2 - SPACING - FONT10H;

	layout.side = layer_create(rect);
	layer_set_update_proc(layout.side, onside);
	layer_add_child(layout.body, layout.side);
}

void
onwinunload(Window *_win)
{
	layer_destroy(layout.hour);
	layer_destroy(layout.minute);
	layer_destroy(layout.bottom);
	layer_destroy(layout.side);
	layer_destroy(layout.body);
}

void
onbody(Layer *layer, GContext *ctx)
{
	graphics_context_set_fill_color(ctx, conf.bg);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void
onhour(Layer *layer, GContext *ctx)
{
	time_t timestamp;
	struct tm *tm;
	char buf[8], *pt;
	GRect bounds;
	GPoint offset;

	timestamp = time(0);
	tm = localtime(&timestamp);
	strftime(buf, sizeof buf, clock_is_24h_style() ? "%H" : "%I", tm);

	pt = buf;

	if (conf.padh && pt[0] == '0')
		pt++;

	bounds = layer_get_bounds(layer);

	offset.x = 0;
	offset.y = 0;
	drawshadow(buf, offset, ctx);
	dither(layer, ctx);

	offset.x = bounds.size.w;
	offset.y = 0;
	drawdigits(pt, offset, ctx);
}

void
onminute(Layer *layer, GContext *ctx)
{
	time_t timestamp;
	struct tm *tm;
	char buf[8];
	GRect bounds;
	GPoint offset;

	timestamp = time(0);
	tm = localtime(&timestamp);
	strftime(buf, sizeof buf, "%M", tm);

	bounds = layer_get_bounds(layer);

	offset.x = 0;
	offset.y = 0;
	drawshadow(buf, offset, ctx);
	dither(layer, ctx);

	offset.x = bounds.size.w;
	offset.y = 0;
	drawdigits(buf, offset, ctx);
}

void
onbottom(Layer *layer, GContext *ctx)
{
	(void)layer;
	(void)ctx;
}

void
onside(Layer *layer, GContext *ctx)
{
	(void)layer;
	(void)ctx;
}


void
ontick(struct tm *_time, TimeUnits change)
{
	layer_mark_dirty(layout.bottom);
	layer_mark_dirty(layout.side);

	if (change & HOUR_UNIT) {
		layer_mark_dirty(layout.hour);
		vibe(conf.onhour);
	}

	if (change & MINUTE_UNIT)
		layer_mark_dirty(layout.minute);
}

int
main(void)
{
	Window *win;
	WindowHandlers wh;
	time_t timestamp;

	/* resources */
	asset.digits = gdraw_command_image_create_with_resource(RESOURCE_ID_DIGITS);
	asset.small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT10));
	asset.tiny = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT7));

	/* config */
	conf.bg = GColorWhite;
	conf.fg = GColorBlack;
	strncpy(conf.side, "%B %Y", sizeof conf.side);
	strncpy(conf.bottom, "%A %d", sizeof conf.bottom);
	conf.bton = VIBE_SILENT;
	conf.btoff = VIBE_SILENT;
	conf.onhour = VIBE_SILENT;
	conf.padh = false;
	conf.shadow = 16;
	conf.seconds = 0;

	/* window */
	win = window_create();
	wh.load = onwinload;
	wh.appear = 0;
	wh.disappear = 0;
	wh.unload = onwinunload;
	window_set_window_handlers(win, wh);
	window_stack_push(win, true);

	/* time */
	timestamp = time(0);
	ontick(localtime(&timestamp), DAY_UNIT | HOUR_UNIT | MINUTE_UNIT);
	/* Tick timer is overwritten in configure() but this is a
	 * default just in case there is something wrong with config
	 * which might happen when phone is disconnected, probably.
	 */
	tick_timer_service_subscribe(MINUTE_UNIT, ontick);

	/* main */
	app_event_loop();

	/* cleanup */
	window_destroy(win);
	gdraw_command_image_destroy(asset.digits);
	fonts_unload_custom_font(asset.small);
	fonts_unload_custom_font(asset.tiny);

	return 0;
}
