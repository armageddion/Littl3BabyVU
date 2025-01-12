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
#define PEAK_FALL 2                 // Rate of peak falling dot [20]
#define TOP (NUM_PIXELS + 2)
#define COLOR_ORDER GRB             // Colour order of LED strip [GRB]
#define LED_TYPE WS2812B            // LED string type [WS2812B]
#define MAX_MILLIAMPS 500           // Maximum current to draw [500]

// ------------------
// -- Audio Vars --
// ------------------
int sensorValue = 0;                // Var to hold sensor value
uint8_t volCnt = 0;                 // Frame counter for storing past volume data
int vol[SAMPLES];                   // Collection of prior volume samples
uint8_t minCnt = 0;                 // Frame counter for storing past minimums
int mins[SAMPLES];                  // Collection of prior minimums
uint8_t maxCnt = 0;                 // Frame counter for storing past maximums
int maxs[SAMPLES];                  // Collection of prior maximums
int val = 0;                        // 'val' is used to store the digital microphone value
int sigMinAvg = 0;                  // For dynamic adjustment of graph low & high
int sigMaxAvg = 1024;
int sigAvg = 0;                     // should be around 512
int sigLvl = 0;                     // Current "dampened" audio level

CRGB ledsLeft[NUM_PIXELS];
CRGB ledsRight[NUM_PIXELS];
void vu1();                         // Rainbow graphics
void vu2();                         // Old school green and red graphics

void setup ()
{
  Serial.begin(9600);
  pinMode(APIN_MIC, INPUT) ;        // configure analog 'mic' as input pin
  pinMode(DPIN_MIC, INPUT) ;        // configure digital 'mic' as input pin
  pinMode(PIN_LED, OUTPUT) ;        // configure 'led' as output pin

  FastLED.addLeds < LED_TYPE, PIN_LED_1, COLOR_ORDER > (ledsLeft, NUM_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds < LED_TYPE, PIN_LED_2, COLOR_ORDER > (ledsRight, NUM_PIXELS).setCorrection(TypicalLEDStrip);
  //FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
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
  Serial.print(", ");
  Serial.print(sigLvl);     

  uint16_t height = auxReading();
  Serial.print(", ");
  Serial.println(height);
  
  dropPeak();
  averageReadings();

  vu2();                            // run animation
}

// ------------------
// -- VU functions --
// ------------------

uint16_t auxReading() {

  int n = 0;
  uint16_t height = 0;

  n = analogRead(APIN_MIC);         // Raw reading from line in
  
  vol[volCnt] = n;                  // Save sample for dynamic leveling
  volCnt = ++volCnt % SAMPLES;

  n = abs(n - sigAvg - DC_OFFSET);  // Center on zero
  //Serial.println("");
  //Serial.print("auxReading: ");
  //Serial.println(n);
  n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
  sigLvl = ((sigLvl * 7) + n) >> 3; // "Dampened" reading else looks twitchy (>>3 is divide by 8)
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
 
  if(++dotCount >= PEAK_FALL) {     //fall rate 
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

  mins[minCnt] = minLvl;            // Save sample for dynamic leveling
  minCnt = ++minCnt % SAMPLES;  
  maxs[maxCnt] = maxLvl;            // Save sample for dynamic leveling
  maxCnt = ++maxCnt % SAMPLES;

  int minsSum = 0;
  int maxsSum = 0;
  for (int i = 1; i < SAMPLES; i++) {
    minsSum += mins[i];
    maxsSum += maxs[i];
  }
  sigMinAvg = minsSum/(SAMPLES-1);  // Calculate running average of minimums
  sigMaxAvg = maxsSum/(SAMPLES-1);  // and maximums
  if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
}