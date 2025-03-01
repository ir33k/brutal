#include <pebble.h>

#define CONFKEY 1
#define MARGIN 5

static struct {
	GColor bg, fg;
	char fmt[16];
} config;

static Layer *background;
static GBitmap *glyphs, *glyph[64];

static int
glyph_indexof(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0' + 10;
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 20;
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 20;
	switch (c) {
	case ' ': return 46;
	case '%': return 47;
	case '#': return 48;
	}
	return 46;	// Default to space
}

static void
Background(Layer *layer, GContext *ctx)
{
	static char buf[32];
	GRect bounds, rect0, rect1;
	time_t timestamp;
	struct tm *tm;
	int i, h, m, g;

	if ((timestamp = time(0)) < 0)
		return;

	if (!(tm = localtime(&timestamp)))
		return;

	h = tm->tm_hour;
	m = tm->tm_min;

	if (!clock_is_24h_style() && h > 12)
		h -= 12;

	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, config.bg);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	// Hour

	g = h % 10;
	rect0 = gbitmap_get_bounds(glyph[g]);
	rect0.origin.x = bounds.size.w - MARGIN - rect0.size.w;
	rect0.origin.y = bounds.origin.y + MARGIN;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect0);

	g = h / 10;
	if (g) {
		rect1 = gbitmap_get_bounds(glyph[g]);
		rect1.origin.x = rect0.origin.x - MARGIN - rect1.size.w;
		rect0.origin.y = bounds.origin.y + MARGIN;
		graphics_draw_bitmap_in_rect(ctx, glyph[g], rect1);
	}

	// Minutes

	g = m % 10;
	rect0 = gbitmap_get_bounds(glyph[g]);
	rect0.origin.x = bounds.size.w - MARGIN - rect0.size.w;
	rect0.origin.y = bounds.origin.y + MARGIN*2 + rect0.size.h;
	graphics_draw_bitmap_in_rect(ctx, glyph[g], rect0);

	g = m / 10;
	if (g) {
		rect1 = gbitmap_get_bounds(glyph[g]);
		rect1.origin.x = rect0.origin.x - MARGIN - rect1.size.w;
		rect1.origin.y = bounds.origin.y + MARGIN*2 + rect1.size.h;
		graphics_draw_bitmap_in_rect(ctx, glyph[g], rect1);
	}

	// Date

	strftime(buf, sizeof buf, "%A %d", tm);
	rect0.origin.x = MARGIN + (17 - strlen(buf)) * 8;
	rect0.origin.y += rect0.size.h + MARGIN;
	rect0.size.w = 8;
	rect0.size.h = 8;

	for (i=0; i < 17 && buf[i]; i++) {
		g = glyph_indexof(buf[i]);
		graphics_draw_bitmap_in_rect(ctx, glyph[g], rect0);
		rect0.origin.x += rect0.size.w;
	}
}

static void
Load(Window *win)
{
	Layer *wl;
	GRect rect;

	wl = window_get_root_layer(win);
	rect = layer_get_bounds(wl);

	background = layer_create(rect);
	layer_set_update_proc(background, Background);
	layer_add_child(wl, background);
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
	// TODO(irek): I will have separate leyer for each of those
	// but right now it will look stupid.
	if (change & MINUTE_UNIT)
		layer_mark_dirty(background);
	if (change & HOUR_UNIT)
		layer_mark_dirty(background);
	if (change & DAY_UNIT)
		layer_mark_dirty(background);
}

static void
configure()
{
	time_t now;

	// Default date format
	if (config.fmt[0] == 0)
		strcpy(config.fmt, PBL_IF_ROUND_ELSE("%a %d", "%A %d"));

	// Update
	layer_mark_dirty(background);
	now = time(0);
	Tick(localtime(&now), DAY_UNIT);
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
	// Big numbers
	glyph[0] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 5, 60, 70));
	glyph[1] = gbitmap_create_as_sub_bitmap(glyphs, GRect(70, 5, 30, 70));
	glyph[2] = gbitmap_create_as_sub_bitmap(glyphs, GRect(105, 5, 60, 70));
	glyph[3] = gbitmap_create_as_sub_bitmap(glyphs, GRect(170, 5, 60, 70));
	glyph[4] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 80, 45, 70));
	glyph[5] = gbitmap_create_as_sub_bitmap(glyphs, GRect(55, 80, 60, 70));
	glyph[6] = gbitmap_create_as_sub_bitmap(glyphs, GRect(120, 80, 60, 70));
	glyph[7] = gbitmap_create_as_sub_bitmap(glyphs, GRect(185, 80, 45, 70));
	glyph[8] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 155, 60, 70));
	glyph[9] = gbitmap_create_as_sub_bitmap(glyphs, GRect(70, 155, 60, 70));
	// Small numbers
	glyph[10] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 243, 8, 8));
	glyph[11] = gbitmap_create_as_sub_bitmap(glyphs, GRect(13, 243, 8, 8));
	glyph[12] = gbitmap_create_as_sub_bitmap(glyphs, GRect(21, 243, 8, 8));
	glyph[13] = gbitmap_create_as_sub_bitmap(glyphs, GRect(29, 243, 8, 8));
	glyph[14] = gbitmap_create_as_sub_bitmap(glyphs, GRect(37, 243, 8, 8));
	glyph[15] = gbitmap_create_as_sub_bitmap(glyphs, GRect(45, 243, 8, 8));
	glyph[16] = gbitmap_create_as_sub_bitmap(glyphs, GRect(53, 243, 8, 8));
	glyph[17] = gbitmap_create_as_sub_bitmap(glyphs, GRect(61, 243, 8, 8));
	glyph[18] = gbitmap_create_as_sub_bitmap(glyphs, GRect(69, 243, 8, 8));
	glyph[19] = gbitmap_create_as_sub_bitmap(glyphs, GRect(77, 243, 8, 8));
	// Small capital letters
	glyph[20] = gbitmap_create_as_sub_bitmap(glyphs, GRect(5, 230, 8, 8));
	glyph[21] = gbitmap_create_as_sub_bitmap(glyphs, GRect(13, 230, 8, 8));
	glyph[22] = gbitmap_create_as_sub_bitmap(glyphs, GRect(21, 230, 8, 8));
	glyph[23] = gbitmap_create_as_sub_bitmap(glyphs, GRect(29, 230, 8, 8));
	glyph[24] = gbitmap_create_as_sub_bitmap(glyphs, GRect(37, 230, 8, 8));
	glyph[25] = gbitmap_create_as_sub_bitmap(glyphs, GRect(45, 230, 8, 8));
	glyph[26] = gbitmap_create_as_sub_bitmap(glyphs, GRect(53, 230, 8, 8));
	glyph[27] = gbitmap_create_as_sub_bitmap(glyphs, GRect(61, 230, 8, 8));
	glyph[28] = gbitmap_create_as_sub_bitmap(glyphs, GRect(69, 230, 8, 8));
	glyph[29] = gbitmap_create_as_sub_bitmap(glyphs, GRect(77, 230, 8, 8));
	glyph[30] = gbitmap_create_as_sub_bitmap(glyphs, GRect(85, 230, 8, 8));
	glyph[31] = gbitmap_create_as_sub_bitmap(glyphs, GRect(93, 230, 8, 8));
	glyph[32] = gbitmap_create_as_sub_bitmap(glyphs, GRect(101, 230, 8, 8));
	glyph[33] = gbitmap_create_as_sub_bitmap(glyphs, GRect(109, 230, 8, 8));
	glyph[34] = gbitmap_create_as_sub_bitmap(glyphs, GRect(117, 230, 8, 8));
	glyph[35] = gbitmap_create_as_sub_bitmap(glyphs, GRect(125, 230, 8, 8));
	glyph[36] = gbitmap_create_as_sub_bitmap(glyphs, GRect(133, 230, 8, 8));
	glyph[37] = gbitmap_create_as_sub_bitmap(glyphs, GRect(141, 230, 8, 8));
	glyph[38] = gbitmap_create_as_sub_bitmap(glyphs, GRect(149, 230, 8, 8));
	glyph[39] = gbitmap_create_as_sub_bitmap(glyphs, GRect(157, 230, 8, 8));
	glyph[40] = gbitmap_create_as_sub_bitmap(glyphs, GRect(165, 230, 8, 8));
	glyph[41] = gbitmap_create_as_sub_bitmap(glyphs, GRect(173, 230, 8, 8));
	glyph[42] = gbitmap_create_as_sub_bitmap(glyphs, GRect(181, 230, 8, 8));
	glyph[43] = gbitmap_create_as_sub_bitmap(glyphs, GRect(189, 230, 8, 8));
	glyph[44] = gbitmap_create_as_sub_bitmap(glyphs, GRect(197, 230, 8, 8));
	glyph[45] = gbitmap_create_as_sub_bitmap(glyphs, GRect(205, 230, 8, 8));
	// Small special characters
	glyph[46] = gbitmap_create_as_sub_bitmap(glyphs, GRect(85, 243, 8, 8));
	glyph[47] = gbitmap_create_as_sub_bitmap(glyphs, GRect(93, 243, 8, 8));
	glyph[48] = gbitmap_create_as_sub_bitmap(glyphs, GRect(101, 243, 8, 8));

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
