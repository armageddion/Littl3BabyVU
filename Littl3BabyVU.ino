#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#include <EEPROM.h>

#define DPIN_MIC 3                  // Arduino pin 3 = the digital output pin of the Microphone board (D0)
#define APIN_MIC A0                 // AUX input from mic
#define PIN_LED_1 8                 // One LED strip
#define PIN_LED_2 9                 // Another LED strip
#define PIN_LED 13                  // Arduino default LED 
#define NUM_PIXELS 30               // Number of pixels per LED strip
#define NOISE 1                     // Noise/hum/interference in aux signal [10]
#define DC_OFFSET 0                 // DC offset in aux signal [0]
#define SAMPLES 60                  // Length of buffer for dynamic level adjustment [60]
#define PEAK_FALL 20                // Rate of peak falling dot [20]
#define TOP (NUM_PIXELS + 2)
#define COLOR_ORDER GRB             // Colour order of LED strip [GRB]
#define LED_TYPE WS2812B            // LED string type [WS2812B]
#define MAX_MILLIAMPS 500          // Maximum current to draw [500]

// ------------------
// -- Audio Vars --
// ------------------
int sensorValue = 0;                // Var to hold sensor value
uint8_t volCnt = 0;                 // Frame counter for storing past volume data
int vol[SAMPLES];                   // Collection of prior volume samples
uint8_t minCnt = 0;
int mins[SAMPLES];
uint8_t maxCnt = 0;
int maxs[SAMPLES];
int val = 0;                        // 'val' is used to store the digital microphone value
int sigMinAvg = 0;                  // For dynamic adjustment of graph low & high
int sigMaxAvg = 1024;
int sigAvg = 0;                     // should be around 512
int sigLvl = 0;                     // Current "dampened" audio level

Adafruit_NeoPixel pixels_1 = Adafruit_NeoPixel(NUM_PIXELS, PIN_LED_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_2 = Adafruit_NeoPixel(NUM_PIXELS, PIN_LED_2, NEO_GRB + NEO_KHZ800);

CRGB ledsLeft[NUM_PIXELS];
CRGB ledsRight[NUM_PIXELS];
void vu1();

// CUSTOM COLORS
const uint32_t colores[] = {
  pixels_1.Color(255, 0, 255),      //pink
  pixels_1.Color(29, 219, 191),     //light blue
  pixels_1.Color(255, 0, 0),        //red
  pixels_1.Color(49, 245, 96)       //mostly green
};
uint32_t black = pixels_1.Color(0, 0, 0);

void setup ()
{
  Serial.begin(9600);
  pinMode(APIN_MIC, INPUT) ;        // configure analog 'mic' as input pin
  pinMode(DPIN_MIC, INPUT) ;        // configure digital 'mic' as input pin
  pinMode(PIN_LED, OUTPUT) ;        // configure 'led' as output pin

  //pinMode(PIN_LED_1, OUTPUT);  
  //pinMode(PIN_LED_2, OUTPUT);   

  //pixels_1.begin();  // This initializes the NeoPixel library
  //pixels_2.begin();

  FastLED.addLeds < LED_TYPE, PIN_LED_1, COLOR_ORDER > (ledsLeft, NUM_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds < LED_TYPE, PIN_LED_2, COLOR_ORDER > (ledsRight, NUM_PIXELS).setCorrection(TypicalLEDStrip);
  //FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);

  Serial.println("testing strips");
  //paintStrip(pixels_1,23);
  //paintStrip(pixels_2,23);
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels_1.fill(colores[4],0,i);
    pixels_2.fill(colores[4],0,i);
    pixels_1.show();
    pixels_2.show();
    delay(50);
  }
  Serial.println("getting avg signal");
  uint16_t height = auxReading();
  dropPeak();
  averageReadings();

  delay(1000);
  Serial.println("clearing strips");
  for (int i = NUM_PIXELS; i >= 0; i--)
  {
    pixels_1.fill(black,i,NUM_PIXELS-i);
    pixels_2.fill(black,i,NUM_PIXELS-i);
    pixels_1.show();
    pixels_2.show();    
    delay(50);
  }
  delay(5000);
}
 
void loop ()
{
  sensorValue = analogRead(APIN_MIC);
  val = digitalRead(DPIN_MIC);      // read value
  if (val == HIGH)    // if he value is high then light the LED or else do not light the LED
  {
    digitalWrite (PIN_LED, HIGH);
    Serial.print(sensorValue);
    Serial.print(", HIGH, ");
  }
  else
  {
    digitalWrite (PIN_LED, LOW);
    Serial.print(sensorValue);
    Serial.print(", LOW , ");
  }
  Serial.print(sigMinAvg);
  Serial.print(" <-> ");
  Serial.print(sigMaxAvg);
  Serial.print(", "); 
  Serial.print(sigAvg);   

  uint16_t height = auxReading();
  Serial.print(", ");
  Serial.println(height);
  
  dropPeak();
  averageReadings();

  vu1();                            // run animation

  //delay(500); //slow things down a little bit... no need for epilepsy
}

void paintStrip(Adafruit_NeoPixel strip, int amplitude)
{
  strip.clear();
  if (amplitude > 30)
  { 
    amplitude = 30;
  }
  if (amplitude > 20)
  {
    for (int i = 1; i <= 20; i++)
    {
      strip.fill(colores[0],0,i);
      strip.show();
      delay(25);
    }
    if (amplitude > 25)
    {
      for (int j = 20; j <= 25; j++)
      {
        int count = j - 19;
        strip.fill(colores[1],20,count);
        strip.show();
        delay(25);
      }
      for (int k = 25; k <= amplitude; k++)
      {
        int count = k - 24;
        strip.fill(colores[2],25,count);
        strip.show();
        delay(25);
      }
    }
    else
    {
      for (int j = 20; j < amplitude; j++)
      {
        int count = j - 19;
        strip.fill(colores[1],20,count);
        strip.show();
        delay(25);
      }
    }
  }
  else
  {
    for (int i = 1; i < amplitude; i++)
    {
      strip.fill(colores[0],0,i);
      strip.show();
      delay(25);
    }
  }
  return;
}

// ------------------
// -- VU functions --
// ------------------

uint16_t auxReading() {

  int n = 0;
  uint16_t height = 0;

  n = analogRead(APIN_MIC);          // Raw reading from line in
  //n = abs(n - 512 - DC_OFFSET); // Center on zero
  
  vol[volCnt] = n;                   // Save sample for dynamic leveling
  volCnt = ++volCnt % SAMPLES;

  n = abs(n - sigAvg - DC_OFFSET);   // Center on zero
  //Serial.println("");
  //Serial.print("auxReading: ");
  //Serial.println(n);
  n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
  sigLvl = ((sigLvl * 7) + n) >> 3;  // "Dampened" reading else looks twitchy (>>3 is divide by 8)
  //Serial.println(sigLvl);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  //height = TOP * (sigLvl - sigMinAvg) / (long)(sigMaxAvg - sigMinAvg);
  height = TOP * n / (long)(sigMaxAvg - sigMinAvg);
  //Serial.println(height);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = constrain(height, 0, TOP); 
  return height;
}

/*
 * Function for dropping the peak
 */
uint8_t peak;
void dropPeak() {
  
  static uint8_t dotCount;
 
  if(++dotCount >= PEAK_FALL) {      //fall rate 
    if(peak > 0) peak--;
    dotCount = 0;
  }
}

/*
 * Function for averaging the sample readings
 */
void averageReadings() {

  uint16_t minLvl, maxLvl;
  int sampleSum = 0;

  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  minLvl = maxLvl = vol[0];
  for (int i = 1; i < SAMPLES; i++) {
    if (vol[i] < minLvl) minLvl = vol[i];
    else if (vol[i] > maxLvl) maxLvl = vol[i];
    sampleSum = sampleSum + vol[i];
  }
  sigAvg = sampleSum/(SAMPLES-1);
  //if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  
  //sigMinAvg = (sigMinAvg * 63 + minLvl) >> 6; // Dampen min/max levels
  //sigMaxAvg = (sigMaxAvg * 63 + maxLvl) >> 6; // (fake rolling average)

  mins[minCnt] = minLvl;             // Save sample for dynamic leveling
  minCnt = ++minCnt % SAMPLES;  
  maxs[maxCnt] = maxLvl;             // Save sample for dynamic leveling
  maxCnt = ++maxCnt % SAMPLES;

  int minsSum = 0;
  int maxsSum = 0;
  for (int i = 1; i < SAMPLES; i++) {
    minsSum += mins[i];
    maxsSum += maxs[i];
  }
  sigMinAvg = minsSum/(SAMPLES-1);
  sigMaxAvg = maxsSum/(SAMPLES-1);
  if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
}