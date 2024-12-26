#include "Utils.hpp"
#include "Parameters.hpp"
#include "Sensors.hpp"
#include "Commands.hpp"

void setup() 
{
  Serial.begin(9600);

  InitSensors();
}

void loop()
{
  WaterPlants();

  ProcessCommands();
}
