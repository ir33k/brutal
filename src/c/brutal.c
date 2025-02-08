/*
Capitalized functions are callbacks.
*/

#include <pebble.h>

#define CONFKEY 1

static struct {
	GColor bg, fg;
	char fmt[16];
} config;

static struct {
	Layer *bg;
	TextLayer *time, *date;
} layer;

static char *format;
static GFont font0, font1;

static void
settime(struct tm *time)
{
	static char buf[16];
	strftime(buf, sizeof buf, format, time);
	text_layer_set_text(layer.time, buf);
}

static void
setdate(struct tm *time)
{
	static char buf[16];
	strftime(buf, sizeof buf, config.fmt, time);
	text_layer_set_text(layer.date, buf);
}

static void
Background(Layer *layer, GContext *gc)
{
	GRect bounds;
	bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(gc, config.bg);
	graphics_fill_rect(gc, bounds, 0, GCornerNone);
}

static void
Load(Window *win)
{
	Layer *wl;
	GRect rect;

	wl = window_get_root_layer(win);
	rect = layer_get_bounds(wl);
	format = clock_is_24h_style() ? "%H:%M" : "%I:%M %p";

	// Background
	layer.bg = layer_create(rect);
	layer_set_update_proc(layer.bg, Background);
	layer_add_child(wl, layer.bg);

	// Time
	layer.time = text_layer_create(rect);
	text_layer_set_background_color(layer.time, GColorClear);
	text_layer_set_text_color(layer.time, GColorBlack);
	text_layer_set_font(layer.time, font0);
	text_layer_set_text_alignment(layer.time, GTextAlignmentCenter);
	layer_add_child(layer.bg, text_layer_get_layer(layer.time));

	// Date
	rect.origin.y += 24;
	layer.date = text_layer_create(rect);
	text_layer_set_background_color(layer.date, GColorClear);
	text_layer_set_text_color(layer.date, GColorBlack);
	text_layer_set_font(layer.date, font1);
	text_layer_set_text_alignment(layer.date, GTextAlignmentCenter);
	layer_add_child(layer.bg, text_layer_get_layer(layer.date));
}

static void
Unload(Window *win)
{
	(void)win;
	layer_destroy(layer.bg);
	text_layer_destroy(layer.time);
	text_layer_destroy(layer.date);
}

static void
Tick(struct tm *time, TimeUnits change)
{
	if (change & DAY_UNIT)
		setdate(time);
	settime(time);
}

static void
configure(void)
{
	time_t now;

	// Default date format
	if (config.fmt[0] == 0)
		strcpy(config.fmt, PBL_IF_ROUND_ELSE("%a %d", "%A %d"));

	// Update
	layer_mark_dirty(layer.bg);
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
main(void)
{
	Window *win;
	WindowHandlers wh;
	time_t now;

	// Resources
	font0 = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
	font1 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

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
	return 0;
}
