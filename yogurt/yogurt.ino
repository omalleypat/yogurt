// This Arduino sketch reads DS18B20 "1-Wire" digital
// temperature sensors.
// Tutorial:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 12

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Therm = { 0x28, 0xF4, 0xCE, 0xF4, 0x03, 0x00, 0x00, 0xAB };
int thermPower = 13;
int relayPin = 9;
int yellowPin = 10;
int speakerPin = 8;
int switchPin = 11;
float T_MAX = 180;
float T_WAIT = 120;
float T_HOLD = 100;
int stage = 1;
int i = 0;
int sdelay = 2000;
int switchState = 0;
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  // Turn on the power for the thermometer
  pinMode(thermPower,OUTPUT);
  digitalWrite(thermPower,HIGH); // this will stay high
  delay(20);
  // Start up the library
  sensors.begin(); // make sure power is on first or first reading will be 185
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(Therm, 10);
  pinMode(relayPin,OUTPUT);
  pinMode(yellowPin,OUTPUT);
  digitalWrite(yellowPin,LOW);
  pinMode(switchPin,INPUT);  
}

void loop(void)
{ 
  // Read the switch
  switchState = digitalRead(switchPin);
  // Read the temperature
  sensors.requestTemperatures();
  float tempF = sensors.getTempF(Therm);
  if (tempF == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Serial.print(millis());
    Serial.print(",");
    Serial.print(tempF);
  }  
  
  // Decide how to proceed
  switch (stage) {
    case 1:  // HEATING TO T_MAX
      crock_on();
      if (tempF > T_MAX){
       stage=2;
       tone(speakerPin, 1000, 1000);
      }  
      break;
    case 2: // COOLING TO T_WAIT
       crock_off();
       if (tempF < T_WAIT) {
         stage=3;
         digitalWrite(yellowPin,HIGH);
         i = 0;
       }
      break;
    case 3: //HOLD AT T_WAIT UNTIL USER FLIPS SWITCH
      if (switchState==LOW) {
        stage=4;
        crock_off(); // this is just to complete the serial write
        digitalWrite(yellowPin,LOW);
      }
      else {
        if (tempF > T_WAIT){
          crock_off();      
        }
        else {
          crock_on();
        }
        if ((i % 10) == 0){
            tone(speakerPin, 1000, 1000);
        }
        i = i+1;
      }
      break;
   case 4: // MAINTAIN T_HOLD forever
     if (tempF > T_HOLD){
       crock_off();      
     }
     else {
       crock_on();
     }
   break;
   }

   delay(sdelay);
}

void crock_on(void)
{
  digitalWrite(relayPin,HIGH);  
  Serial.print(",1\n");
}
  
void crock_off(void)
{
  digitalWrite(relayPin,LOW);  
  Serial.print(",0\n");
}
