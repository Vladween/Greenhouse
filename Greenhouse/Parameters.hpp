#ifndef __PARAMETERS_HPP__
#define __PARAMETERS_HPP__

// Number of soil sensors
#define SOIL_SENSORS_COUNT 5

// Is pump on?
bool pump_on = false;

// Temperature, Minimal valid moisture
uint8_t temperature = 0, min_moisture = 50;

// Soil moistures
uint8_t moistures[SOIL_SENSORS_COUNT];

// Average moisture
uint16_t avg_moisture = 0;

// Watering time, timer
uint32_t water_time = 300, timer = millis();


#endif
