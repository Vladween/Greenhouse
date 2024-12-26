#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

void Log(String& resp)
{
  resp += "OK: {\n\t\"temperature\": " + String(temperature)
       + ",\n\t\"moistures\": [ ";
  for(uint8_t i=0; i<SOIL_SENSORS_COUNT; i++)
  {
    resp += moistures[i];
    if(i + 1 != SOIL_SENSORS_COUNT)
      resp += ", ";
  }
  resp += " ]";
  resp += ",\n\t\"avg_moisture\": " + String(avg_moisture)
       + ",\n\t\"pump_on\": " + (pump_on ? String("true") : String("false"))
       + ",\n\t\"time_left\": " + String((pump_on ? -(timer - millis()) / 1000 : 0))
       
     + ",\n\n\t\"min_moisture\": " + String(min_moisture) 
       + ",\n\t\"water_time\": " + String(water_time)
       + "\n}\n";
}

#define SUCCESS 0
#define UNKNOWN_VARIABLE 1
#define INVALID_VALUE 2

int Set(String& resp, const String& param, const String& value)
{
  unsigned int v;
  if(!isInt(value, v)) return INVALID_VALUE;
  
  if(param == "min_moisture")
  {
    if(v >= 100) return INVALID_VALUE;

    min_moisture = v;
  }
  else if(param == "water_time")
  {
    water_time = v;
  }
  else
  {
    return UNKNOWN_VARIABLE;
  }

  return SUCCESS;
}

String ProcessCommand(uint8_t argc, String argv[])
{
  if(argc == 0) return "";

  String resp = "";

  if(argc == 1 && argv[0] == "log")
  {
    Log(resp);
  }
  else if(argc == 3 && argv[0] == "set")
  {
    int res = Set(resp, argv[1], argv[2]);
    
    if(res == UNKNOWN_VARIABLE)
    {
      resp = "ERROR: Unknown variable!\n";
    }
    else if (res == INVALID_VALUE)
    {
      resp = "ERROR: Invalid value!\n";
    }
    else
    {
      resp = "OK\n";
    }
  }
  else
  {
    resp = "ERROR: Unknown command!\n";
  }

  return resp;
}


void ProcessCommands()
{
  if(Serial.available())
  {
    String request = Serial.readString();

    String* argv = 0;
    uint8_t argc = 0;

    String temp = "";
    for(uint8_t i=0; i<request.length(); i++)
    {
      if(request[i] == '\n' || request[i] == '\r') continue;

      if(request[i] == ' ' && temp != "")
      {
        pushBack(temp, argv, argc);
        temp = "";
        continue;
      }

      temp += request[i];
    }
    if(temp != "") pushBack(temp, argv, argc);

    Serial.print(ProcessCommand(argc, argv));

    if(argc != 0) delete[] argv;
  }
}

#endif
