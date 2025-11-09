var Clay = require('pebble-clay')

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
        "messageKey": "BGCOLOR",
        "label": "Background",
        "allowGray": false,
        "defaultValue": "ffffff"
      },
      {
        "type": "color",
        "messageKey": "FGCOLOR",
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
        "messageKey": "VIBEBTOFF",
        "label": "Bluetooth disconnected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEBTON",
        "label": "Bluetooth connected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEEACHHOUR",
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

    bottomSelect.on("change", function () {
      bottomInput.set(bottomSelect.get())
    })

    sideSelect.on("change", function () {
      sideInput.set(sideSelect.get())
    })
  }.bind(this))
})
