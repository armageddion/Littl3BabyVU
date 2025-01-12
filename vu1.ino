/*
 * VU: Rainbow from bottom or middle, green through purple
 */

void vu1() {
  
  CRGB* leds;
  uint8_t i = 0;
  uint8_t *peak;      // Pointer variable declaration
  uint16_t height = auxReading();

  //leds = ledsLeft;    // Store address of peak in peak, then use *peak to
  //peak = &peak;   // access the value of that address
  
  // Draw vu meter part
  fill_solid(ledsLeft, NUM_PIXELS, CRGB::Black);
  fill_solid(ledsRight, NUM_PIXELS, CRGB::Black);
  // Fill with color gradient
  fill_gradient(ledsLeft, 0, CHSV(96, 255, 255), NUM_PIXELS - 1, CHSV(224, 255, 255),SHORTEST_HUES);
  fill_gradient(ledsRight, 0, CHSV(96, 255, 255), NUM_PIXELS - 1, CHSV(224, 255, 255),SHORTEST_HUES);
  
  //Black out end
  for (i = 0; i < NUM_PIXELS; i++) {
    if(i >= height) ledsLeft[i] = CRGB::Black;
    if(i >= height) ledsRight[i] = CRGB::Black;
  }
  
  // Draw peak dot  
  if(height > *peak)
    *peak = height; // Keep 'peak' dot at top
    
  if(*peak > 0 && *peak <= NUM_PIXELS-1)
    ledsLeft[*peak] = CHSV(rainbowHue2(*peak, NUM_PIXELS), 255, 255); // Set peak colour correctly
    ledsRight[*peak] = CHSV(rainbowHue2(*peak, NUM_PIXELS), 255, 255); // Set peak colour correctly

  dropPeak();
  averageReadings();
  FastLED.show();
}

uint8_t rainbowHue2(uint8_t pixel, uint8_t num_pixels) {
  uint8_t hue = 96 - pixel * (145 / num_pixels);
  return hue;
}