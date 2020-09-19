// NOTES
// OLED wiring, SDA = pin 4, SCL = pin 5
// Sensor wiring, data (txd) = pin 2

#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(2, 3);

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// this is the Width and Height of Display which is 128 xy 32 
#define LOGO16_GLCD_HEIGHT 64
#define LOGO16_GLCD_WIDTH  128 

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {              
  delay(3000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
  display.clearDisplay(); 
  // sensor baud rate is 9600
  pmsSerial.begin(9600);
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;
    
void loop() {
  if (readPMSdata(&pmsSerial)) {
   String vString5 = String(data.pm25_env);
   String vString6 = String(data.pm100_env);
   
  display.clearDisplay();

  showText("PM2.5:", 1, 14, 1, false);
  showText(vString5+" ug/M^2", 50, 14, 1, false);
  
  showText("PM10 :", 1, 24, 1, false);
  showText(vString6+" ug/M^2", 50, 24, 1, false);

  int aqival = calcAQI25(data.pm25_env);
  showText("AQI:" + String(aqival), 1, 1, 1, false);
  String aqi_text = make_aqi_words(aqival);
  showText(aqi_text, 50, 1, 1, false);
  
  display.display();
   
  delay(1000);  //readings only once per second
  }
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}

void showText(String text, int x, int y,int size, boolean d) {
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
}


int calcAQI25(int pm25){

  // Uses formula AQI = ( (pobs - pmin) x (aqimax - aqimin) ) / (pmax - pmin)  + aqimin
  float pmin, pmax, amin, amax;
  
  if (pm25 <= 12) {
    pmin = 0; pmax = 12; amin = 0; amax = 50;            goto aqicalc; 
  }
  if (pm25 <= 35.5) {
    pmin = 12; pmax = 35.5; amin = 50; amax = 100;        goto aqicalc;
  }
  if (pm25 <= 55.5) {
    pmin = 35.5; pmax = 55.5; amin = 100; amax = 150;     goto aqicalc;
  }
  if (pm25 <= 150.5) {
    pmin = 55.5; pmax = 150.5; amin = 150; amax = 200;    goto aqicalc;
  }
  if (pm25 <= 250.5){
    pmin = 150.5; pmax = 250.5; amin = 200; amax = 300;    goto aqicalc;
  }
  if (pm25 <= 350.5){
    pmin = 250.5; pmax = 350.5; amin = 300; amax = 400;    goto aqicalc;
  }
  if (pm25 <= 500.5){
    pmin = 350.5; pmax = 500.5; amin = 400; amax = 500;    goto aqicalc;
  } else {
    return 999;
  }
  
  aqicalc:
    float aqi = ( ((pm25 - pmin) * (amax - amin))  / (pmax - pmin) ) + amin;
    return aqi;

}

String make_aqi_words(float aqival){
  if (aqival <= 12){ return "GOOD!";  } 
  if (aqival <= 35.5){ return "MEDIUM";  } 
  if (aqival <= 55.5){ return "BAD";  } 
  if (aqival <= 150.5){ return "VERY BAD";  } 
  if (aqival <= 250.5){ return "REALLY BAD";  } 
  if (aqival <= 350.5){ return "DEADLY";  } 
  if (aqival <= 500.5){ return "GO. GO NOW!";  } 
  return "Error";
}
