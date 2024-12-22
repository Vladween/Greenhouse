#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

void Log(String& resp)
{
  resp += "OK: {\n\t\"temperature\": " + String(temp_value)
       + ",\n\t\"moistures\": [ ";
  for(uint8_t i=0; i<SOIL_SENSORS_COUNT; i++)
  {
    resp += soil_values[i];
    if(i + 1 != SOIL_SENSORS_COUNT)
      resp += ", ";
  }
  resp += " ]";
  resp += ",\n\t\"avg_moisture\": " + String(avg_soil)
       + ",\n\t\"min_moisture\": " + String(min_moisture) 
       + ",\n\t\"pump_on\": " + (pump_on ? String("true") : String("false"))
  + "\n}\n"; 
}

bool SetMinMoisture(String& resp, const String& value)
{
  int v = value.toInt();

  if(v <= 0 || v > 100) return false;

  min_moisture = v;
  resp = "OK: " + String(min_moisture) + "\n";

  return true;
}

String ProcessCommand(uint8_t argc, String argv[])
{
  if(argc == 0) return "";

  String resp = "";

  if(argc == 1 && argv[0] == "log")
  {
    Log(resp);
  }
  else if(argc == 2 && argv[0] == "setmois")
  {
    if(!SetMinMoisture(resp, argv[1]))
      resp = "ERROR: Invalid argument!\n";
  }
  else
  {
    resp = "ERROR: Unknown command!\n";
  }

  return resp;
}

template<typename T>
void pushBack(const T& value, T*& arr, uint8_t& size) noexcept
{
  if(size == 0)
  {
    arr = new T[1];
    arr[0] = value;
    size++;
    return;
  }

  T* t = new T[size];
  for(uint8_t j=0; j<size; j++)
    t[j] = arr[j];

  delete[] arr;
  arr = new T[size + 1];

  for(uint8_t j=0; j<size; j++)
    arr[j] = t[j];

  arr[size] = value;
  size++;

  delete[] t;
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