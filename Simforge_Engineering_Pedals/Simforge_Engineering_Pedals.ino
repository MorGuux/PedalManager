#include <Joystick.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "Pedal.h"

Joystick_ Joystick;
Adafruit_ADS1115 ads;

Pedal pedals[] = {Pedal(0), Pedal(1), Pedal(2)};

bool serialOpen = false;

String firmwareVersion = "1.0.0";

void setup()
{

  //Initialize Joystick and ADS module
  Joystick.begin();
  ads.begin();

  Serial.begin(115200);

  ads.setGain(GAIN_TWOTHIRDS);

  Joystick.setThrottleRange(0, 32767);
  Joystick.setRxAxisRange(0, 32767);
  Joystick.setRyAxisRange(0, 32767);

}

void loop()
{

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
            Serial.println("c;" + (String)cmdMain.charAt(2) + ";r;" + pedals[pedalIndex].getValues());
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
    Serial.println("l;" + (String)pedals[0].getValue() + ";" + (String)pedals[1].getValue() + ";" + (String)pedals[2].getValue()); //live data
}
