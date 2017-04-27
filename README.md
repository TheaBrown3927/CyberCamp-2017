# CyberCamp-2017 #
This is the code as well as the schematic for the electronics portion of CyberCamp 2017

# Set Up #
This circuit is powered by a Wemos D1 Mini, a Maxium7219, and an anode 8x8 LED matrix

## Programming With the Arduino IDE ##
--------------------------------------

### Adding the ESP8266 chip to your Arduino IDE ###
1. copy http://arduino.esp8266.com/stable/package_esp8266com_index.json into file -> preferences -> additional boards manager URLs
 2. Go to tools -> boards: -> board manager, find the esp8266 board, click on it and click install


### Adding the MAX72xxPanel Library to the Arduino IDE ###
1. You will need to download [Max72xxPanel][download] and [Adafruit_GFX][gfx-download]
2. Then in the Arduino IDE go to Sketch > Include Library > Add .zip library
3. Select the two libraries and they are now included in the IDE


### Uploading to the board ###
Make sure the chip is selected, you can do this by going to tools > board > Wemos D1 R2 & mini





[download]: https://github.com/markruys/arduino-Max72xxPanel/archive/master.zip "Download Max72xxPanel library"
[gfx-download]: https://github.com/adafruit/Adafruit-GFX-Library "Download Adafruit GFX Graphics Library"
