#ifndef __SENSORS_HPP__
#define __SENSORS_HPP__

// Pins
#define PUMP_PIN 2
#define TEMP_PIN A0
const uint8_t soil_pins[SOIL_SENSORS_COUNT] = { A1, A2, A3, A4, A5 };

// Checks if sensor values changed
bool ValuesChanged()
{
  bool changed = false;
  
  avg_moisture = 0;

  uint8_t read;
  for(uint8_t i=0; i<SOIL_SENSORS_COUNT; i++)
  {
    read = map(analogRead(soil_pins[i]), 0, 1023, 100, 0);
    if(moistures[i] != read)
    {
      changed = true;
      moistures[i] = read;
    }
    avg_moisture += read;
  }
 
  avg_moisture /= SOIL_SENSORS_COUNT;

  read = map(analogRead(TEMP_PIN), 0, 1023, 15, 150);
  if(temperature != read)
  {
    changed = true;
    temperature = read;
  }

  return changed;
}

// Turns on pump if needed
void WaterPlants()
{
  if(millis() - timer <= water_time * 1000 && pump_on)
  {
    Serial.println("Skipped check");
  }
  
  if(pump_on)
  {
    pump_on = false;
    digitalWrite(PUMP_PIN, LOW);
  }
  
  if(!ValuesChanged()) return;

  if(avg_moisture <= min_moisture)
  {
    pump_on = true;
    digitalWrite(PUMP_PIN, HIGH);
    timer = millis();
  } 
}

// Initializes sensors
void InitSensors()
{
  pinMode(PUMP_PIN, OUTPUT);

  WaterPlants();
}

#endif
