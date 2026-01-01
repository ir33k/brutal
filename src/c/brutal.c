#include <pebble.h>

#define CONFKEY		1
#define CHRONOGRAPHKEY	2
#define WEATHERKEY	3
#define MARGIN		5
#define SPACING		5
#define DIGITSH		70
#define DIGITSW		60
#define FONT7W		7
#define FONT7H		7
#define FONT10W		8
#define FONT10H		10
#define LETTERSPACING	2
#define SIDEMAX ((PBL_DISPLAY_HEIGHT - MARGIN*2 - FONT10H - SPACING + LETTERSPACING*2) / FONT7H)
#define BOTTOMMAX ((PBL_DISPLAY_WIDTH - MARGIN*2 + LETTERSPACING) / FONT10W)
#define NA		"N/A"		/* not available */
#define WEATHERINTERVAL (30*60)
#define DIVIDER		0x7F

enum vibe {
	VIBE_SILENT,
	VIBE_SHORT,
	VIBE_LONG,
	VIBE_DOUBLE,
};

enum icon {
	ICON_DEGREE     = 0x22,
	ICON_SUN        = 0x23,
	ICON_CLEARSKY   = 0x24,
	ICON_CLOUDS     = 0x26,
	ICON_FOG        = 0x27,
	ICON_DRIZZLE    = 0x28,
	ICON_RAIN       = 0x29,
	ICON_STORM      = 0x2B,
	ICON_SNOW       = 0x2C,
	ICON_BATTERY0   = 0x3B,
	ICON_BATTERY20  = 0x3C,
	ICON_BATTERY40  = 0x3D,
	ICON_BATTERY60  = 0x3E,
	ICON_BATTERY80  = 0x3F,
	ICON_BATTERY100 = 0x40,
	ICON_CHARGING   = 0x5B,
	ICON_WARNING    = 0x5C,
	ICON_HEART      = 0x5D,
	ICON_SHOE       = 0x5F,
	ICON_QUIET      = 0x60,
};

/* https://open-meteo.com/en/docs#weather_variable_documentation */
enum weather {		/* WMO Weather interpretation codes */
	WMO_CLEAR_SKY                = 0,
	WMO_MAINLY_CLEAR             = 1,
	WMO_PARTLY_CLOUDY            = 2,
	WMO_OVERCAST                 = 3,
	WMO_FOG                      = 45,
	WMO_DEPOSITING_RIME_FOG      = 48,
	WMO_DRIZZLE_LIGHT            = 51,
	WMO_DRIZZLE_MODERATE         = 53,
	WMO_DRIZZLE_DENSE            = 55,
	WMO_FREEZING_DRIZZLE_LIGHT   = 56,
	WMO_FREEZING_DRIZZLE_DENSE   = 57,
	WMO_RAIN_SLIGHT              = 61,
	WMO_RAIN_MODERATE            = 63,
	WMO_RAIN_HEAVY               = 65,
	WMO_FREEZING_RAIN_LIGHT      = 66,
	WMO_FREEZING_RAIN_HEAVY      = 67,
	WMO_SNOW_FALL_SLIGHT         = 71,
	WMO_SNOW_FALL_MODERATE       = 73,
	WMO_SNOW_FALL_HEAVY          = 75,
	WMO_SNOW_GRAINS              = 77,
	WMO_RAIN_SHOWERS_SLIGHT      = 80,
	WMO_RAIN_SHOWERS_MODERATE    = 81,
	WMO_RAIN_SHOWERS_VIOLENT     = 82,
	WMO_SNOW_SHOWERS_SLIGHT      = 85,
	WMO_SNOW_SHOWERS_HEAVY       = 86,
	WMO_THUNDERSTORM             = 95,
	WMO_THUNDERSTORM_SLIGHT_HAIL = 96,
	WMO_THUNDERSTORM_HEAVY_HAIL  = 99,
	/* not part of WMO standard */
	WMO_UNKNOWN                  = 100,
};

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef enum vibe	Vibe;
typedef enum icon	Icon;
typedef enum weather	Weather;

#ifdef PBL_RECT
static void	spread		(char*, u8 max);
#endif
static int	normal		(int v, int vmin, int vmax, int min, int max);
static void	configure	();
static void	uppercase	(char*);
static char*	formatstr	(char*);
static void	vibe		(Vibe);
static void	weatherget	();
static char*	weather2str	(Weather);
static Icon	weather2ico	(Weather);
static void	drawpixel	(GBitmapDataRowInfo*, i16, GColor);
static void	dither		(Layer*, GContext*, u8);
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
static void	oninbox		(DictionaryIterator*, void*);
static void	onbattery	(BatteryChargeState);
static void	onconnection	(bool);
static void	ontap		(AccelAxisType, i32);
static void	ontimer		(void*);
static void	onquickview	(AnimationProgress, void*);
#ifdef PBL_HEALTH
static void	onhealth	(HealthEventType, void*);
#endif

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
	u8	tempunit;	/* C/F (Celsius/Fahrenheit) */
} conf;

static struct {
	GDrawCommandImage*	digits;
	GFont	font10;
	GFont	font7;
} asset;

static struct {
	Layer*	body;
	Layer*	hour;
	Layer*	minute;
	Layer*	bottom;
	Layer*	side;
} layout;

static struct {
	u8	battery;
	bool	charging;
	bool	connected;
	time_t	chronograph;
	bool	useweather;
	time_t	lastweather;
} state = {0};

static struct {
	HealthValue	steps;
	HealthValue	dista;		/* distance in meters */
	HealthValue	heart;
	HealthValue	sleep;
	HealthValue	rest;		/* restful / deep sleep */
} health = {0};

static struct {
	/* TODO(irek): Use integer for temperature */
	char	temp[8];
	char	temphigh[8];
	char	templow[8];
	u8	code;
	u8	aireu;		/* 0-100 air quality index in europe */
	u16	airus;		/* 0-500 air quality index in us */
} weather = {NA, NA, NA, WMO_UNKNOWN, 0, 0};

static const i16 digitswidth[10] = {60, 30, 60, 60, 45, 60, 60, 45, 60, 60};

#ifdef PBL_RECT
void
spread(char *str, u8 max)
{
	i16 i, di, len, gap;

	len = strlen(str);
	gap = max - len;
	di = -1;

	for (i=0; str[i]; i++)
		switch (str[i]) {
		case DIVIDER:
			di = i;
			break;
		case ICON_SUN:
		case ICON_CLEARSKY:
		case ICON_CLOUDS:
		case ICON_FOG:
		case ICON_DRIZZLE:
		case ICON_RAIN:
		case ICON_STORM:
		case ICON_SNOW:
			gap--;		/* weather icons use one extra cell */
			break;
		}

	if (gap <= 0)
		return;

	if (di == -1)
		return;

	memmove(str + di + gap, str+di, len - di + 1);
	memset(str+di, ' ', gap + 1);
}
#endif

// Normalize V value with VMIN minimum possible value and VMAX maximum
// possible value to fit in range from MIN to MAX.
int
normal(int v, int vmin, int vmax, int min, int max)
{
	return ((float)(v-vmin) / (float)(vmax-vmin)) * (float)(max-min) + min;
}

void
configure()
{
	
	if (strstr(conf.side, "#b") || strstr(conf.bottom, "#b") ||
	    strstr(conf.side, "*b") || strstr(conf.bottom, "*b") ||
	    strstr(conf.side, "*c") || strstr(conf.bottom, "*c")) {
		battery_state_service_subscribe(onbattery);
		onbattery(battery_state_service_peek());
	} else {
		battery_state_service_unsubscribe();
	}

	if (strchr(conf.side, '&') || strchr(conf.bottom, '&')) {
		if (!state.useweather) {
			state.useweather = true;
			state.lastweather = 0;
		}
	} else {
		state.useweather = false;
	}

	if (state.useweather || conf.bton || conf.btoff ||
	    strstr(conf.side, "*w") || strstr(conf.bottom, "*w")) {
		connection_service_subscribe((ConnectionHandlers){ onconnection, 0 });
		state.connected = connection_service_peek_pebble_app_connection();
	} else {
		connection_service_unsubscribe();
	}

	weatherget();

#ifdef PBL_HEALTH
	if (strstr(conf.side, "#s") || strstr(conf.bottom, "#s") ||
	    strstr(conf.side, "#d") || strstr(conf.bottom, "#d") ||
	    strstr(conf.side, "#z") || strstr(conf.bottom, "#z") ||
	    strstr(conf.side, "#Z") || strstr(conf.bottom, "#Z") ||
	    strstr(conf.side, "#h") || strstr(conf.bottom, "#h")) {
		health_service_events_subscribe(onhealth, 0);
		onhealth(HealthEventSignificantUpdate, 0);
	} else {
		health_service_events_unsubscribe();
	}
#endif
	tick_timer_service_subscribe(conf.seconds == -1 ? SECOND_UNIT : MINUTE_UNIT, ontick);

	layer_mark_dirty(layout.body);
}

void
uppercase(char *str)
{
	for (; *str; str++)
		if (*str >= 'a' && *str <= 'z')
			*str &= ~0x20;
}

char*
formatstr(char *fmt)
{
	static char buf[64];
	char *str;
	time_t timestamp, diff;
	struct tm *tm;
	i8 wday;
	u32 i, n;
	Icon icon;

	n = sizeof buf;
	timestamp = time(0);
	tm = localtime(&timestamp);

	for (i=0; *fmt && i < sizeof buf -1; fmt++)
		switch (*fmt) {
		case '#':		/* pebble values */
			fmt++;
			switch (*fmt) {
			case 'b':
				i += snprintf(buf+i, n-i, "%u", state.battery);
				break;
			case 's':
				i += snprintf(buf+i, n-i, "%ld", health.steps);
				break;
			case 'd':
				i += snprintf(buf+i, n-i, "%ld", health.dista);
				break;
			case 'h':
				i += snprintf(buf+i, n-i, "%ld", health.heart);
				break;
			case 'z':
				i += snprintf(buf+i, n-i, "%ld:%.2ld",
					      health.sleep/60/60,
					      (health.sleep/60)%60);
				break;
			case 'Z':
				i += snprintf(buf+i, n-i, "%ld:%.2ld",
					      health.rest/60/60,
					      (health.rest/60)%60);
				break;
			case 'w':
				i += snprintf(buf+i, n-i, "MTWTFSS");
				wday = tm->tm_wday - 1;
				if (wday < 0) wday = 6;
				buf[i - 7 + wday] = '*';
				break;
			case 'W':
				i += snprintf(buf+i, n-i, "SMTWTFS");
				wday = tm->tm_wday;
				buf[i - 7 + wday] = '*';
				break;
			case 'c':
				diff = timestamp - state.chronograph;
				i += snprintf(buf+i, n-i, "%ld", diff / 60);
				break;
			}
			break;
		case '*':		/* icons */
			fmt++;
			icon = 0;
			switch (*fmt) {
			case '*': icon = '*'; break;
			case 'h': icon = ICON_HEART; break;
			case 's': icon = ICON_SHOE; break;
			case 'd': icon = ICON_DEGREE; break;
			case 'q':
				if (quiet_time_is_active())
					icon = ICON_QUIET;
				break;
			case 'w':
				if (!state.connected)
					icon = ICON_WARNING;
				break;
			case 'c':
				if (state.charging)
					icon = ICON_CHARGING;
				break;
			case 'b':
				icon = ICON_BATTERY0;
				/**/ if (state.battery>95) icon = ICON_BATTERY100;
				else if (state.battery>75) icon = ICON_BATTERY80;
				else if (state.battery>55) icon = ICON_BATTERY60;
				else if (state.battery>35) icon = ICON_BATTERY40;
				else if (state.battery>15) icon = ICON_BATTERY20;
				break;
			}
			if (icon)
				buf[i++] = icon;
			break;
		case '&':		/* weather */
			fmt++;
			switch (*fmt) {
			case 't':
				i += snprintf(buf+i, n-i, "%s", weather.temp);
				break;
			case 'h':
				i += snprintf(buf+i, n-i, "%s", weather.temphigh);
				break;
			case 'l':
				i += snprintf(buf+i, n-i, "%s", weather.templow);
				break;
			case 'u':
				buf[i++] = conf.tempunit;
				break;
			case 'c':
				str = weather2str(weather.code);
				i += snprintf(buf+i, n-i, "%s", str);
				break;
			case 'i':
				buf[i++] = weather2ico(weather.code);
				break;
			case 'a':
				i += snprintf(buf+i, n-i, "%u", weather.aireu);
				break;
			case 'A':
				i += snprintf(buf+i, n-i, "%u", weather.airus);
				break;
			}
			break;
		case '%':		/* avoid modyfing strftime() symbols */
			fmt++;
			buf[i++] = '%';
			buf[i++] = *fmt;
			break;
		case ',':
			buf[i++] = DIVIDER;
			break;
		case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A:
		case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
		case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:
		case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
		case 0x1F: case 0x7F:	/* skip white chars */
			break;
		case '"': case '$': case '\'':case '(': case ')':
		case '+': case ';': case '<': case '=': case '>':
		case '?': case '@': case '[': case '\\':case ']':
		case '^': case '_': case '`': case '{': case '|':
		case '}': case '~':
			/* skip unsupported characters */
			break;
		default:
			buf[i++] = *fmt;
		}

	buf[i] = 0;
	return buf;
}

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
weatherget()
{
	time_t timestamp;
	DictionaryIterator *iter;

	if (!state.useweather)
		return;

	if (!state.connected)
		return;

	timestamp = time(0);

	if (state.lastweather + WEATHERINTERVAL > timestamp)
		return;

	app_message_outbox_begin(&iter);
	dict_write_data(iter, WEATHERKEY, &conf.tempunit, sizeof conf.tempunit);
	app_message_outbox_send();
}

char*
weather2str(u8 code)
{
	switch (code) {
	case WMO_CLEAR_SKY:                return "Clear";
	case WMO_MAINLY_CLEAR:             return "Clear";
	case WMO_PARTLY_CLOUDY:            return "Cloudy";
	case WMO_OVERCAST:                 return "Overcast";
	case WMO_FOG:                      return "Fog";
	case WMO_DEPOSITING_RIME_FOG:      return "Rime fog";
	case WMO_DRIZZLE_LIGHT:            return "Drizzle";
	case WMO_DRIZZLE_MODERATE:         return "Drizzle!";
	case WMO_DRIZZLE_DENSE:            return "Drizzle!!";
	case WMO_FREEZING_DRIZZLE_LIGHT:   return "Drizzle*";
	case WMO_FREEZING_DRIZZLE_DENSE:   return "Drizzle*!";
	case WMO_RAIN_SLIGHT:              return "Rain";
	case WMO_RAIN_MODERATE:            return "Rain!";
	case WMO_RAIN_HEAVY:               return "rain!!";
	case WMO_FREEZING_RAIN_LIGHT:      return "Rain*";
	case WMO_FREEZING_RAIN_HEAVY:      return "Rain*!";
	case WMO_SNOW_FALL_SLIGHT:         return "Snow";
	case WMO_SNOW_FALL_MODERATE:       return "Snow!";
	case WMO_SNOW_FALL_HEAVY:          return "Snow!!";
	case WMO_SNOW_GRAINS:              return "Snow*";
	case WMO_RAIN_SHOWERS_SLIGHT:      return "Rain*";
	case WMO_RAIN_SHOWERS_MODERATE:    return "Rain*!";
	case WMO_RAIN_SHOWERS_VIOLENT:     return "Rain*!!";
	case WMO_SNOW_SHOWERS_SLIGHT:      return "Snow*";
	case WMO_SNOW_SHOWERS_HEAVY:       return "Snow*!!";
	case WMO_THUNDERSTORM:             return "Storm";
	case WMO_THUNDERSTORM_SLIGHT_HAIL: return "Storm!";
	case WMO_THUNDERSTORM_HEAVY_HAIL:  return "Storm!!";
	}
	return "Unknown";
}

Icon
weather2ico(u8 code)
{
	switch (code) {
	case WMO_CLEAR_SKY:                return ICON_SUN;
	case WMO_MAINLY_CLEAR:             return ICON_CLEARSKY;
	case WMO_PARTLY_CLOUDY:            return ICON_CLOUDS;
	case WMO_OVERCAST:                 return ICON_CLOUDS;
	case WMO_FOG:                      return ICON_FOG;
	case WMO_DEPOSITING_RIME_FOG:      return ICON_FOG;
	case WMO_DRIZZLE_LIGHT:            return ICON_DRIZZLE;
	case WMO_DRIZZLE_MODERATE:         return ICON_DRIZZLE;
	case WMO_DRIZZLE_DENSE:            return ICON_DRIZZLE;
	case WMO_FREEZING_DRIZZLE_LIGHT:   return ICON_DRIZZLE;
	case WMO_FREEZING_DRIZZLE_DENSE:   return ICON_DRIZZLE;
	case WMO_RAIN_SLIGHT:              return ICON_RAIN;
	case WMO_RAIN_MODERATE:            return ICON_RAIN;
	case WMO_RAIN_HEAVY:               return ICON_RAIN;
	case WMO_FREEZING_RAIN_LIGHT:      return ICON_RAIN;
	case WMO_FREEZING_RAIN_HEAVY:      return ICON_RAIN;
	case WMO_SNOW_FALL_SLIGHT:         return ICON_SNOW;
	case WMO_SNOW_FALL_MODERATE:       return ICON_SNOW;
	case WMO_SNOW_FALL_HEAVY:          return ICON_SNOW;
	case WMO_SNOW_GRAINS:              return ICON_SNOW;
	case WMO_RAIN_SHOWERS_SLIGHT:      return ICON_RAIN;
	case WMO_RAIN_SHOWERS_MODERATE:    return ICON_RAIN;
	case WMO_RAIN_SHOWERS_VIOLENT:     return ICON_RAIN;
	case WMO_SNOW_SHOWERS_SLIGHT:      return ICON_SNOW;
	case WMO_SNOW_SHOWERS_HEAVY:       return ICON_SNOW;
	case WMO_THUNDERSTORM:             return ICON_STORM;
	case WMO_THUNDERSTORM_SLIGHT_HAIL: return ICON_STORM;
	case WMO_THUNDERSTORM_HEAVY_HAIL:  return ICON_STORM;
	}
	return ICON_WARNING;
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
dither(Layer *layer, GContext *ctx, u8 amount)
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

	rect.origin.x = PBL_DISPLAY_WIDTH - MARGIN - DIGITSW*2 - SPACING;
	rect.origin.y = MARGIN;
	rect.size.w = DIGITSW*2 + SPACING;
	rect.size.h = DIGITSH;

	layout.hour = layer_create(rect);
	layer_set_update_proc(layout.hour, onhour);
	layer_add_child(layout.body, layout.hour);

	rect.origin.y += SPACING + DIGITSH;

	layout.minute = layer_create(rect);
	layer_set_update_proc(layout.minute, onminute);
	layer_add_child(layout.body, layout.minute);

	rect.origin.x = MARGIN;
	rect.origin.y += SPACING + DIGITSH - LETTERSPACING;
	rect.size.w = PBL_DISPLAY_WIDTH - MARGIN*2 + LETTERSPACING;
	rect.size.h = FONT10H;

	layout.bottom = layer_create(rect);
	layer_set_update_proc(layout.bottom, onbottom);
	layer_add_child(layout.body, layout.bottom);

	rect.origin.x = MARGIN;
	rect.origin.y = MARGIN - LETTERSPACING;
	rect.size.w = FONT7W;
	rect.size.h = PBL_DISPLAY_HEIGHT - MARGIN*2 - SPACING - FONT10H + LETTERSPACING*2;

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
	GRect frame, bounds;
	GPoint offset;
	i16 overlap;

	timestamp = time(0);
	tm = localtime(&timestamp);
	strftime(buf, sizeof buf, clock_is_24h_style() ? "%H" : "%I", tm);

	pt = buf;

	if (conf.padh && pt[0] == '0')
		pt++;

	bounds = layer_get_bounds(layer);
	frame = layer_get_frame(layout.minute);
	overlap = MARGIN + DIGITSH + SPACING - frame.origin.y;

	if (overlap < 51) {	/* TODO(irek): Magic number */
		offset.x = 0;
		offset.y = 0;
		drawshadow(buf, offset, ctx);
		dither(layer, ctx, 255 - normal(overlap, 51, 0, 0, conf.shadow));
	}

	if (overlap) {
		offset.x = bounds.size.w;
		offset.y = 0;
		drawdigits(pt, offset, ctx);
		dither(layer, ctx, 255 - normal(overlap, 0, 51, 0, conf.shadow));

		bounds.origin.x = bounds.size.w - FONT10W*2 - LETTERSPACING;
		bounds.origin.y = -2;
		bounds.size.w = FONT10W*2 + LETTERSPACING*2;
		bounds.size.h = FONT10H + LETTERSPACING;
		graphics_context_set_fill_color(ctx, conf.bg);
		graphics_fill_rect(ctx, bounds, 0, GCornerNone);

		graphics_context_set_text_color(ctx, conf.fg);
		graphics_draw_text(ctx, pt, asset.font10, bounds,
				   GTextOverflowModeWordWrap,
				   GTextAlignmentRight, NULL);
	} else {
		offset.x = bounds.size.w;
		offset.y = 0;
		drawdigits(pt, offset, ctx);
	}
}

void
onminute(Layer *layer, GContext *ctx)
{
	time_t timestamp;
	struct tm *tm;
	char buf[8];
	GRect bounds, frame;
	GPoint offset;
	i16 overlap;

	timestamp = time(0);
	tm = localtime(&timestamp);
	strftime(buf, sizeof buf, "%M", tm);

	bounds = layer_get_bounds(layer);
	frame = layer_get_frame(layout.minute);
	overlap = MARGIN + DIGITSH + SPACING - frame.origin.y;

	if (overlap < 51) {	/* TODO(irek): Magic number */
		offset.x = 0;
		offset.y = 0;
		drawshadow(buf, offset, ctx);
		dither(layer, ctx, 255 - normal(overlap, 51, 0, 0, conf.shadow));
	}

	offset.x = bounds.size.w;
	offset.y = 0;
	drawdigits(buf, offset, ctx);
}

void
onbottom(Layer *layer, GContext *ctx)
{
	time_t timestamp;
	struct tm *tm;
	char buf[64], *fmt;
	GRect bounds;

	timestamp = time(0);
	tm = localtime(&timestamp);
	bounds = layer_get_bounds(layer);

	graphics_context_set_text_color(ctx, conf.fg);

	fmt = formatstr(conf.bottom);
	strftime(buf, sizeof buf, fmt, tm);
	uppercase(buf);
#ifdef PBL_RECT
	spread(buf, BOTTOMMAX);
#endif
	graphics_draw_text(ctx, buf, asset.font10, bounds,
			   GTextOverflowModeWordWrap,
			   GTextAlignmentRight, NULL);
}

void
onside(Layer *layer, GContext *ctx)
{
	time_t timestamp;
	struct tm *tm;
	char tmp[64], buf[128], *fmt;
	GRect bounds;
	u16 i, j;

	timestamp = time(0);
	tm = localtime(&timestamp);
	bounds = layer_get_bounds(layer);

	graphics_context_set_text_color(ctx, conf.fg);

	fmt = formatstr(conf.side);
	strftime(tmp, sizeof tmp, fmt, tm);
	uppercase(tmp);
#ifdef PBL_RECT
	spread(tmp, SIDEMAX);
#endif

	for (i=0, j=0; tmp[i] && j < sizeof buf - 1; i+=1) {
		buf[j++] = tmp[i];
		buf[j++] = '\n';
	}
	buf[j] = 0;

	graphics_draw_text(ctx, buf, asset.font7, bounds,
			   GTextOverflowModeWordWrap,
			   GTextAlignmentLeft, NULL);
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

void
oninbox(DictionaryIterator *di, void *_ctx)
{
	Tuple *tuple;
	time_t timestamp;

	timestamp = time(0);


	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERTEMP))) {
		strncpy(weather.temp, tuple->value->cstring,
			sizeof weather.temp -1);
		state.lastweather = timestamp;
	}

	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERTEMPHIGH))) {
		strncpy(weather.temphigh, tuple->value->cstring,
			sizeof weather.temphigh -1);
		state.lastweather = timestamp;
	}

	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERTEMPLOW))) {
		strncpy(weather.templow, tuple->value->cstring,
			sizeof weather.templow -1);
		state.lastweather = timestamp;
	}

	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERCODE))) {
		weather.code = tuple->value->uint8;
		state.lastweather = timestamp;
	}

	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERAIREU))) {
		weather.aireu = tuple->value->uint8;
		state.lastweather = timestamp;
	}

	if ((tuple = dict_find(di, MESSAGE_KEY_WEATHERAIRUS))) {
		weather.airus = tuple->value->uint16;
		state.lastweather = timestamp;
	}
	
	if (dict_find(di, MESSAGE_KEY_READY))
		state.lastweather = 0;

	/* config */

	if ((tuple = dict_find(di, MESSAGE_KEY_BG)))
		conf.bg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_FG)))
		conf.fg = GColorFromHEX(tuple->value->int32);

	if ((tuple = dict_find(di, MESSAGE_KEY_SIDE)))
		strncpy(conf.side, tuple->value->cstring,
			sizeof conf.side -1);

	if ((tuple = dict_find(di, MESSAGE_KEY_BOTTOM)))
		strncpy(conf.bottom, tuple->value->cstring,
			sizeof conf.bottom -1);

	if ((tuple = dict_find(di, MESSAGE_KEY_BTON)))
		conf.bton = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_BTOFF)))
		conf.btoff = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_ONHOUR)))
		conf.onhour = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_PADH)))
		conf.padh = !!tuple->value->int8;

	if ((tuple = dict_find(di, MESSAGE_KEY_SHADOW)))
		conf.shadow = tuple->value->uint8;

	if ((tuple = dict_find(di, MESSAGE_KEY_SECONDS)))
		conf.seconds = atoi(tuple->value->cstring);

	if ((tuple = dict_find(di, MESSAGE_KEY_TEMPUNIT))) {
		conf.tempunit = tuple->value->cstring[0];
		state.lastweather = 0;
	}

	persist_write_data(CONFKEY, &conf, sizeof conf);
	persist_write_data(WEATHERKEY, &weather, sizeof weather);
	configure();
}

void
onbattery(BatteryChargeState bcs)
{
	state.battery = bcs.charge_percent;
	state.charging = bcs.is_charging;

	layer_mark_dirty(layout.side);
	layer_mark_dirty(layout.bottom);
}

void
onconnection(bool status)
{
	state.connected = status;
	vibe(status ? conf.bton : conf.btoff);
	weatherget();
	layer_mark_dirty(layout.side);
	layer_mark_dirty(layout.bottom);
}

void
ontap(AccelAxisType _axis, i32 _direction)
{
	static u8 count;

	count = 0;

	state.chronograph = time(0);
	persist_write_data(CHRONOGRAPHKEY, &state.chronograph,
			   sizeof state.chronograph);

	layer_mark_dirty(layout.side);
	layer_mark_dirty(layout.bottom);

	if (conf.seconds <= 0)
		return;

	app_timer_register(1000, ontimer, &count);
}

void
ontimer(void *ctx)
{
	u8 *count;

	count = ctx;
	(*count)++;

	layer_mark_dirty(layout.side);
	layer_mark_dirty(layout.bottom);

	if (*count >= conf.seconds)
		return;

	app_timer_register(1000, ontimer, count);
}

void
onquickview(AnimationProgress _progress, void *_ctx)
{
	GRect bounds, frame;

	bounds = layer_get_unobstructed_bounds(layout.body);

	frame = layer_get_frame(layout.minute);
	frame.origin.y = bounds.size.h - MARGIN - FONT10H - DIGITSH - SPACING + LETTERSPACING;
	layer_set_frame(layout.minute, frame);

	frame = layer_get_frame(layout.bottom);
	frame.origin.y = bounds.size.h - MARGIN - FONT10H;
	layer_set_frame(layout.bottom, frame);

	frame = layer_get_frame(layout.side);
	frame.size.h = bounds.size.h - MARGIN*2 - FONT10H - 1;	/* TODO(irek): Magic number */
	layer_set_frame(layout.side, frame);

	layer_mark_dirty(layout.hour);
}

#ifdef PBL_HEALTH
void
onhealth(HealthEventType event, void *_ctx)
{
	switch (event) {
	case HealthEventSignificantUpdate:
		health.steps = health_service_sum_today(HealthMetricStepCount);
		health.dista = health_service_sum_today(HealthMetricWalkedDistanceMeters);
		health.heart = health_service_sum_today(HealthMetricHeartRateBPM);
		health.sleep = health_service_sum_today(HealthMetricSleepSeconds);
		health.rest = health_service_sum_today(HealthMetricSleepRestfulSeconds);
		break;
	case HealthEventMovementUpdate:
		health.steps = health_service_sum_today(HealthMetricStepCount);
		health.dista = health_service_sum_today(HealthMetricWalkedDistanceMeters);
		break;
	case HealthEventSleepUpdate:
		health.sleep = health_service_sum_today(HealthMetricSleepSeconds);
		health.rest = health_service_sum_today(HealthMetricSleepRestfulSeconds);
		break;
	case HealthEventMetricAlert:
		break;
	case HealthEventHeartRateUpdate:
		health.heart = health_service_sum_today(HealthMetricHeartRateBPM);
		break;
	}
}
#endif

int
main(void)
{
	Window *win;
	WindowHandlers wh;
	time_t timestamp;
	UnobstructedAreaHandlers quickview;

	/* resources */
	asset.digits = gdraw_command_image_create_with_resource(RESOURCE_ID_DIGITS);
	asset.font10 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT10));
	asset.font7 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT7));

	/* window */
	win = window_create();
	wh.load = onwinload;
	wh.appear = 0;
	wh.disappear = 0;
	wh.unload = onwinunload;
	window_set_window_handlers(win, wh);
	window_stack_push(win, true);

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
	conf.tempunit = 'C';
	persist_read_data(CONFKEY, &conf, sizeof conf);
	persist_read_data(WEATHERKEY, &weather, sizeof weather);
	configure();
	app_message_register_inbox_received(oninbox);
	app_message_open(app_message_inbox_size_maximum(), 32);	/* TODO(irek): Magic number warning */

	/* time */
	timestamp = time(0);
	ontick(localtime(&timestamp), DAY_UNIT | HOUR_UNIT | MINUTE_UNIT);
	/* Tick timer is overwritten in configure() but this is a
	 * default just in case there is something wrong with config
	 * which might happen when phone is disconnected, probably.
	 */
	tick_timer_service_subscribe(MINUTE_UNIT, ontick);
	state.chronograph = timestamp;

	/* unobstructed area (quick view) */
	quickview.will_change = 0;
	quickview.change = onquickview;
	quickview.did_change = 0;
	unobstructed_area_service_subscribe(quickview, 0);
	onquickview(0, NULL);
	/* NOTE(irek): Aplite has unobstructed_area_service_subscribe
	 * macro doing nothing.  In result the variable "quickview" is
	 * never used and compailer complains. */
	(void)quickview;

	/* services */
	accel_tap_service_subscribe(ontap);

	/* main */
	app_event_loop();

	/* cleanup */
	window_destroy(win);
	gdraw_command_image_destroy(asset.digits);
	fonts_unload_custom_font(asset.font10);
	fonts_unload_custom_font(asset.font7);

	return 0;
}
