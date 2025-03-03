#include <pebble.h>

#define CONFKEY 1
#define MARGIN 5

#define ASCII_RANGE_PRINTABLE ('~' - ' ')

enum font {
	TINY  = (0 - ' '),
	SMALL = (0 - ' ' + ASCII_RANGE_PRINTABLE),
	BIG   = (0 - '0' + ASCII_RANGE_PRINTABLE * 2),
};

enum vibe { SILENT, SHORT, LONG, DOUBLE };

static struct {
	GColor bg, fg;
	char left[32], bottom[32];
	enum vibe bt_on, bt_off, each_hour;
} config;

static Layer *body, *hours, *minutes, *left, *bottom;
static GBitmap *glyphs, *glyph[256]={};
static GColor palette[2];

static struct tm *
now()
{
	time_t timestamp;
	struct tm *tm;

	/* NOTE(irek): Pebble OS assumes that this never fail */
	timestamp = time(0);
	tm = localtime(&timestamp);

	return tm;
}

static GBitmap *
glyph_get(enum font font, char c)
{
	// Check if C is in range of FONT, use default if not
	switch (font) {
	case BIG:
		if (!(c >= '0' && c <= '9'))
			c = '0';
		break;
	case SMALL:
	case TINY:
		if (c >= 'a' && c <= 'z')
			c = 0xDF & c;	// Uppercase

		if (!(c >= '0' && c <= '9') &&
		    !(c >= 'A' && c <= 'Z') &&
		    c != ' ' && c != '%' && c != '#')
			c = ' ';
		break;
	}
	return glyph[font + c];
}

static void
vibe(enum vibe type)
{
	switch (type) {
	case SILENT: break;
	case SHORT:  vibes_short_pulse();  break;
	case LONG:   vibes_long_pulse();   break;
	case DOUBLE: vibes_double_pulse(); break;
	}
}

static void
Body(Layer *layer, GContext *ctx)
{
	GRect bounds;
	BatteryChargeState battery;
	int percent;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void
Hours(Layer *layer, GContext *ctx)
{
	char buf[4];
	GRect bounds, rect, tmp;
	struct tm *tm;
	GBitmap *g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_compositing_mode(ctx, GCompOpSet);

	tm = now();
	strftime(buf, sizeof buf, clock_is_24h_style() ? "%H" : "%I", tm);

	g = glyph_get(BIG, buf[1]);
	tmp = gbitmap_get_bounds(g);
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	rect.origin.x = bounds.size.w - tmp.size.w;
	rect.origin.y = bounds.origin.y;
	graphics_draw_bitmap_in_rect(ctx, g, rect);

	if (buf[0] == '0')
		return;

	g = glyph_get(BIG, buf[0]);
	tmp = gbitmap_get_bounds(g);
	rect.origin.x -= tmp.size.w + MARGIN;
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	graphics_draw_bitmap_in_rect(ctx, g, rect);
}

static void
Minutes(Layer *layer, GContext *ctx)
{
	char buf[4];
	GRect bounds, rect, tmp;
	struct tm *tm;
	GBitmap *g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_compositing_mode(ctx, GCompOpSet);

	tm = now();
	strftime(buf, sizeof buf, "%M", tm);

	g = glyph_get(BIG, buf[1]);
	tmp = gbitmap_get_bounds(g);
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	rect.origin.x = bounds.size.w - tmp.size.w;
	rect.origin.y = bounds.origin.y;
	graphics_draw_bitmap_in_rect(ctx, g, rect);

	g = glyph_get(BIG, buf[0]);
	tmp = gbitmap_get_bounds(g);
	rect.origin.x -= tmp.size.w + MARGIN;
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	graphics_draw_bitmap_in_rect(ctx, g, rect);
}

static void
Left(Layer *layer, GContext *ctx)
{
	static char buf[21];
	GRect bounds, rect;
	struct tm *tm;
	int i;
	GBitmap *g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_compositing_mode(ctx, GCompOpSet);

	if (config.left[0] == 0)
		return;

	tm = now();
	strftime(buf, sizeof buf, config.left, tm);

	rect.origin.x = bounds.origin.x;
	rect.origin.y = bounds.origin.y;
	rect.size.w = 4;
	rect.size.h = 5;

	for (i=0; buf[i]; i++) {
		g = glyph_get(TINY, buf[i]);
		graphics_draw_bitmap_in_rect(ctx, g, rect);
		rect.origin.y += rect.size.h +2;
	}
}

static void
Bottom(Layer *layer, GContext *ctx)
{
	static char buf[18];
	GRect bounds, rect;
	struct tm *tm;
	int i;
	GBitmap *g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_compositing_mode(ctx, GCompOpSet);

	if (config.bottom[0] == 0)
		return;

	tm = now();
	strftime(buf, sizeof buf, config.bottom, tm);

	rect.origin.x = bounds.origin.x + ((sizeof buf -1) - strlen(buf)) * 8;
	rect.origin.y = bounds.origin.y;
	rect.size.w = 6;
	rect.size.h = 8;

	for (i=0; buf[i]; i++) {
		g = glyph_get(SMALL, buf[i]);
		graphics_draw_bitmap_in_rect(ctx, g, rect);
		rect.origin.x += rect.size.w +2;
	}
}

static void
Load(Window *win)
{
	Layer *wl;
	GRect bounds, rect;

	wl = window_get_root_layer(win);
	bounds = layer_get_bounds(wl);

	body = layer_create(bounds);
	layer_set_update_proc(body, Body);
	layer_add_child(wl, body);

	rect.origin.x = 4 + MARGIN*2;
	rect.origin.y = MARGIN;
	rect.size.w = bounds.size.w - MARGIN*3 - 4;
	rect.size.h = 70;
	hours = layer_create(rect);
	layer_set_update_proc(hours, Hours);
	layer_add_child(body, hours);

	rect.origin.x = 4 + MARGIN*2;
	rect.origin.y = 70 + MARGIN*2;
	rect.size.w = bounds.size.w - MARGIN*3 - 4;
	rect.size.h = 70;
	minutes = layer_create(rect);
	layer_set_update_proc(minutes, Minutes);
	layer_add_child(body, minutes);

	rect.origin.x = MARGIN;
	rect.origin.y = MARGIN;
	rect.size.w = 4;
	rect.size.h = 70*2 + MARGIN;
	left = layer_create(rect);
	layer_set_update_proc(left, Left);
	layer_add_child(body, left);

	rect.origin.x = MARGIN;
	rect.origin.y = bounds.size.h - MARGIN - 8;
	rect.size.h = 8;
	rect.size.w = bounds.size.w - MARGIN*2;
	bottom = layer_create(rect);
	layer_set_update_proc(bottom, Bottom);
	layer_add_child(body, bottom);
}

static void
Unload(Window *win)
{
	(void)win;
	layer_destroy(body);
	layer_destroy(hours);
	layer_destroy(minutes);
	layer_destroy(left);
	layer_destroy(bottom);
}

static void
Tick(struct tm *time, TimeUnits change)
{
	if (change & MINUTE_UNIT)
		layer_mark_dirty(minutes);

	if (change & HOUR_UNIT) {
		layer_mark_dirty(hours);
		vibe(config.each_hour);
	}

	if (change & DAY_UNIT) {
		layer_mark_dirty(bottom);
		layer_mark_dirty(left);
	}
}

static void
Bluetooth(bool connected)
{
	vibe(connected ? config.bt_on : config.bt_off);
}

static void
Battery(BatteryChargeState state)
{
	(void)state;
	layer_mark_dirty(body);
}

static void
configure()
{
	connection_service_unsubscribe();
	if (config.bt_on || config.bt_off)
		connection_service_subscribe((ConnectionHandlers){ Bluetooth, 0 });

	palette[0] = config.fg;
	palette[1] = GColorClear;

	layer_mark_dirty(body);
}

static void
Msg(DictionaryIterator *di, void *ctx)
{
	Tuple *tuple;
	(void)ctx;

	if ((tuple = dict_find(di, MESSAGE_KEY_BGCOLOR)))
		config.bg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_FGCOLOR)))
		config.fg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_LEFT)))
		strcpy(config.left, tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_BOTTOM)))
		strcpy(config.bottom, tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEBTON)))
		config.bt_on = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEBTOFF)))
		config.bt_off = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEEACHHOUR)))
		config.each_hour = atoi(tuple->value->cstring);

	persist_write_data(CONFKEY, &config, sizeof config);
	configure();
}

int
main()
{
	Window *win;
	WindowHandlers wh;
	time_t now;
	unsigned i;

	// Resources
	glyphs = gbitmap_create_with_resource(RESOURCE_ID_GLYPHS);
	gbitmap_set_palette(glyphs, palette, false);

	// Glyphs for fonts
#define SPRITE(X,Y,W,H) gbitmap_create_as_sub_bitmap(glyphs, GRect(X,Y,W,H));

	glyph[BIG+'0'] = SPRITE(26, 5, 60, 70);
	glyph[BIG+'1'] = SPRITE(91, 5, 30, 70);
	glyph[BIG+'2'] = SPRITE(126, 5, 60, 70);
	glyph[BIG+'3'] = SPRITE(191, 5, 60, 70);
	glyph[BIG+'4'] = SPRITE(26, 80, 45, 70);
	glyph[BIG+'5'] = SPRITE(76, 80, 60, 70);
	glyph[BIG+'6'] = SPRITE(141, 80, 60, 70);
	glyph[BIG+'7'] = SPRITE(206, 80, 45, 70);
	glyph[BIG+'8'] = SPRITE(26, 155, 60, 70);
	glyph[BIG+'9'] = SPRITE(91, 155, 60, 70);

	glyph[SMALL+'0'] = SPRITE(5, 243, 6, 8);
	glyph[SMALL+'1'] = SPRITE(13, 243, 6, 8);
	glyph[SMALL+'2'] = SPRITE(21, 243, 6, 8);
	glyph[SMALL+'3'] = SPRITE(29, 243, 6, 8);
	glyph[SMALL+'4'] = SPRITE(37, 243, 6, 8);
	glyph[SMALL+'5'] = SPRITE(45, 243, 6, 8);
	glyph[SMALL+'6'] = SPRITE(53, 243, 6, 8);
	glyph[SMALL+'7'] = SPRITE(61, 243, 6, 8);
	glyph[SMALL+'8'] = SPRITE(69, 243, 6, 8);
	glyph[SMALL+'9'] = SPRITE(77, 243, 6, 8);
	glyph[SMALL+'A'] = SPRITE(5, 230, 6, 8);
	glyph[SMALL+'B'] = SPRITE(13, 230, 6, 8);
	glyph[SMALL+'C'] = SPRITE(21, 230, 6, 8);
	glyph[SMALL+'D'] = SPRITE(29, 230, 6, 8);
	glyph[SMALL+'E'] = SPRITE(37, 230, 6, 8);
	glyph[SMALL+'F'] = SPRITE(45, 230, 6, 8);
	glyph[SMALL+'G'] = SPRITE(53, 230, 6, 8);
	glyph[SMALL+'H'] = SPRITE(61, 230, 6, 8);
	glyph[SMALL+'I'] = SPRITE(69, 230, 6, 8);
	glyph[SMALL+'J'] = SPRITE(77, 230, 6, 8);
	glyph[SMALL+'K'] = SPRITE(85, 230, 6, 8);
	glyph[SMALL+'L'] = SPRITE(93, 230, 6, 8);
	glyph[SMALL+'M'] = SPRITE(101, 230, 6, 8);
	glyph[SMALL+'N'] = SPRITE(109, 230, 6, 8);
	glyph[SMALL+'O'] = SPRITE(117, 230, 6, 8);
	glyph[SMALL+'P'] = SPRITE(125, 230, 6, 8);
	glyph[SMALL+'Q'] = SPRITE(133, 230, 6, 8);
	glyph[SMALL+'R'] = SPRITE(141, 230, 6, 8);
	glyph[SMALL+'S'] = SPRITE(149, 230, 6, 8);
	glyph[SMALL+'T'] = SPRITE(157, 230, 6, 8);
	glyph[SMALL+'U'] = SPRITE(165, 230, 6, 8);
	glyph[SMALL+'V'] = SPRITE(173, 230, 6, 8);
	glyph[SMALL+'W'] = SPRITE(181, 230, 6, 8);
	glyph[SMALL+'X'] = SPRITE(189, 230, 6, 8);
	glyph[SMALL+'Y'] = SPRITE(197, 230, 6, 8);
	glyph[SMALL+'Z'] = SPRITE(205, 230, 6, 8);
	glyph[SMALL+' '] = SPRITE(85, 243, 6, 8);
	glyph[SMALL+'%'] = SPRITE(93, 243, 6, 8);
	glyph[SMALL+'#'] = SPRITE(101, 243, 6, 8);

	glyph[TINY+'0'] = SPRITE(5, 5, 4, 5);
	glyph[TINY+'1'] = SPRITE(5, 12, 4, 5);
	glyph[TINY+'2'] = SPRITE(5, 19, 4, 5);
	glyph[TINY+'3'] = SPRITE(5, 26, 4, 5);
	glyph[TINY+'4'] = SPRITE(5, 33, 4, 5);
	glyph[TINY+'5'] = SPRITE(5, 40, 4, 5);
	glyph[TINY+'6'] = SPRITE(5, 47, 4, 5);
	glyph[TINY+'7'] = SPRITE(5, 54, 4, 5);
	glyph[TINY+'8'] = SPRITE(5, 61, 4, 5);
	glyph[TINY+'9'] = SPRITE(5, 68, 4, 5);
	glyph[TINY+'A'] = SPRITE(13, 5, 4, 5);
	glyph[TINY+'B'] = SPRITE(13, 12, 4, 5);
	glyph[TINY+'C'] = SPRITE(13, 19, 4, 5);
	glyph[TINY+'D'] = SPRITE(13, 26, 4, 5);
	glyph[TINY+'E'] = SPRITE(13, 33, 4, 5);
	glyph[TINY+'F'] = SPRITE(13, 40, 4, 5);
	glyph[TINY+'G'] = SPRITE(13, 47, 4, 5);
	glyph[TINY+'H'] = SPRITE(13, 54, 4, 5);
	glyph[TINY+'I'] = SPRITE(13, 61, 4, 5);
	glyph[TINY+'J'] = SPRITE(13, 68, 4, 5);
	glyph[TINY+'K'] = SPRITE(13, 75, 4, 5);
	glyph[TINY+'L'] = SPRITE(13, 82, 4, 5);
	glyph[TINY+'M'] = SPRITE(13, 89, 4, 5);
	glyph[TINY+'N'] = SPRITE(13, 96, 4, 5);
	glyph[TINY+'O'] = SPRITE(13, 103, 4, 5);
	glyph[TINY+'P'] = SPRITE(13, 110, 4, 5);
	glyph[TINY+'Q'] = SPRITE(13, 117, 4, 5);
	glyph[TINY+'R'] = SPRITE(13, 124, 4, 5);
	glyph[TINY+'S'] = SPRITE(13, 131, 4, 5);
	glyph[TINY+'T'] = SPRITE(13, 138, 4, 5);
	glyph[TINY+'U'] = SPRITE(13, 145, 4, 5);
	glyph[TINY+'V'] = SPRITE(13, 152, 4, 5);
	glyph[TINY+'W'] = SPRITE(13, 159, 4, 5);
	glyph[TINY+'X'] = SPRITE(13, 166, 4, 5);
	glyph[TINY+'Y'] = SPRITE(13, 173, 4, 5);
	glyph[TINY+'Z'] = SPRITE(13, 180, 4, 5);
	glyph[TINY+' '] = SPRITE(5, 75, 4, 5);
	glyph[TINY+'%'] = SPRITE(5, 82, 4, 5);
	glyph[TINY+'#'] = SPRITE(5, 89, 4, 5);

#undef SPRITE

	// Window
	win = window_create();
	wh.load = Load;
	wh.appear = 0;
	wh.disappear = 0;
	wh.unload = Unload;
	window_set_window_handlers(win, wh);
	window_stack_push(win, true);

	// Time
	now = time(0);
	Tick(localtime(&now), DAY_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, Tick);

	// Config
	config.bg = GColorWhite;
	config.fg = GColorBlack;
	strcpy(config.left, "%B %Y");
	strcpy(config.bottom, "%A %d");
	config.bt_on = SILENT;
	config.bt_off = SILENT;
	config.each_hour = SILENT;
	persist_read_data(CONFKEY, &config, sizeof config);
	configure();
	app_message_register_inbox_received(Msg);
	app_message_open(dict_calc_buffer_size(8, 16), 0);

	// Main
	app_event_loop();

	// Cleanup
	window_destroy(win);
	for (i=0; i < ARRAY_LENGTH(glyph); i++)
		gbitmap_destroy(glyph[i]);
	gbitmap_destroy(glyphs);

	return 0;
}
