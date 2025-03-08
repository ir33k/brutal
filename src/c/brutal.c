#include <pebble.h>

#define CONFKEY 1
#define BMPW (64/8)	// Bitmaps width in bytes

#define GET_BIT(buf, x, y) (((buf)[BMPW*(y) + (x)/8]) &  (0x80 >> (x)%8))
#define SET_BIT(buf, x, y) (((buf)[BMPW*(y) + (x)/8]) |= (0x80 >> (x)%8))

enum font { BIG, SMALL, TINY };
enum tile { EMPTY, FULL, CORNER0, CORNER1, CORNER2 };
enum direction { UP, RIGHT, DOWN, LEFT, HORIZONTAL };
enum vibe { SILENT, SHORT, LONG, DOUBLE };
enum layer { BODY, HOURS, MINUTES, SIDE, BOTTOM };
enum fmt { BATTERY, STEPS, COPY };

static struct {
	GColor bg, fg;
	char side[32], bottom[32];
	enum vibe bt_on, bt_off, each_hour;
	bool pad_h;
	uint8_t shadow;
} config;

static Layer *body, *hours, *minutes, *side, *bottom;
static GBitmap *glyphs;
static uint8_t opacity = 255;
static uint8_t battery = 0;
static HealthValue steps = 0;

static GRect bounds[] = {
#ifdef PBL_RECT
	[BODY] = {{0, 0}, {PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT}},
	[HOURS] = {{5*2+4, 5}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[MINUTES] = {{5*2+4, 5*2+70}, {PBL_DISPLAY_WIDTH-5*3-4, 70}},
	[SIDE] = {{5, 5}, {4, 70*2+5}},
	[BOTTOM] = {{5, 5*3+70*2}, {PBL_DISPLAY_WIDTH-5*2, 8}}
#else
	[BODY] = {{0, 0}, {PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT}},
	[HOURS] = {{(PBL_DISPLAY_WIDTH-60*2-5)/2, 15}, {60*2+5, 70}},
	[MINUTES] = {{(PBL_DISPLAY_WIDTH-60*2-5)/2, 60}, {60*2+5, 70}},
	[SIDE] = {{(PBL_DISPLAY_WIDTH-45*2-5)/2, 60+70+5*3+8}, {45*2+5, 5}},
	[BOTTOM] = {{(PBL_DISPLAY_WIDTH-60*2-5)/2, 60+70+5}, {60*2+5, 8}}
#endif
};

static const GRect fonts[3][128] = {
	[BIG] = {
		[' '] = {{ 1, 1},{12,14}}, // Space -> 0 -> default
		['0'] = {{ 1, 1},{12,14}}, ['1'] = {{14, 1},{ 6,14}},
		['2'] = {{21, 1},{12,14}}, ['3'] = {{34, 1},{12,14}},
		['4'] = {{47, 1},{ 9,14}}, ['5'] = {{ 1,16},{12,14}},
		['6'] = {{14,16},{12,14}}, ['7'] = {{27,16},{ 9,14}},
		['8'] = {{37,16},{12,14}}, ['9'] = {{50,16},{12,14}},
		// Tiles used to scalue up BIG font
		[FULL]    = {{ 57,67},{5,5}},
		[CORNER0] = {{ 57, 1},{5,5}},
		[CORNER1] = {{ 56, 6},{5,5}},
		[CORNER2] = {{ 56,10},{5,5}}
	},
	[SMALL] = {
		['A'] = {{ 0,31},{6,8}}, ['a'] = {{ 0,31},{6,8}},
		['B'] = {{ 7,31},{6,8}}, ['b'] = {{ 7,31},{6,8}},
		['C'] = {{14,31},{6,8}}, ['c'] = {{14,31},{6,8}},
		['D'] = {{21,31},{6,8}}, ['d'] = {{21,31},{6,8}},
		['E'] = {{28,31},{6,8}}, ['e'] = {{28,31},{6,8}},
		['F'] = {{35,31},{6,8}}, ['f'] = {{35,31},{6,8}},
		['G'] = {{42,31},{6,8}}, ['g'] = {{42,31},{6,8}},
		['H'] = {{49,31},{6,8}}, ['h'] = {{49,31},{6,8}},
		['I'] = {{56,31},{6,8}}, ['i'] = {{56,31},{6,8}},
		['J'] = {{ 0,40},{6,8}}, ['j'] = {{ 0,40},{6,8}},
		['K'] = {{ 7,40},{6,8}}, ['k'] = {{ 7,40},{6,8}},
		['L'] = {{14,40},{6,8}}, ['l'] = {{14,40},{6,8}},
		['M'] = {{21,40},{6,8}}, ['m'] = {{21,40},{6,8}},
		['N'] = {{28,40},{6,8}}, ['n'] = {{28,40},{6,8}},
		['O'] = {{35,40},{6,8}}, ['o'] = {{35,40},{6,8}},
		['P'] = {{42,40},{6,8}}, ['p'] = {{42,40},{6,8}},
		['Q'] = {{49,40},{6,8}}, ['q'] = {{49,40},{6,8}},
		['R'] = {{56,40},{6,8}}, ['r'] = {{56,40},{6,8}},
		['S'] = {{ 0,49},{6,8}}, ['s'] = {{ 0,49},{6,8}},
		['T'] = {{ 7,49},{6,8}}, ['t'] = {{ 7,49},{6,8}},
		['U'] = {{14,49},{6,8}}, ['u'] = {{14,49},{6,8}},
		['V'] = {{21,49},{6,8}}, ['v'] = {{21,49},{6,8}},
		['W'] = {{28,49},{6,8}}, ['w'] = {{28,49},{6,8}},
		['X'] = {{35,49},{6,8}}, ['x'] = {{35,49},{6,8}},
		['Y'] = {{42,49},{6,8}}, ['y'] = {{42,49},{6,8}},
		['Z'] = {{49,49},{6,8}}, ['z'] = {{49,49},{6,8}},
		['0'] = {{56,49},{6,8}}, ['1'] = {{ 0,58},{6,8}},
		['2'] = {{ 7,58},{6,8}}, ['3'] = {{14,58},{6,8}},
		['4'] = {{21,58},{6,8}}, ['5'] = {{28,58},{6,8}},
		['6'] = {{35,58},{6,8}}, ['7'] = {{42,58},{6,8}},
		['8'] = {{49,58},{6,8}}, ['9'] = {{56,58},{6,8}},
		['%'] = {{ 0,67},{6,8}}, ['.'] = {{ 7,67},{6,8}},
		['-'] = {{14,67},{6,8}}, ['/'] = {{21,67},{6,8}},
		[':'] = {{28,67},{6,8}}, [' '] = {{35,67},{6,8}}
	},
	[TINY] = {
		['A'] = {{ 0,76},{4,5}}, ['a'] = {{ 0,76},{4,5}},
		['B'] = {{ 5,76},{4,5}}, ['b'] = {{ 5,76},{4,5}},
		['C'] = {{10,76},{4,5}}, ['c'] = {{10,76},{4,5}},
		['D'] = {{15,76},{4,5}}, ['d'] = {{15,76},{4,5}},
		['E'] = {{20,76},{4,5}}, ['e'] = {{20,76},{4,5}},
		['F'] = {{25,76},{4,5}}, ['f'] = {{25,76},{4,5}},
		['G'] = {{30,76},{4,5}}, ['g'] = {{30,76},{4,5}},
		['H'] = {{35,76},{4,5}}, ['h'] = {{35,76},{4,5}},
		['I'] = {{40,76},{4,5}}, ['i'] = {{40,76},{4,5}},
		['J'] = {{45,76},{4,5}}, ['j'] = {{45,76},{4,5}},
		['K'] = {{50,76},{4,5}}, ['k'] = {{50,76},{4,5}},
		['L'] = {{55,76},{4,5}}, ['l'] = {{55,76},{4,5}},
		['M'] = {{60,76},{4,5}}, ['m'] = {{60,76},{4,5}},
		['N'] = {{ 0,82},{4,5}}, ['n'] = {{ 0,82},{4,5}},
		['O'] = {{ 5,82},{4,5}}, ['o'] = {{ 5,82},{4,5}},
		['P'] = {{10,82},{4,5}}, ['p'] = {{10,82},{4,5}},
		['Q'] = {{15,82},{4,5}}, ['q'] = {{15,82},{4,5}},
		['R'] = {{20,82},{4,5}}, ['r'] = {{20,82},{4,5}},
		['S'] = {{25,82},{4,5}}, ['s'] = {{25,82},{4,5}},
		['T'] = {{30,82},{4,5}}, ['t'] = {{30,82},{4,5}},
		['U'] = {{35,82},{4,5}}, ['u'] = {{35,82},{4,5}},
		['V'] = {{40,82},{4,5}}, ['v'] = {{40,82},{4,5}},
		['W'] = {{45,82},{4,5}}, ['w'] = {{45,82},{4,5}},
		['X'] = {{50,82},{4,5}}, ['x'] = {{50,82},{4,5}},
		['Y'] = {{55,82},{4,5}}, ['y'] = {{55,82},{4,5}},
		['Z'] = {{60,82},{4,5}}, ['z'] = {{60,82},{4,5}},
		['0'] = {{ 0,88},{4,5}}, ['1'] = {{ 5,88},{4,5}},
		['2'] = {{10,88},{4,5}}, ['3'] = {{15,88},{4,5}},
		['4'] = {{20,88},{4,5}}, ['5'] = {{25,88},{4,5}},
		['6'] = {{30,88},{4,5}}, ['7'] = {{35,88},{4,5}},
		['8'] = {{40,88},{4,5}}, ['9'] = {{45,88},{4,5}},
		['%'] = {{50,88},{4,5}}, ['.'] = {{55,88},{4,5}},
		['/'] = {{60,88},{4,5}}, [' '] = {{35,67},{4,5}}
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
parsefmt(char *dst, unsigned sz, char *fmt)
{
	char buf[2][16];	// [enum fmt state][]
	enum fmt state;
	unsigned i, j, k;

	snprintf(buf[BATTERY], sizeof buf[BATTERY], "%u", battery);
	snprintf(buf[STEPS], sizeof buf[STEPS], "%ld", steps);

	state = COPY;
	for (i=0, j=0, k=0; i<sz-1;) {
		switch (state) {
		case COPY:
			if (!fmt[j]) {
				dst[i] = 0;
				return;		// End
			}
			if (fmt[j] == '#') {
				switch (fmt[j+1]) {
				case 'B': state = BATTERY; break;
				case 'S': state = STEPS; break;
				}
			}
			if (state != COPY) {
				k = 0;
				j += 2;
				continue;
			}
			dst[i++] = fmt[j++];
			break;
		case BATTERY:
		case STEPS:
			if (buf[state][k])
				dst[i++] = buf[state][k++];
			else
				state = COPY;
			break;
		}
	}
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
scale_glyph(GRect glyph, uint8_t **pixels)
{
	static uint8_t RES = 5;
	// Enough to fit scalled up (5 times) glyph of BIG font encoded in 1bit way
	static uint8_t buf[BMPW*70] = {};
	GRect scaled;
	uint8_t x, y, maxx, maxy;
	uint8_t bx, by, maxbx, maxby;
	uint8_t pixel;
	uint8_t neighbors;
	enum tile tile;

	scaled.origin.x = 0;
	scaled.origin.y = 0;
	scaled.size.w = glyph.size.w * RES;
	scaled.size.h = glyph.size.h * RES;

	maxx = glyph.origin.x + glyph.size.w;
	maxy = glyph.origin.y + glyph.size.h;

	memset(buf, 0, sizeof buf);

	for (y=glyph.origin.y; y<maxy; y++)
	for (x=glyph.origin.x; x<maxx; x++) {
		pixel = GET_BIT(*pixels, x, y);
		tile = EMPTY;

		if (pixel) {
			tile = FULL;
		} else {
			neighbors = 0;

			if (GET_BIT(*pixels, x-1, y-1)) neighbors |= 0b10000000;
			if (GET_BIT(*pixels, x  , y-1)) neighbors |= 0b01000000;
			if (GET_BIT(*pixels, x+1, y-1)) neighbors |= 0b00100000;
			if (GET_BIT(*pixels, x-1, y  )) neighbors |= 0b00010000;
			if (GET_BIT(*pixels, x+1, y  )) neighbors |= 0b00001000;
			if (GET_BIT(*pixels, x-1, y+1)) neighbors |= 0b00000100;
			if (GET_BIT(*pixels, x  , y+1)) neighbors |= 0b00000010;
			if (GET_BIT(*pixels, x+1, y+1)) neighbors |= 0b00000001;

			switch (neighbors) {
			case 0b11010000: tile = CORNER0; break;
			case 0b01101000: tile = CORNER1; break;
			case 0b00001011: tile = CORNER2; break;
			}
		}

		if (tile == EMPTY)
			continue;

		by = (y - glyph.origin.y) * RES;
		maxby = by + RES;

		for (; by<maxby; by++) {
			bx = (x - glyph.origin.x) * RES;
			maxbx = bx + RES;

			for (; bx<maxbx; bx++) {
				if (!GET_BIT(*pixels,
					     fonts[BIG][tile].origin.x + (bx%RES),
					     fonts[BIG][tile].origin.y + (by%RES)))
					continue;

				SET_BIT(buf, bx, by);
			}
		}
	}

	*pixels = buf;

	return scaled;
}

static GRect
get_glyph(enum font font, char c, uint8_t **pixels)
{
	GRect glyph;

	// Ignore non ASCII and white characters
	if (c > 128 || c < ' ')
		c = ' ';

	glyph = fonts[font][(int)c];

	// No glyph has w=0, use space as default
	if (!glyph.size.w)
		glyph = fonts[font][' '];

	*pixels = gbitmap_get_data(glyphs);

	if (font == BIG)
		glyph = scale_glyph(glyph, pixels);

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
	int16_t x, y, maxx, maxy;
	int16_t px, py, maxpx, maxpy;
	uint8_t *pixels;

	len = strlen(str);

	if (len == 0)
		return;

#ifndef PBL_RECT
	direction = HORIZONTAL;
#endif

	switch (direction) {
	case RIGHT:
	case DOWN:
		break;
	case LEFT:
	case HORIZONTAL:
		offset = rect.size.w;
		offset -= (len-1) * spacing;
		for (i=0; i<len; i++) {
			glyph = get_glyph(font, str[i], &pixels);
			offset -= glyph.size.w;
		}

		if (direction == HORIZONTAL)
			offset /= 2;

		rect.origin.x += offset;
		rect.size.w -= offset;
		break;
	case UP:
		offset = rect.size.h;
		offset -= (len-1) * spacing;
		for (i=0; i<len; i++) {
			glyph = get_glyph(font, str[i], &pixels);
			offset -= glyph.size.h;
		}
		rect.origin.y += offset;
		rect.size.h -= offset;
		break;
	}

	maxy = rect.origin.y + rect.size.h;
	fb = graphics_capture_frame_buffer(ctx);

	for (i=0; i<len; i++) {
		glyph = get_glyph(font, str[i], &pixels);
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
				if (!GET_BIT(pixels, px, py))
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
		case HORIZONTAL:
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


#ifdef PBL_RECT
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
#else
	print_font(ctx, bounds[HOURS], BIG, HORIZONTAL, buf, 5, config.shadow);

	len = strlen(buf);
	rect.origin.x = bounds[HOURS].size.w/2 - (len*8 +2)/2;
	rect.origin.y = 0;
	rect.size.w = len*8 +2;
	rect.size.h = 10;
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, rect, 0, GCornerNone);
	print_font(ctx, bounds[HOURS], SMALL, HORIZONTAL, buf, 2, 255);
#endif
}

static void
Minutes(Layer *_layer, GContext *ctx)
{
	char buf[4];
	struct tm *tm;

	tm = now();
	strftime(buf, sizeof buf, "%M", tm);

	print_font(ctx, bounds[MINUTES], BIG, LEFT, buf, 5, 255);

#ifdef PBL_RECT
	if (opacity == 255)
		print_font(ctx, bounds[MINUTES], BIG, RIGHT, buf, 5, config.shadow);
#endif
}

static void
Side(Layer *_layer, GContext *ctx)
{
	char fmt[32], buf[32];
	struct tm *tm;

	tm = now();
	parsefmt(fmt, sizeof fmt, config.side);
	strftime(buf, sizeof buf, fmt, tm);
	buf[20] = 0;
	print_font(ctx, bounds[SIDE], TINY, DOWN, buf, 2, 255);
}

static void
Bottom(Layer *_layer, GContext *ctx)
{
	char fmt[32], buf[32];
	struct tm *tm;

	tm = now();
	parsefmt(fmt, sizeof fmt, config.bottom);
	strftime(buf, sizeof buf, fmt, tm);
	buf[17] = 0;
	print_font(ctx, bounds[BOTTOM], SMALL, LEFT, buf, 2, 255);
}

static void
Unobstructed(AnimationProgress _p, void *win)
{
	static int16_t minutes_y = 0;
	static int16_t bottom_y = 0;
	Layer *root;
	GRect rect0, rect1;
	int16_t offset;

	if (!minutes_y)
		minutes_y = bounds[MINUTES].origin.y;

	if (!bottom_y)
		bottom_y = bounds[BOTTOM].origin.y;

	root = window_get_root_layer((Window *) win);
	rect0 = layer_get_bounds(root);
	rect1 = layer_get_unobstructed_bounds(root);

	bounds[MINUTES].origin.y = minutes_y;
	bounds[BOTTOM].origin.y = bottom_y;
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
Battery(BatteryChargeState state)
{
	battery = state.charge_percent;
	layer_mark_dirty(bottom);
	layer_mark_dirty(side);
}

#if defined(PBL_HEALTH)
static void
Health(HealthEventType event, void *_ctx) {
	switch (event) {
	case HealthEventSignificantUpdate:
	case HealthEventMovementUpdate:
		steps = health_service_sum_today(HealthMetricStepCount);
		break;
	case HealthEventHeartRateUpdate:
	case HealthEventSleepUpdate:
	case HealthEventMetricAlert:
		break;
	}
}
#endif

static void
configure()
{
	if (config.bt_on || config.bt_off)
		connection_service_subscribe((ConnectionHandlers){ Bluetooth, 0 });
	else
		connection_service_unsubscribe();

	if (strstr(config.bottom, "#B") || strstr(config.side, "#B")) {
		battery_state_service_subscribe(Battery);
		Battery(battery_state_service_peek());
	} else {
		battery_state_service_unsubscribe();
	}

#if defined(PBL_HEALTH)
	if (strstr(config.bottom, "#B") || strstr(config.side, "#B")) {
		health_service_events_subscribe(Health, NULL);
		Health(HealthEventSignificantUpdate, 0);
	} else {
		health_service_events_unsubscribe();
	}
#endif

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
	config.shadow = PBL_IF_ROUND_ELSE(32, 16);
	persist_read_data(CONFKEY, &config, sizeof config);
	configure();
	app_message_register_inbox_received(Received);
	app_message_open(app_message_inbox_size_maximum(), 0);

	// Resources
	glyphs = gbitmap_create_with_resource(RESOURCE_ID_GLYPHS);

	// Unobstructed area (quick view)
	UnobstructedAreaHandlers handlers = { 0, Unobstructed, 0 };
	unobstructed_area_service_subscribe(handlers, win);
	Unobstructed(0, win);
	// NOTE(irek): Aplite has unobstructed_area_service_subscribe
	// macro doing nothing.  In result the variable "handlers" is
	// never used and I'm getting compailer error.
	(void)handlers;

	// Main
	app_event_loop();

	// Cleanup
	window_destroy(win);
	gbitmap_destroy(glyphs);

	return 0;
}
