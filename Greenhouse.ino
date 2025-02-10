// Заголовки //
#include <LiquidCrystal.h> ///< Библиотека для работы с жидкокристаллическим экраном

// Макросы //
#define SOIL_SENSORS_COUNT 5 ///< Кол-во дотчиков влажности почвы
/// Пины
#define PUMP_PIN        2  ///< Помпа
#define LCD_BG_PIN      7  ///< Подсветка жидкокристаллического экрана
#define TEMPERATURE_PIN A0 ///< Датчик температуры


// Классы //

extern void UpdateAll(); ///< Объявление функции 'UpdateAll()'

/*
 * Виртуальный класс, который, при унаследовании от него, 
 * во время создания объекта добавляет указатель на объект класса-наследника в динамический массив для вызова перегруженного метода `update()` в функции `UpdateAll()`,
 * а также убирает этот указатель из массива во время разрушения объекта
 */
class Updatable
{
  friend void UpdateAll(); ///< Предоставление доступа к защищённым элементам
public:
  /*
   * Контруктор по умолчанию, добавляет указатель на создаваемый объект в динамический массив для обработки в функции `UpdateAll()`
   */
  Updatable()
  {
    if(_Size == 0)
    {
      _All = new Updatable*[1];
      _All[0] = this;
      _Size = 1;
      return;
    }

    Updatable** temp = new Updatable*[_Size + 1];
    for(size_t i=0; i<_Size; i++)
      temp[i] = _All[i];
    temp[_Size] = this;

    delete[] _All;
    _All = temp;
    _Size++;
  }

  /*
   * Виртуальный деструктор, убирает указатель на удаляемый объект из динамического массива
   */
  virtual ~Updatable()
  {
    if(_Size == 1)
    {
      delete[] _All;
      _All = 0;
      _Size = 0;
      return;
    }

    _Size--;

    Updatable** temp = new Updatable*[_Size];
    for(size_t i=0; i<_Size; i++)
      temp[i] = _All[i];

    delete[] _All;
    _All = temp;
  }

  /*
   * Возвращает указатель на первый элемент динамического массива указателей на элементы, которые обрабатываются в функции 'UpdateAll()'
   */
  static Updatable** All()
  {
    return _All;
  }

  /*
   * Возвращает количество обновляемых объектов в динамическом массиве
   */
  static uint8_t NumberOf()
  {
    return _Size;
  }
protected:
  /*
   * Чисто виртуальная функция обновления, которая вызывается в функции `UpdateAll()`
   */
  virtual void update() = 0;
private:
  inline static Updatable** _All = 0; ///< Динамический массив указателей на обноваляемые объекты
  inline static uint8_t _Size = 0;       ///< Кол-во обновляемых элементов
};

/*
 * Класс для взаимодействия с тактовой кнопкой.
 * Является обновляемым, следовательно, все объекты этого класса будут обновляться в функции `UpdateAll()`
 */
class Button : public Updatable
{
public:
  /*
   * Конструктор, принимающий пин кнопки
   */
  Button(byte pin)
    : m_pin(pin), m_isPressed(false), m_isClicked(false)
  {
    pinMode(m_pin, INPUT_PULLUP);
  }

  /*
   * Зажата ли кнопка в данный момент?
   * Возвращает `true`, если да, в противном случае возвращает `false`
   */
  bool isPressed() const
  {
    return m_isPressed;
  }

  /*
   * Была ли кнопка нажата в данный момент?
   * Возвращает `true`, если да, в противном случае возвращает `false`
   */
  bool isClicked() const
  {
    return m_isClicked;
  }
protected:
  /*
   * Перегрузка функции обновления
   */
  void update() override
  {
    bool read = digitalRead(m_pin) == HIGH;
    m_isClicked = !read && m_isPressed;
    m_isPressed = read;
  }
private:
  byte m_pin;       ///< Пин кнопки
  bool m_isPressed; ///< Зажата ли кнопка? (Да/Нет)
  bool m_isClicked; ///< Нажата ли кнопка? (Да/Нет)
};

/*
 * Класс для взаимодействия c датчиком влажности почвы.
 * Является обновляемым, следовательно, все объекты этого класса будут обновляться в функции `UpdateAll()`
 */
class SoilSensor : public Updatable
{
public:
  /*
   * Конструктор, принимающий аналоговый пин, к которому подключен датчик влажности почвы
   */
  SoilSensor(byte pin)
    : m_pin(pin) {}

  /*
   * Возвращает текущую влажность почвы целым значением в процентах (в промежутке [0; 100])
   */
  uint8_t value() const
  {
    return m_value;
  }

  /*
   * Отличается ли текущая влажность почвы от предыдущей?
   * Возвращает `true`, если да, в противном случае возвращает `false`
   */
  bool changed() const
  {
    return m_changed;
  }

  /*
   * Отличается ли текущая влажность почвы любого датчика от предыдущей влажности почвы этого датчика?
   * Возвращает `true`, если да, в противном случае возвращает `false`
   */
  static bool AverageChanged()
  {
    return _Changed;
  }

  /*
   * Возвращает целое число - среднее арифметическое значение всех датчиков влажности почвы
   */
  static uint8_t Average()
  {
    return _Average / SOIL_SENSORS_COUNT;
  }

  /*
   * Очищает параметры для корректного расчёта следующей средней арифметической влажности почвы.
   * !!! Обязательно вызывать перед функцией `UpdateAll()` !!!
   */
  static void Reset()
  {
    _Average = 0;
    _Changed = false;
  }
protected:
  /*
   * Перегрузка функции обновления
   */
  void update() override
  {
    uint8_t read = map(analogRead(m_pin), 0, 1023, 100, 0);

    m_changed = read != m_value;
    _Changed |= m_changed;

    m_value = read;
    _Average += m_value;
  }
private:
  byte m_pin;             ///< Пин датчика влажности почвы
  uint8_t m_value = 0;    ///< Текущее значение влажности почвы в процентах
  bool m_changed = false; ///< Отличается ли текущее значение от предыдущего? (Да/Нет)

  inline static bool _Changed = false; ///< Изменилось ли среднее арифметическое значение? (Да/Нет)
  inline static uint16_t _Average = 0; ///< Средее арифметическое значение влажности почвы в процентах
};

/*
 * Класс для взаимодействия c датчиком температуры LM35.
 * Является обновляемым, следовательно, все объекты этого класса будут обновляться в функции `UpdateAll()`
 */
class Temperature : public Updatable
{
public:
  /*
   * Конструктор, принимающий аналоговый пин, к которому подключен датчик температуры
   */
  Temperature(byte pin)
    : m_pin(pin)
  {}

  /*
   * Возвращает текущую температуру в градусах Цельсия в промежутке [10; 120]
   */
  uint8_t value() const
  {
    return m_value;
  }

  /*
   * Отличается ли текущая температура от предыдущей?
   * Возвращает `true`, если да, в противном случае возвращает `false`
   */
  bool changed() const
  {
    return m_changed;
  }
protected:
  /*
   * Перегрузка функции обновления
   */
  void update() override
  {
    byte read = map(analogRead(m_pin), 0, 1023, 10, 120);
    m_changed = read != m_value;
    m_value = read;
  }
private:
  byte m_pin;             ///< Пин датчика температуры
  uint8_t m_value = 0;    ///< Текущее значение температуры в градусах Цельсия
  bool m_changed = false; ///< Отличается ли текущее значение от предыдущего? (Да/Нет)
};

// Детали //
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); ///< Жидкокристаллический экран
Button powerBtn   (3); ///< Тактовая кнопка включения/выключения экрана
Button nextBtn    (4); ///< Тактовая кнопка перелистывания выводимых на экран данных
Button increaseBtn(5); ///< Тактовая кнопка увеличения изменяемого значения
Button decreaseBtn(6); ///< Тактовая кнопка уменьшения изменяемого значения
SoilSensor soilSensors[SOIL_SENSORS_COUNT] = {
  SoilSensor(A5), 
  SoilSensor(A4),
  SoilSensor(A3),
  SoilSensor(A2),
  SoilSensor(A1)
}; ///< Датчики влажности почвы
Temperature temperature(A0); ///< Датчик температуры LM35

// Переменные //

/// Русские строки
const String moistureStr    = "B\xBB""a\xB6\xBD""oc\xBF\xC4";                    ///< "Влажность"
const String temperatureStr = "Te\xBC\xBE""epa\xBF""ypa";                        ///< "Температура"
const String pumpStr        = "\xA8""o""\xBC\xBE""a";                            ///< "Помпа"
const String onStr          = "BK\xA7\xB0\xAB""EHA";                             ///< "ВКЛЮЧЕНА"
const String offStr         = "B\xAE""K\xA7\xB0\xAB""EHA";                       ///< "ВЫКЛЮЧЕНА"
const String timeLeftStr    = "\xE0""o \xB3\xC3\xBA\xBB\xC6\xC0""e\xBD\xB8\xC7"; ///< "До выключения"
const String waterTimeStr   = "Bpe\xBC\xC7"" \xBE""o\xBB\xB8\xB3""a";            ///< "Время полива"
const String minMoistureStr = "M\xB8\xBD"". " + moistureStr;                     ///< "Мин. Влажность"
const String secStr         = "ce\xBA";                                          ///< "сек"

byte degreeGlyph[] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
}; ///< Значок градуса ('°')

/// Перечисление типов выводимых на экран данных
enum State { 
  Temperature, ///< Вывод температуры
  Moisture1,   ///< Вывод влажности почвы у 1 датчика 
  Moisture2,   ///< Вывод влажности почвы у 2 датчика 
  Moisture3,   ///< Вывод влажности почвы у 3 датчика 
  Moisture4,   ///< Вывод влажности почвы у 4 датчика 
  Moisture5,   ///< Вывод влажности почвы у 5 датчика 
  AvgMoisture, ///< Вывод средней арфиметической влажности почвы
  Pump,        ///< Вывод состояния помпы
  TimeLeft,    ///< Вывод оставшегося времени полива
  MinMoisture, ///< Вывод и изменение минимальной позволенной влажности почвы
  WaterTime,   ///< Вывод и изменение времени полива
  Last         ///< Указывает на то, что надо переключиться на первый тип выводимых данных (температуру)
};
State currentState = Temperature; /// Текущий тип выводимых данных

/// Изменяемые параметры
uint8_t minMoisture = 50; ///< Минимальная позволенная влажность почвы. Если средняя арифметическая влажность почвы меньше этого значения, включается помпа
uint32_t waterTime  = 5;  ///< Время полива

/// Состояние помпы
bool pumpOn           = false; ///< Текущее состояние помпы (Вкл/Выкл)
bool prevPumpOn       = false; ///< Предыдущее состояне помпы (Вкл/Выкл)
bool pumpStateChanged = false; ///< Изменилось ли состояние помпы? (Да/Нет)

bool powerOn = true; ///< Включен ли экран? (Да/Нет)

/// Таймеры
uint32_t waterTimer  = millis(); ///< Таймер полива. Время полива изменяется пользователем
uint32_t updateTimer = millis(); ///< Таймер обновления экрана при выводе с датчиков данных. Экран обновляется, если отображаемое значение изменилось и с последнего обновления прошло более 250 миллисекунд
uint32_t clockTimer  = millis(); ///< Таймер обновления экрана пра выводе оставшегося до конца полива времени. Экран обновляется каждую секунду

/*
 * Обновляет все обновляемые объекты в динамическом массиве
 */
void UpdateAll()
{
  for(size_t i=0; i<Updatable::NumberOf(); i++)
    Updatable::All()[i]->update();
}

/*
 * Выводит данное в градусах значение на экран 
 */
void PrintDegrees(uint8_t degrees)
{
  lcd.setCursor(10, 1);
  lcd.print(degrees);
  lcd.setCursor(13, 1);
  lcd.print('\x05');
  lcd.print("C");
}

/*
 * Выводит данное в процентах значение на экран
 */
void PrintPercent(uint8_t percent)
{
  lcd.setCursor(11, 1);
  lcd.print(percent);
  lcd.setCursor(14, 1);
  lcd.print("%");
}

/*
 * Выводит данное в секундах значение на экран
 */
void PrintTime(uint32_t time)
{
  lcd.setCursor(7, 1);
  lcd.print(time);
  lcd.setCursor(11, 1);
  lcd.print(secStr);
}

/*
 * Выводит текущий тип данных на экран
 */
void Print()
{
  lcd.clear();
  switch(currentState)
  {
  case Temperature: ///< Вывод температуры 
    lcd.print(temperatureStr + ":");
    PrintDegrees(temperature.value());
    break;
  case Moisture1: ///<
  case Moisture2: ///<
  case Moisture3: ///< Вывод влажности почвы у одного из датчиков
  case Moisture4: ///<
  case Moisture5: ///<
    lcd.print(moistureStr + " " + String((int)currentState) + ":");
    PrintPercent(soilSensors[(int)currentState - 1].value());
    break;
  case AvgMoisture: ///< Вывод средней арифметической влажности почвы
    lcd.print("Cp. " + moistureStr + ":");
    PrintPercent(SoilSensor::Average());
    break;
  case Pump: ///< Вывод состояния помпы
    lcd.print(pumpStr + ":");
    lcd.setCursor(6, 1);
    lcd.print((pumpOn ? onStr : offStr));
    break;
  case TimeLeft: ///< Вывод оставшегося до конца полива времени
    lcd.print(timeLeftStr + ":");
    PrintTime((pumpOn ? waterTime - (millis() - waterTimer) / 1000 : 0));
    break;
  case MinMoisture: ///< Вывод минимальной позволенной влажности почвы
    lcd.print(minMoistureStr + ":");
    PrintPercent(minMoisture);
    break;
  case WaterTime: ///< Вывод времени полива
    lcd.print(waterTimeStr + ":");
    PrintTime(waterTime);
    break;
  }
}

/*
 * Обновляет экран если отображаемое значение датчика изменилось
 */
void UpdateLCD()
{
  if(millis() - updateTimer <= 250) return;

  static bool needPrint = false;

  needPrint = false;
  switch(currentState)
  {
  case Temperature:
    needPrint = temperature.changed();
    break;
  case Moisture1:
  case Moisture2:
  case Moisture3:
  case Moisture4:
  case Moisture5:
    needPrint = soilSensors[(int)currentState - 1].changed();
    break;
  case AvgMoisture:
    needPrint = SoilSensor::AverageChanged();
    break;
  case Pump:
    needPrint = pumpStateChanged;
    break;
  case TimeLeft:
    needPrint = millis() - clockTimer > 1000;
    if(needPrint) clockTimer = millis();
    break;
  }

  if(needPrint)
  {
    Print();
    updateTimer = millis();
  }
}

/*
 * Исполняется при запуске устройства и инициализирует все датчики, переменные и т.п.
 */
void setup() 
{
  /// Инициализания пинов
  pinMode( PUMP_PIN,   OUTPUT );
  pinMode( LCD_BG_PIN, OUTPUT );

  /// Инициализация жидкокристаллического экрана
  digitalWrite(LCD_BG_PIN, HIGH);
  lcd.begin(16, 2);
  lcd.createChar('\x05', degreeGlyph);
}

/*
 * Основной цикл программы
 */
void loop()
{
  /// Обновление датчиков
  SoilSensor::Reset();
  UpdateAll();

  /// Если время полива истекло, то выключаем помпу
  if(pumpOn && millis() - waterTimer > waterTime * 1000)
  {
    pumpOn = false;
    digitalWrite(PUMP_PIN, LOW);
  }

  /// Если среднее арифметическое значение влажности почвы меньше минимального позволенного, то включаем помпу
  if(SoilSensor::Average() < minMoisture && !pumpOn)
  {
    pumpOn = true;
    waterTimer = millis();
    digitalWrite(PUMP_PIN, HIGH);
  }

  /// Проверяем, изменилось ли состояние помпы
  pumpStateChanged = prevPumpOn != pumpOn;
  prevPumpOn = pumpOn;

  /// Обрабатываем нажатие кнопки выключения
  if(powerBtn.isClicked())
  {
    if(powerOn)
    {
      lcd.clear();
      digitalWrite(LCD_BG_PIN, LOW);
    }
    else
    {
      digitalWrite(LCD_BG_PIN, HIGH);
      Print();
    }
    powerOn = !powerOn;
  }

  /// Если экран выключен, то проверять остальные события ненужно, выходим из функции
  if(!powerOn) return;
  
  /// Обрабатываем нажатие кнопки перелистывания данных
  if(nextBtn.isClicked())
  {
    currentState = (State)((int)currentState + 1);
    if(currentState == Last)
      currentState = (State)0;
    Print();
  }
  else if(increaseBtn.isClicked() || decreaseBtn.isClicked()) ///< Обрабатываем нажатия кнопок увеличения и уменьшения значения
  {
    bool valueChanged = false;  

    if(increaseBtn.isClicked())
    {
      if(currentState == MinMoisture && minMoisture < 100)
      {
        minMoisture++;
        valueChanged = true;
      }
      else if(currentState == WaterTime && waterTime < 3600)
      {
        waterTime++;
        valueChanged = true;
      }  
    }
    else if(decreaseBtn.isClicked())
    {
      if(currentState == MinMoisture && minMoisture > 0)
      {
        minMoisture--;
        valueChanged = true;
      }
      else if(currentState == WaterTime && waterTime > 1)
      {
        waterTime--;
        valueChanged = true;
      }
    }

    /// Если значение было изменено пользователем, то обновляем экран и выходим из функции
    if(valueChanged)
    {
      Print();
      return;
    }
  }

  /// Если программа только что была запущена, то нужно отобразить значения сразу,
  /// а если нет, то отобразить значения нужно только тогда, когда они изменились и промежуток между изменениями больше 250 миллисекунд
  static bool started = false;
  if(!started)
  {
    Print();
    started = true;
  }
  else UpdateLCD();
}
