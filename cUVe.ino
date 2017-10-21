#include <Adafruit_TSL2561_U.h>
//#include <pgmspace.h>

#include <Adafruit_Sensor.h>

#include <Adafruit_ILI9340.h>

#include <Adafruit_GFX.h>
#include <gfxfont.h>

#include "SPI.h"

#include "Wire.h"

/****************** 
*	DEFINE PINS   *
******************/

//SPI Pins
#define sclk 13
#define miso 12
#define mosi 11
#define csTFT 10
#define csUI 9
#define dc 8
#define rst 17

//Fans
#define pin_fanBoard 2
#define pin_fanHeating 3

//Leds
#define pin_led1 7
#define pin_led2 6
#define pin_led3 5
//Heating
#define pin_heating 4

//Temp Sensors
#define pin_temp1 15
#define pin_temp2 16


//External device - If this Pin is high, a device is connected and I2C is interfaced
#define pin_externalSensor 14



/*********************
* DEFINE ADC CHANNEL *
*********************/

#define channel_temp 2
#define channel_light 1
#define channel_timer 3

#define channel_button 0

#define button_threshold 800



/****************** 
*	GLOBAL VARS   *
******************/

int button = 0;
float temp = 0;
float timer = 0;
float light = 0;
int oldValues = 0;
int deviceCode = 0;
int lux = 0;
int tempChamber=0;
int tempElectronics;

int tempStatus=0;

//Set input range
int tempMax=85; //in °C
int timerMax=240; //in min
int lightMax = 7; //Only 7 steps from combinations of light array 1,2,3


/****************** 
*	INIT OBJECTS  *
******************/

// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(csTFT, dc, rst);


//The User Interface (3 Potis + Button) are connected via an MCP3008), therefor
// the SPI Pins need to be set (http://arduinolearning.com/code/arduino-and-mcp3008.php)
//MCP3008 adc(sclk,mosi,miso,csUI); //Uncommented. SPI directly implemented

//Initialize External light Sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);



/****************** 
*	 FUNCTIONS    *
******************/

// Input functions
int readUI() {
  button = readADC(channel_button);

  if (button>button_threshold){
    button =1;
    }
  else{
    button=0;
    }
  
  temp = readADC(channel_temp);
  temp = int(((1024-temp)/1024)*(tempMax-20)+20);

  
  timer = readADC(channel_timer);
  timer = int((((1024-timer)/1024)*(timerMax-5))/5);
  timer = timer*5+5;

  light = readADC(channel_light);
  light = int((((1024-light)/1024)*lightMax));
  
//  DEBUG:
//  Serial.print(" -> Button: ");
//  Serial.print(button);
//  Serial.print(" -> Temp: ");
//  Serial.println(temp);

//  Serial.print(" -> Timer: ");
//  Serial.println(timer);

//  Serial.print(" -> Light: ");
//  Serial.println(light);
}

unsigned int readADC(int ch) {
  if (ch < 0 || ch > 7) return -1;
  
  int command = 0b11 << 6;
  command |= (ch & 0x07) << 3;
  digitalWrite(csUI, HIGH);
  digitalWrite(csUI, LOW);

  byte val = 0;
  unsigned int result = 0;

  val = SPI.transfer(command);
  result = (val & 0x01) << 9;
  val = SPI.transfer(0);
  result |= (val & 0xFF) << 1;
  val = SPI.transfer(0);
  result |= (val & 0x80) >> 7;
  result = result & 0x3FF;
  
//   DEBUG:
//  Serial.print("ADC Channel: ");
//  Serial.print(ch);
//  Serial.print(" = ");
//  Serial.print("result");

  return result;
}

int readLight() {  
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    Serial.print(event.light); Serial.println(" lux");
    lux = event.light;
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }
}

int readTemp(){
  Serial.print("Read temperature from sensors.\n");
  int tempOld=tempChamber;
  
  tempChamber = int((((analogRead(pin_temp1)*5.0)/1024.0)-0.5)*100);

  
  tempElectronics = int((((analogRead(pin_temp2)*5.0)/1024.0)-0.5)*100);

  if (tempChamber != tempOld){
    screen_updateTemp();
  }
  
//  DEBUG
  
//  Serial.print("Chamber temperature:    \t");
//  Serial.print(tempChamber);
//  Serial.print("°C\n");
//  
//  Serial.print("Electronics temperature:\t");
//  Serial.print(tempElectronics);
//  Serial.print("°C\n");
  
}


// Screen functions
int screen_default() {
  tft.fillScreen(ILI9340_WHITE);
  tft.fillRect(0, 0, tft.width(),35,0x7BEF);
  tft.fillRect(0, tft.height()-35, tft.width(),tft.height(),0x7BEF);
  tft.fillRect(95,15, 142,75,0xffff);
  tft.drawLine(102, 10, 229, 10,0xfb20);
  tft.drawLine(99, 11, 231, 11,0xfb20);
  tft.drawLine(98, 12, 233, 12,0xfb20);
  tft.drawLine(96, 13, 234, 13,0xfb20);
  tft.drawLine(95, 14, 235, 14,0xfb20);
  tft.drawLine(95, 15, 236, 15,0xfb20);
  tft.drawLine(95, 16, 237, 16,0xfb20);
  tft.drawLine(93, 17, 225, 17,0xfb20);
  tft.drawLine(226, 17, 238, 17,0xfb20);
  tft.drawLine(93, 18, 103, 18,0xfb20);
  tft.drawLine(230, 18, 238, 18,0xfb20);
  tft.drawLine(92, 19, 101, 19,0xfb20);
  tft.drawLine(231, 19, 239, 19,0xfb20);
  tft.drawLine(93, 20, 100, 20,0xfb20);
  tft.drawLine(232, 20, 239, 20,0xfb20);
  tft.drawLine(92, 21, 99, 21,0xfb20);
  tft.drawLine(233, 21, 239, 21,0xfb20);
  tft.drawLine(92, 22, 98, 22,0xfb20);
  tft.drawLine(234, 22, 239, 22,0xfb20);
  tft.drawLine(92, 23, 98, 23,0xfb20);
  tft.drawLine(234, 23, 239, 23,0xfb20);
  tft.drawLine(90, 24, 98, 24,0xfb20);
  tft.drawLine(235, 24, 239, 24,0xfb20);
  tft.drawLine(90, 25, 98, 25,0xfb20);
  tft.drawLine(212, 25, 216, 25,0x0);
  tft.drawLine(235, 25, 239, 25,0xfb20);
  tft.drawLine(90, 26, 98, 26,0xfb20);
  tft.drawLine(210, 26, 218, 26,0x0);
  tft.drawLine(235, 26, 239, 26,0xfb20);
  tft.drawLine(90, 27, 98, 27,0xfb20);
  tft.drawLine(209, 27, 212, 27,0x0);
  tft.drawLine(218, 27, 219, 27,0x0);
  tft.drawLine(223, 27, 224, 27,0x0);
  tft.drawLine(235, 27, 239, 27,0xfb20);
  tft.drawLine(90, 28, 98, 28,0xfb20);
  tft.drawLine(208, 28, 210, 28,0x0);
  tft.drawLine(223, 28, 225, 28,0x0);
  tft.drawLine(235, 28, 239, 28,0xfb20);
  tft.drawLine(90, 29, 98, 29,0xfb20);
  tft.drawLine(108, 29, 114, 29,0x0);
  tft.drawLine(124, 29, 131, 29,0x0);
  tft.drawLine(140, 29, 147, 29,0x0);
  tft.drawLine(155, 29, 162, 29,0x0);
  tft.drawLine(175, 29, 183, 29,0x0);
  tft.drawLine(192, 29, 200, 29,0x0);
  tft.drawLine(207, 29, 210, 29,0x0);
  tft.drawLine(214, 29, 216, 29,0x0);
  tft.drawLine(219, 29, 220, 29,0x0);
  tft.drawLine(224, 29, 226, 29,0x0);
  tft.drawLine(235, 29, 239, 29,0xfb20);
  tft.drawLine(90, 30, 98, 30,0xfb20);
  tft.drawLine(107, 30, 115, 30,0x0);
  tft.drawLine(122, 30, 133, 30,0x0);
  tft.drawLine(140, 30, 147, 30,0x0);
  tft.drawLine(154, 30, 163, 30,0x0);
  tft.drawLine(175, 30, 185, 30,0x0);
  tft.drawLine(192, 30, 200, 30,0x0);
  tft.drawLine(207, 30, 210, 30,0x0);
  tft.drawLine(215, 30, 220, 30,0x0);
  tft.drawLine(225, 30, 227, 30,0x0);
  tft.drawLine(235, 30, 239, 30,0xfb20);
  tft.drawLine(90, 31, 98, 31,0xfb20);
  tft.drawLine(106, 31, 115, 31,0x0);
  tft.drawLine(121, 31, 134, 31,0x0);
  tft.drawLine(139, 31, 147, 31,0x0);
  tft.drawLine(153, 31, 164, 31,0x0);
  tft.drawLine(175, 31, 186, 31,0x0);
  tft.drawLine(192, 31, 200, 31,0x0);
  tft.drawLine(207, 31, 210, 31,0x0);
  tft.drawLine(217, 31, 219, 31,0x0);
  tft.drawLine(225, 31, 227, 31,0x0);
  tft.drawLine(235, 31, 239, 31,0xfb20);
  tft.drawLine(90, 32, 98, 32,0xfb20);
  tft.drawLine(106, 32, 110, 32,0x0);
  tft.drawLine(113, 32, 114, 32,0x0);
  tft.drawLine(120, 32, 134, 32,0x0);
  tft.drawLine(139, 32, 143, 32,0x0);
  tft.drawLine(152, 32, 165, 32,0x0);
  tft.drawLine(175, 32, 187, 32,0x0);
  tft.drawLine(192, 32, 197, 32,0x0);
  tft.drawLine(208, 32, 210, 32,0x0);
  tft.drawLine(225, 32, 228, 32,0x0);
  tft.drawLine(235, 32, 239, 32,0xfb20);
  tft.drawLine(90, 33, 98, 33,0xfb20);
  tft.drawLine(106, 33, 109, 33,0x0);
  tft.drawLine(120, 33, 124, 33,0x0);
  tft.drawLine(130, 33, 135, 33,0x0);
  tft.drawLine(139, 33, 142, 33,0x0);
  tft.drawLine(151, 33, 155, 33,0x0);
  tft.drawLine(162, 33, 165, 33,0x0);
  tft.drawLine(175, 33, 178, 33,0x0);
  tft.drawLine(183, 33, 188, 33,0x0);
  tft.drawLine(192, 33, 195, 33,0x0);
  tft.drawLine(208, 33, 210, 33,0x0);
  tft.drawLine(225, 33, 228, 33,0x0);
  tft.drawLine(235, 33, 239, 33,0xfb20);
  tft.drawLine(90, 34, 98, 34,0xfb20);
  tft.drawLine(106, 34, 110, 34,0x0);
  tft.drawLine(119, 34, 123, 34,0x0);
  tft.drawLine(132, 34, 135, 34,0x0);
  tft.drawLine(139, 34, 142, 34,0x0);
  tft.drawLine(151, 34, 154, 34,0x0);
  tft.drawLine(175, 34, 178, 34,0x0);
  tft.drawLine(184, 34, 188, 34,0x0);
  tft.drawLine(192, 34, 195, 34,0x0);
  tft.drawLine(208, 34, 210, 34,0x0);
  tft.drawLine(218, 34, 219, 34,0x0);
  tft.drawLine(225, 34, 227, 34,0x0);
  tft.drawLine(235, 34, 239, 34,0xfb20);
  tft.drawLine(90, 35, 98, 35,0xfb20);
  tft.drawLine(106, 35, 113, 35,0x0);
  tft.drawLine(119, 35, 123, 35,0x0);
  tft.drawLine(132, 35, 136, 35,0x0);
  tft.drawLine(139, 35, 147, 35,0x0);
  tft.drawLine(151, 35, 154, 35,0x0);
  tft.drawLine(175, 35, 178, 35,0x0);
  tft.drawLine(185, 35, 188, 35,0x0);
  tft.drawLine(192, 35, 199, 35,0x0);
  tft.drawLine(208, 35, 210, 35,0x0);
  tft.drawLine(216, 35, 219, 35,0x0);
  tft.drawLine(224, 35, 226, 35,0x0);
  tft.drawLine(235, 35, 239, 35,0xfb20);
  tft.drawLine(90, 36, 98, 36,0xfb20);
  tft.drawLine(106, 36, 114, 36,0x0);
  tft.drawLine(119, 36, 122, 36,0x0);
  tft.drawLine(133, 36, 136, 36,0x0);
  tft.drawLine(139, 36, 147, 36,0x0);
  tft.drawLine(150, 36, 154, 36,0x0);
  tft.drawLine(159, 36, 166, 36,0x0);
  tft.drawLine(175, 36, 178, 36,0x0);
  tft.drawLine(185, 36, 188, 36,0x0);
  tft.drawLine(192, 36, 199, 36,0x0);
  tft.drawLine(208, 36, 210, 36,0x0);
  tft.drawLine(216, 36, 218, 36,0x0);
  tft.drawLine(222, 36, 226, 36,0x0);
  tft.drawLine(235, 36, 239, 36,0xfb20);
  tft.drawLine(90, 37, 98, 37,0xfb20);
  tft.drawLine(106, 37, 115, 37,0x0);
  tft.drawLine(119, 37, 122, 37,0x0);
  tft.drawLine(133, 37, 136, 37,0x0);
  tft.drawLine(139, 37, 147, 37,0x0);
  tft.drawLine(150, 37, 154, 37,0x0);
  tft.drawLine(159, 37, 166, 37,0x0);
  tft.drawLine(175, 37, 178, 37,0x0);
  tft.drawLine(185, 37, 188, 37,0x0);
  tft.drawLine(192, 37, 199, 37,0x0);
  tft.drawLine(208, 37, 210, 37,0x0);
  tft.drawLine(215, 37, 217, 37,0x0);
  tft.drawLine(223, 37, 225, 37,0x0);
  tft.drawLine(235, 37, 239, 37,0xfb20);
  tft.drawLine(90, 38, 98, 38,0xfb20);
  tft.drawLine(110, 38, 116, 38,0x0);
  tft.drawLine(119, 38, 122, 38,0x0);
  tft.drawLine(132, 38, 136, 38,0x0);
  tft.drawLine(139, 38, 143, 38,0x0);
  tft.drawLine(151, 38, 154, 38,0x0);
  tft.drawLine(159, 38, 166, 38,0x0);
  tft.drawLine(175, 38, 178, 38,0x0);
  tft.drawLine(185, 38, 188, 38,0x0);
  tft.drawLine(192, 38, 197, 38,0x0);
  tft.drawLine(209, 38, 211, 38,0x0);
  tft.drawLine(215, 38, 217, 38,0x0);
  tft.drawLine(223, 38, 225, 38,0x0);
  tft.drawLine(235, 38, 239, 38,0xfb20);
  tft.drawLine(90, 39, 98, 39,0xfb20);
  tft.drawLine(112, 39, 116, 39,0x0);
  tft.drawLine(119, 39, 123, 39,0x0);
  tft.drawLine(132, 39, 135, 39,0x0);
  tft.drawLine(139, 39, 142, 39,0x0);
  tft.drawLine(151, 39, 154, 39,0x0);
  tft.drawLine(159, 39, 160, 39,0x0);
  tft.drawLine(161, 39, 166, 39,0x0);
  tft.drawLine(175, 39, 178, 39,0x0);
  tft.drawLine(184, 39, 188, 39,0x0);
  tft.drawLine(192, 39, 195, 39,0x0);
  tft.drawLine(209, 39, 211, 39,0x0);
  tft.drawLine(215, 39, 219, 39,0x0);
  tft.drawLine(223, 39, 225, 39,0x0);
  tft.drawLine(235, 39, 239, 39,0xfb20);
  tft.drawLine(90, 40, 98, 40,0xfb20);
  tft.drawLine(113, 40, 116, 40,0x0);
  tft.drawLine(120, 40, 124, 40,0x0);
  tft.drawLine(131, 40, 135, 40,0x0);
  tft.drawLine(139, 40, 142, 40,0x0);
  tft.drawLine(151, 40, 155, 40,0x0);
  tft.drawLine(162, 40, 165, 40,0x0);
  tft.drawLine(175, 40, 178, 40,0x0);
  tft.drawLine(183, 40, 187, 40,0x0);
  tft.drawLine(192, 40, 195, 40,0x0);
  tft.drawLine(209, 40, 211, 40,0x0);
  tft.drawLine(215, 40, 219, 40,0x0);
  tft.drawLine(223, 40, 225, 40,0x0);
  tft.drawLine(235, 40, 239, 40,0xfb20);
  tft.drawLine(90, 41, 98, 41,0xfb20);
  tft.drawLine(106, 41, 109, 41,0x0);
  tft.drawLine(112, 41, 116, 41,0x0);
  tft.drawLine(120, 41, 126, 41,0x0);
  tft.drawLine(129, 41, 134, 41,0x0);
  tft.drawLine(139, 41, 142, 41,0x0);
  tft.drawLine(152, 41, 156, 41,0x0);
  tft.drawLine(161, 41, 165, 41,0x0);
  tft.drawLine(175, 41, 179, 41,0x0);
  tft.drawLine(182, 41, 187, 41,0x0);
  tft.drawLine(192, 41, 195, 41,0x0);
  tft.drawLine(209, 41, 211, 41,0x0);
  tft.drawLine(214, 41, 216, 41,0x0);
  tft.drawLine(217, 41, 219, 41,0x0);
  tft.drawLine(223, 41, 225, 41,0x0);
  tft.drawLine(235, 41, 239, 41,0xfb20);
  tft.drawLine(90, 42, 98, 42,0xfb20);
  tft.drawLine(106, 42, 115, 42,0x0);
  tft.drawLine(121, 42, 134, 42,0x0);
  tft.drawLine(139, 42, 142, 42,0x0);
  tft.drawLine(152, 42, 164, 42,0x0);
  tft.drawLine(175, 42, 186, 42,0x0);
  tft.drawLine(192, 42, 200, 42,0x0);
  tft.drawLine(209, 42, 211, 42,0x0);
  tft.drawLine(214, 42, 216, 42,0x0);
  tft.drawLine(217, 42, 220, 42,0x0);
  tft.drawLine(223, 42, 225, 42,0x0);
  tft.drawLine(235, 42, 239, 42,0xfb20);
  tft.drawLine(90, 43, 98, 43,0xfb20);
  tft.drawLine(106, 43, 115, 43,0x0);
  tft.drawLine(122, 43, 133, 43,0x0);
  tft.drawLine(139, 43, 142, 43,0x0);
  tft.drawLine(153, 43, 164, 43,0x0);
  tft.drawLine(169, 43, 171, 43,0x0);
  tft.drawLine(175, 43, 185, 43,0x0);
  tft.drawLine(192, 43, 200, 43,0x0);
  tft.drawLine(209, 43, 211, 43,0x0);
  tft.drawLine(214, 43, 216, 43,0x0);
  tft.drawLine(219, 43, 220, 43,0x0);
  tft.drawLine(223, 43, 226, 43,0x0);
  tft.drawLine(235, 43, 239, 43,0xfb20);
  tft.drawLine(90, 44, 98, 44,0xfb20);
  tft.drawLine(106, 44, 115, 44,0x0);
  tft.drawLine(123, 44, 132, 44,0x0);
  tft.drawLine(140, 44, 142, 44,0x0);
  tft.drawLine(155, 44, 162, 44,0x0);
  tft.drawLine(169, 44, 171, 44,0x0);
  tft.drawLine(175, 44, 183, 44,0x0);
  tft.drawLine(192, 44, 200, 44,0x0);
  tft.drawLine(210, 44, 211, 44,0x0);
  tft.drawLine(214, 44, 216, 44,0x0);
  tft.drawLine(220, 44, 221, 44,0x0);
  tft.drawLine(224, 44, 226, 44,0x0);
  tft.drawLine(235, 44, 239, 44,0xfb20);
  tft.drawLine(90, 45, 98, 45,0xfb20);
  tft.drawLine(107, 45, 113, 45,0x0);
  tft.drawLine(126, 45, 130, 45,0x0);
  tft.drawLine(157, 45, 160, 45,0x0);
  tft.drawLine(210, 45, 212, 45,0x0);
  tft.drawLine(213, 45, 216, 45,0x0);
  tft.drawLine(221, 45, 226, 45,0x0);
  tft.drawLine(235, 45, 239, 45,0xfb20);
  tft.drawLine(90, 46, 98, 46,0xfb20);
  tft.drawLine(211, 46, 216, 46,0x0);
  tft.drawLine(222, 46, 226, 46,0x0);
  tft.drawLine(235, 46, 239, 46,0xfb20);
  tft.drawLine(90, 47, 98, 47,0xfb20);
  tft.drawLine(212, 47, 216, 47,0x0);
  tft.drawLine(223, 47, 225, 47,0x0);
  tft.drawLine(235, 47, 239, 47,0xfb20);
  tft.drawLine(90, 48, 98, 48,0xfb20);
  tft.drawLine(213, 48, 216, 48,0x0);
  tft.drawLine(235, 48, 239, 48,0xfb20);
  tft.drawLine(90, 49, 98, 49,0xfb20);
  tft.drawLine(235, 49, 239, 49,0xfb20);
  tft.drawLine(92, 50, 98, 50,0xfb20);
  tft.drawLine(234, 50, 239, 50,0xfb20);
  tft.drawLine(92, 51, 98, 51,0xfb20);
  tft.drawLine(235, 51, 239, 51,0xfb20);
  tft.drawLine(92, 52, 99, 52,0xfb20);
  tft.drawLine(234, 52, 239, 52,0xfb20);
  tft.drawLine(93, 53, 100, 53,0xfb20);
  tft.drawLine(233, 53, 239, 53,0xfb20);
  tft.drawLine(93, 54, 101, 54,0xfb20);
  tft.drawLine(232, 54, 238, 54,0xfb20);
  tft.drawLine(94, 55, 102, 55,0xfb20);
  tft.drawLine(231, 55, 238, 55,0xfb20);
  tft.drawLine(94, 56, 104, 56,0xfb20);
  tft.drawLine(228, 56, 237, 56,0xfb20);
  tft.drawLine(95, 57, 236, 57,0xfb20);
  tft.drawLine(96, 58, 235, 58,0xfb20);
  tft.drawLine(97, 59, 234, 59,0xfb20);
  tft.drawLine(98, 60, 233, 60,0xfb20);
  tft.drawLine(100, 61, 231, 61,0xfb20);
  tft.setCursor(30,185);
  tft.setTextSize(2);
  tft.setTextColor(ILI9340_BLACK);
  tft.print("Temp:");

  tft.setTextSize(3);

  tft.drawCircle(53,216,3,0xC618);
  tft.setCursor(59,213);
  tft.setTextColor(0xC618);
  tft.print("C");

  tft.setCursor(130, 213);
  tft.setTextColor(0xC618);
  tft.print("lux");

  tft.setTextColor(0xC618);
  tft.setCursor(260, 213);
  tft.print("min");

  return 0;
}


int screen_updateStatus(byte processStatus){
  Serial.print("Update status on screen.\n");
  tft.fillRect(0,80,tft.width(),105,ILI9340_WHITE);
  tft.setTextSize(3);
  tft.setTextColor(ILI9340_BLACK);
  
  switch(processStatus){
    case 1:{
      tft.setCursor(60,80);
      tft.print("Press button");
      tft.setCursor(100,120);
      tft.print("to start");
      break;}
    case 2:{
      tft.setCursor(80,140);
      tft.print("Heating Up");
      break;}
    case 3:{
      tft.setCursor(45,140);
      tft.print("Processing...");
      break;}
  }
}


int screen_updateTemp(){
  Serial.print("Update current temperature on screen.\n");
  tft.setTextColor(ILI9340_BLACK,ILI9340_WHITE);
  tft.setTextSize(2);
  tft.setCursor(100,185);
  tft.print(tempChamber);
  tft.print(" C -");
  tft.drawCircle(130,187,2,ILI9340_BLACK);
}

int screen_updateHeatingStatus(){
  Serial.print("Update heating status on screen.\n");
  tft.setCursor(180,185);
  tft.setTextSize(2);

  if (tempStatus == 1){
  tft.setTextColor(ILI9340_RED,ILI9340_WHITE);
  tft.print("HEATING  ");
  }
  else if (tempStatus == -1){
  tft.setTextColor(ILI9340_BLUE,ILI9340_WHITE);
  tft.print("COOL DOWN");
  }
  else{
  tft.setTextColor(ILI9340_BLACK,ILI9340_WHITE);
  tft.print("TEMP OK  ");
  }
}

int screen_updateInput() {
  Serial.print("Update input values on screen.\n");

  tft.setTextColor(ILI9340_WHITE,0x7BEF);
  tft.setTextSize(3);


  tft.setCursor(10, 213);
  tft.print(temp,0);
  
  tft.setCursor(113, 213);
  tft.print(light,0);



  tft.setCursor(204, 213);

  int offset = 0;
  
  if (timer < 9) {
    tft.print("  ");
    (offset = 36);
  }

  else if (timer < 99) {
    tft.print(" ");
    (offset = 18);
  }
  
  tft.setCursor(204 + offset , 213);
  tft.print(timer,0);
  
}

int screen_updateTimer(int Min,int Sec){
//  tft.fillRect(0,75,tft.width(),40,ILI9340_WHITE);
  
  tft.setTextSize(5);
  tft.setTextColor(ILI9340_BLACK,ILI9340_WHITE);

  String timer_str = "";
  int offset = 10;

  tft.setCursor(70+offset,80);
  
  if (Min < 100) {
    timer_str.concat(" ");
    tft.setCursor(55+offset,80);
  }
  
  if (Min < 10) {
    timer_str.concat(" ");
    tft.setCursor(40+offset,80);
  }

  timer_str.concat(Min);

  timer_str.concat(":");

  if (Sec < 10) timer_str.concat("0");

  timer_str.concat(Sec);

  timer_str.concat("  ");

  tft.print(timer_str);
  
}



// Test functions
int testRelay(){
  //testRelay
    Serial.println("Testing Relay");
  
    digitalWrite(pin_heating,HIGH);
    digitalWrite(pin_led1,HIGH);
    digitalWrite(pin_led2,HIGH);
    digitalWrite(pin_led3,HIGH);
    delay(500);
    digitalWrite(pin_led1,LOW);
    delay(500);
    digitalWrite(pin_led2,LOW);
    delay(500);
    digitalWrite(pin_led3,LOW);
    delay(500);
    digitalWrite(pin_heating,LOW);
    delay(500);
}


// Control functions
int ctrl_adjustTemp(){

    int oldStatus= tempStatus;

    if (tempChamber > (temp+2)){
      digitalWrite(pin_heating,LOW); //Heater off
      digitalWrite(pin_fanHeating,HIGH);    //Fan on
      tempStatus=-1;
      Serial.println("Chamber Temperature to high, turn off heater.");
      }

    else if (tempChamber < temp){
      digitalWrite(pin_heating,HIGH); //Heater on
      digitalWrite(pin_fanHeating,HIGH);    //Fan on
      tempStatus=1;
      Serial.println("Chamber Temperature to low, turn on fan and heater.");
      }

    else{
      digitalWrite(pin_heating,LOW); //Heater off
      tempStatus=0;
      Serial.println("Chamber Temperature is ok.");
    }


    if (tempElectronics > 20){
      digitalWrite(pin_fanBoard,HIGH); //Fan on
      Serial.println("Electronics temperature to high, turn on fan.");
    }

    else{
      digitalWrite(pin_fanBoard,LOW);
      Serial.println("Electronics temperature is ok, turn fan off.");
    }

    if (oldStatus != tempStatus) {
      screen_updateHeatingStatus();
    }
}

int ctrl_setLight(int leds){
  switch (leds){
    case 0:{
    digitalWrite(pin_led1,LOW);
    digitalWrite(pin_led2,LOW);
    digitalWrite(pin_led3,LOW);
    break;
    }
    case 1:{
    digitalWrite(pin_led1,HIGH);
    digitalWrite(pin_led2,LOW);
    digitalWrite(pin_led3,LOW);
    break;
    }
    case 2:{
    digitalWrite(pin_led1,LOW);
    digitalWrite(pin_led2,HIGH);
    digitalWrite(pin_led3,LOW);
    break;
    }
    case 3:{
    digitalWrite(pin_led1,LOW);
    digitalWrite(pin_led2,LOW);
    digitalWrite(pin_led3,HIGH);
    break;
    }
    case 4:{
    digitalWrite(pin_led1,HIGH);
    digitalWrite(pin_led2,HIGH);
    digitalWrite(pin_led3,LOW);
    break;
    }
    case 5:{
    digitalWrite(pin_led1,HIGH);
    digitalWrite(pin_led2,LOW);
    digitalWrite(pin_led3,HIGH);
    break;
    }
    case 6:{
    digitalWrite(pin_led1,LOW);
    digitalWrite(pin_led2,HIGH);
    digitalWrite(pin_led3,HIGH);
    break;
    }
    case 7:{
    digitalWrite(pin_led1,HIGH);
    digitalWrite(pin_led2,HIGH);
    digitalWrite(pin_led3,HIGH);
    break;
    }
  }
}

int ctrl_finished(){
    digitalWrite(pin_led1,LOW);
    digitalWrite(pin_led2,LOW);
    digitalWrite(pin_led3,LOW);
    //chamberLed(1);
}


void setup() {
  
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Starting up...");

  // Setting Pin Mode
  byte pins_out[] = {pin_fanBoard, pin_fanHeating, pin_led1, pin_led2, pin_led3, pin_heating,csUI,csTFT};
  byte pins_in[] = {pin_temp1, pin_temp2, pin_externalSensor};

  Serial.println("Setting pins...");
  for (int pin = 0; pin < (sizeof(pins_out)/sizeof(byte)); pin++) {
    pinMode(pins_out[pin], OUTPUT);
    digitalWrite(pins_out[pin],LOW);
  }
  
 for (int pin = 0; pin < (sizeof(pins_in)/sizeof(byte)); pin++) {
    pinMode(pins_in[pin], INPUT);
  }

  Serial.println("Start SPI for UI and TFT...");
 
  tft.begin();
  
  SPI.begin(); // This is important or it will crush
  
  SPI.setClockDivider(SPI_CLOCK_DIV8);

  
  //Set TFT options
  Serial.println("Set TFT options...");
  tft.setRotation(1);
  screen_default();
  //READ UI
  readUI();

  //TEST RELAY
  testRelay();
  
  //Checking for external connected devices
  //device_code = analogRead(external_device);
  deviceCode = 0;
  if (deviceCode > 0){
    sensor_t sensor;
    tsl.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
    Serial.println("------------------------------------");
    Serial.println("");
    
    /* You can also manually set the gain or enable auto-gain support */
    // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
    // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
    tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
    /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  
    /* Update these values depending on what you've set above! */  
    Serial.println("------------------------------------");
    Serial.print  ("Gain:         "); Serial.println("Auto");
    Serial.print  ("Timing:       "); Serial.println("13 ms");
    Serial.println("------------------------------------");

    if(!tsl.begin())
    {
      /* There was a problem detecting the TSL2561 ... check your connections */
      Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
      while(1);
    }
  }
}


void loop() {
  Serial.print("Starting main program.\n");
  while (true){

    screen_updateStatus(1);

    screen_updateInput();

    screen_updateHeatingStatus();
    
    button = 0;
    
    while (readADC(channel_button) < button_threshold){
      
      oldValues = (temp+timer*(light+1)); //Hash old values

      readUI();

      if ((temp+timer*(light+1)) !=  oldValues){
        
        screen_updateInput();
      
      }
      
      readTemp();
      
      ctrl_adjustTemp();
    }
    
    screen_updateStatus(2);
    
    screen_updateTimer(timer,0);

    button=0;

    delay(1000);
    
    while ((tempStatus !=0) && (readADC(channel_button) < button_threshold)){
      Serial.println("Waiting until heated to preset temperatur");
      readTemp();
      ctrl_adjustTemp();
    }

    screen_updateStatus(3);
    unsigned long endTime = (millis()+(((timer*60)*1000)));
    while ((millis()<endTime) && (readADC(channel_button) < button_threshold)){
      Serial.println("Starting processing...");
      ctrl_setLight(light);
      int timeLeftSec = ((endTime-(millis()))/1000);
      int timeLeftMin = int(timeLeftSec/60);
      timeLeftSec = timeLeftSec%60;
      screen_updateTimer(timeLeftMin,timeLeftSec);
      delay(800);
      readTemp();
      ctrl_adjustTemp();  
    }
    ctrl_setLight(0);
    if (button==0){
      ctrl_finished();
    }
  }
}
