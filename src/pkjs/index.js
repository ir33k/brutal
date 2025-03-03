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
        "type": "text",
        "defaultValue": "Fits up to 13 characters."
      },
      {
        "id": "bottom-select",
        "type": "select",
        "label": "Pressets",
        "defaultValue": "%A %d",
        "options": [
          { "value": "",         "label": "NOTHING" },
          { "value": "%A %d",    "label": "DEFAULT: Monday 18" },
          { "value": "%a %d",    "label": "Sun 18" },
          { "value": "%B %d",    "label": "November 18" },
          { "value": "%b %d",    "label": "Nov 18" },
          { "value": "%m/%d/%y", "label": "11/18/24" },
          { "value": "%Y.%m.%d", "label": "2022.11.18" },
          { "value": "%d.%m.%Y", "label": "18.11.2022" }
        ]
      },
      {
        "id": "bottom-input",
        "type": "input",
        "messageKey": "BOTTOM",
        "label": "Format",
        "defaultValue": "%A %d",
	"description": "Format strings follows strftime(3) manual page."
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Left text"
      },
      {
        "type": "text",
        "defaultValue": "Fits up to 20 characters."
      },
      {
        "id": "left-select",
        "type": "select",
        "label": "Pressets",
        "defaultValue": "%B %Y",
        "options": [
          { "value": "",         "label": "NOTHING" },
          { "value": "%B %Y",    "label": "DEFAULT: November 2024" },
          { "value": "%A %d",    "label": "Monday 18" },
          { "value": "%a %d",    "label": "Sun 18" },
          { "value": "%B %d",    "label": "November 18" },
          { "value": "%b %d",    "label": "Nov 18" },
          { "value": "%m/%d/%y", "label": "11/18/24" },
          { "value": "%Y.%m.%d", "label": "2022.11.18" },
          { "value": "%d.%m.%Y", "label": "18.11.2022" }
        ]
      },
      {
        "id": "left-input",
        "type": "input",
        "messageKey": "LEFT",
        "label": "Format",
        "defaultValue": "%B %Y",
	"description": "Format strings follows strftime(3) manual page."
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
        "messageKey": "VIBEBTON",
        "label": "Bluetooth disconnected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEBTOFF",
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
    "type": "submit",
    "defaultValue": "Save"
  }
], function () {
  this.on(this.EVENTS.AFTER_BUILD, function () {
    var bottomSelect = this.getItemById("bottom-select")
    var bottomInput = this.getItemById("bottom-input")
    var leftSelect = this.getItemById("left-select")
    var leftInput = this.getItemById("left-input")

    bottomSelect.on("change", function () {
      bottomInput.set(bottomSelect.get())
    })

    leftSelect.on("change", function () {
      leftInput.set(leftSelect.get())
    })
  }.bind(this))
})
