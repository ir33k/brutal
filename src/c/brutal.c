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
} config;

static Layer *body, *hours, *minutes, *side, *bottom;
static GBitmap *glyphs;
static uint8_t *pixels;

static const GRect bounds[] = {
	[BODY] = {{0, 0}, {PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT}},
	[HOURS] = {{5*2+4, 5}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[MINUTES] = {{5*2+4, 5*2+70}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[SIDE] = {{5, 5}, {4, 70*2+5}},
	[BOTTOM] = {{5, 5*3+70*2}, {PBL_DISPLAY_WIDTH-5*2, 8}},
};

static const GRect fonts[3][128] = {
	[BIG] = {
		[' '] = {{ 26,  5},{60,70}}, // space -> 0 -> default
		['0'] = {{ 26,  5},{60,70}}, ['1'] = {{ 91,  5},{30,70}},
		['2'] = {{126,  5},{60,70}}, ['3'] = {{191,  5},{60,70}},
		['4'] = {{ 26, 80},{45,70}}, ['5'] = {{ 76, 80},{60,70}},
		['6'] = {{141, 80},{60,70}}, ['7'] = {{206, 80},{45,70}},
		['8'] = {{ 26,155},{60,70}}, ['9'] = {{ 91,155},{60,70}}
	},
	[SMALL] = {
		['0']  = {{  5,246},{6,8}}, ['1'] = {{ 13,246},{6,8}},
		['2']  = {{ 21,246},{6,8}}, ['3'] = {{ 29,246},{6,8}},
		['4']  = {{ 37,246},{6,8}}, ['5'] = {{ 45,246},{6,8}},
		['6']  = {{ 53,246},{6,8}}, ['7'] = {{ 61,246},{6,8}},
		['8']  = {{ 69,246},{6,8}}, ['9'] = {{ 77,246},{6,8}},
		['A']  = {{  5,230},{6,8}}, ['a'] = {{  5,230},{6,8}},
		['B']  = {{ 13,230},{6,8}}, ['b'] = {{ 13,230},{6,8}},
		['C']  = {{ 21,230},{6,8}}, ['c'] = {{ 21,230},{6,8}},
		['D']  = {{ 29,230},{6,8}}, ['d'] = {{ 29,230},{6,8}},
		['E']  = {{ 37,230},{6,8}}, ['e'] = {{ 37,230},{6,8}},
		['F']  = {{ 45,230},{6,8}}, ['f'] = {{ 45,230},{6,8}},
		['G']  = {{ 53,230},{6,8}}, ['g'] = {{ 53,230},{6,8}},
		['H']  = {{ 61,230},{6,8}}, ['h'] = {{ 61,230},{6,8}},
		['I']  = {{ 69,230},{6,8}}, ['i'] = {{ 69,230},{6,8}},
		['J']  = {{ 77,230},{6,8}}, ['j'] = {{ 77,230},{6,8}},
		['K']  = {{ 85,230},{6,8}}, ['k'] = {{ 85,230},{6,8}},
		['L']  = {{ 93,230},{6,8}}, ['l'] = {{ 93,230},{6,8}},
		['M']  = {{101,230},{6,8}}, ['m'] = {{101,230},{6,8}},
		['N']  = {{109,230},{6,8}}, ['n'] = {{109,230},{6,8}},
		['O']  = {{117,230},{6,8}}, ['o'] = {{117,230},{6,8}},
		['P']  = {{125,230},{6,8}}, ['p'] = {{125,230},{6,8}},
		['Q']  = {{133,230},{6,8}}, ['q'] = {{133,230},{6,8}},
		['R']  = {{141,230},{6,8}}, ['r'] = {{141,230},{6,8}},
		['S']  = {{149,230},{6,8}}, ['s'] = {{149,230},{6,8}},
		['T']  = {{157,230},{6,8}}, ['t'] = {{157,230},{6,8}},
		['U']  = {{165,230},{6,8}}, ['u'] = {{165,230},{6,8}},
		['V']  = {{173,230},{6,8}}, ['v'] = {{173,230},{6,8}},
		['W']  = {{181,230},{6,8}}, ['w'] = {{181,230},{6,8}},
		['X']  = {{189,230},{6,8}}, ['x'] = {{189,230},{6,8}},
		['Y']  = {{197,230},{6,8}}, ['y'] = {{197,230},{6,8}},
		['Z']  = {{205,230},{6,8}}, ['z'] = {{205,230},{6,8}},
		['[']  = {{173,246},{6,8}}, [']'] = {{181,246},{6,8}},
		['{']  = {{189,246},{6,8}}, ['}'] = {{197,246},{6,8}},
		['\\'] = {{133,246},{6,8}}, ['/'] = {{125,246},{6,8}},
		['(']  = {{157,246},{6,8}}, [')'] = {{165,246},{6,8}},
		[' ']  = {{ 85,246},{6,8}}, ['.'] = {{109,246},{6,8}},
		['!']  = {{141,246},{6,8}}, ['?'] = {{149,246},{6,8}},
		['#']  = {{101,246},{6,8}}, ['%'] = {{ 93,246},{6,8}},
		['\''] = {{245,246},{6,8}}, ['"'] = {{237,246},{6,8}},
		['+']  = {{213,246},{6,8}}, ['-'] = {{117,246},{6,8}},
		[':']  = {{221,246},{6,8}}, [';'] = {{229,246},{6,8}},
		['|']  = {{205,246},{6,8}}
	},
	[TINY] = {
		[' ']  = {{ 5, 75},{4,5}}, ['.'] = {{ 5, 89},{4,5}},
		['!']  = {{ 5,117},{4,5}}, ['?'] = {{ 5,124},{4,5}},
		['%']  = {{ 5, 82},{4,5}}, ['-'] = {{ 5, 96},{4,5}},
		['0']  = {{ 5,  5},{4,5}}, ['1'] = {{ 5, 12},{4,5}},
		['2']  = {{ 5, 19},{4,5}}, ['3'] = {{ 5, 26},{4,5}},
		['4']  = {{ 5, 33},{4,5}}, ['5'] = {{ 5, 40},{4,5}},
		['6']  = {{ 5, 47},{4,5}}, ['7'] = {{ 5, 54},{4,5}},
		['8']  = {{ 5, 61},{4,5}}, ['9'] = {{ 5, 68},{4,5}},
		['A']  = {{13,  5},{4,5}}, ['a'] = {{13,  5},{4,5}},
		['B']  = {{13, 12},{4,5}}, ['b'] = {{13, 12},{4,5}},
		['C']  = {{13, 19},{4,5}}, ['c'] = {{13, 19},{4,5}},
		['D']  = {{13, 26},{4,5}}, ['d'] = {{13, 26},{4,5}},
		['E']  = {{13, 33},{4,5}}, ['e'] = {{13, 33},{4,5}},
		['F']  = {{13, 40},{4,5}}, ['f'] = {{13, 40},{4,5}},
		['G']  = {{13, 47},{4,5}}, ['g'] = {{13, 47},{4,5}},
		['H']  = {{13, 54},{4,5}}, ['h'] = {{13, 54},{4,5}},
		['I']  = {{13, 61},{4,5}}, ['i'] = {{13, 61},{4,5}},
		['J']  = {{13, 68},{4,5}}, ['j'] = {{13, 68},{4,5}},
		['K']  = {{13, 75},{4,5}}, ['k'] = {{13, 75},{4,5}},
		['L']  = {{13, 82},{4,5}}, ['l'] = {{13, 82},{4,5}},
		['M']  = {{13, 89},{4,5}}, ['m'] = {{13, 89},{4,5}},
		['N']  = {{13, 96},{4,5}}, ['n'] = {{13, 96},{4,5}},
		['O']  = {{13,103},{4,5}}, ['o'] = {{13,103},{4,5}},
		['P']  = {{13,110},{4,5}}, ['p'] = {{13,110},{4,5}},
		['Q']  = {{13,117},{4,5}}, ['q'] = {{13,117},{4,5}},
		['R']  = {{13,124},{4,5}}, ['r'] = {{13,124},{4,5}},
		['S']  = {{13,131},{4,5}}, ['s'] = {{13,131},{4,5}},
		['T']  = {{13,138},{4,5}}, ['t'] = {{13,138},{4,5}},
		['U']  = {{13,145},{4,5}}, ['u'] = {{13,145},{4,5}},
		['V']  = {{13,152},{4,5}}, ['v'] = {{13,152},{4,5}},
		['W']  = {{13,159},{4,5}}, ['w'] = {{13,159},{4,5}},
		['X']  = {{13,166},{4,5}}, ['x'] = {{13,166},{4,5}},
		['Y']  = {{13,173},{4,5}}, ['y'] = {{13,173},{4,5}},
		['Z']  = {{13,180},{4,5}}, ['z'] = {{13,180},{4,5}},
		['\\'] = {{ 5,110},{4,5}}, ['/'] = {{ 5,103},{4,5}}
	}
};

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
			if (str[i] > 128)
				continue;

			glyph = fonts[font][(int)str[i]];

			if (!glyph.origin.x)
				glyph = fonts[font][' '];

			offset -= glyph.size.w;
		}
		rect.origin.x += offset;
		rect.size.w -= offset;
		break;
	case UP:
		offset = rect.size.h;
		offset -= (len-1) * spacing;
		for (i=0; i<len; i++) {
			if (str[i] > 128)
				continue;

			glyph = fonts[font][(int)str[i]];

			if (!glyph.origin.x)
				glyph = fonts[font][' '];

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
		if (str[i] > 128)
			continue;	// Ignore non ASCII

		glyph = fonts[font][(int)str[i]];

		if (!glyph.origin.x)
			glyph = fonts[font][' '];

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

	tm = now();
	strftime(buf, sizeof buf, clock_is_24h_style() ? "%H" : "%I", tm);

	if (buf[0] == '0') {	// Avoid printing leading 0
		buf[0] = buf[1];
		buf[1] = 0;
	}
	print_font(ctx, bounds[HOURS], BIG, LEFT, buf, 5, 128+64);
}

static void
Minutes(Layer *_layer, GContext *ctx)
{
	char buf[4];
	struct tm *tm;

	tm = now();
	strftime(buf, sizeof buf, "%M", tm);
	print_font(ctx, bounds[MINUTES], BIG, LEFT, buf, 5, 255);
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
Msg(DictionaryIterator *di, void *_ctx)
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

	persist_write_data(CONFKEY, &config, sizeof config);
	configure();
}

int
main()
{
	Window *win;
	WindowHandlers wh;
	time_t now;

	// Resources
	glyphs = gbitmap_create_with_resource(RESOURCE_ID_GLYPHS);
	pixels = gbitmap_get_data(glyphs);

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
	persist_read_data(CONFKEY, &config, sizeof config);
	configure();
	app_message_register_inbox_received(Msg);
	app_message_open(dict_calc_buffer_size(8, 16), 0);

	// Main
	app_event_loop();

	// Cleanup
	window_destroy(win);
	gbitmap_destroy(glyphs);

	return 0;
}
