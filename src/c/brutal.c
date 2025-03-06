#include <pebble.h>

#define CONFKEY 1

enum font { BIG, SMALL, TINY };
enum direction { UP, RIGHT, DOWN, LEFT };
enum vibe { SILENT, SHORT, LONG, DOUBLE };
enum layer { BODY, HOURS, MINUTES, SIDE, BOTTOM };

static struct {
	GColor bg, fg;
	char side[32], bottom[32];
	enum vibe bt_on, bt_off, each_hour;
	bool pad_h;
	uint8_t shadow;
} config;

static Layer *body, *hours, *minutes, *side, *bottom;
static GBitmap *glyphs;
static uint8_t *pixels;
static uint8_t opacity = 255;

static GRect bounds[] = {
	[BODY] = {{0, 0}, {PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT}},
	[HOURS] = {{5*2+4, 5}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[MINUTES] = {{5*2+4, 5*2+70}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[SIDE] = {{5, 5}, {4, 70*2+5}},
	[BOTTOM] = {{5, 5*3+70*2}, {PBL_DISPLAY_WIDTH-5*2, 8}},
};

static const GRect fonts[3][128] = {
	[BIG] = {
		[' '] = {{  0,9},{6,8}}, // Space -> 0 -> default
		['0'] = {{139,18},{12,14}}, ['1'] = {{152,18},{6,14}},
		['2'] = {{159,18},{12,14}}, ['3'] = {{172,18},{12,14}},
		['4'] = {{185,18},{ 9,14}}, ['5'] = {{195,18},{12,14}},
		['6'] = {{208,18},{12,14}}, ['7'] = {{221,18},{ 9,14}},
		['8'] = {{231,18},{12,14}}, ['9'] = {{244,18},{12,14}},
	},
	[SMALL] = {
		['0']  = {{  0,9},{6,8}}, ['1'] = {{ 16,9},{6,8}},
		['2']  = {{ 24,9},{6,8}}, ['3'] = {{ 32,9},{6,8}},
		['4']  = {{ 40,9},{6,8}}, ['5'] = {{ 48,9},{6,8}},
		['6']  = {{ 56,9},{6,8}}, ['7'] = {{ 64,9},{6,8}},
		['8']  = {{ 72,9},{6,8}}, ['9'] = {{ 80,9},{6,8}},
		['A']  = {{  0,0},{6,8}}, ['a'] = {{  0,0},{6,8}},
		['B']  = {{  8,0},{6,8}}, ['b'] = {{  8,0},{6,8}},
		['C']  = {{ 16,0},{6,8}}, ['c'] = {{ 16,0},{6,8}},
		['D']  = {{ 24,0},{6,8}}, ['d'] = {{ 24,0},{6,8}},
		['E']  = {{ 32,0},{6,8}}, ['e'] = {{ 32,0},{6,8}},
		['F']  = {{ 40,0},{6,8}}, ['f'] = {{ 40,0},{6,8}},
		['G']  = {{ 48,0},{6,8}}, ['g'] = {{ 48,0},{6,8}},
		['H']  = {{ 56,0},{6,8}}, ['h'] = {{ 56,0},{6,8}},
		['I']  = {{ 64,0},{6,8}}, ['i'] = {{ 64,0},{6,8}},
		['J']  = {{ 72,0},{6,8}}, ['j'] = {{ 72,0},{6,8}},
		['K']  = {{ 80,0},{6,8}}, ['k'] = {{ 80,0},{6,8}},
		['L']  = {{ 88,0},{6,8}}, ['l'] = {{ 88,0},{6,8}},
		['M']  = {{ 96,0},{6,8}}, ['m'] = {{ 96,0},{6,8}},
		['N']  = {{104,0},{6,8}}, ['n'] = {{104,0},{6,8}},
		['O']  = {{112,0},{6,8}}, ['o'] = {{112,0},{6,8}},
		['P']  = {{120,0},{6,8}}, ['p'] = {{120,0},{6,8}},
		['Q']  = {{128,0},{6,8}}, ['q'] = {{128,0},{6,8}},
		['R']  = {{136,0},{6,8}}, ['r'] = {{136,0},{6,8}},
		['S']  = {{144,0},{6,8}}, ['s'] = {{144,0},{6,8}},
		['T']  = {{152,0},{6,8}}, ['t'] = {{152,0},{6,8}},
		['U']  = {{160,0},{6,8}}, ['u'] = {{160,0},{6,8}},
		['V']  = {{168,0},{6,8}}, ['v'] = {{168,0},{6,8}},
		['W']  = {{176,0},{6,8}}, ['w'] = {{176,0},{6,8}},
		['X']  = {{184,0},{6,8}}, ['x'] = {{184,0},{6,8}},
		['Y']  = {{192,0},{6,8}}, ['y'] = {{192,0},{6,8}},
		['Z']  = {{200,0},{6,8}}, ['z'] = {{200,0},{6,8}},
		['[']  = {{168,9},{6,8}}, [']'] = {{176,9},{6,8}},
		['{']  = {{184,9},{6,8}}, ['}'] = {{192,9},{6,8}},
		['\\'] = {{128,9},{6,8}}, ['/'] = {{120,9},{6,8}},
		['(']  = {{152,9},{6,8}}, [')'] = {{160,9},{6,8}},
		[' ']  = {{ 80,9},{6,8}}, ['.'] = {{104,9},{6,8}},
		['!']  = {{136,9},{6,8}}, ['?'] = {{144,9},{6,8}},
		['#']  = {{ 96,9},{6,8}}, ['%'] = {{ 88,9},{6,8}},
		['\''] = {{240,9},{6,8}}, ['"'] = {{232,9},{6,8}},
		['+']  = {{208,9},{6,8}}, ['-'] = {{112,9},{6,8}},
		[':']  = {{216,9},{6,8}}, [';'] = {{224,9},{6,8}},
		['|']  = {{200,9},{6,8}}
	},
	[TINY] = {
		[' ']  = {{130,18},{4,5}}, ['.'] = {{ 55,24},{4,5}},
		['!']  = {{ 75,24},{4,5}}, ['?'] = {{ 80,24},{4,5}},
		['%']  = {{ 50,24},{4,5}}, ['-'] = {{ 60,24},{4,5}},
		['0']  = {{  0,24},{4,5}}, ['1'] = {{  5,24},{4,5}},
		['2']  = {{ 10,24},{4,5}}, ['3'] = {{ 15,24},{4,5}},
		['4']  = {{ 20,24},{4,5}}, ['5'] = {{ 25,24},{4,5}},
		['6']  = {{ 30,24},{4,5}}, ['7'] = {{ 35,24},{4,5}},
		['8']  = {{ 40,24},{4,5}}, ['9'] = {{ 45,24},{4,5}},
		['A']  = {{  0,18},{4,5}}, ['a'] = {{  0,18},{4,5}},
		['B']  = {{  5,18},{4,5}}, ['b'] = {{  5,18},{4,5}},
		['C']  = {{ 10,18},{4,5}}, ['c'] = {{ 10,18},{4,5}},
		['D']  = {{ 15,18},{4,5}}, ['d'] = {{ 15,18},{4,5}},
		['E']  = {{ 20,18},{4,5}}, ['e'] = {{ 20,18},{4,5}},
		['F']  = {{ 25,18},{4,5}}, ['f'] = {{ 25,18},{4,5}},
		['G']  = {{ 30,18},{4,5}}, ['g'] = {{ 30,18},{4,5}},
		['H']  = {{ 35,18},{4,5}}, ['h'] = {{ 35,18},{4,5}},
		['I']  = {{ 40,18},{4,5}}, ['i'] = {{ 40,18},{4,5}},
		['J']  = {{ 45,18},{4,5}}, ['j'] = {{ 45,18},{4,5}},
		['K']  = {{ 50,18},{4,5}}, ['k'] = {{ 50,18},{4,5}},
		['L']  = {{ 55,18},{4,5}}, ['l'] = {{ 55,18},{4,5}},
		['M']  = {{ 60,18},{4,5}}, ['m'] = {{ 60,18},{4,5}},
		['N']  = {{ 65,18},{4,5}}, ['n'] = {{ 65,18},{4,5}},
		['O']  = {{ 70,18},{4,5}}, ['o'] = {{ 70,18},{4,5}},
		['P']  = {{ 75,18},{4,5}}, ['p'] = {{ 75,18},{4,5}},
		['Q']  = {{ 80,18},{4,5}}, ['q'] = {{ 80,18},{4,5}},
		['R']  = {{ 85,18},{4,5}}, ['r'] = {{ 85,18},{4,5}},
		['S']  = {{ 90,18},{4,5}}, ['s'] = {{ 90,18},{4,5}},
		['T']  = {{ 95,18},{4,5}}, ['t'] = {{ 95,18},{4,5}},
		['U']  = {{100,18},{4,5}}, ['u'] = {{100,18},{4,5}},
		['V']  = {{105,18},{4,5}}, ['v'] = {{105,18},{4,5}},
		['W']  = {{110,18},{4,5}}, ['w'] = {{110,18},{4,5}},
		['X']  = {{115,18},{4,5}}, ['x'] = {{115,18},{4,5}},
		['Y']  = {{120,18},{4,5}}, ['y'] = {{120,18},{4,5}},
		['Z']  = {{125,18},{4,5}}, ['z'] = {{125,18},{4,5}},
		['\\'] = {{ 70,24},{4,5}}, ['/'] = {{ 65,24},{4,5}}
	}
};

int
normal(int v, int vmin, int vmax, int min, int max)
{
	return ((float)(v-vmin) / (float)(vmax-vmin)) * (float)(max-min) + min;
}

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

static void
draw_pixel(GBitmapDataRowInfo info, int16_t x, GColor color)
{
#if defined(PBL_COLOR)
	memset(info.data + x, color.argb, 1);
#elif defined(PBL_BW)
	uint8_t byte  = x / 8;
	uint8_t bit   = x % 8;
	uint8_t value = gcolor_equal(color, GColorWhite) ? 1 : 0;
	uint8_t *bp   = &info.data[byte];
	*bp ^= (-value ^ *bp) & (1 << bit);
#endif
}

static GRect
get_glyph(enum font font, char c)
{
	GRect glyph;

	// Ignore non ASCII
	if (c > 128)
		c = ' ';

	glyph = fonts[font][(int)c];

	// No glyph has w=0, use space as default
	if (!glyph.size.w)
		glyph = fonts[font][' '];

	return glyph;
}

static void
print_font(GContext *ctx, GRect rect,
	   enum font font, enum direction direction, char *str,
	   uint8_t spacing, uint8_t dither)
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
	int16_t offset;
	unsigned i, len;
	GRect glyph;
	GBitmap *fb;
	GBitmapDataRowInfo info;
	uint8_t pixel;
	int16_t x, y, maxx, maxy;
	int16_t px, py, maxpx, maxpy;

	len = strlen(str);

	if (len == 0)
		return;

	switch (direction) {
	case LEFT:
		offset = rect.size.w;
		offset -= (len-1) * spacing;
		for (i=0; i<len; i++) {
			glyph = get_glyph(font, str[i]);
			offset -= glyph.size.w;
		}
		rect.origin.x += offset;
		rect.size.w -= offset;
		break;
	case UP:
		offset = rect.size.h;
		offset -= (len-1) * spacing;
		for (i=0; i<len; i++) {
			glyph = get_glyph(font, str[i]);
			offset -= glyph.size.h;
		}
		rect.origin.y += offset;
		rect.size.h -= offset;
		break;
	case RIGHT:
	case DOWN:
		break;
	}

	maxy = rect.origin.y + rect.size.h;
	fb = graphics_capture_frame_buffer(ctx);

	for (i=0; i<len; i++) {
		glyph = get_glyph(font, str[i]);
		py = glyph.origin.y;
		maxpx = glyph.origin.x + glyph.size.w;
		maxpy = glyph.origin.y + glyph.size.h;

		for (y=rect.origin.y; y < maxy && py < maxpy; y++, py++) {
			info = gbitmap_get_data_row_info(fb, y);
			px = glyph.origin.x;

			maxx = rect.origin.x + rect.size.w;
			if (info.max_x < maxx)
				maxx = info.max_x;

			for (x=rect.origin.x; x < maxx && px < maxpx; x++, px++) {
				pixel = pixels[(256/8)*py + px/8];
				if ((pixel & (0x80 >> px%8)) == 0)
					continue;

				if (dither > map[y%8][x%8])
					draw_pixel(info, x, config.fg);
			}
		}

		switch (direction) {
		case UP:
		case DOWN:
			rect.origin.y += glyph.size.h + spacing;
			break;
		case RIGHT:
		case LEFT:
			rect.origin.x += glyph.size.w + spacing;
			break;
		}
	}

	graphics_release_frame_buffer(ctx, fb);
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
Body(Layer *_layer, GContext *ctx)
{
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds[BODY], 0, GCornerNone);
}

static void
Hours(Layer *_layer, GContext *ctx)
{
	char buf[4];
	struct tm *tm;
	GRect rect;
	unsigned len;

	tm = now();
	strftime(buf, sizeof buf, clock_is_24h_style() ? "%H" : "%I", tm);

	// Remove leading 0
	if (config.pad_h && buf[0] == '0') {
		buf[0] = buf[1];
		buf[1] = 0;
	}

	print_font(ctx, bounds[HOURS], BIG, LEFT, buf, 5, opacity);

	if (opacity == 255) {
		print_font(ctx, bounds[HOURS], BIG, RIGHT, buf, 5, config.shadow);
		return;
	}

	len = strlen(buf);
	rect.origin.x = bounds[HOURS].size.w - len*8 -2;
	rect.origin.y = 0;
	rect.size.w = len*8 +2;
	rect.size.h = 10;
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, rect, 0, GCornerNone);
	print_font(ctx, bounds[HOURS], SMALL, LEFT, buf, 2, 255);
}

static void
Minutes(Layer *_layer, GContext *ctx)
{
	char buf[4];
	struct tm *tm;

	tm = now();
	strftime(buf, sizeof buf, "%M", tm);

	print_font(ctx, bounds[MINUTES], BIG, LEFT, buf, 5, 255);

	if (opacity == 255)
		print_font(ctx, bounds[MINUTES], BIG, RIGHT, buf, 5, config.shadow);
}

static void
Side(Layer *_layer, GContext *ctx)
{
	char buf[32];
	struct tm *tm;

	tm = now();
	strftime(buf, sizeof buf, config.side, tm);
	buf[20] = 0;
	print_font(ctx, bounds[SIDE], TINY, DOWN, buf, 2, 255);
}

static void
Bottom(Layer *_layer, GContext *ctx)
{
	char buf[32];
	struct tm *tm;

	tm = now();
	strftime(buf, sizeof buf, config.bottom, tm);
	buf[17] = 0;
	print_font(ctx, bounds[BOTTOM], SMALL, LEFT, buf, 2, 255);
}

static void
Unobstructed(AnimationProgress _p, void *win)
{
	Layer *root;
	GRect rect0, rect1;
	int16_t offset;

	root = window_get_root_layer((Window *) win);
	rect0 = layer_get_bounds(root);
	rect1 = layer_get_unobstructed_bounds(root);

	bounds[MINUTES].origin.y = 5*2+70;
	bounds[BOTTOM].origin.y = 5*3+70*2;
	opacity = 255;
	offset = rect0.size.h - rect1.size.h;
	if (offset > 0) {
		bounds[MINUTES].origin.y -= offset;
		bounds[BOTTOM].origin.y -= offset;
		opacity -= normal(offset, 0, 51, 0, 225);
	}
}

static void
Load(Window *win)
{
	Layer *root;

	root = window_get_root_layer(win);

	body = layer_create(bounds[BODY]);
	layer_set_update_proc(body, Body);
	layer_add_child(root, body);

	hours = layer_create(bounds[HOURS]);
	layer_set_update_proc(hours, Hours);
	layer_add_child(body, hours);

	minutes = layer_create(bounds[MINUTES]);
	layer_set_update_proc(minutes, Minutes);
	layer_add_child(body, minutes);

	side = layer_create(bounds[SIDE]);
	layer_set_update_proc(side, Side);
	layer_add_child(body, side);

	bottom = layer_create(bounds[BOTTOM]);
	layer_set_update_proc(bottom, Bottom);
	layer_add_child(body, bottom);

	UnobstructedAreaHandlers handlers = { 0, Unobstructed, 0 };
	unobstructed_area_service_subscribe(handlers, win);
	Unobstructed(0, win);

	// NOTE(irek): Aplite has unobstructed_area_service_subscribe
	// macro doing nothing.  In result the variable "handlers" is
	// never used and I'm getting compailer error.
	(void)handlers;
}

static void
Unload(Window *_win)
{
	layer_destroy(body);
	layer_destroy(hours);
	layer_destroy(minutes);
	layer_destroy(side);
	layer_destroy(bottom);
}

static void
Tick(struct tm *_time, TimeUnits change)
{
	if (change & MINUTE_UNIT) {
		layer_mark_dirty(minutes);
	}
	if (change & HOUR_UNIT) {
		layer_mark_dirty(hours);
		vibe(config.each_hour);
	}
	if (change & DAY_UNIT) {
		layer_mark_dirty(bottom);
		layer_mark_dirty(side);
	}
}

static void
Bluetooth(bool connected)
{
	vibe(connected ? config.bt_on : config.bt_off);
}

static void
configure()
{
	connection_service_unsubscribe();
	if (config.bt_on || config.bt_off)
		connection_service_subscribe((ConnectionHandlers){ Bluetooth, 0 });

	layer_mark_dirty(body);
}

static void
Received(DictionaryIterator *di, void *_ctx)
{
	Tuple *tuple;

	if ((tuple = dict_find(di, MESSAGE_KEY_BGCOLOR)))
		config.bg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_FGCOLOR)))
		config.fg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_SIDE)))
		strncpy(config.side, tuple->value->cstring, sizeof config.side);

	if ((tuple = dict_find(di, MESSAGE_KEY_BOTTOM)))
		strncpy(config.bottom, tuple->value->cstring, sizeof config.bottom);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEBTON)))
		config.bt_on = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEBTOFF)))
		config.bt_off = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEEACHHOUR)))
		config.each_hour = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_PADH)))
		config.pad_h = tuple->value->int8;;

	if ((tuple = dict_find(di, MESSAGE_KEY_SHADOW)))
		config.shadow = tuple->value->int32;

	persist_write_data(CONFKEY, &config, sizeof config);
	configure();
}

int
main()
{
	Window *win;
	WindowHandlers wh;
	time_t now;

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
	strncpy(config.side, "%B %Y", sizeof config.side);
	strncpy(config.bottom, "%A %d", sizeof config.bottom);
	config.bt_on = SILENT;
	config.bt_off = SILENT;
	config.each_hour = SILENT;
	config.pad_h = false;
	config.shadow = 16;
	persist_read_data(CONFKEY, &config, sizeof config);
	configure();
	app_message_register_inbox_received(Received);
	app_message_open(app_message_inbox_size_maximum(), 0);

	// Resources
	glyphs = gbitmap_create_with_resource(RESOURCE_ID_GLYPHS);
	pixels = gbitmap_get_data(glyphs);

	// Main
	app_event_loop();

	// Cleanup
	window_destroy(win);
	gbitmap_destroy(glyphs);

	return 0;
}
