#include <Adafruit_NeoPixel.h>

#define PIN_LED_1 8
#define PIN_LED_2 9

#define NUM_PIXELS 30

int led= 13;     // 'led' is the Arduino onboard LED
int mic= 3;     // 'mic' is the Arduino pin 3 = the digital output pin of the Microphone board (D0)
int val = 0;     // 'val' is used to store the digital microphone value
const int sensorPin = A0;
int sensorValue = 0;
int signalMin = 1024;
int signalMax = 0;
int signalAvg = 0; //should be around 514

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
  pinMode(sensorPin, INPUT) ;
  pinMode (led, OUTPUT) ;     // configure 'led' as output pin
  pinMode (mic, INPUT) ;     // configure 'mic' as input pin

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
    sensorValue = analogRead(sensorPin);
    Serial.println(sensorValue);
    signalAvg += sensorValue;
    delay(500);
  }
  signalAvg = signalAvg/20;
  Serial.print("AverageSignal = ");
  Serial.println(signalAvg);
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
  sensorValue = analogRead(sensorPin);
  if (sensorValue < signalMin)
  {
    signalMin = sensorValue;
  }
  if (sensorValue > signalMax)
  {
    signalMax = sensorValue;
  }
  val = digitalRead(mic);     // read value
  if (val == HIGH)    // if he value is high then light the LED or else do not light the LED
  {
    digitalWrite (led, HIGH);
    Serial.print(sensorValue);
    Serial.print(", HIGH, ");
  }
  else
  {
    digitalWrite (led, LOW);
    Serial.print(sensorValue);
    Serial.print(", LOW, ");
  }
  Serial.print(signalMin);
  Serial.print(" <-> ");
  Serial.print(signalMax);
  Serial.print(", ");
  Serial.println(signalAvg);

  pixels_1.clear();
  pixels_1.show();
  pixels_2.clear();
  pixels_2.show();
  // set pixels 1
  //menial sensor value 515
  if (sensorValue > (signalAvg+1))
  {
    int amplitude = (sensorValue - signalAvg)*2.5;
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
  else if (sensorValue < (signalAvg-1))
  {
    int amplitude = (signalAvg - sensorValue)*2.5;
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