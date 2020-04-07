#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

//######################################################################

#define CLEAVAGE_PIN    12
#define CLEAVAGE_COUNT 2

#define UNDER_PIN    6
#define UNDER_COUNT 16

#define BELT_PIN  9
#define WING_PIN  10

Adafruit_LSM303 lsm;

int limit=100;

Adafruit_NeoPixel under = Adafruit_NeoPixel(UNDER_COUNT, UNDER_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel cleavage = Adafruit_NeoPixel(CLEAVAGE_COUNT, CLEAVAGE_PIN, NEO_RGB + NEO_KHZ800);


void setup() {      
    // Try to initialise and warn if we couldn't detect the chip
    if (!lsm.begin())
    {
      //Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
      while (1);
    }
  
    pinMode(BELT_PIN, OUTPUT);
    pinMode(WING_PIN, OUTPUT);
    
    analogWrite(BELT_PIN, 127);
    analogWrite(WING_PIN, 127);
    
    under.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    under.show();            // Turn OFF all pixels ASAP
    under.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

    cleavage.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    cleavage.show();            // Turn OFF all pixels ASAP
    cleavage.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)


    //###########################################

    ble.echo(false);

  ble.info();

 
  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }



  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    //Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
 
  ble.setMode(BLUEFRUIT_MODE_DATA);


}

void loop() {
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  /* Got a packet! */
  // printHex(packetbuffer, len);

  // Color
  if (packetbuffer[1] == 'C') {
    uint8_t red = packetbuffer[2];
    uint8_t green = packetbuffer[3];
    uint8_t blue = packetbuffer[4];
    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue, HEX);
  }
  
  lsm.read();
  int ax=(int)lsm.accelData.x;
  int ay=(int)lsm.accelData.y;
  int az=(int)lsm.accelData.z;
  amax=ax;
  if(ay>amax){
    amax=ay;
  }
  if(az>amax){
    amax=az;
  }
  if(amax==az){
    //nookie alert
  }

  if(amax>limit){
    lumin=255;
    under.setBrightness(100);
    cleavage.setBrightness(100);
  }
  else{
    lumin=127;
    under.setBrightness(50);
    cleavage.setBrightness(50);
  }

  analogWrite(BELT_PIN, lumin);
  analogWrite(WING_PIN, lumin);
  cleavageSet(cleavage.Color(red,green,blue);
  underSet(cleavage.Color(red,green,blue);
  
  //theaterChase(under.Color(lumin, 0, 0), 50); 
  delay(1000);
}

void cleavageSet(uint32_t color){
  for(int i=0;i<cleavage.numPixels();i++){
    cleavage.setPixelColor(i,color);
  }
}

void underSet(uint32_t color){
  for(int i=0;i<under.numPixels();i++){
    under.setPixelColor(i,color);
  }
}

void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      under.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        under.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      under.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}
