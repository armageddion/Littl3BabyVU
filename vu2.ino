/*
 * VU: Old-skool green and red from bottom
 */

void vu2() {

  uint8_t i = 0;
  uint8_t *peak;                    // Pointer variable declaration
  uint16_t height = auxReading();

  if (height > *peak)
    *peak = height;                 // Keep 'peak' dot at top

  // Color pixels based on old school green/red vu
  for (uint8_t i = 0; i < NUM_PIXELS; i++) {
    if (i >= height)
    {
      ledsLeft[i] = CRGB::Black;
      ledsRight[i] = CRGB::Black;
    }
    else if (i > NUM_PIXELS - (NUM_PIXELS / 3)) 
    {
      ledsLeft[i] = CRGB::Red;
      ledsRight[i] = CRGB::Red;
    }
    else 
    {
      ledsLeft[i] = CRGB::Green;
      ledsRight[i] = CRGB::Green;
    }
  }

  // Draw peak dot
  if (*peak > 0 && *peak <= NUM_PIXELS - 1)
    if (*peak > NUM_PIXELS - (NUM_PIXELS / 3)) 
    {
      ledsLeft[*peak] = CRGB::Red;
      ledsRight[*peak] = CRGB::Red;
    }
    else 
    {
      ledsLeft[*peak] = CRGB::Green;
      ledsRight[*peak] = CRGB::Green;
    }
  
  dropPeak();
  averageReadings();
  FastLED.show();
}