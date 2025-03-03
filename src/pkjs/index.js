var Clay = require('pebble-clay')

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
        "allowGray": true,
        "defaultValue": "ffffff"
      },
      {
        "type": "color",
        "messageKey": "FGCOLOR",
        "label": "Foreground",
        "allowGray": true,
        "defaultValue": "000000"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
])
