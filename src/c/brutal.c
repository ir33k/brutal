#include <pebble.h>

static void	onwinload	(Window*);
static void	onwinunload	(Window*);

static BitmapLayer*	body;
static GBitmap*		bitmap;

void
onwinload(Window *win)
{
	Layer *root;
	GRect rect;

	root = window_get_root_layer(win);
	rect = layer_get_bounds(root);

	body = bitmap_layer_create(rect);
	bitmap_layer_set_bitmap(body, bitmap);
	layer_add_child(root, bitmap_layer_get_layer(body));
}

void
onwinunload(Window *_win)
{
	bitmap_layer_destroy(body);
}

int
main(void)
{
	Window *win;
	WindowHandlers wh;

	/* resources */
	bitmap = gbitmap_create_with_resource(RESOURCE_ID_TEST);

	/* window */
	win = window_create();
	wh.load = onwinload;
	wh.appear = 0;
	wh.disappear = 0;
	wh.unload = onwinunload;
	window_set_window_handlers(win, wh);
	window_stack_push(win, true);

	/* main */
	app_event_loop();

	/* cleanup */
	window_destroy(win);
	gbitmap_destroy(bitmap);

	return 0;
}
