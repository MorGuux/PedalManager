
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
#include "HX711.h"
#include <MultiMap.h>

enum PedalType
{
  Pot,
  ADS,
  LoadCell
};

class Pedal
{

  public:
    //initialise pedal
    Pedal(byte pedalIndex, PedalType type)
    {
      this->pedalType = type;
      this->pedalIndex = pedalIndex;
      loadEEPROM();
      enableFilter();
    }

    void setDeadzone(uint16_t dzValue, bool dzUpper)
    {
      if (dzUpper)
      {
        calibration.maxDeadzone = dzValue;
      }
      else
      {
        calibration.minDeadzone = dzValue;
      }
    }

    void setRange(uint16_t rValue, bool rUpper)
    {
      if (rUpper)
      {
        calibration.maxRange = rValue;
      }
      else
      {
        calibration.minRange = rValue;
      }
    }

    void setFilter(uint16_t fValue)
    {
      calibration.smoothingValue = fValue;
      enableFilter();
    }

    void setMapping(float values[])
    {
      calibration.smoothingValue = values;
    }

    uint16_t getValue()
    {
      return this->value;
    }

    uint16_t getRawValue()
    {
      return this->rawValue;
    }

    uint16_t readPedalValue(byte analogInput)
    {
      rawValue = map(analogRead(analogInput), 0, 1023, 0, 32767);
      updatePedal(rawValue);
    }

    uint16_t readPedalValue(Adafruit_ADS1115 ads)
    {
      rawValue = ads.readADC_SingleEnded(pedalIndex);
      updatePedal(rawValue);
    }

    uint16_t readPedalValue(HX711 loadcell)
    {
      updatePedal(loadcell.read());
    }

    uint16_t updatePedal(uint16_t pedalRaw)
    {
      //filter pedal value if applicable
      if (calibration.smoothingValue != 0)
      {
        pedalFilter.add(pedalRaw);
        pedalRaw = pedalFilter.get();
      }
      
      uint16_t pedalOutput;

      //constrain pedal max
      if (pedalRaw > calibration.maxRange - calibration.maxDeadzone)
        pedalOutput = calibration.maxRange - calibration.maxDeadzone;
      //constrain pedal min
      else if (pedalRaw < calibration.minRange + calibration.minDeadzone)
        pedalOutput = calibration.minRange + calibration.minDeadzone;
        
      else
        pedalOutput = pedalRaw;

      float pedalMappedRaw = mapf(pedalOutput, calibration.minRange + calibration.minDeadzone, calibration.maxRange - calibration.maxDeadzone, 0, 100);  //map to percentage

      float inputMap[6] = {0, 20, 40, 60, 80, 100}; //input map (percentage values)

      uint16_t pedalMapped = mapf(multiMap<float>(pedalMappedRaw, inputMap, calibration.outputMapping, 6), 0, 100, 0, 32767);  //add mapping function, and convert to 15bit

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

    void saveEEPROM()
    {
      int eepromLocation = pedalIndex * sizeof(CalibrationValues);
      EEPROM.put(eepromLocation, calibration);
    }

  private:

    struct CalibrationValues
    {
      uint16_t minDeadzone;
      uint16_t maxDeadzone;
      uint16_t minRange;
      uint16_t maxRange;
      uint16_t smoothingValue;
      float outputMapping[6]; //percentage (0% to 100%)
    };

    CalibrationValues calibration;

    void enableFilter()
    {
      if (calibration.smoothingValue != 0)
        pedalFilter.begin(SMOOTHED_EXPONENTIAL, calibration.smoothingValue);
    }

    float mapf(float x, float in_min, float in_max, float out_min, float out_max)
    {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void loadEEPROM()
    {
      int eepromLocation = pedalIndex * sizeof(CalibrationValues);
      EEPROM.get(eepromLocation, calibration);
    }

    byte pedalIndex;
    uint16_t value;
    uint16_t rawValue;

    Smoothed <uint16_t> pedalFilter;

    PedalType pedalType;
};

#endif
