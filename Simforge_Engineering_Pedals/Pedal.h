
/*
  Pedal.h - Pedal class
  Created by Morgan Gardner, November 26th, 2020.
  Released under the GPL-3.0 license.
*/

#ifndef Pedal_h
#define Pedal_h

#include "Arduino.h"
#include <Smoothed.h>
#include <EEPROM.h>

class Pedal
{

  public:
    //initialise pedal
    Pedal(byte pedalIndex)
    {
      this->pedalIndex = pedalIndex;
      loadEEPROM();
      enableFilter();
    }

    void setDeadzone(uint16_t dzValue, bool dzUpper)
    {
      if (dzUpper)
      {
        calibration.maxDeadzone = dzValue;
        saveEEPROM();
      }
      else
      {
        calibration.minDeadzone = dzValue;
        saveEEPROM();
      }
    }

    void setRange(uint16_t rValue, bool rUpper)
    {
      if (rUpper)
      {
        calibration.maxRange = rValue;
        saveEEPROM();
      }
      else
      {
        calibration.minRange = rValue;
        saveEEPROM();
      }
    }

    void setFilter(uint16_t fValue)
    {
      calibration.smoothingValue = fValue;
      saveEEPROM();
      enableFilter();
    }

    uint16_t getValue()
    {
      return this->value;
    }

    uint16_t updatePedal(Adafruit_ADS1115 ads)
    {
      uint16_t pedalRaw = ads.readADC_SingleEnded(pedalIndex);
      if (calibration.smoothingValue != 0)
      {
        pedalFilter.add(pedalRaw);
        pedalRaw = pedalFilter.get();
      }
      uint16_t pedalOutput;

      if (pedalRaw > calibration.maxRange - calibration.maxDeadzone)
        pedalOutput = calibration.maxRange - calibration.maxDeadzone;

      else if (pedalRaw < calibration.minRange + calibration.minDeadzone)
        pedalOutput = calibration.minRange + calibration.minDeadzone;

      else
        pedalOutput = pedalRaw;


      uint16_t pedalMapped = map(pedalOutput, calibration.minRange + calibration.minDeadzone, calibration.maxRange - calibration.maxDeadzone, 0, 32767);

      this->value = pedalMapped;
      return pedalMapped;
    }

    String getEEPROM()
    {
      String output = "";
      output += (String)calibration.minDeadzone;
      output += ";" + (String)calibration.maxDeadzone;
      output += ";" + (String)calibration.minRange;
      output += ";" + (String)calibration.maxRange;
      output += ";" + (String)calibration.smoothingValue;
      return output;
    }

  private:

    struct CalibrationValues
    {
      uint16_t minDeadzone;
      uint16_t maxDeadzone;
      uint16_t minRange;
      uint16_t maxRange;
      uint16_t smoothingValue;
    };

    CalibrationValues calibration;

    void enableFilter()
    {
        if (calibration.smoothingValue != 0)
        pedalFilter.begin(SMOOTHED_EXPONENTIAL, calibration.smoothingValue);
    }

    void loadEEPROM()
    {
      int eepromLocation = pedalIndex * sizeof(CalibrationValues);
      EEPROM.get(eepromLocation, calibration);
    }

    void saveEEPROM()
    {
      int eepromLocation = pedalIndex * sizeof(CalibrationValues);
      EEPROM.put(eepromLocation, calibration);
    }

    byte pedalIndex;
    uint16_t value;

    Smoothed <uint16_t> pedalFilter;

};

#endif
