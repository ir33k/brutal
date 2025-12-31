var Clay = require('pebble-clay')

var WEATHERKEY = 3

var vibs = [
  { "value": 0, "label": "None" },
  { "value": 1, "label": "Short" },
  { "value": 2, "label": "Long" },
  { "value": 3, "label": "Double" }
]

new Clay([
  {
    "type": "heading",
    "defaultValue": "BRUTAL"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "BG",
        "label": "Background",
        "allowGray": false,
        "defaultValue": "ffffff"
      },
      {
        "type": "color",
        "messageKey": "FG",
        "label": "Foreground",
        "allowGray": false,
        "defaultValue": "000000"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bottom text"
      },
      {
	"capabilities": ["NOT_PLATFORM_EMERY"],
        "type": "text",
        "defaultValue": "Fits up to 17 characters."
      },
      {
	"capabilities": ["PLATFORM_EMERY"],
        "type": "text",
        "defaultValue": "Fits up to 24 characters."
      },
      {
	"capabilities": ["RECT"],
        "type": "text",
        "defaultValue": "You can divide text to left and right parts with single comma (,) character."
      },
      {
        "id": "bottom-select",
        "type": "select",
        "label": "Presets",
        "defaultValue": "%A %d",
        "options": [
          { "value": "",              "label": "NOTHING" },
          { "value": "%A %d",         "label": "DEFAULT: Monday 18" },
          { "value": "%p",            "label": "AM/PM" },
          { "value": "%a %d",         "label": "Sun 18" },
          { "value": "%B %d",         "label": "November 18" },
          { "value": "%b %d",         "label": "Nov 18" },
          { "value": "%m/%d/%y",      "label": "11/18/24" },
          { "value": "%Y.%m.%d",      "label": "2022.11.18" },
          { "value": "%d.%m.%Y",      "label": "18.11.2022" },
          { "value": "Battery: #B%%", "label": "Battery: 75%" },
          { "value": "Steps: #S",     "label": "Steps: 500" },
          { "value": "%B %d,%S",      "label": "Date with seconds" }
        ]
      },
      {
        "id": "bottom-input",
        "type": "input",
        "messageKey": "BOTTOM",
        "label": "Format",
        "defaultValue": "%A %d",
        "description": "Format strings follows strftime(3) manual page. Add '#B' to print battery charge percent. Add '#S' to print steps count from today."
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Side text"
      },
      {
	"capabilities": ["NOT_PLATFORM_EMERY"],
        "type": "text",
        "defaultValue": "Fits up to 21 characters."
      },
      {
	"capabilities": ["PLATFORM_EMERY"],
        "type": "text",
        "defaultValue": "Fits up to 29 characters."
      },
      {
	"capabilities": ["RECT"],
        "type": "text",
        "defaultValue": "You can divide text to top and bottom parts with single comma (,) character."
      },
      {
        "id": "side-select",
        "type": "select",
        "label": "Presets",
        "defaultValue": "%B %Y",
        "options": [
          { "value": "",              "label": "NOTHING" },
          { "value": "%B %Y",         "label": "DEFAULT: November 2024" },
          { "value": "%p",            "label": "AM/PM" },
          { "value": "%A %d",         "label": "Monday 18" },
          { "value": "%a %d",         "label": "Sun 18" },
          { "value": "%B %d",         "label": "November 18" },
          { "value": "%b %d",         "label": "Nov 18" },
          { "value": "%m/%d/%y",      "label": "11/18/24" },
          { "value": "%Y.%m.%d",      "label": "2022.11.18" },
          { "value": "%d.%m.%Y",      "label": "18.11.2022" },
          { "value": "Rebble %Y",     "label": "Rebble 2022" },
          { "value": "Battery #B",    "label": "Battery 75" },
          { "value": "Steps #S",      "label": "Steps 500" },
          { "value": "Steps #S,#B%%", "label": "Steps with battery" }
        ]
      },
      {
        "id": "side-input",
        "type": "input",
        "messageKey": "SIDE",
        "label": "Format",
        "defaultValue": "%B %Y",
        "description": "Format strings follows strftime(3) manual page. Add '#B' to print battery charge percent. Add '#S' to print steps count from today."
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Vibrations"
      },
      {
        "type": "select",
        "messageKey": "BTOFF",
        "label": "Bluetooth disconnected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "BTON",
        "label": "Bluetooth connected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "ONHOUR",
        "label": "Hourly",
        "defaultValue": 0,
        "options": vibs
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather",
      },
      {
        "type": "text",
        "defaultValue": "Weather features are activated when you use at least one weather placeholder in one of text areas.  Weather information is fetched every 30 min based on your current location (GPS) and temperature unit settings. After successful fetch your last GPS location is stored so next time even if GPS is inactive you will get weather for your last location."
      },
      {
        "type": "select",
        "messageKey": "TEMPUNIT",
        "defaultValue": "C",
        "label": "Temperature unit",
        "options": [
          { "value": "C", "label": "Celcius" },
          { "value": "F", "label": "Fahrenheit" }
        ]
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Other"
      },
      {
        "type": "toggle",
        "messageKey": "PADH",
        "label": "Avoid drawing leading 0 in hours",
        "defaultValue": false
      },
      {
        "type": "slider",
        "messageKey": "SHADOW",
        "defaultValue": 16,
        "label": "Shadow strength",
        "description": "Dithering for numbers shadow. Some values works better than other because they give more uniform look, values like 4, 8, 16.  Use value of 0 to disable.",
        "min": 0,
        "max": 128,
        "step": 4
      },
      {
        "messageKey": "SECONDS",
        "type": "select",
        "label": "Seconds",
	"description": "First option disables seconds update; time, side and bottom texts are updated only each minute. Second option enables updates each second for side and bottom texts, use %S in their format to show seconds. Every other option enables seconds update after Tap (whist shake) for some amount of time to avoid draining battery.",
	// NOTE(Irek): Select input always return value as a string.
	// I have to do atoi() conversion in main code anyway.
        "defaultValue": "0",
        "options": [
          { "value": "0",  "label": "Disabled" },
          { "value": "-1", "label": "Enable" },
          { "value": "5",  "label": "5 seconds" },
          { "value": "10", "label": "10 seconds" },
          { "value": "15", "label": "15 seconds" },
          { "value": "30", "label": "30 seconds" },
          { "value": "60", "label": "1 min" },
          { "value": "300", "label": "5 min" }
        ]
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
], function () {
  this.on(this.EVENTS.AFTER_BUILD, function () {
    var bottomSelect = this.getItemById("bottom-select")
    var bottomInput = this.getItemById("bottom-input")
    var sideSelect = this.getItemById("side-select")
    var sideInput = this.getItemById("side-input")

    bottomSelect.on("click", function () {
      bottomInput.set(bottomSelect.get())
    })
    bottomSelect.on("change", function () {
      bottomInput.set(bottomSelect.get())
    })

    sideSelect.on("click", function () {
      sideInput.set(sideSelect.get())
    })
    sideSelect.on("change", function () {
      sideInput.set(sideSelect.get())
    })
  }.bind(this))
})

function fetch(url, onResponse, onError) {
  var xhr = new XMLHttpRequest()
  xhr.onload = function (event) {
    onResponse(this.responseText)
  }
  xhr.onerror = function (error) {
    console.log("fetch onerror", JSON.stringify(this), JSON.stringify(error))
    onError(error)
  }
  xhr.open("GET", url)
  xhr.send()
}

function fetchAirQuality(pos) {
  var date = new Date()
  var startHour = date.toISOString().slice(0, 16)

  var url = "https://air-quality-api.open-meteo.com/v1/air-quality" +
      "?latitude=" + pos.coords.latitude +
      "&longitude=" + pos.coords.longitude +
      "&start_hour=" + startHour +
      "&end_hour=" + startHour +
      "&current=european_aqi,us_aqi" +
      "&timezone=auto"

  fetch(url, function (res) {
    var data = JSON.parse(res)

    Pebble.sendAppMessage({
      WEATHERAIREU: data.current.european_aqi,
      WEATHERAIRUS: data.current.us_aqi,
    })
  })
}

function fetchWeather(temperatureUnit, pos) {
  // https://open-meteo.com/en/docs
  var url = "https://api.open-meteo.com/v1/forecast" +
      "?latitude=" + pos.coords.latitude +
      "&longitude=" + pos.coords.longitude +
      "&temperature_unit=" + temperatureUnit +
      "&current_weather=true" +
      "&daily=temperature_2m_max,temperature_2m_min,weathercode" +
      "&timezone=auto"
  
  fetch(url, function (res) {
    var data = JSON.parse(res)

    Pebble.sendAppMessage({
      WEATHERTEMP: data.current_weather.temperature.toString(),
      WEATHERTEMPHIGH: data.daily.temperature_2m_max[0].toString(),
      WEATHERTEMPLOW: data.daily.temperature_2m_min[0].toString(),
      WEATHERCODE: data.current_weather.weathercode,
    })
  })
}

function weatherGet(temperatureUnit) {
  navigator.geolocation.getCurrentPosition(
    function (pos) {
      localStorage.setItem("currentPosition", JSON.stringify(pos))
      fetchWeather(temperatureUnit, pos)
      fetchAirQuality(pos)
    },
    function (error) {
      var pos = localStorage.getItem("currentPosition")

      if (!pos)
        return

      pos = JSON.parse(pos)

      fetchWeather(temperatureUnit, pos)
      fetchAirQuality(pos)
    },
    { timeout: 15000, maximumAge: 60000 }
  )
}

Pebble.addEventListener("ready", function() {
  console.log("onready")
  Pebble.sendAppMessage({ READY: 1 })
})

Pebble.addEventListener("appmessage", function (event) {
  console.log("onappmessage", JSON.stringify(event.payload))

  if (event.payload[WEATHERKEY])
    weatherGet(event.payload[WEATHERKEY][0] === 0x46 ? "fahrenheit" : "celsius")
})
