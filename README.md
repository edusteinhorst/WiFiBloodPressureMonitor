## Blood Pressure Monitor WiFi Interface - ESP8266

This is the Arduino code for a ESP8266 used to add WiFi capability to a Panasonic blood presure monitor.

Please check out the full project at http://www.edusteinhorst.com/hacking-a-blood-pressure-monitor/

## Disclaimer

This is a very rough initial version of the code. Don't expect reliability. Also, keep in mind your modifying a medical device and that CAN interfere with it's accuracy and reliability. Remeber a WiFi device emmits RF which can affect other devices. If you choose to modify it, don't use it as a medical device anymore. Use this code at your own risk. I cannot be held liable if, knock on wood, your house burns down our your dog dies!

## Motivation

I had an old blood pressure monitor that I wanted to make capable of pushing data to the cloud. This project adds allows the user to send measurements to an IoT log database with the push of a button. The data can then be used to plot charts and identify trends, correlate fluctuations with other health data, etc.

The IoT database used was SparkFun's data.sparkfun.com.

## Installation

Please check the details at http://www.edusteinhorst.com/hacking-a-blood-pressure-monitor/

## Usage

Modify the constants in the code to reflect your access point ssid/pwd and your data.sparkfun.com keys.

## Contributors

Feel free to fork this and have at it. You can reach me at edusteinhorst.com

## License

This project is licensed under the terms of the MIT license.
