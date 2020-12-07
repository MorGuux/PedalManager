#include <Joystick.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "Pedal.h"

Joystick_ Joystick;
Adafruit_ADS1115 ads;

String firmwareVersion = "1.0.2";

///<!---WARNING---!>///
/* Please be careful with these settings!
 * SAMPLE_GAIN set incorrectly can destroy your ADS module!
 * Please ensure you have read the documentation for the
 * ADS1115 module and understand the risks involved.
 * GAIN_TWOTHIRDS supports +/- 6.144V (this is the safest choice!)
 * If your input voltage exceeds this range, it can destroy the module.
 * 
 * +/-6.144V range = GAIN_TWOTHIRDS
 * +/-4.096V range = GAIN_ONE
 * +/-2.048V range = GAIN_TWO
 * +/-1.024V range = GAIN_FOUR
 * +/-0.512V range = GAIN_EIGHT
 * +/-0.256V range = GAIN_SIXTEEN
 */
adsGain_t SAMPLE_GAIN = GAIN_TWOTHIRDS;

//Sample rate of the ADS module, set at 860 per second
adsSPS_t SAMPLE_RATE = ADS1115_DR_860SPS;


Pedal pedals[] = {Pedal(0), Pedal(1), Pedal(2)};

bool serialOpen = false;

unsigned long ldPreviousMillis = 0;
const long ldInterval = 16;           //live data refresh rate

void setup()
{

  //Initialize Joystick and ADS module
  Joystick.begin();
  ads.begin();
  ads.setSPS(SAMPLE_RATE);
  ads.setGain(SAMPLE_GAIN);

  Serial.begin(250000);

  Joystick.setThrottleRange(0, 32767);
  Joystick.setRxAxisRange(0, 32767);
  Joystick.setRyAxisRange(0, 32767);

}

void loop()
{

  unsigned long currentMillis = millis();

  Joystick.setThrottle(pedals[0].updatePedal(ads));
  Joystick.setRxAxis(pedals[1].updatePedal(ads));
  Joystick.setRyAxis(pedals[2].updatePedal(ads));

  if (Serial.available() > 0)
  {
    String cmdMain = Serial.readStringUntil("\n");   //cmd

    switch (cmdMain.charAt(0))
    {
      case 'i':       //initialisation

        switch (cmdMain.charAt(2))
        {
          case 'v':  //version request
            Serial.println("i;v;" + firmwareVersion);
            break;

          case 'c':  //company name request
            Serial.println("i;c;Simforge Engineering");
            break;

          case 'o':  //toggle serial connection status
            serialOpen = !serialOpen;
            break;
        }
        break;

      case 'c':       //calibration

        byte pedalIndex = atoi(cmdMain.charAt(2));
        uint16_t data = cmdMain.substring(cmdMain.length() - 6, cmdMain.length() - 1).toInt();
        switch (cmdMain.charAt(4))
        {
          case 'a':   //upper deadzone
            pedals[pedalIndex].setDeadzone(data, true);   //update upper deadzone
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";a;saved");
            break;

          case 'b':   //lower deadzone
            pedals[pedalIndex].setDeadzone(data, false);   //update lower deadzone
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";b;saved");
            break;

          case 'c':   //upper range
            pedals[pedalIndex].setRange(data, true);   //update upper range
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";c;saved");
            break;

          case 'd':   //lower range
            pedals[pedalIndex].setRange(data, false);   //update lower range
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";d;saved");
            break;

          case 'f':   //filter
            pedals[pedalIndex].setFilter(data);   //update filter
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";f;saved");
            break;
          case 'r':   //requesting values
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";r;" + pedals[pedalIndex].getEEPROM());
            break;
          case 'z':   //reset calibration values
            pedals[pedalIndex].setDeadzone(0, true);
            pedals[pedalIndex].setDeadzone(0, false);
            pedals[pedalIndex].setRange(0, false);
            pedals[pedalIndex].setRange(32767, true);
            pedals[pedalIndex].setFilter(0);
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";z;reset");
            break;
        }
        break;
    }
  }


  if (serialOpen)
  {
    if (currentMillis - ldPreviousMillis >= ldInterval)
    {
      ldPreviousMillis = currentMillis;
      Serial.println("l;" + (String)pedals[0].getValue() + ";" + (String)pedals[1].getValue() + ";" + (String)pedals[2].getValue()); //live data
    }
  }
}
