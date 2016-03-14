#include <math.h>
#include <LiquidCrystal.h>


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int caliTime = 5;        // time in seconds for PIRs to calibrate
float luxValue = 80.0;   // cutoff point at which the LED turns off/on. Above = off/Below = on
float brightnessSteps = 255.0; // steps in an 8-bit output
float Res = 3.3;        // resistance used that is connected to photocell

const int cells = 3;      // number of units
int photocellPin[] = {0, 1, 2};   // the photocell connected to analog in
int LED[] = {6, 9, 10}; // led pins write analog
int PIR[] = {7, 8, 13}; // pir pins read digital
unsigned long TIMER[cells] = {};   // timer array to be called for each i
int lux[cells];
int volt[cells];
int lcdPixels[cells] = {0, 6, 11};


unsigned long ledinterval = 2000;   // time for LED code to run
unsigned long lcdTimer;

void setup(void) {

  Serial.begin(9600);
  lcd.begin(16, 2);
  for (int i = 0; i < cells; i++) {

    pinMode(PIR[i], INPUT);    // initialize PIR pins as input
    pinMode(LED[i], OUTPUT);   // initialize LED pins as output
    digitalWrite(PIR[i], LOW); // initialize pin state as low
  }

  lcd.setCursor(0, 0);
  lcd.print("Calibrating");
  lcd.setCursor(0, 1);

  for (int i = 0; i < caliTime; i++) {  // calibration for pir

    lcd.print(".");

    delay(1000);

  }

  lcd.clear();
  lcd.print("SENSOR ACTIVE");
  delay(1000);

}

int gfunc(int x, int c) {                                                   // insert x into Gaussian function
  return pow(2, -((pow(x, 2)) / .4)) * c;                               // x is cell number, c is multiplier to change maximum of the function - dependent on lux
}

void togglelcd ()
{
  lcd.begin(16, 2);
  for (int i = 0; i < cells; i++) {

    lcd.setCursor(lcdPixels[i], 0);
    lcd.print("lux");
    lcd.setCursor(lcdPixels[i], 1);
    lcd.print(lux[i]);
    lcdTimer = millis ();
  }
}
void loop(void) {

  for ( int i = 0; i < cells; i++) {
    int photocellRead = analogRead(photocellPin[i]);
    float Vout = photocellRead * 0.0087890625;
    lux[i] = ((4500 / Vout) - 500) / Res;
    volt[i] = abs(lux[i]  * (brightnessSteps / luxValue) - brightnessSteps);
  }

  int totallux = 0;
  for ( int i = 0; i < cells; i++) {
    totallux += lux[i];
  }

  int averagelux = totallux / 3;

      
  if (averagelux < luxValue) {                            // if lux is below a cutoff point run code for pirs, otherwise do nothing

    int activeLength = 0;
    int active[cells];

    for (int i = 0; i < cells; i++) {

      unsigned long ontimer = millis () - TIMER[i];            // when pir[i] detects movement, call timer [i] to reset ontimer

      if (digitalRead(PIR[i]) == HIGH) {
        TIMER[i] = millis();                                 // see comment above

      }

      if (ontimer <= ledinterval) {
        active[activeLength] = i;                            // add i to active[]

        activeLength++;                                    // add 1 to length for each activated sensor
      }
      else if (i == cells - 1 && activeLength == 0) {
        for (int c = 0; c < cells; c++) {
          analogWrite(LED[c], 0);                           // turn off leds
        }
      }

    }

    if (activeLength > 0) {
      for (int i = 0; i < activeLength; i++) {
        if (i == 0) { // if i is the first active sensor in the array for all activated sensors
          analogWrite(LED[active[i]], volt[i]); // set to 100% light
          if (active[i] != 0) {   // if there are any lights before this one in the hallway then Gaussian function
            for (int c = active[i] - 1; c >= 0; c--) {
              analogWrite(LED[c], gfunc(active[i] - c, volt[c]));   // write to all leds below i
            }
          }
          if (activeLength > 1) {   // if there's more than one active sensor, activate lights halfway up to next motion sensor
            if ((active[i] + active[i + 1]) % 2 == 1) {   // if the halfway is odd
              for (int c = active[i] + 1; c < ceil((active[i] + active[i + 1]) / 2); c++) {   // round up halfway number and turn on every light in between using Gaussian function
                analogWrite(LED[c], gfunc(c - active[i], volt[c]));
              }
            }
            else {    // if the halway number is even do almost same as above, except no need to round up/down
              for (int c = active[i] + 1; c < (active[i] + active[i + 1]) / 2; c++) {
                analogWrite(LED[c], gfunc(c - active[i], volt[c]));
              }
            }
          }
          if (activeLength == 1) {    // if there are no other active sensors
            for (int c = active[i] + 1; c < cells; c++) {   // turn on all leds after the current i while still dimming to a certain amount using Gaussian function
              analogWrite(LED[c], gfunc(c - active[i], volt[c]));
            }
          }
        }
        else if (i == activeLength - 1) {   // if it's the last active sensor in the active array
          analogWrite(LED[active[i]], volt[i]);   // self to 100%
          for (int c = active[i] + 1; c < cells; c++) { // activate all lights after this.
            analogWrite(LED[c], gfunc(c - active[i], volt[c]));
          }
          if ((active[i] + active[i - 1]) % 2 == 1) {   // if odd
            for (int c = active[i] - 1; c > floor((active[i] + active[i - 1]) / 2); c--) { // round down the halfway number
              analogWrite(LED[c], gfunc(active[i] - c, volt[c]));
            }
          }
          else {    // if even
            for (int c = active[i] - 1; c > (active[i] + active[i - 1]) / 2; c--) {
              analogWrite(LED[c], gfunc(active[i] - c, volt[c]));
            }
          }
        }
        else {    // if the current active sensor is neither the first nor the last in the active array
          analogWrite(LED[active[i]], volt[i]); // self to 100%
          if ((active[i] + active[i + 1]) % 2 == 1) {   // if odd
            for (int c = active[i] + 1; c < ceil((active[i] + active[i + 1]) / 2); c++) {   // round up
              analogWrite(LED[c], gfunc(c - active[i], volt[c]));
            }
          }
          else { // If even...
            for (int c = active[i] + 1; c < (active[i] + active[i + 1]) / 2; c++) {
              analogWrite(LED[c], gfunc(c - active[i], volt[c]));
            }
          }
          if ((active[i] + active[i - 1]) % 2 == 1) {   // if odd
            for (int c = active[i] - 1; c > floor((active[i] + active[i - 1]) / 2); c--) { // round down
              analogWrite(LED[c], gfunc(active[i] - c, volt[c]));
            }
          }
          else { // if even
            for (int c = active[i] - 1; c > (active[i] + active[i - 1]) / 2; c--) {
              analogWrite(LED[c], gfunc(active[i] - c, volt[c]));
            }
          }
        }
      }
    }
  }
  else {
    for (int i = 0; i < cells; i++) {
      analogWrite(LED[i], 0);
    }
  }

  if ((millis () - lcdTimer) >=  1000)   //toggle LCD
    togglelcd ();

  delay(10);
}



