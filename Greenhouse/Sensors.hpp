#ifndef __SENSORS_HPP__
#define __SENSORS_HPP__

// Pump pin
#define PUMP_PIN 2
// Pump value
bool pump_on = false;

// Temperature pin
#define TEMP_PIN A0
// Temperature value
uint8_t temp_value = 0;

// Number of moisture sensors
#define SOIL_SENSORS_COUNT 5
// Moisture sensor pins
uint8_t soil_pins[SOIL_SENSORS_COUNT] = { A1, A2, A3, A4, A5 };
// Moisture sensor values
uint8_t soil_values[SOIL_SENSORS_COUNT];
// Average moisture
uint16_t avg_soil = 0;

// Minimal moisture
uint8_t min_moisture = 50;

// Checks if sensor values changed
bool ValuesChanged()
{
  bool changed = false;
  
  avg_soil = 0;

  uint8_t read;
  for(uint8_t i=0; i<SOIL_SENSORS_COUNT; i++)
  {
    read = map(analogRead(soil_pins[i]), 0, 1023, 100, 0);
    if(soil_values[i] != read)
    {
      changed = true;
      soil_values[i] = read;
    }
    avg_soil += read;
  }

  avg_soil /= SOIL_SENSORS_COUNT;

  read = map(analogRead(TEMP_PIN), 0, 1023, 15, 150);
  if(temp_value != read)
  {
    changed = true;
    temp_value = read;
  }

  return changed;
}

// Turns on pump if needed
void WaterPlants()
{
  if(!ValuesChanged()) return;

  if(avg_soil <= min_moisture)
    pump_on = true;
  else
    pump_on = false;

  digitalWrite(PUMP_PIN, (pump_on) ? HIGH : LOW);
}

// Initializes sensors
void InitSensors()
{
  pinMode(PUMP_PIN, OUTPUT);

  WaterPlants();
}

#endif