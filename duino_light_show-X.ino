/****************************************************************
 Этот код написан и принадлежит компании duino.ru
 При использование кода указание авторства обязательно
****************************************************************/
#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <ffft.h>
#include <math.h>


#define USE_REMOTE true
#if USE_REMOTE
//Maximum supported 2.8.0  lib revision !!!
#include <IRremote.h>
#endif


typedef struct 
{
  uint8_t baseColor;
  uint8_t age;
  uint8_t magnitude;
  uint8_t rnd;
} peak_t;

#define DEBUG false  //режим отладки true

#define N_BANDS 8 //число светодиодов для индикации в режиме настройки яркости
#define N_FRAMES 5
#define N_PEAKS 25


//---------------Световые алгоритмические схемы ---------------------

#define PATTERN_DANCE_PARTY             0
#define PATTERN_SINGLE_DIR_DANCE_PARTY  1
#define PATTERN_PULSE                   2
#define PATTERN_LIGHT_BAR               33
#define PATTERN_COLOR_BARS              3
#define PATTERN_COLOR_BARS2             4
#define PATTERN_FLASHBULBS              5
#define PATTERN_FIREFLIES               6
#define PATTERN_RANDOM                  7
//------------------------------------------------------------------
#define PATTERN_MUSIC_LAST      PATTERN_RANDOM           // последний режим цветомузыки
#define N_MODES_LS              (PATTERN_MUSIC_LAST + 1) // количество режимов цветомузыки

//---------------Режимы эффектов бегущих огрней----------------------
#define PATTERN_COLOR_LAST  PATTERN_COL_RANDOM          // последний режим бегущих огрней

#define  PATTERN_BACKGROUND   N_MODES_LS
#define  PATTERN_RAINBOW      (N_MODES_LS + 1)
#define  PATTERN_SPARKLE      (N_MODES_LS + 2)
#define  PATTERN_COL_RANDOM   (N_MODES_LS + 3)
#define  PATTERN_BRIGHT       (N_MODES_LS + 4)
#define  PATTERN_LAMP         (N_MODES_LS + 5)
#define  PATTERN_CYLON        (N_MODES_LS + 6)
#define  PATTERN_FLAME        (N_MODES_LS + 7)
#define  PATTERN_MATRIX       (N_MODES_LS + 8)
#define  PATTERN_BALLS        (N_MODES_LS + 9)
#define  PATTERN_FIRE         (N_MODES_LS + 10)

//---------------------------------------------------
#define N_MODES (PATTERN_COLOR_LAST + 1) // общее количество режимов

#define PATTERN_BUTTON_PIN  9 // Кнопка PATERN
#define COLOR_BUTTON_PIN    8 // Rнопка COLOR
//*******************************************************************************************************
// !!!!! Если используете только 2 кнопки, то закомментируйте #define JUMP_BUTTON_PIN
//*******************************************************************************************************
#define JUMP_BUTTON_PIN     7 // Кнопка JUMP

#ifdef JUMP_BUTTON_PIN
#define ALL_BUTTONS         (_BV(0)|_BV(1)|_BV(2))
#else
#define ALL_BUTTONS         (_BV(0)|_BV(1))
#endif

#define btPATTERN           (_BV(0))
#define btCOLOR             (_BV(1))
#ifdef JUMP_BUTTON_PIN
#define btJUMP              (_BV(2))  
#endif

#define ADC_CHANNEL 0   //аналоговый вход, для аудио
#define PARM_POT  2   // аналоговый вход для потенциометрам PARAM
#ifdef JUMP_BUTTON_PIN
//*******************************************************************************************************
// !!!!! Если используете только 1 потенциометр, то закомментируйте #define PARM_POT_2
//*******************************************************************************************************
#define PARM_POT_2  1   // аналоговый вход для потенциометрам PARAM_2
#endif

//---------------цветовые схемы---------------------
#define N_COLOR_MODES 3 //число режимов 
#define COLOR_RANDOM 0 //Случайная двухцветная схема: выбраны два случайных цвета
#define COLOR_CYCLE 1 //меняется как радуга с течением времени
#define COLOR_BAND 2 //Цветные частоты. Самая низкая красного цвета, и т.д. вверх красный,оранжевый,желтый,зеленый,голубой,синий,фиолетовый,белый
//-----------------выбор конфигурации ленты--------------------------------
#define LEDCONFIG_60 0
#define LEDCONFIG_120 1
#define LEDCONFIG_140 2
//-------------------------------------------------
#define MAX_COLOR_BARS 22
#define MAXCOLORINDEX 256
#define EEPROM_MAGIC_NUMBER 0xbad1 //магическое число, для индетификации первого запуска

#define UNUSED 255


// Define sensor pin
const int RECV_PIN = 4;

// RJ TECH remote
//#define RECV_TYPE         NEC
//#define RECV_BTTN_UP      0xFF48B7
//#define RECV_BTTN_DOWN    0xFFC837
//#define RECV_BTTN_LEFT    0xFF8877
//#define RECV_BTTN_RIGHT   0xFF08F7
//#define RECV_BTTN_OK      0xFFA857
//#define RECV_BTTN_1       0xFF20DF
//#define RECV_BTTN_2       0xFFA05F
//#define RECV_BTTN_3       0xFF9867
//#define RECV_BTTN_4       0xFFE01F
//#define RECV_BTTN_5       0xFF10EF
//#define RECV_BTTN_6       0xFF708F
//#define RECV_BTTN_7       0xFF50AF
//#define RECV_BTTN_8       0xFF38C7
//#define RECV_BTTN_9       0xFFE817
//#define RECV_BTTN_0       0xFF7887
//#define RECV_BTTN_GOTO    0xFFB847
//#define RECV_BTTN_REP     0xFFF807
//#define RECV_BTTN_SETUP   0xFF32CD
//#define RECV_BTTN_MENU    0xFF0AF5
//#define RECV_BTTN_TITLE   0xFF30CF

// APEX remote
#define RECV_TYPE         NEC
#define RECV_BTTN_1       0xFF9867
#define RECV_BTTN_2       0xFFD827
#define RECV_BTTN_3       0xFF8877
#define RECV_BTTN_4       0xFFA857
#define RECV_BTTN_5       0xFFE817 
#define RECV_BTTN_6       0xFF48B7 
#define RECV_BTTN_7       0xFF6897 
#define RECV_BTTN_8       0xFF32CD
#define RECV_BTTN_9       0xFF02FD
#define RECV_BTTN_0       0xFF12ED
#define RECV_BTTN_AB      0xFF28D7
#define RECV_BTTN_REP     0xFF38C7
#define RECV_BTTN_SETUP   0xFFC03F
#define RECV_BTTN_MENU    0xFF807F
#define RECV_BTTN_TITLE   0xFFB847

//---------------------------------
#define RECV_BTTN_JUMP   RECV_BTTN_AB
#define RECV_BTTN_NEXT   RECV_BTTN_REP
#define RECV_BTTN_FREQ   RECV_BTTN_SETUP
#define RECV_BTTN_COLOR  RECV_BTTN_MENU
#define RECV_BTTN_BKGRND RECV_BTTN_TITLE

#if USE_REMOTE
// Define IR Receiver and Results Objects
IRrecv irrecv(RECV_PIN);
decode_results results;
#endif
 
#define BACKGROUND ((uint32_t) 0x000006) // фоновая подсветка при тишине
#define BACKGROUND_COUNT   (31 - 1)
#define BACK_COLOR_EEPROM_ADR     7
#define BACK_BRIGHT_EEPROM_ADR    (BACK_COLOR_EEPROM_ADR + 1)


#define LED_PIN 13            //системный светодиод, нужно использовать выводы только с ШИМ
#define LED_STRIP_PIN 6       // пин подключения к LED ленте
#define N_LED_LAMP 30         // какое количество светождиодов использовать в светильнике
#define NUM_LED 120           // количество светодиодов в ленте по умолчанию
uint8_t N_LEDS = NUM_LED;       
uint8_t MAX_AGE = 0;
uint8_t buttons = 0;
uint8_t ir_mode = N_MODES;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(140, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

#ifdef JUMP_BUTTON_PIN
uint8_t lastMusic = 0;
uint8_t lastShow = PATTERN_BACKGROUND;
#ifdef  PARM_POT_2
uint8_t BackIndex = 0;
uint8_t BackBrightness = 0;
#endif
#endif

peak_t peaks[N_PEAKS];
uint8_t peakIndex = 0;
uint8_t newPeakFlags = 0;
uint8_t mode = 0;
uint8_t pattern = 0;
uint8_t cutoffFreqBand = 7;
uint8_t colorMode = COLOR_CYCLE; //цветовая схема по умолчанию
uint8_t colorIndexIncFreq = 100;
uint8_t colorIndex = 0;
uint8_t randColor[] = {0, 0};
uint8_t randColorCount[] = {0, 0};
uint8_t randColorChangeParm = 10;
uint8_t ledConfig = LEDCONFIG_120; //число светодиодов по умолчанию
uint16_t parm = 1024;
uint16_t lastParm;
uint8_t colorBars[MAX_COLOR_BARS];
uint16_t loopCounter = 0;
uint16_t loopCounterMax = 500;
boolean randomized = false;
uint8_t recentPatterns[3];
uint8_t recentPatternIndex = 0;
uint8_t transitionWaitTime = 0;

// FFT_N = 128
#if ! DEBUG
int16_t       capture[FFT_N];    // Audio буфер захвата
complex_t     bfly_buff[FFT_N];  // FFT "butterfly" buffer
uint16_t      spectrum[FFT_N / 2]; // Spectrum output buffer
#endif

#define MAX_BRIGHTNESS  200
volatile uint8_t  smp; //порог срабатывания, уровень шума NoiseThreshold=0,
volatile uint8_t samplePos = 0;     // Начальнаячя позиция в буфере
uint8_t maxBrightness = MAX_BRIGHTNESS;       //  яркость максимальная
float brightnessScale = 1.0;    // коэффициент яркости

byte
bandPeakLevel[8],      // Peak level of each band
  bandPeakCounter = 0, // Frame counter for delaying the fall of the band peak
  bandPeakDecay = 6, // peak decreases by 1 every bandPeakDecay frames. Larger value is slower decay
  bandCount = 0;      // Frame counter for storing past band data

int
band[8][N_FRAMES],   // band levels for the prior N_FRAMES frames
     minLvlAvg[8], // For dynamic adjustment of low & high ends of graph,
     maxLvlAvg[8], // pseudo rolling averages for the prior few frames.
     bandDiv[8];    // Used when filtering FFT output to 8 bands

// declare all control variables here so we can more easily measure memory consumption
uint8_t j, noiseThreshold, *data, nBins, binNum, c;
uint16_t minLvl, maxLvl;
int16_t level, sum;
bool next_run = false;

const uint8_t PROGMEM noiseFloor[64] = {8, 6, 6, 5, 3, 4, 4, 4, 3, 4, 4, 3, 2, 3, 3, 4, 2, 1, 2, 1, 3, 2, 3, 2, 1, 2, 3, 1, 2, 3, 4, 4, 3, 2, 2, 2, 2, 2, 2, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4};
const uint8_t PROGMEM eq[64] = {255, 175, 218, 225, 220, 198, 147, 99, 68, 47, 33, 22, 14, 8, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t PROGMEM bandBinCounts[] = {2, 4, 5, 8, 11, 17, 25, 37};
const uint8_t PROGMEM bandBinStarts[] = {1, 1, 2, 3, 5, 7, 11, 16};
const uint8_t PROGMEM band0weights[] = {181, 40};
const uint8_t PROGMEM band1weights[] = {19, 186, 38, 2};
const uint8_t PROGMEM band2weights[] = {11, 176, 118, 16, 1};
const uint8_t PROGMEM band3weights[] = {5, 55, 165, 164, 71, 18, 4, 1};
const uint8_t PROGMEM band4weights[] = {3, 24, 89, 139, 148, 118, 54, 20, 6, 2, 1};
const uint8_t PROGMEM band5weights[] = {2, 9, 29, 70, 125, 172, 185, 162, 118, 74, 41, 21, 10, 5, 2, 1, 1};
const uint8_t PROGMEM band6weights[] = {1, 4, 11, 25, 49, 83, 121, 156, 180, 185, 174, 149, 118, 87, 60, 40, 25, 16, 10, 6, 4, 2, 1, 1, 1};
const uint8_t PROGMEM band7weights[] = {1, 2, 5, 10, 18, 30, 46, 67, 92, 118, 143, 164, 179, 185, 184, 174, 158, 139, 118, 97, 77, 60, 45, 34, 25, 18, 13, 9, 7, 5, 3, 2, 2, 1, 1, 1, 1};
const uint8_t PROGMEM * const bandWeights[] = {band0weights, band1weights, band2weights, band3weights, band4weights, band5weights, band6weights, band7weights};

#ifdef  PARM_POT_2
const uint32_t BackColors[] PROGMEM = 
{
  0xff0000, 0xff3300, 0xff6600, 0xff9900, 0xffcc00, 0xffff00, 0xccff00, 0x99ff00,
  0x66ff00, 0x33ff00, 0x00ff00, 0x00ff33, 0x00ff66, 0x00ff99, 0x00ffcc, 0x00ffff,
  0x00ccff, 0x0099ff, 0x0066ff, 0x0033ff, 0x0000ff, 0x3300ff, 0x6600ff, 0x9900ff,
  0xcc00ff, 0xff00ff, 0xff00cc, 0xff0099, 0xff0066, 0xff0033, 0x000000
};
#endif
//*******************************************************************************************************
void setup() 
{
#if DEBUG//для отладки
  Serial.begin(115200);
#endif  
  strip.setBrightness(maxBrightness);
  randomSeed(analogRead(2)); // получаем стартовое случайное значение 
  memset(bandPeakLevel, 0, sizeof(bandPeakLevel)); //создать буфер в RAM
  memset(band, 0, sizeof(band));//создать буфер в RAM

  for (uint8_t i = 0; i < N_BANDS; i++)
  {
    binNum = pgm_read_byte(&bandBinStarts[i]);
    nBins = pgm_read_byte(&bandBinCounts[i]);

    minLvlAvg[i] = 0;
    maxLvlAvg[i] = 512;
    data = (uint8_t *)pgm_read_word(&bandWeights[i]);
    for (bandDiv[i] = 0, j = 0; j < nBins; j++) 
    {
      bandDiv[i] += pgm_read_byte(&data[j]);
    }
  }

  for (uint8_t i = 0; i < N_PEAKS; i++) 
  {
    peaks[i].age = 0;
    peaks[i].magnitude = 0;
  }
  
  for (uint8_t i = 0; i < MAX_COLOR_BARS; i++) 
  {
    colorBars[i] = UNUSED;
  }
  
  for (uint8_t i = 0; i < 3; i++) 
  {
    recentPatterns[i] = UNUSED;
  }

  lastParm = 0;
  parm = analogRead(PARM_POT); //прочитать положение потенциометра PARAM
  
  setADCFreeRunning(); //запустить прерывание от АЦП
  DIDR0  = 1 << ADC_CHANNEL; // Отключение цифрового входа для указанного вывода АЦП

  pinMode(PATTERN_BUTTON_PIN, INPUT_PULLUP); //кнопка PATTERN
  pinMode(COLOR_BUTTON_PIN, INPUT_PULLUP);  //кнопка COLOR
#ifdef JUMP_BUTTON_PIN  
  pinMode(JUMP_BUTTON_PIN, INPUT_PULLUP);
#endif  
 /* pinMode(LED_PIN, OUTPUT);                 //системный светодиод

  for (i = 0; i < 5; i++) //мигнуть 5 раз системным светодиодом
  {
    digitalWrite(LED_PIN, HIGH);//включить системный светодиод
    delay(50);
    digitalWrite(LED_PIN, LOW); //выключить системный светодиод
    delay(50);
  }*/

  sei(); // разрешить все прерывания

  strip.begin();
  strip.show(); 

  // проверяем, есть ли сохраненные настройки EEPROM?
  // если да, то считываем их
  if (EEPROMValid()
#ifdef JUMP_BUTTON_PIN  
  && (digitalRead(JUMP_BUTTON_PIN) == HIGH)
#endif  
                            ) //проверяем на первое включение контролера
  {
    ledConfig = EEPROM.read(2); //конфигурация светодиодной ленты
    if (ledConfig > LEDCONFIG_140) ledConfig = LEDCONFIG_140;

    mode = EEPROM.read(3); //последний выбраный режим световой схемы
    if (mode > N_MODES) mode = 0;
 
    pattern = mode;
      if (pattern == PATTERN_RANDOM) 
       {
         randomized = true;
          pattern = chooseRandomPattern();
          loopCounter = 0;
        } else { randomized = false;  }
    
    maxBrightness = EEPROM.read(4);     //последние сохраненные настройки яркости ленты
    brightnessScale = maxBrightness / 255.0;
    colorMode = EEPROM.read(5);         //режим цветовой схемы
    if (colorMode > COLOR_BAND) colorMode = COLOR_BAND;
    cutoffFreqBand = EEPROM.read(6);    //настройки частоты среза
    if (cutoffFreqBand > 7) cutoffFreqBand = 7;
    //NoiseThreshold = EEPROM.read(7);
  } else {
          test(); // запуск диагностики при первом включении питания
          ledConfig = LEDCONFIG_120;  // если EEPROM не записана, то по умолчанию 120 LEDs, 
          mode = PATTERN_BACKGROUND;                  // первый режим
          maxBrightness = MAX_BRIGHTNESS;        //яркость максимальная
          colorMode = COLOR_BAND;    //цветовая схема COLOR_BAND
          cutoffFreqBand = 8;         //частотный срез все 8 частот
         // NoiseThreshold = 4;         //уровень срабатывания АЦП
          saveConfig();
#if DEBUG //для отладки
          Serial.println("Config RESET!!!");
#endif            
         }
#ifdef  PARM_POT_2
  BackIndex = EEPROM.read(BACK_COLOR_EEPROM_ADR);
  if (BackIndex > BACKGROUND_COUNT)    BackIndex = 0;
  BackBrightness = EEPROM.read(BACK_BRIGHT_EEPROM_ADR);
#endif

#if DEBUG //для отладки
  Serial.print("ledConfig - ");
  Serial.println(ledConfig);
    Serial.print("mode - ");
  Serial.println(mode);
    Serial.print("maxBrightness - ");
  Serial.println(maxBrightness);
    Serial.print("colorMode - "); 
  Serial.println(colorMode);
    Serial.print("mcutoffFreqBandode - ");
  Serial.println(cutoffFreqBand);
#endif

  configure(); 
  setConfig();
  setParameters();
  
#if DEBUG //для отладки
  Serial.println("setParameters");
#endif 
  
  TIMSK0 = 0;                // Выключить Timer0
  
#if DEBUG //для отладки
  Serial.println("Timer");
#endif 
#if USE_REMOTE
    // Enable the IR Receiver
  irrecv.enableIRIn();
#endif
#if DEBUG //для отладки
  Serial.println(getMemory());
#endif  
}

#define BTTN_DELAY 750

void msdelay(uint32_t period)
{
  uint8_t  timer = TIMSK0;

  TIMSK0 = 1;
  delay(period);
  TIMSK0 = timer;
}

uint8_t get_key_set(void)
{
  return ALL_BUTTONS & ~(digitalRead(PATTERN_BUTTON_PIN) | (digitalRead(COLOR_BUTTON_PIN) << 1)
#ifdef JUMP_BUTTON_PIN
  | (digitalRead(JUMP_BUTTON_PIN) << 2)
#endif  
  );
}

uint8_t get_buttons(void)
{
  static uint8_t key_set = 0;
  uint8_t new_set = get_key_set();

  if(new_set)
  {
     if(new_set > key_set) key_set = new_set;
     return 0;
  }
  else
  {
    new_set = key_set;
    key_set = 0;

    if(!new_set)
    {
#if USE_REMOTE      
        if (irrecv.decode(&results))
        {
#if DEBUG
            Serial.println(results.value, HEX);
#endif          
            if(results.decode_type == RECV_TYPE)
            {
                switch (results.value) 
                {
                    case RECV_BTTN_NEXT:
                      new_set = btPATTERN;
                      ir_mode = N_MODES;
                      break;
                    case RECV_BTTN_COLOR:
                      new_set = btCOLOR;
                      break;
                    case RECV_BTTN_FREQ:
                      new_set =  btPATTERN + btCOLOR;
                      break;                   
#ifdef JUMP_BUTTON_PIN                      
                    case RECV_BTTN_JUMP:
                      new_set = btJUMP;
                      break;
#ifdef PARM_POT_2                      
                    case RECV_BTTN_BKGRND:
                      new_set = btJUMP + btCOLOR;
                      break;
#endif
#endif
                    case RECV_BTTN_0:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_BACKGROUND;
                      break;
                    case RECV_BTTN_1:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_DANCE_PARTY;
                      break;                      
                    case RECV_BTTN_2:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_SINGLE_DIR_DANCE_PARTY;
                      break;                      
                    case RECV_BTTN_3:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_PULSE;
                      break;                      
                    // case RECV_BTTN_4:
                    //   new_set = btPATTERN;
                    //   ir_mode = PATTERN_LIGHT_BAR;
                    //   break;                      
                    case RECV_BTTN_4:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_COLOR_BARS;
                      break;                      
                    case RECV_BTTN_5:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_COLOR_BARS2;
                      break;                      
                    case RECV_BTTN_6:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_FLASHBULBS;
                      break;                      
                    case RECV_BTTN_7:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_FIREFLIES;
                      break;                      
                    case RECV_BTTN_8:
                      new_set = btPATTERN;
                      ir_mode = PATTERN_RANDOM;
                      break;                     
                      
                }
            }
            irrecv.resume();
        }
#endif
    }

    return new_set;  
  } 

}
void background()
{
    while(true)
    {
      for (uint8_t i = 0; i < N_LEDS; i++)
#ifdef  PARM_POT_2
        strip.setPixelColor(i, pgm_read_dword(&BackColors[BackIndex]));
      strip.setBrightness(BackBrightness);
#else
        strip.setPixelColor(i, BACKGROUND);
#endif      
#if USE_REMOTE        
      if(irrecv.isIdle()) 
#endif
      { strip.show(); }
      button_pattern();
      if (pattern != PATTERN_BACKGROUND) return;
    }
}
//************************************Главный цикл программы*******************************************
void loop() 
{
#if USE_REMOTE        
  if(irrecv.isIdle()) 
#endif
  {
  //Пока прерывание АЦП включено, ждем завершения    
  // Обработчик прерывания собирает аудио семплы в буфер захвата.
  while (ADCSRA & _BV(ADIE));

  strip.show(); //отправляем последние сформированые данные на светодиодную ленту.
#if ! DEBUG
  fft_input(capture, bfly_buff);// Выполнить алгоритм FFT для преобразования выборок в сложные числа.
#endif  
  }

  // Чтобы получить показания потенциометра, переводим АЦП в стандартный режим
  setADCDefault();    //переводим АЦП в стандартный режим

  buttons = get_buttons();
    
  lastParm = parm;
  parm = analogRead(PARM_POT); //прочитать состояние  потенциометра
  if (parm != lastParm) // меняем настройки если положение потенциометра было изменено
  {
    setParameters();
  }

  if ((colorMode == COLOR_CYCLE) && ((loopCounter % colorIndexIncFreq) == 0)) 
  {
    colorIndex++;
  }
  
  loopCounter++;
  if ((randomized) && (loopCounter > loopCounterMax)) 
  {
    loopCounter = 0;
    loopCounterMax = random(500, 1000);
    transitionWaitTime = determineWaitTime();
  }

  if (transitionWaitTime > 0) 
  {
    transitionWaitTime--;
    if (transitionWaitTime == 0) 
    {
      pattern = chooseRandomPattern();
      setParameters();
      reset();
    }
  }

  button_pattern(false); //проверяем нажатие кнопок

  if(!next_run)
  {
    next_run = true;
    if(mode >= N_MODES_LS)
    {
      //dirty hack ;)
      mode--;
      setMode(false);
    }
  }

#if USE_REMOTE        
  if(irrecv.isIdle()) 
#endif
  {
    setADCFreeRunning();             // снова настраиваем АЦП на работу по прерыванию
#if ! DEBUG
    // The rest of the FFT computation:
    fft_execute(bfly_buff);          // Process complex data
    fft_output(bfly_buff, spectrum); // Complex -> spectrum
#endif  
    // Now call this to analyze the audio. See comments in this function for details.
    analyzeAudioSamples();
  
  
    if (transitionWaitTime == 0)  // only do this if not waiting to transition to new mode
    {
  
      // The peak values for each of the 8 bands has been computed. A bit in the 8-bit
      // value newPeakFlags indicates whether the analysis found a *new* peak in the band.
      for (uint8_t i = 0; i <= cutoffFreqBand; i++) 
      {
  
        // If a new peak was found in band i...
        if (newPeakFlags & (1 << i)) {
          // Map the peak value to a magnitude in range [0,255]. We pass in the band
          // number because the mapping is different for different bands.
          uint8_t magnitude = getMagnitude(i, bandPeakLevel[i]);
  
          // A nonzero magnitude means that the peak value is large enough to do
          // something visually with it. We ignore small peaks.
          if (magnitude > 0) 
          {
            // We want to store the information about the peak in a peak_t structure.
            // When we actually draw a visualization, the peak_t structures in peaks[]
            // represent the "visually active" band peaks.
            if (pattern != PATTERN_PULSE) 
            {
              // Look through the list of peak structures 'peaks' for an unused one.
              for (j = 0; j < N_PEAKS; j++) 
              {
                if (peaks[j].magnitude == 0) 
                {
                  // unused peak found
                  peakIndex = j;
                  break;
                }
              }
            } else {
                      peakIndex = i; // для импульсного режима, просто используеся пик для каждой полосы частот
                    }
              // Если не найден, мы используем последний используемый (peakIndex).     
            // Инициализация структуры
            peaks[peakIndex].age = 0;
            peaks[peakIndex].rnd = random(255); //случайное значение для визуализации
            if (colorMode == COLOR_BAND) 
            {
              peaks[peakIndex].baseColor = i * 32;
            }
            if (colorMode == COLOR_RANDOM) 
            {
              peaks[peakIndex].baseColor = getRandomBaseColor(peaks[peakIndex].rnd);
            }
            if (colorMode == COLOR_CYCLE) 
            {
              peaks[peakIndex].baseColor = colorIndex;
            }
            peaks[peakIndex].magnitude = magnitude;
          }
        }
      }
    }

    // if (pattern != PATTERN_LIGHT_BAR) 
    // {
      strip.clear();   // выключаем все светодиоды в ленте
    // }
  
    strip.setBrightness(maxBrightness);
  
    doVisualization(); // визуализация

  }
}

//************************проверяем нажатие кнопки PATTERN**************************
void button_pattern()
{
#if USE_REMOTE        
      if(irrecv.isIdle()) 
#endif
      { button_pattern(true); }
}
void button_pattern(bool flag)
{

  if(flag) buttons = get_buttons();
  //if ((digitalRead(PATTERN_BUTTON_PIN) == LOW) && (digitalRead(COLOR_BUTTON_PIN) == HIGH))  // Если кнопка pattern нажата
  if(buttons == btPATTERN)
  {
    //digitalWrite(LED_PIN, HIGH); 
    setMode(flag);
  }
#ifdef JUMP_BUTTON_PIN  
  if(buttons == btJUMP)
  {
//    digitalWrite(LED_PIN, HIGH);

      if(mode < N_MODES_LS) lastMusic  = mode;
      else lastShow = mode;
      if(mode < N_MODES_LS ) mode = lastShow - 1;
      else  mode = lastMusic - 1;

      setMode(flag);
//    analogWrite(LED_PIN, LED_BRIGHTNESS);
  }
#endif  


  // проверяем нажатие кнопки COLOR
  //if ((digitalRead(COLOR_BUTTON_PIN) == LOW) && (digitalRead(PATTERN_BUTTON_PIN) == HIGH)) 
  if(buttons == btCOLOR)
  {
    //digitalWrite(LED_PIN, HIGH);//включить системный светодиод
    setColorMode();
  }

  // Если кнопки COLOR и pattern нажаты, то меняем частотные срезы
//  if ((digitalRead(COLOR_BUTTON_PIN) == LOW) && (digitalRead(PATTERN_BUTTON_PIN) == LOW)) 
  if(buttons == (btPATTERN + btCOLOR))
  {
    // Both buttons held. Set cutoff frequency band.
    //digitalWrite(LED_PIN, HIGH);//включить системный светодиод
    setCutoffFreqBand();
  }

#ifdef  PARM_POT_2
  if(buttons == (btJUMP + btCOLOR))
  {
//    digitalWrite(LED_PIN, HIGH);
    setBackgroundColor();
//    analogWrite(LED_PIN, LED_BRIGHTNESS);
  }
#endif

}  

//****************************************включить и сохранить выбранный режим***************************************************************
void setMode(bool flag) 
{

 TIMSK0 = 0;                // Выключить Timer0

  //test for an ir button
  if(ir_mode != N_MODES) mode = ir_mode;
  else  mode++;
  
  ir_mode = N_MODES;
      
  if (mode >= N_MODES) //если достигли конца выбора эффектов, то начнем сначала
  {
    mode = 0;
    transitionWaitTime = 0;
    randomized = false;
  }

  pattern = mode;
  if (pattern == PATTERN_RANDOM) // если выбран "случайный режим"
  {
    randomized = true;
    pattern = chooseRandomPattern();
    loopCounter = 0;
  } else 
        {
          randomized = false;
        }

  strip.clear(); //обнулить всю ленту
  if (!randomized) 
  {
      if(mode < PATTERN_MUSIC_LAST )
      {
          strip.setPixelColor(mode, 64, 64, 64);   /// включаем светодиод ленты белым цветом а в качестве индикатора 
      }
      else
      {
          strip.setPixelColor(mode, 0, 0, 128); //синий это режимы световых эффектов(бещие огни)
          TIMSK0 = 1; 
      }  
  }
  else 
  {
      strip.setPixelColor(mode, 128, 0, 0); // красный для выбора режима случайной световой схемы
  }
  strip.show(); //применить схему
  //while ((digitalRead(PATTERN_BUTTON_PIN) == LOW) && (digitalRead(COLOR_BUTTON_PIN) == HIGH)); //ждем отпускания кнопки PATTERN
  msdelay(BTTN_DELAY);
  
  saveConfig();  //сохранить изменения в EEPROM
  
  if(flag) return;

  while(mode >= N_MODES_LS)
  {
    setParameters(); //устанавливаем параметры для выбранного режима
#if DEBUG //для отладки
  Serial.println(pattern);
#endif
  }

  reset();
  strip.clear(); //погасить всю ленту
}
//*******************************// Выбор режима световой схемы*********************************************
void setParameters() 
{

  switch (pattern) 
  {
    case PATTERN_DANCE_PARTY:
      MAX_AGE = N_LEDS / 2 + N_LEDS / 8;
      break;
    case PATTERN_PULSE:
      MAX_AGE = 15;
      break;
    case PATTERN_SINGLE_DIR_DANCE_PARTY:
      MAX_AGE = N_LEDS + N_LEDS / 4;
      break;
    // case PATTERN_LIGHT_BAR:
    //   MAX_AGE = 60;
    //   break;
    case PATTERN_COLOR_BARS:
    case PATTERN_FLASHBULBS:
      MAX_AGE = 30;
      break;
    case PATTERN_FIREFLIES:
      MAX_AGE = 30;
      break;
    case PATTERN_COLOR_BARS2:
      MAX_AGE = 25;
      break;
  }
  
  //-----------------------------
  // don't call it from setup
  if(next_run)
  {
   switch (pattern) 
   {
    case PATTERN_BACKGROUND:
      background();
      break;
    case PATTERN_RAINBOW:
      rainbow();
      break;
    case PATTERN_SPARKLE:
      sparkle();
      break;
    case PATTERN_COL_RANDOM:
      col_random();
      break;
//    case PATTERN_CYLON:
//      CylonBounce();
//      break;
    case PATTERN_BRIGHT:
      LedBright();
      break;
    case PATTERN_LAMP:  
      LampDimmer();
      break;
//     case PATTERN_FLAME:
//     Serial.println("flame1");
//      flame();
//      Serial.println("flame2");
//      break;
//      
//     case PATTERN_FIRE:
//      fire(700,120, 100);
//      break;
//    case PATTERN_MATRIX:
//      matrix(0,255,0,5,90,true,40);
//      break;
//    case PATTERN_BALLS:
//      {
//        uint8_t colors[5][3] = {{255,0,0},{255,0,255},{0,0,255},{255,255,0},{0,255,0}};
//        bouncing_balls(5, colors); 
//       }
//       break;    
   }
  }

  smp = (map(parm, 0, 1023, 0, 63)); //уменьшить разрядность
}

//******************************случайный режим********************************************************************
uint8_t chooseRandomPattern() 
{
  boolean found = false;
  uint8_t p;
  while (!found) 
  {
    p = random(PATTERN_RANDOM); // PATTERN_RANDOM 8
    found = true; // assume we've chosen a pattern we haven't used recently
    for (uint8_t i = 0; i < 3; i++) 
    {
      if (p == recentPatterns[i]) 
      {
        // we used this pattern recently, so try again
        found = false;
        break;
      }
    }
  }
  recentPatterns[recentPatternIndex] = p;
  recentPatternIndex++;
  if (recentPatternIndex == 3) 
  {
    recentPatternIndex = 0;
  }
  return p;
}

//**********************Обработчик прерывания от АЦП*****************************************************
ISR(ADC_vect) //прерывание АЦП. по окончании оцифровки семпла 
{
  int16_t sample = ADC; // семпл доступен в регистре ADC = 0-1023
  // преобразуем беззнаковый семпл в отриц или в положительный, нужно для FFT
  // нолем считается 512, все что больше 512 положительные и все что меньше 512 это отрицательные значения
  // все что меньше порогового уровня шума будет обнулятся
  if(sample>(512+smp)) sample-=smp; else if(sample<(512-smp))sample+=smp; //уменьшить уровень звука
#if ! DEBUG
  capture[samplePos] = ((sample > (511 - smp)) && (sample < (511 + smp))) ? 0 : sample - 512;
#endif
  if (++samplePos >= FFT_N) ADCSRA &= ~_BV(ADIE); //буфер заполнен семплами, поэтому отключаем прерывания от АЦП
}

 /* static const int16_t noiseThreshold = 4; // ignore small voltage variations.

  // the sample is available in the ADC register
  int16_t sample = ADC; // 0-1023

  capture[samplePos] =
    ((sample > (512 - noiseThreshold)) &&
     (sample < (512 + noiseThreshold))) ? 0 :
    sample - 512; // Sign-convert for FFT; -512 to +511

  if (++samplePos >= FFT_N) {
    // The sample buffer is full, so disable the interrupt
    ADCSRA &= ~_BV(ADIE);
  }
*/
//****************************** регулируем яркость светодиодов.*******************************************
uint32_t adjustBrightness(uint32_t c, float amt) 
{
  // конвертируем 32 битного числа в R,G,B
  uint8_t r = (uint8_t)(c >> 16);
  uint8_t g = (uint8_t)(c >> 8);
  uint8_t b = (uint8_t)c;

  // изменяем яркость каждого цвета. умножаем на коэффициент
  r = r * amt;
  g = g * amt;
  b = b * amt;

  return strip.Color(r, g, b); //преобразуем из R,G,B в 32 битное число
}
//*******************************************************************************************************
void reset() 
{
  for (uint8_t i = 0; i < N_PEAKS; i++) 
  {
    peaks[i].age = 0;
    peaks[i].magnitude = 0;
  }
  for (uint8_t i = 0; i < MAX_COLOR_BARS; i++) 
  {
    colorBars[i] = UNUSED;
  }
}
//**************************Анализатор аудио семплов**************************************************************
void analyzeAudioSamples() 
{
  // The peaks in each band need to decay so that new peaks can happen later.
  // The value bandPeakDecay determines how fast they decay. Currently set to 6,
  // so every 6th time through the main loop, each band peak decays by 1.
  // If it decays too fast (original value was 3), then there are too many peaks
  // detected and display is messy. Too few and you'll miss some real audio peaks
  // that should be displayed.
  if (++bandPeakCounter >= bandPeakDecay) {
    bandPeakCounter = 0;
    for (uint8_t i = 0; i <= cutoffFreqBand; i++) {
      if (bandPeakLevel[i] > 0) {
        bandPeakLevel[i]--;
      }
    }
  }

#if ! DEBUG
  //Удалить шум и применить уровни эквалайзера. This is from the Piccolo project.
  for (uint8_t i = 0; i < FFT_N / 2; i++) 
  {
    noiseThreshold = pgm_read_byte(&noiseFloor[i]);
    if (spectrum[i] < noiseThreshold)
    {
      spectrum[i] = 0;
    } else {
      spectrum[i] -= noiseThreshold;
      uint8_t eqVal = pgm_read_byte(&eq[i]);
      if (eqVal > 0) {
        spectrum[i] = (  ( spectrum[i] * (256L - eqVal)) >> 8);
      }
    }
  }

  // Downsample spectrum output to 8 bands.
  newPeakFlags = 0;
  for (uint8_t i = 0; i <= cutoffFreqBand; i++) {
    data   = (uint8_t *)pgm_read_word(&bandWeights[i]);
    binNum = pgm_read_byte(&bandBinStarts[i]);
    nBins = pgm_read_byte(&bandBinCounts[i]);
    for (sum = 0, j = 0; j < nBins; j++) {
      sum += spectrum[binNum++] * pgm_read_byte(&data[j]);
    }
    band[i][bandCount] = sum / bandDiv[i];
    minLvl = maxLvl = band[i][0];
    for (j = 1; j < N_FRAMES; j++) { // Get range of prior frames
      if (band[i][j] < minLvl)      minLvl = band[i][j];
      else if (band[i][j] > maxLvl) maxLvl = band[i][j];
    }
    if ((maxLvl - minLvl) < 8) maxLvl = minLvl + 8;
    minLvlAvg[i] = (minLvlAvg[i] * 7 + minLvl) >> 3; // Dampen min/max levels
    maxLvlAvg[i] = (maxLvlAvg[i] * 7 + maxLvl) >> 3; // (fake rolling average)

    // Second fixed-point scale based on dynamic min/max levels:
    level = 5L * (band[i][bandCount] - minLvlAvg[i]) /
            (long)(maxLvlAvg[i] - minLvlAvg[i]);

    if (level < 0L)      c = 0;
    else if (level > 20) c = 20;
    else                 c = (uint8_t)level;

    if (c > bandPeakLevel[i]) {
      bandPeakLevel[i] = c; // New peak found for this frequency band.
      newPeakFlags = newPeakFlags | (1 << i); // set new peak flag for this band
    }
  }
#endif
  if (++bandCount >= N_FRAMES) bandCount = 0;

}
#define MAX_BG_BRIGHT 6000
uint32_t easyBGBright = 0;

void check4background(bool yes)
{

    if(yes)
    {
// #if DEBUG
//       Serial.println("***Bakgroung!");  
// #endif          
      easyBGBright++;
      if(++easyBGBright > MAX_BG_BRIGHT) easyBGBright = MAX_BG_BRIGHT;

      for (uint8_t i = 0; i < N_LEDS; i++)
#ifdef  PARM_POT_2
        strip.setPixelColor(i, pgm_read_dword(&BackColors[BackIndex]));
      strip.setBrightness(BackBrightness * easyBGBright / MAX_BG_BRIGHT);
#else
        strip.setPixelColor(i, BACKGROUND);
#endif
    }
    else easyBGBright = 0;  
}

//*******************************Визуализация**********************************************************
void doVisualization() 
{
  float ageScale, b;
  uint32_t color;
  uint8_t r;

  bool Silence = true;

  // Bright peaks emanating from center moving outward.
  if ((pattern == PATTERN_DANCE_PARTY) ||
      (pattern == PATTERN_SINGLE_DIR_DANCE_PARTY)) {

    for (uint8_t i = 0; i < N_PEAKS; i++) {
      if (peaks[i].magnitude > 0) {

        Silence = false;
        // peak is visually active

        // The age of the visual peak will be used to determine brightness.
        ageScale = (float)(1.0 - ((float)peaks[i].age / (float)MAX_AGE));

        // If brightness has been altered by a parameter, scale everything further.
        // Get the right color for this peak's band and adjust the brightness.
        color = adjustBrightness(getColor(peaks[i].baseColor, peaks[i].rnd), ageScale * brightnessScale);

        // Calculate horizontal position relative to starting point.
        // The first term is a value from 0.0 to 1.0 indicating speed.
        // It is a function of the peak's magnitude.
        // Minimum speed is 0.5 (127.0 / 255.0). Maximum is 1.0 (255.0 / 255.0);
        // So speed is how many LEDs does it "move" every step. Multiply this by
        // the peak's age to get the position.

        uint8_t pos;
        if (peaks[i].magnitude > 150) 
        {
          pos = peaks[i].age;
        } else {
                  pos = ((127 + (peaks[i].magnitude / 2)) / 255.0) * peaks[i].age;
                }

        switch (pattern) 
        {
          case PATTERN_DANCE_PARTY:
            // Draw one right of center and one left of center.
            strip.setPixelColor((N_LEDS / 2) + pos, color);
            strip.setPixelColor(((N_LEDS / 2) - 1) - pos, color);
            break;
          case PATTERN_SINGLE_DIR_DANCE_PARTY:
            // Draw pixel relative to 0
            strip.setPixelColor(pos, color);
            break;
        }

        // Increment age
        peaks[i].age++;

        // If too old, retire this peak, making it available again.
        if (peaks[i].age > MAX_AGE) 
        {
          peaks[i].magnitude = 0;
          continue;
        }

        if ((pos >= N_LEDS / 2) && (pattern != PATTERN_SINGLE_DIR_DANCE_PARTY)) 
        {
          // Off the edge of the strip. This peak can now be returned to the pool
          // of peak structures that are available for use.
          peaks[i].magnitude = 0;
        }
        if ((pos >= N_LEDS) && (pattern == PATTERN_SINGLE_DIR_DANCE_PARTY)) 
        {
          // Off the edge of the strip. This peak can now be returned to the pool
          // of peak structures that are available for use.
          peaks[i].magnitude = 0;
        }
      }
    }

    check4background(Silence);
    
    return;
  }


  // Pulse from center of strip.
  if (pattern == PATTERN_PULSE) 
  {
    uint8_t maxWidth = N_LEDS / 2;
    uint8_t sortedBand[8];
    uint8_t sortedVal[8];
    uint8_t v;
    for (uint8_t i = 0; i < N_BANDS; i++) 
    {
      sortedBand[i] = 255;
      sortedVal[i] = 255;
    }

    for (uint8_t i = 0; i < N_BANDS; i++) 
    {
      v = map(peaks[i].magnitude, 0, 255, 0, maxWidth);

      ageScale = (float)(1.0 - ((float)peaks[i].age / (float)MAX_AGE));
      v = v * ageScale;

      // Insert into sorted list of bands.
      for (j = 0; j < N_BANDS; j++) 
      {
        if (sortedVal[j] > v) 
        {
          for (uint8_t k = 7; k > j; k--) 
          {
            sortedVal[k] = sortedVal[k - 1];
            sortedBand[k] = sortedBand[k - 1];
          }
          sortedVal[j] = v;
          sortedBand[j] = i;
          break;
        }
      }

      // Increment age
      peaks[i].age++;

      // If too old, retire this peak, making it available again.
      if (peaks[i].age > MAX_AGE) 
      {
        peaks[i].magnitude = 0;
        continue;
      }
    }

    uint8_t i = 8;
    while (i > 0) 
    {
      i--;
      v = sortedVal[i];
      if (v == 0) continue;
      color = getColor(peaks[sortedBand[i]].baseColor, peaks[sortedBand[i]].rnd);
      b = 0.5 + (float)v / (float)(2 * maxWidth);

      for (j = 0; j < v; j++) {
        uint32_t c2 = adjustBrightness(color, b * (float)(1.0 - (float)j / (float)v));
        strip.setPixelColor((N_LEDS / 2) + j, c2);
        strip.setPixelColor(((N_LEDS / 2) - 1) - j, c2);
      }
    }
    return;
  }

  // Entire strip is same color displaying most recent peak color.
  // if (pattern == PATTERN_LIGHT_BAR) 
  // {

  //   uint32_t color;
  //   float ageScale;
  //   if (peaks[peakIndex].magnitude > 0) 
  //   {
  //     Silence = false;
      
  //     if (peaks[peakIndex].age == 0) 
  //     {
  //       byte baseColor = peaks[peakIndex].baseColor;
  //       // Since the light bar is so bright, scale down the max brightness
  //       // for this mode to avoid blindness.
  //       byte tmp = maxBrightness;
  //       float tmpF = brightnessScale;
  //       maxBrightness = maxBrightness >> 2; // divide by 4
  //       brightnessScale = maxBrightness / 255.0;
  //       for (uint8_t i = 0; i < N_LEDS; i++) {
  //         color = getColor(baseColor, random(255));
  //         strip.setPixelColor(i, color);
  //       }
  //       // restore brightness settings
  //       maxBrightness = tmp;
  //       brightnessScale = tmpF;
  //     } else {
  //       // Adjust brightness for age
  //       ageScale = (float)(1.0 - ((float)peaks[peakIndex].age / (float)MAX_AGE));
  //       uint32_t tmpColor ;
  //       if ((peaks[peakIndex].rnd % 2) == 0) 
  //       {
  //         // shift to right
  //         tmpColor = strip.getPixelColor(N_LEDS - 1);
  //         for (int p = (N_LEDS - 1); p >= 0; p--) 
  //         {
  //           if (p == 0) 
  //           {
  //             color = tmpColor;
  //           } else {
  //             color = strip.getPixelColor(p - 1);
  //           }
  //           color = adjustBrightness(color, ageScale);
  //           strip.setPixelColor(p, color);
  //         }
  //       } else {
  //               // shift to left
  //                tmpColor = strip.getPixelColor(0);
  //                 for (int p = 0; p < N_LEDS; p++) 
  //                 {
  //                   if (p == (N_LEDS - 1)) 
  //                   {
  //                     color = tmpColor;
  //                    } else {
  //                             color = strip.getPixelColor(p + 1);
  //                           }
  //           color = adjustBrightness(color, ageScale);
  //           strip.setPixelColor(p, color);
  //         }
  //       }

  //     }

  //     // age peak
  //     peaks[peakIndex].age++;

  //     if (peaks[peakIndex].age > MAX_AGE) 
  //     {
  //       peaks[peakIndex].magnitude = 0;
  //     }
  //   }

  //   check4background(Silence);
    
  //   return;
  // }

  // Visual peaks are assigned one of 15 color bars.
  if ((pattern == PATTERN_COLOR_BARS) || (pattern == PATTERN_COLOR_BARS2)) 
  {
    uint8_t k = 0;
    uint8_t oldest;
    uint8_t nbars;
    switch (N_LEDS) 
    {
      case 60:
      case 120:
        nbars = 15;
        break;
      case 140:
        nbars = 22;
        break;
    }
    if ((N_LEDS == 60) && (pattern == PATTERN_COLOR_BARS2)) 
    {
      nbars = nbars / 2;
    }
    for (uint8_t i = 0; i < N_PEAKS; i++) 
    {

      if ((peaks[i].magnitude > 0))
        Silence = false;
      
      if ((peaks[i].magnitude > 0) && (peaks[i].age == 0)) 
      {
        // New peak (age == 0).
        // Find an unused color bar.
        j = random(nbars);
        k = 0;
        oldest = j;
        while (k < nbars) {
          if (colorBars[j] == UNUSED) 
          {
            colorBars[j] = i;
            break;
          } else {
            if (peaks[colorBars[j]].age > peaks[colorBars[oldest]].age) 
            {
              oldest = j;
            }
          }
          // keep looking for an unused bar.
          if ((i % 2) == 0) 
          {
            j = (j + 1) % nbars;
          } else {
                   if (j == 0) 
                    {
                      j = nbars;
                     } else {
                              j = j - 1;
                            }
                  }
                  k++;
        }
        if (k == nbars) 
        {
          // did not find an unused color bar, so use oldest.
          colorBars[oldest] = i;
        }
      }
    }

    uint8_t maxWidth = N_LEDS / nbars;
    uint8_t width;
    for (uint8_t i = 0; i < nbars; i++) 
    {
      j = colorBars[i];
      if (j == UNUSED) continue;
      if (peaks[j].age == 0) 
      {
        // If peak is brand new, make it max brightness!
        ageScale = 1.0;
      } else {
              // Otherwise, in range [0.0,0.5] as function of age.
              ageScale = 0.5 * (float)(1.0 - ((float)peaks[j].age / (float)MAX_AGE));
              }

      color = adjustBrightness(getColor(peaks[j].baseColor, peaks[j].rnd), ageScale);
      if (peaks[j].age == MAX_AGE) 
      {
        // mark the assigned color bar as unused.
        colorBars[i] = UNUSED;
      }

      if (pattern == PATTERN_COLOR_BARS) 
      {
        width = maxWidth;
      } else {
        // Width is function of age.
        if (peaks[j].age < 5) 
        {
          // If young, force to max width
          width = maxWidth;
        } else {
          // Then shrink based on age.
          width = map(peaks[j].age - 5, 0, MAX_AGE - 4, maxWidth, 1);
          width -= width % 2; // force width to be multiple of 2
        }
      }

      for (j = 0; j < (width / 2); j++) 
      {
        // Adjust brightness based on how far LED is from "center" of the bar.
        // Brighter at center, dimmer outside.
        color = adjustBrightness(color, (float)(1.0 - (float)((j * 2) / width)));

        k = (i * maxWidth) + ((maxWidth / 2) - 1 - j);
        strip.setPixelColor(k % N_LEDS, color);
        k = k + 1 + (j * 2);
        strip.setPixelColor(k % N_LEDS, color);
      }

    }

    // Age all the peaks.
    for (uint8_t i = 0; i < N_PEAKS; i++) 
    {
      if (peaks[i].magnitude > 0) 
      {
        peaks[i].age++;
        if (peaks[i].age > MAX_AGE) 
        {
          peaks[i].magnitude = 0;
        }
      }
    }

    check4background(Silence);

    return;
  }

  if ((pattern == PATTERN_FLASHBULBS) || (pattern == PATTERN_FIREFLIES)) 
  {
    uint8_t pos, width;
    for (uint8_t i = 0; i < N_PEAKS; i++) 
    {
      if (peaks[i].magnitude > 0) 
      {
        Silence = false;
        
        if (peaks[i].age == 0) 
        {
          // If peak is brand new, make it max brightness!
          ageScale = 1.0;
        } else {
          // Otherwise, in range [0.0,0.5] as function of age.
          ageScale = 0.5 * (float)(1.0 - ((float)peaks[i].age / (float)MAX_AGE));
        }

        if ((peaks[i].age < 3) && (pattern == PATTERN_FLASHBULBS)) 
        {
          // make it white for the first 3 times through the loop
          color = adjustBrightness(strip.Color(maxBrightness, maxBrightness, maxBrightness), ageScale);
        } else {
          color = adjustBrightness(getColor(peaks[i].baseColor, peaks[i].rnd), ageScale);
        }
        pos = (peaks[i].rnd * 7) % N_LEDS;
        if (pattern == PATTERN_FIREFLIES) 
        {
          uint8_t delta = (min(255, 64 + peaks[i].magnitude) / 255.0) * peaks[i].age;
          if (pos % 2 == 0) 
          {
            pos += delta;
          } else {
            pos -= delta;
          }
        }
        if (pos < N_LEDS) 
        {
          strip.setPixelColor(pos, color);
        }
      }
    }

    // Age all the peaks.
    for (uint8_t i = 0; i < N_PEAKS; i++) 
    {
      if (peaks[i].magnitude > 0) 
      {
        peaks[i].age++;
        if (peaks[i].age > MAX_AGE) 
        {
          peaks[i].magnitude = 0;
        }
      }
    }

    check4background(Silence);
    
    return;
  }

}

//**********************// Настройки АЦП для работы с прерыванием после преобразования**********************
void setADCFreeRunning() 
{
  analogReference(EXTERNAL);
  samplePos = 0; // Reset sample counter
  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion
   // 00 AREF, 01 AVcc с конденсатором на AREF, 10 Резерв, 11 Внутренний 1.1В источник конденсатором на AREF
  ADMUX  = ADC_CHANNEL; // Channel sel, right-adj, use AREF pin
  //ADMUX |= _BV(REFS0); //внутрений источник опрного напряжений 5V  // 1.1В (1 << REFS1)|(1 << REFS0)
  // Start a conversion. We want to discard the first conversion after
  // changing the voltage reference reference.
  ADCSRB = 0;
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC);   // ADC start

  while (ADCSRA & _BV(ADSC)); // ждем завершения преобразования

  // Now start auto-triggered conversions with interrupt enabled.
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC)  | // ADC start
           _BV(ADATE) | // Auto trigger
           _BV(ADIE)  | // Interrupt enable
           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
}

void setADCDefault() // Конфигурируем АЦП для стандартной работы ардуино
{
  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion
  analogReference(DEFAULT);
  //ADMUX  = 0;
  ADMUX = _BV(REFS0); //внутрений источник опрного напряжений 5V  // 1.1В (1 << REFS1)|(1 << REFS0)
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
  ADCSRB = 0;                // Free run mode, no high MUX bit
  analogRead(PARM_POT); // отказаться от первого преобразования
}

//***********************// Выбор конфигурации ленты****************************************************
void setConfig() 
{
  switch (ledConfig)
  {
    case 0:
      N_LEDS = 60;
      break;
    case 1:
      N_LEDS = 120;
      break;
    case 2:
      N_LEDS = 140;
      break;
  }
}

//*******************************************************************************************************
// определяем, как долго ждать, пока текущая визуализация завершится
// поэтому мы можем перейти к новой визуализации
uint8_t determineWaitTime() 
{
  uint8_t youngest = MAX_AGE;
  if (pattern == PATTERN_LIGHT_BAR) 
  {
    youngest = peaks[peakIndex].age;
  } 
  else
  {
    for (uint8_t i = 0; i < N_PEAKS; i++) 
    {
      if (peaks[i].magnitude > 0) 
      {
        if (peaks[i].age < youngest) 
        {
          youngest = peaks[i].age;
        }
      }
    }
  }
  return (MAX_AGE - youngest) + 3;
}

//*******************************************************************************************************
// Get a random color. There are 2 random colors that change over
// time depending on randColorChangeParm.
// This gives a 2-color scheme that changes over time.
uint8_t getRandomBaseColor(uint8_t r)
{
  if ((r % 2) == 0) 
  {
    if (randColorChangeParm > 0) 
    {
      if (randColorCount[0]-- == 0) 
      {
        randColorCount[0] = 31 - randColorChangeParm;
        randColor[0] = random(255);
      }
    }
    return randColor[0];
  } else {
    if (randColorChangeParm > 0) 
    {
      if (randColorCount[1]-- == 0) 
      {
        randColorCount[1] = 31 - randColorChangeParm;
        randColor[1] = random(255);
      }
    }
    return randColor[1];
  }

}

//*******************************************************************************************************
// Map a color index in range [0-255] to a color.
// This is a uniform colorspace over 8 base colors:
// [red, orange, yellow, green, cyan, blue, magenta, white]
uint32_t getColor(uint8_t index, uint8_t rnd) 
{
  byte r, g, b;

  // advance the color by the random amount for variation
  if (colorMode != COLOR_RANDOM) 
  {
    index += (rnd >> 4);
  } else {
    index += (rnd >> 5); // not as much random variation in 2-color scheme mode
  }
  byte f = (index % 32) * 8; // fractional part of color component
  switch (index / 32) 
  {
    case 0: // red
      r = 255;
      g = f >> 1;
      b = 0;
      break;
    case 1: // orange
      r = 255;
      g = 127 + (f >> 1);
      b = 0;
      break;
    case 2: // yellow
      r = 255 - f;
      g = 255;
      b = 0;
      break;
    case 3: // green
      r = 0;
      g = 255;
      b = f;
      break;
    case 4: // cyan
      r = 0;
      g = 255 - f;
      b = 255;
      break;
    case 5: // blue
      r = f;
      g = 0;
      b = 255;
      break;
    case 6: // magenta
      r = 255 - f / 2;
      g = f / 2;
      b = 255 - f / 2;
      break;
    case 7: // white
      r = 127 + f / 2;
      g = 127 - f / 2;
      b = 127 - f / 2;
      break;
  }

  if (maxBrightness < 255) 
  {
    // scale by brightnessScale;
    // optimize for 255 and 0 cases to avoid floating point multiply
    if (r == 255) 
    {
      r = maxBrightness;
    } else {
      if (r > 0) 
      {
        r = ((float)r * brightnessScale);
      }
    }
    if (g == 255) 
    {
      g = maxBrightness;
    } else {
      if (g > 0) 
      {
        g = ((float)g * brightnessScale);
      }
    }
    if (b == 255)
    {
      b = maxBrightness;
    } else {
      if (b > 0) 
      {
        b = ((float)b * brightnessScale);
      }
    }
  }
  return strip.Color(r, g, b);
}

//*******************************************************************************************************
// Map the band peak level to a magnitude in range [0,255].
// This mapping depends on band number because different bands have different
// characteristics.
uint8_t getMagnitude(uint8_t band, uint8_t peakValue) {
  uint8_t peakMin, peakMax;
  switch (band) 
  {
    case 0:
      // bass bands are really strong, so don't show anything with peak value
      // smaller than 8. They can have high values, so anything bigger than 20 is
      // maximum magnitude.
      peakMin = 8;
      peakMax = 20;
      break;
    case 1:
      peakMin = 8;
      peakMax = 20;
      break;
    case 2: // fall through...
    case 3:
    case 4:
    case 5:
      // These middle bands need a little more sensitivity, so allow values as low
      // as 4 to be represented visually.
      // These bands never go that high, so let's say that value of 8 is the high end.
      peakMin = 4;
      peakMax = 8;
      break;
    case 6:
    case 7:
      // High bands are not so sensitive and go a bit higher in practice.
      peakMin = 6;
      peakMax = 12;
      break;
  }

  if (pattern == PATTERN_PULSE) 
  {
    // This mode needs to be more sensitive.
    if (band != 0) 
    {
      peakMin = 1;
    } else {
      peakMin = 3;
    }
  }

  // If below min, then return 0 so we don't do anything visually.
  if (peakValue < peakMin) 
  {
    return 0;
  }
  // Constrain to range [peakMin,peakMax]
  peakValue = constrain(peakValue, peakMin, peakMax);

  // Map to range [1,255] for all bands.
  return map(peakValue, peakMin, peakMax, 1, 255);
}

//********************************смотрим сколько RAM доступно***********************************************************************
// Выделяем память до тех пор, пока она не заполнится. И возвращаем размер буфера
int getMemory() 
{
  int size = 2000;
  byte *buf;
  while ((size > 0) && ((buf = (byte *) malloc(--size)) == NULL));
  free(buf);
  return size;
}

//***********// Определим, была ли в EEPROM когда-либо сохранена конфигурация*******************************************************
// Если была, тогда  в адресах [0,1] должно быть магическое число.
boolean EEPROMValid() 
{
  unsigned int magic = EEPROM.read(0);
  magic = magic << 8;
  magic |= EEPROM.read(1);
  return (magic == EEPROM_MAGIC_NUMBER);
}

//*********************** Запись магического числа по адресу [0,1].*****************************************************************
void setEEPROMValid() 
{
  EEPROM.write(0, EEPROM_MAGIC_NUMBER >> 8);
  EEPROM.write(1, (EEPROM_MAGIC_NUMBER & 0xFF));
}

//******************************сохранить настройки в EEPROM*****************************************************************
void saveConfig()
{
  setEEPROMValid();
  EEPROM.write(2, ledConfig);
  EEPROM.write(3, mode);
  EEPROM.write(4, maxBrightness);
  EEPROM.write(5, colorMode);
  EEPROM.write(6, cutoffFreqBand);
 // EEPROM.write(7, NoiseThreshold);
}

//*******************************************************************************************************
void setColorMode() 
{
  byte tmp = colorMode;
  colorMode++;
  if (colorMode >= N_COLOR_MODES) 
  {
    colorMode = 0;
  }
  strip.clear();
  strip.setPixelColor(colorMode, strip.Color(0, 64, 64));
  strip.show();

  msdelay(BTTN_DELAY);
  strip.clear();
  saveConfig();
}

//*******************************************************************************************************
void setCutoffFreqBand() // настройки частот среза, потенциометром PARAM
{
  setADCDefault(); //настроить АЦП на стандартный режим
  uint16_t reading1 = map(analogRead(PARM_POT), 0, 1024, 0, 8); //прочитать положение потенциометра (PARAM), всего 8 значений
  uint8_t tmpColorMode = colorMode;
  colorMode = COLOR_BAND;

  while(buttons != get_buttons())
  {
    strip.clear();                          //очистить всю ленту
    for (uint8_t i = 0; i <= cutoffFreqBand; i++) 
    {
      strip.setPixelColor(i, getColor(i * 32, 0)); // выбрать комбинация светодиодов на ленте
    }
      //включить все выбраные в функции setPixelColor светодиоды в ленте
#if USE_REMOTE        
      if(irrecv.isIdle()) 
#endif
      { strip.show(); }
    
    uint16_t reading2 = map(analogRead(PARM_POT), 0, 1024, 0, 8);//прочитать новое положение потенциометра (PARAM), всего 8 значений
    if (reading2 != reading1)  //сравнить значения потенциометра PARAM
    {
      reading1 = reading2;
      cutoffFreqBand = reading2;
    }
  }

  colorMode = tmpColorMode;

  saveConfig();
  msdelay(BTTN_DELAY);
  strip.clear();
}

#ifdef  PARM_POT_2
void setBackgroundColor()
{
  setADCDefault();

  for (uint16_t i = 0; i < N_LEDS; i++)
      strip.setPixelColor(i, pgm_read_dword(&BackColors[BackIndex]));
  strip.setBrightness(BackBrightness);
  strip.show();

  uint16_t ReadResult = analogRead(PARM_POT_2);
  if (ReadResult <= 100)
  {
    while((btJUMP + btCOLOR) != get_buttons())
    {
      ReadResult = map(analogRead(PARM_POT_2), 0, 1024, 0, 255);
      for (uint16_t i = 0; i < N_LEDS; i++)
        strip.setPixelColor(i, pgm_read_dword(&BackColors[BackIndex]));
      strip.setBrightness(ReadResult);
#if USE_REMOTE        
      if(irrecv.isIdle()) 
#endif
      { strip.show(); }
    }

    BackBrightness = ReadResult;
    EEPROM.write(BACK_BRIGHT_EEPROM_ADR, ReadResult);
  }
  else
  {
    while((btJUMP + btCOLOR) != get_buttons())
    {
      ReadResult = map(analogRead(PARM_POT_2), 0, 1024, 0, BACKGROUND_COUNT);
      for (uint16_t i = 0; i < N_LEDS; i++)
        strip.setPixelColor(i, pgm_read_dword(&BackColors[ReadResult]));
#if USE_REMOTE        
      if(irrecv.isIdle()) 
#endif
      { strip.show(); }
    }

    BackIndex = ReadResult;
    EEPROM.write(BACK_COLOR_EEPROM_ADR, BackIndex);
  }
  
  msdelay(BTTN_DELAY);
  strip.clear();
}
#endif

//******************************Конфигурация*************************************************************************
void configure() 
{
  boolean buttonReleased = false;//статус кнопки COLOR
  if ((digitalRead(COLOR_BUTTON_PIN) == LOW) && (digitalRead(PATTERN_BUTTON_PIN) == HIGH)) // если нажата кнопка COLOR  при подаче питания
  {
    delay(20); // debounce
    setADCDefault(); //перевести АЦП в стандартный режим

    while ((!buttonReleased) || (digitalRead(COLOR_BUTTON_PIN) == HIGH)) //если нажата кнопка COLOR
     {
      if (digitalRead(COLOR_BUTTON_PIN) == HIGH) 
      {
        buttonReleased = true;
        delay(20); // debounce
      }

      maxBrightness = analogRead(PARM_POT) >> 2; //настройки максимальной яркости потенциометром
      brightnessScale = maxBrightness / 255.0; //расчитываем коэффициент яркости

      strip.clear();
      for (uint8_t i = 0; i < N_BANDS; i++) 
      {
        strip.setPixelColor(i, getColor(i * 32, 0)); //отображаем настройки яркости светодиодов визуально
      }
      strip.show();
    }
    strip.clear();

    setADCFreeRunning(); //настроить АЦП в режим работы по прерыванию
    saveConfig(); // сохранить конфигурацию
    delay(20); // debounce
    while (digitalRead(COLOR_BUTTON_PIN) == LOW); //ждем отпускания кнопки COLOR
    return;
  }

//Если при включении питания нажата кнопка pattern, то выбираем конфигурацию ленты 60,120 или 140 светодиодов
  if ((digitalRead(PATTERN_BUTTON_PIN) == LOW) && (digitalRead(COLOR_BUTTON_PIN) == HIGH)) 
  {
    delay(20); // debounce
    // Configure the ADC (analog to digital converter) to be in normal Arduino mode:
    setADCDefault();//перевести АЦП в стандартный режим
    buttonReleased = false;
    while ((!buttonReleased) || (digitalRead(PATTERN_BUTTON_PIN) == HIGH)) //если нажата кнопка PATTERN
    {
      if (digitalRead(PATTERN_BUTTON_PIN) == HIGH) 
      {
        buttonReleased = true;
        delay(20); // debounce
      }
      
      ledConfig = map(analogRead(PARM_POT), 0, 1024, LEDCONFIG_60, LEDCONFIG_140+1); //читаем состояние потенциометра

      strip.clear();
      for (uint8_t i = 0; i <= ledConfig; i++) 
      {
        strip.setPixelColor(i, strip.Color(64, 0, 0)); //индикация режима 1,2 или 3 светодиод
       }
      strip.show();
    }
    strip.clear();

    setADCFreeRunning(); //настроить АЦП в режим работы по прерыванию
    saveConfig();  // сохранить конфигурацию
    delay(20); // debounce
    while (digitalRead(PATTERN_BUTTON_PIN) == LOW); //  //ждем отпускания кнопки PATTERN
    return;
  }

  
//если при включении питания нажаты обе кнопки, то вызов настроек уровня срабатывания АЦП
  if ((digitalRead(COLOR_BUTTON_PIN) == LOW) && (digitalRead(PATTERN_BUTTON_PIN) == LOW)) 
  {
    delay(20); // debounce
    test();
    return;
  }
}

//*******************************************************************************************************
void test() //диагностика ленты при первом включении питания и настройка уровня срабатывания АЦП
{

  for (uint8_t i = 0; i < N_LEDS; i++) // бегущий огонь белого цвета от начала ленты до последнего светодиода
  {
    strip.clear(); //погасить всю ленту
    strip.setPixelColor(i, 64, 64, 64);
    strip.show(); //загрузить комбинацию в светодиодную ленту
    delay(10); //задержка между переключениями
  }

  strip.clear(); //очистить всю ленту
  strip.show(); //применить
 
}
