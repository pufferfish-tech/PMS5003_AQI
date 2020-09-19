This arduino code enables you to quickly get an AQI readout (and PM2.5 / PM10) from a PMSXXXX dust sensor from plantower. 
It uses the Adafruit SSD1306 library for a 128 x 64 display. 

To use it you will need to install Adafruit_GFX and Adafruit_SSD1306 libraries in arduino IDE. 

Wiring on arduino UNO: OLED SDA = Pin 4, SCL = Pin 5
On sensor only pin "txd" is connected to arduino Pin 2

Code is fairly simple to understand. 

Note that it gives instantaneous readings and so while "in the spirit" of the AQI calculation, it does not average over 24 hours or whatever. 
This may be something I code for in future. 
