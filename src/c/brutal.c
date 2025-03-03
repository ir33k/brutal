#include <pebble.h>

#define CONFKEY 1
#define MARGIN 5

static struct {
	GColor bg, fg;
	char fmt[16];
} config;

static Layer *background, *hours, *minutes, *bottom, *left;
static GBitmap *glyphs, *glyph[128];
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

static int
glyph_small_indexof(char c)
{
	if (c >= '0' && c <= '9') return c - '0' + 10;
	if (c >= 'A' && c <= 'Z') return c - 'A' + 20;
	if (c >= 'a' && c <= 'z') return c - 'a' + 20;

	switch (c) {
	case ' ': return 46;
	case '%': return 47;
	case '#': return 48;
	}

	return 46;	// Default to space
}

static int
glyph_tiny_indexof(char c)
{
	if (c >= '0' && c <= '9') return c - '0' + 60;
	if (c >= 'A' && c <= 'Z') return c - 'A' + 70;
	if (c >= 'a' && c <= 'z') return c - 'a' + 70;

	switch (c) {
	case ' ': return 96;
	case '%': return 97;
	}

	return 96;	// Default to space
}

static void
Background(Layer *layer, GContext *ctx)
{
	GRect bounds;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void
Hours(Layer *layer, GContext *ctx)
{
	GRect bounds, rect, tmp;
	struct tm *tm;
	int h, g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	tm = now();
	h = tm->tm_hour;

	if (!clock_is_24h_style() && h > 12)
		h -= 12;

	g = h % 10;
	tmp = gbitmap_get_bounds(glyph[g]);
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	rect.origin.x = bounds.size.w - tmp.size.w;
	rect.origin.y = bounds.origin.y;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);

	g = h / 10;
	if (!g)
		return;

	tmp = gbitmap_get_bounds(glyph[g]);
	rect.origin.x -= tmp.size.w + MARGIN;
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);
}

static void
Minutes(Layer *layer, GContext *ctx)
{
	GRect bounds, rect, tmp;
	struct tm *tm;
	int g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	tm = now();

	g = tm->tm_min % 10;
	tmp = gbitmap_get_bounds(glyph[g]);
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	rect.origin.x = bounds.size.w - tmp.size.w;
	rect.origin.y = bounds.origin.y;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);

	g = tm->tm_min / 10;
	tmp = gbitmap_get_bounds(glyph[g]);
	rect.origin.x -= tmp.size.w + MARGIN;
	rect.size.w = tmp.size.w;
	rect.size.h = tmp.size.h;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);
}

static void
Bottom(Layer *layer, GContext *ctx)
{
	static char buf[18];	// Fits 17 characters
	GRect bounds, rect;
	struct tm *tm;
	int i, g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	tm = now();

	strftime(buf, sizeof buf, "%A %d", tm);
	rect.origin.x = bounds.origin.x + ((sizeof buf -1) - strlen(buf)) * 8;
	rect.origin.y = bounds.origin.y;
	rect.size.w = 6;
	rect.size.h = 8;

	for (i=0; buf[i]; i++) {
		g = glyph_small_indexof(buf[i]);
		graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);
		rect.origin.x += rect.size.w +2;
	}
}

static void
Left(Layer *layer, GContext *ctx)
{
	static char buf[21];	// Fits 20 characters
	GRect bounds, rect;
	struct tm *tm;
	int i, g;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	tm = now();

	strftime(buf, sizeof buf, "%B %Y", tm);
	rect.origin.x = bounds.origin.x;
	rect.origin.y = bounds.origin.y;
	rect.size.w = 4;
	rect.size.h = 5;

	for (i=0; buf[i]; i++) {
		g = glyph_tiny_indexof(buf[i]);
		graphics_draw_bitmap_in_rect(ctx, glyph[g], rect);
		rect.origin.y += rect.size.h +2;
	}
}

static void
Load(Window *win)
{
	Layer *wl;
	GRect bounds, rect;

	wl = window_get_root_layer(win);
	bounds = layer_get_bounds(wl);

	background = layer_create(bounds);
	layer_set_update_proc(background, Background);
	layer_add_child(wl, background);

	rect.origin.x = bounds.origin.x + MARGIN + 4 + MARGIN;
	rect.origin.y = bounds.origin.y + MARGIN;
	rect.size.w = bounds.size.w - rect.origin.x - MARGIN;
	rect.size.h = 70;
	hours = layer_create(rect);
	layer_set_update_proc(hours, Hours);
	layer_add_child(background, hours);

	rect.origin.y += rect.size.h + MARGIN;
	minutes = layer_create(rect);
	layer_set_update_proc(minutes, Minutes);
	layer_add_child(background, minutes);

	rect.origin.x = MARGIN;
	rect.origin.y += rect.size.h + MARGIN;
	rect.size.h = 8;
	rect.size.w = bounds.size.w - MARGIN*2;
	bottom = layer_create(rect);
	layer_set_update_proc(bottom, Bottom);
	layer_add_child(background, bottom);

	rect.origin.x = MARGIN;
	rect.origin.y = MARGIN;
	rect.size.h = bounds.size.h - MARGIN*3 - 8;
	rect.size.w = 4;
	left = layer_create(rect);
	layer_set_update_proc(left, Left);
	layer_add_child(background, left);
}

static void
Unload(Window *win)
{
	(void)win;
	layer_destroy(background);
}

static void
Tick(struct tm *time, TimeUnits change)
{
	if (change & MINUTE_UNIT)
		layer_mark_dirty(minutes);

	if (change & HOUR_UNIT)
		layer_mark_dirty(hours);

	if (change & DAY_UNIT) {
		layer_mark_dirty(bottom);
		layer_mark_dirty(left);
	}
}

static void
configure()
{
	// time_t now;

	// Default date format
	// if (config.fmt[0] == 0)
	// 	strcpy(config.fmt, PBL_IF_ROUND_ELSE("%a %d", "%A %d"));

	palette[0] = config.fg;
	palette[1] = config.bg;	

	// Update
	layer_mark_dirty(background);
	// TODO(irek): Restore
	// now = time(0);
	// Tick(localtime(&now), DAY_UNIT);
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

	if ((tuple = dict_find(di, MESSAGE_KEY_DATE)))
		strcpy(config.fmt, tuple->value->cstring);

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
	gbitmap_set_palette(glyphs, palette, false);

	// Font x70 numbers
	glyph[0] = gbitmap_create_as_sub_bitmap(glyphs, GRect(26, 5, 60, 70));
	glyph[1] = gbitmap_create_as_sub_bitmap(glyphs, GRect(91, 5, 30, 70));
	glyph[2] = gbitmap_create_as_sub_bitmap(glyphs, GRect(126, 5, 60, 70));
	glyph[3] = gbitmap_create_as_sub_bitmap(glyphs, GRect(191, 5, 60, 70));
	glyph[4] = gbitmap_create_as_sub_bitmap(glyphs, GRect(26, 80, 45, 70));
	glyph[5] = gbitmap_create_as_sub_bitmap(glyphs, GRect(76, 80, 60, 70));
	glyph[6] = gbitmap_create_as_sub_bitmap(glyphs, GRect(141, 80, 60, 70));
	glyph[7] = gbitmap_create_as_sub_bitmap(glyphs, GRect(206, 80, 45, 70));
	glyph[8] = gbitmap_create_as_sub_bitmap(glyphs, GRect(26, 155, 60, 70));
	glyph[9] = gbitmap_create_as_sub_bitmap(glyphs, GRect(91, 155, 60, 70));

	// Font 8x6 numbers
	glyph[10] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 243, 6, 8));
	glyph[11] = gbitmap_create_as_sub_bitmap(glyphs, GRect(13, 243, 6, 8));
	glyph[12] = gbitmap_create_as_sub_bitmap(glyphs, GRect(21, 243, 6, 8));
	glyph[13] = gbitmap_create_as_sub_bitmap(glyphs, GRect(29, 243, 6, 8));
	glyph[14] = gbitmap_create_as_sub_bitmap(glyphs, GRect(37, 243, 6, 8));
	glyph[15] = gbitmap_create_as_sub_bitmap(glyphs, GRect(45, 243, 6, 8));
	glyph[16] = gbitmap_create_as_sub_bitmap(glyphs, GRect(53, 243, 6, 8));
	glyph[17] = gbitmap_create_as_sub_bitmap(glyphs, GRect(61, 243, 6, 8));
	glyph[18] = gbitmap_create_as_sub_bitmap(glyphs, GRect(69, 243, 6, 8));
	glyph[19] = gbitmap_create_as_sub_bitmap(glyphs, GRect(77, 243, 6, 8));

	// Font 8x6 uppercase letters
	glyph[20] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 230, 6, 8));
	glyph[21] = gbitmap_create_as_sub_bitmap(glyphs, GRect(13, 230, 6, 8));
	glyph[22] = gbitmap_create_as_sub_bitmap(glyphs, GRect(21, 230, 6, 8));
	glyph[23] = gbitmap_create_as_sub_bitmap(glyphs, GRect(29, 230, 6, 8));
	glyph[24] = gbitmap_create_as_sub_bitmap(glyphs, GRect(37, 230, 6, 8));
	glyph[25] = gbitmap_create_as_sub_bitmap(glyphs, GRect(45, 230, 6, 8));
	glyph[26] = gbitmap_create_as_sub_bitmap(glyphs, GRect(53, 230, 6, 8));
	glyph[27] = gbitmap_create_as_sub_bitmap(glyphs, GRect(61, 230, 6, 8));
	glyph[28] = gbitmap_create_as_sub_bitmap(glyphs, GRect(69, 230, 6, 8));
	glyph[29] = gbitmap_create_as_sub_bitmap(glyphs, GRect(77, 230, 6, 8));
	glyph[30] = gbitmap_create_as_sub_bitmap(glyphs, GRect(85, 230, 6, 8));
	glyph[31] = gbitmap_create_as_sub_bitmap(glyphs, GRect(93, 230, 6, 8));
	glyph[32] = gbitmap_create_as_sub_bitmap(glyphs, GRect(101, 230, 6, 8));
	glyph[33] = gbitmap_create_as_sub_bitmap(glyphs, GRect(109, 230, 6, 8));
	glyph[34] = gbitmap_create_as_sub_bitmap(glyphs, GRect(117, 230, 6, 8));
	glyph[35] = gbitmap_create_as_sub_bitmap(glyphs, GRect(125, 230, 6, 8));
	glyph[36] = gbitmap_create_as_sub_bitmap(glyphs, GRect(133, 230, 6, 8));
	glyph[37] = gbitmap_create_as_sub_bitmap(glyphs, GRect(141, 230, 6, 8));
	glyph[38] = gbitmap_create_as_sub_bitmap(glyphs, GRect(149, 230, 6, 8));
	glyph[39] = gbitmap_create_as_sub_bitmap(glyphs, GRect(157, 230, 6, 8));
	glyph[40] = gbitmap_create_as_sub_bitmap(glyphs, GRect(165, 230, 6, 8));
	glyph[41] = gbitmap_create_as_sub_bitmap(glyphs, GRect(173, 230, 6, 8));
	glyph[42] = gbitmap_create_as_sub_bitmap(glyphs, GRect(181, 230, 6, 8));
	glyph[43] = gbitmap_create_as_sub_bitmap(glyphs, GRect(189, 230, 6, 8));
	glyph[44] = gbitmap_create_as_sub_bitmap(glyphs, GRect(197, 230, 6, 8));
	glyph[45] = gbitmap_create_as_sub_bitmap(glyphs, GRect(205, 230, 6, 8));

	// Font 8x6 special characters
	glyph[46] = gbitmap_create_as_sub_bitmap(glyphs, GRect(85, 243, 6, 8));
	glyph[47] = gbitmap_create_as_sub_bitmap(glyphs, GRect(93, 243, 6, 8));
	glyph[48] = gbitmap_create_as_sub_bitmap(glyphs, GRect(101, 243, 6, 8));

	// Font 4x5 numbers
	glyph[60] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 5, 4, 5));
	glyph[61] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 12, 4, 5));
	glyph[62] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 19, 4, 5));
	glyph[63] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 26, 4, 5));
	glyph[64] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 33, 4, 5));
	glyph[65] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 40, 4, 5));
	glyph[66] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 47, 4, 5));
	glyph[67] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 54, 4, 5));
	glyph[68] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 61, 4, 5));
	glyph[69] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 68, 4, 5));

	// Font 4x5 uppercase letters
	glyph[70] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 5, 4, 5));
	glyph[71] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 12, 4, 5));
	glyph[72] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 19, 4, 5));
	glyph[73] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 26, 4, 5));
	glyph[74] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 33, 4, 5));
	glyph[75] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 40, 4, 5));
	glyph[76] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 47, 4, 5));
	glyph[77] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 54, 4, 5));
	glyph[78] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 61, 4, 5));
	glyph[79] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 68, 4, 5));
	glyph[80] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 75, 4, 5));
	glyph[81] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 82, 4, 5));
	glyph[82] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 89, 4, 5));
	glyph[83] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 96, 4, 5));
	glyph[84] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 103, 4, 5));
	glyph[85] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 110, 4, 5));
	glyph[86] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 117, 4, 5));
	glyph[87] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 124, 4, 5));
	glyph[88] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 131, 4, 5));
	glyph[89] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 138, 4, 5));
	glyph[90] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 145, 4, 5));
	glyph[91] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 152, 4, 5));
	glyph[92] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 159, 4, 5));
	glyph[93] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 166, 4, 5));
	glyph[94] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 173, 4, 5));
	glyph[95] = gbitmap_create_as_sub_bitmap(glyphs, GRect(14, 180, 4, 5));

	// Font 4x5 special characters
	glyph[96] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 75, 4, 5));
	glyph[97] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 82, 4, 5));

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
	config.fmt[0] = 0;
	persist_read_data(CONFKEY, &config, sizeof config);
	configure();
	app_message_register_inbox_received(Msg);
	app_message_open(dict_calc_buffer_size(8, 16), 0);

	// Main
	app_event_loop();
	window_destroy(win);
	gbitmap_destroy(glyphs);
	return 0;
}
