#include <TinyPICO.h>

// Initialise the TinyPICO library
TinyPICO tp = TinyPICO();


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  tp.DotStar_SetPixelColor( 0, 255, 0 );
  delay(1000);  
  tp.DotStar_SetPixelColor( 0,0,255 );
  delay(1000);  
}
