/**
 * Board: Arduino Pro or Pro Mini
 */

#include <Wire.h>
#include <avr/pgmspace.h>
#include "src/DMXSerial.h"
#include "src/Adafruit_PWMServoDriver.h"

#define STATUS_LED        13
#define CONTROLPIN_RX_TX  4

#define DMX_ADDR_1        12
#define DMX_ADDR_2        11
#define DMX_ADDR_3        10
#define DMX_ADDR_4        9
#define DMX_ADDR_5        8
#define DMX_ADDR_6        7
#define DMX_ADDR_7        6
#define DMX_ADDR_8        5

#define USE_LOG           true

// Log Table: 8bit (DMX) -> 12bit (PWM)
const uint16_t pwmtable[] PROGMEM = 
     {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
      1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 
      2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 
      3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 
      4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 
      5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 
      7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 
      10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 
      14, 14, 15, 15, 16, 16, 17, 17, 18, 19, 
      19, 20, 21, 21, 22, 23, 23, 24, 25, 26, 
      27, 27, 28, 29, 30, 31, 32, 33, 35, 36, 
      37, 38, 39, 41, 42, 43, 45, 46, 48, 49, 
      51, 53, 54, 56, 58, 60, 62, 64, 66, 68, 
      71, 73, 75, 78, 80, 83, 86, 89, 91, 95, 
      98, 101, 104, 108, 111, 115, 119, 123, 127, 131, 
      135, 140, 144, 149, 154, 159, 164, 170, 175, 181, 
      187, 193, 200, 206, 213, 220, 227, 235, 242, 250, 
      259, 267, 276, 285, 295, 304, 314, 325, 336, 347, 
      358, 370, 382, 395, 408, 421, 435, 450, 464, 480, 
      496, 512, 529, 546, 564, 583, 602, 622, 643, 664, 
      686, 708, 732, 756, 781, 807, 833, 861, 889, 919, 
      949, 980, 1013, 1046, 1081, 1116, 1153, 1191, 1231, 1271, 
      1313, 1357, 1402, 1448, 1496, 1545, 1596, 1649, 1703, 1759, 
      1818, 1878, 1940, 2004, 2070, 2138, 2209, 2282, 2357, 2435, 
      2515, 2598, 2684, 2773, 2864, 2959, 3057, 3158, 3262, 3370, 
      3481, 3596, 3715, 3837, 3964, 4095, 4095};

// can be read out from dip-switches or preconfigured
uint16_t dmx_addr = 0;

// Maping "Channel of PWM Chip" => "Output Channel"
uint8_t MAPPING_CH_PWM[] = {0, 15, 1, 14, 2, 13, 3, 12, 4, 11, 5, 10, 6, 8, 7, 9};

// PWM output chip
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup () {

  // init DMX and PWM-Chip
  DMXSerial.init(DMXReceiver);
  Wire.setClock(100000);
  pwm.begin();
  pwm.setPWMFreq(200); // PWM Freq in Hz, up to 1600 Hz are possible, for LEDs 200Hz should be enough

  // init PINS
  pinMode(STATUS_LED,       OUTPUT);
  pinMode(CONTROLPIN_RX_TX, OUTPUT);
  pinMode(DMX_ADDR_1,       INPUT_PULLUP);
  pinMode(DMX_ADDR_2,       INPUT_PULLUP);
  pinMode(DMX_ADDR_3,       INPUT_PULLUP);
  pinMode(DMX_ADDR_4,       INPUT_PULLUP);
  pinMode(DMX_ADDR_5,       INPUT_PULLUP);
  pinMode(DMX_ADDR_6,       INPUT_PULLUP);
  pinMode(DMX_ADDR_7,       INPUT_PULLUP);
  pinMode(DMX_ADDR_8,       INPUT_PULLUP);

  digitalWrite(CONTROLPIN_RX_TX, LOW);
  digitalWrite(STATUS_LED, HIGH);
}


void loop() 
{
  // Calculate how long no data backet was received
  unsigned long lastPacket = DMXSerial.noDataSince();
  
  if (lastPacket < 5000) {
    digitalWrite(STATUS_LED, LOW);
    
    for(int i = 0; i < 16; i++)
    {
      uint8_t dmx_val = DMXSerial.read(dmx_addr + 1 + i);
      uint16_t output_val = 0;

      if(USE_LOG)
      {
        output_val = pgm_read_word_near(pwmtable + dmx_val);
      }
      else
      {
        output_val = (uint16_t)dmx_val * 16;
      }
      
      pwm.setPWM(MAPPING_CH_PWM[i], 0, output_val);
    }
  } else {
    // maybe DMX Bus is broken? should we just switch on some lights? should we do nothing?
    digitalWrite(STATUS_LED, HIGH);
  } 
}

