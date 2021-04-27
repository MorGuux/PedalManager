#include <Joystick.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "Pedal.h"

Joystick_ Joystick;
Adafruit_ADS1115 ads;

String firmwareVersion = "1.0.3";

///<!---WARNING---!>///
/* Please be careful with these settings!
   SAMPLE_GAIN set incorrectly can destroy your ADS module!
   Please ensure you have read the documentation for the
   ADS1115 module and understand the risks involved.
   GAIN_TWOTHIRDS supports +/- 6.144V (this is the safest choice!)
   If your input voltage exceeds this range, it can destroy the module.

   +/-6.144V range = GAIN_TWOTHIRDS
   +/-4.096V range = GAIN_ONE
   +/-2.048V range = GAIN_TWO
   +/-1.024V range = GAIN_FOUR
   +/-0.512V range = GAIN_EIGHT
   +/-0.256V range = GAIN_SIXTEEN
*/
adsGain_t SAMPLE_GAIN = GAIN_TWOTHIRDS;

//Sample rate of the ADS module, set at 860 per second
adsSPS_t SAMPLE_RATE = ADS1115_DR_860SPS;

byte pedalCount = 3;
Pedal pedals[3] = {Pedal(0), Pedal(1), Pedal(2)};

//const int LOADCELL_DOUT_PIN = 2;
//const int LOADCELL_SCK_PIN = 3;

bool serialOpen = false;

unsigned long ldPreviousMillis = 0;
const long ldInterval = 16;           //live data refresh rate

//HX711 loadCell;

void setup()
{
  //Initialize Joystick and ADS module
  Joystick.begin();
  ads.begin();
  ads.setSPS(SAMPLE_RATE);
  ads.setGain(SAMPLE_GAIN);

  //loadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.begin(500000);

  Joystick.setThrottleRange(0, 32767);
  Joystick.setRxAxisRange(0, 32767);
  Joystick.setRyAxisRange(0, 32767);

}

void loop()
{

  /*
  //analogreads
  pedals[0].updatePedal(map(analogRead(0), 0, 1023, 0, 32767));
  pedals[1].updatePedal(map(analogRead(1), 0, 1023, 0, 32767));
  pedals[2].updatePedal(map(analogRead(2), 0, 1023, 0, 32767));
  */

  //ads1115
  pedals[0].updatePedal(ads.readADC_SingleEnded(0));
  pedals[1].updatePedal(ads.readADC_SingleEnded(1));
  pedals[2].updatePedal(ads.readADC_SingleEnded(2));

  /*
  //hx711
  pedals[0].updatePedal(loadcell.read());
  pedals[1].updatePedal(loadcell.read());
  pedals[2].updatePedal(loadcell.read());
  */
  

  unsigned long currentMillis = millis();

  Joystick.setThrottle(pedals[0].getValue());
  Joystick.setRxAxis(pedals[1].getValue());
  Joystick.setRyAxis(pedals[2].getValue());

  if (Serial.available() > 0)
  {
    String cmdMain = Serial.readStringUntil('\n');   //cmd

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
            serialOpen = cmdMain.charAt(4) == '1' ? true : false;
            break;
        }
        break;

      case 'c':       //calibration

        if (cmdMain.charAt(2) == 'r') //request all calibration settings from all pedals
        {
          String calibrationValues = "";
          calibrationValues += (String)pedals[0].getEEPROM();
          for (int i = 1; i < pedalCount; i++)
            calibrationValues += ":" + (String)pedals[i].getEEPROM();
          Serial.println(calibrationValues);
        }
        else if (cmdMain.charAt(2) == 's')  //save all calibration settings to EEPROM
        {
            for (int i = 0; i < pedalCount; i++)
            pedals[i].saveEEPROM();
            Serial.println("c;s;saved");
        }
        else
        {
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
              pedals[pedalIndex].setRange(65535, true);
              pedals[pedalIndex].setFilter(0);
              float defaultMap[] = {0, 20, 40, 60, 80, 100};
              pedals[pedalIndex].setMapping(defaultMap);
              Serial.println("c;" + (String)cmdMain.charAt(2) + ";z;reset");
              break;
          }
          break;
        }
    }
  }


  if (serialOpen)
  {
    if (currentMillis - ldPreviousMillis >= ldInterval)
    {
      ldPreviousMillis = currentMillis;
      String liveDataOutput = "l;";
      liveDataOutput += (String)pedals[0].getRawValue();
      for (int i = 1; i < pedalCount; i++)
        liveDataOutput += ";" + (String)pedals[i].getRawValue();
      Serial.println(liveDataOutput);
    }
  }
}
