module.exports = [
  {
    "type": "heading",
    "defaultValue": "Balanced Settings"
  },
  {
    "type": "text",
    "defaultValue": "Version 2.1",
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "General"
      },
      {
        "type": "toggle",
        "messageKey": "darkMode",
        "label": "Dark Mode",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "trimLeadingZero",
        "label": "Hide Leading Zero",
        "defaultValue": true
      },
      {
        "type": "text",
        "defaultValue": "For example 1:00 vs 01:00.",
      },
      {
        "type": "toggle",
        "messageKey": "dateAboveTime",
        "label": "Date Above Time",
        "defaultValue": true
      },
      {
        "type": "text",
        "defaultValue": "When enabled the date will appear above the time and health bellow it. When disabled the opposite will happen.",
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Month Bar"
      },
      {
        "type": "toggle",
        "messageKey": "abridgedMonth",
        "label": "Shortened Month and include Day of the Week",
        "defaultValue": false
      },
      {
        "type": "text",
        "defaultValue": "For example February 3 vs Fri, Feb 3"
      },
      {
        "type": "toggle",
        "messageKey": "hideDate",
        "label": "Hide Date",
        "defaultValue": false
      },
      {
        "type": "select",
        "messageKey": "dateAlignment",
        "defaultValue": 0,
        "label": "Date Alignment",
        "options": [
          { 
            "label": "Right", 
            "value": 0
          },
          { 
            "label": "Center", 
            "value": 1
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bluetooth"
      },
      {
        "type": "toggle",
        "messageKey": "showBluetoothIcon",
        "label": "Show Bluetooth Icon on Disconnect",
        "defaultValue": true
      },
      {
        "type": "select",
        "messageKey": "bluetoothVibrationType",
        "defaultValue": 0,
        "label": "Disconnection Vibration Type",
        "options": [
          { 
            "label": "Off", 
            "value": 0
          },
          { 
            "label": "Short", 
            "value": 1
          },
          { 
            "label": "Long",
            "value": 2 
          },
          { 
            "label": "Double Vibration",
            "value": 3
          }
        ]
      },
      {
        "type": "select",
        "messageKey": "bluetoothReconnectionVibrationType",
        "defaultValue": 0,
        "label": "Reconnection Vibration Type",
        "options": [
          { 
            "label": "Off", 
            "value": 0
          },
          { 
            "label": "Short", 
            "value": 1
          },
          { 
            "label": "Long",
            "value": 2 
          },
          { 
            "label": "Double Vibration",
            "value": 3
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Health Bar"
      },
      {
        "type": "toggle",
        "messageKey": "healthEnabled",
        "label": "Show Health Bar",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
 