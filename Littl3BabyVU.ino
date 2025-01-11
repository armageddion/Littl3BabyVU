#include <Adafruit_NeoPixel.h>

#define DPIN_MIC 3                  // Arduino pin 3 = the digital output pin of the Microphone board (D0)
#define APIN_MIC A0                 // AUX input from mic
#define PIN_LED_1 8                 // One LED strip
#define PIN_LED_2 9                 // Another LED strip
#define PIN_LED 13                  // Arduino default LED 
#define NUM_PIXELS 30               // Number of pixels per LED strip
#define NOISE 20                    // Noise/hum/interference in aux signal [10]
#define DC_OFFSET 0                 // DC offset in aux signal [0]
#define SAMPLES 60                  // Length of buffer for dynamic level adjustment [60]
# define PEAK_FALL 20               // Rate of peak falling dot [20]
#define TOP (NUM_PIXELS + 2)

// ------------------
// -- Audio Vars --
// ------------------
int sensorValue = 0;                // Var to hold sensor value
uint8_t volCnt = 0;                 // Frame counter for storing past volume data
int vol[SAMPLES];                   // Collection of prior volume samples
int val = 0;                        // 'val' is used to store the digital microphone value
int sigMinAvg = 1024;               // For dynamic adjustment of graph low & high
int sigMaxAvg = 0;
int sigAvg = 0;                    //should be around 512
int sigLvl = 0;                    // Current "dampened" audio level


Adafruit_NeoPixel pixels_1 = Adafruit_NeoPixel(NUM_PIXELS, PIN_LED_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_2 = Adafruit_NeoPixel(NUM_PIXELS, PIN_LED_2, NEO_GRB + NEO_KHZ800);

// CUSTOM COLORS
const uint32_t colores[] = {
  pixels_1.Color(255, 0, 255),   //pink
  pixels_1.Color(29, 219, 191),  //light blue
  pixels_1.Color(255, 0, 0),  //red
  pixels_1.Color(49, 245, 96)   //mostly green
};
uint32_t black = pixels_1.Color(0, 0, 0);

void setup ()
{
  Serial.begin(9600);
  pinMode(APIN_MIC, INPUT) ;
  pinMode(DPIN_MIC, INPUT) ;     // configure 'mic' as input pin
  pinMode(PIN_LED, OUTPUT) ;     // configure 'led' as output pin

  pinMode(PIN_LED_1, OUTPUT);  
  pinMode(PIN_LED_2, OUTPUT);   

  pixels_1.begin();  // This initializes the NeoPixel library
  pixels_2.begin();

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
  for (int i = 0; i < 20; i++)
  {
    sensorValue = analogRead(APIN_MIC);
    Serial.println(sensorValue);
    sigAvg += sensorValue;
    delay(500);
  }
  sigAvg = sigAvg/20;
  Serial.print("AverageSignal = ");
  Serial.println(sigAvg);
  delay(1000);
  Serial.println("clearing strips");
  for (int i = NUM_PIXELS; i >= 0; i--)
  {
    pixels_1.fill(black,i,NUM_PIXELS);
    pixels_2.fill(black,i,NUM_PIXELS);
    pixels_1.show();
    pixels_2.show();    
    delay(50);
  }
  delay(5000);
}
 
void loop ()
{
  sensorValue = analogRead(APIN_MIC);
  if (sensorValue < sigMinAvg)
  {
    sigMinAvg = sensorValue;
  }
  if (sensorValue > sigMaxAvg)
  {
    sigMaxAvg = sensorValue;
  }
  val = digitalRead(DPIN_MIC);     // read value
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
    Serial.print(", LOW, ");
  }
  Serial.print(sigMinAvg);
  Serial.print(" <-> ");
  Serial.print(sigMaxAvg);
  Serial.print(", ");
  Serial.println(sigMaxAvg);

  pixels_1.clear();
  pixels_1.show();
  pixels_2.clear();
  pixels_2.show();
  // set pixels 1
  //menial sensor value 515
  if (sensorValue > (sigAvg+1))
  {
    int amplitude = (sensorValue - sigAvg)*2.5;
    //paintStrip(pixels_1, amplitude);
    if (amplitude > 20)
    {
      for (int i = 1; i <= 20; i++)
      {
        pixels_1.fill(colores[0],0,i);
        pixels_1.show();
      }
      if (amplitude > 25)
      {
        for (int j = 20; j <= 25; j++)
        {
          pixels_1.fill(colores[1],20,(j-19));
          pixels_1.show();
        }
        for (int k = 25; k <= amplitude; k++)
        {
          pixels_1.fill(colores[2],25,(k-24));
          pixels_1.show();
        }
      }
      else
      {
        for (int j = 20; j < amplitude; j++)
        {
          pixels_1.fill(colores[1],20,(j-19));
          pixels_1.show();
        }
      }
    }
    else
    {
      for (int i = 1; i <= amplitude; i++)
      {
        pixels_1.fill(colores[0],0,i);
        pixels_1.show();
      }
    }      
  }
  else if (sensorValue < (sigAvg-1))
  {
    int amplitude = (sigAvg - sensorValue)*2.5;
    //paintStrip(pixels_2, amplitude);
    if (amplitude > 20)
    {
      for (int i = 1; i <= 20; i++)
      {
        pixels_2.fill(colores[0],0,i);
        pixels_2.show();
      }
      if (amplitude > 25)
      {
        for (int j = 20; j <= 25; j++)
        {
          pixels_2.fill(colores[1],20,(j-19));
          pixels_2.show();
        }
        for (int k = 25; k <= amplitude; k++)
        {
          pixels_2.fill(colores[2],25,(k-24));
          pixels_2.show();
        }
      }
      else
      {
        for (int j = 20; j < amplitude; j++)
        {
          pixels_2.fill(colores[1],20,(j-19));
          pixels_2.show();
        }
      }
    }
    else
    {
      for (int i = 1; i < amplitude; i++)
      {
        pixels_2.fill(colores[0],0,i);
        pixels_2.show();
      }
    }      
  }
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

/*
 * Function for averaging the sample readings
 */
uint16_t auxReading(uint8_t channel) {

  int n = 0;
  uint16_t height = 0;

  n = analogRead(APIN_MIC); // Raw reading from line in
  n = abs(n - 512 - DC_OFFSET); // Center on zero
  n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
  sigLvl = ((sigLvl * 7) + n) >> 3; // "Dampened" reading else looks twitchy (>>3 is divide by 8)
  vol[volCnt] = n; // Save sample for dynamic leveling
  volCnt = ++volCnt % SAMPLES;
  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (sigLvl - sigMinAvg) / (long)(sigMaxAvg - sigMinAvg);

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = constrain(height, 0, TOP);
  return height;
}

/*
 * Function for dropping the peak
 */
uint8_t peak;
void dropPeak(uint8_t channel) {
  
  static uint8_t dotCount;
 
  if(++dotCount >= PEAK_FALL) { //fall rate 
    if(peak > 0) peak--;
    dotCount = 0;
  }
}

/*
 * Function for averaging the sample readings
 */
void averageReadings() {

  uint16_t minLvl, maxLvl;

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
  }
  if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  
  sigMinAvg = (sigMinAvg * 63 + minLvl) >> 6; // Dampen min/max levels
  sigMaxAvg = (sigMaxAvg * 63 + maxLvl) >> 6; // (fake rolling average)
}