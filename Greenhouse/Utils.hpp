#ifndef __UTILS_HPP__
#define __UTILS_HPP__

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

bool isInt(char sym, uint8_t& num)
{
  switch(sym)
  {
  case '0':
    num = 0;
    break;   
  case '1':
    num = 1;
    break;   
  case '2':
    num = 2;
    break;   
  case '3':
    num = 3;
    break;   
  case '4':
    num = 4;
    break;   
  case '5':
    num = 5;
    break;   
  case '6':
    num = 6;
    break;   
  case '7':
    num = 7;
    break;   
  case '8':
    num = 8;
    break;   
  case '9':
    num = 9;
    break;   
  default:
    return false;
  }

  return true;
}

bool isInt(const String& str, unsigned int& num)
{
  unsigned int res = 0;
  for(size_t i=0; i<str.length(); i++)
  {
    uint8_t n;
    if(!isInt(str[i], n)) return false;
    res = res * 10 + n;
  }
  num = res;
  return true;
}

#endif
