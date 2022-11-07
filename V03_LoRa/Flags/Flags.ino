/* 
Author: Samuel Eames
Created: 2018-01-20

Various updates since then

COMPILE NOTES: Made to run on ATTINY85
 * Use digispark by digistump --> set Arduino to compile to that
 * Use DigisparkIRLib
 * Targetting the digispark doesn't seem to include the Arduino EEPROM.h file so manually add this to the working directory


Code used to intermittently send out IR codes from within capture the flag flags.
Received by base stations to know which flags are at them at any given time.

BATTERY NOTES
 * Currently setup to run on A23 battery
 * A23 batteries have a capacity around 55mAh
 * System draws about 23mA in the main loop
 * This works out to be roughly 2hrs 23mins run time
 * I would have liked if this was longer, but oh well... I'll try and fix this for rev 2!

Battery Life = Battery Capacity in mAh / Load Current in mAh


WIRING
 * Button      --> P0
 * IR Output   --> P1
 * Pixels      --> P2


USAGE
 * Hold down button when powering on to enter setup mode
      * LED will flash white three times to indicate you've entered setup mode
      * Press button to select flag colour (Red, Orange, Yellow, Green, Cyan, Blue, Magenta, Pink, White)
      * Hold button for a second once colour is chosen
      * LED will flash white three times again to indicate colour has been set
      * Press the button to select the flag ID number
      * Hold the button for a second once the flag number is chosen
      * LED will flash white three times again to indicate number has been set
      * This value is stored to internal memory for next time the unit is booted
      * LED will then flash in the colour set the number of times according to the ID
 * 


*/ 


#include <IRLib.h>
#include <Adafruit_NeoPixel.h>
#include "EEPROM.h"

IRsendNEC IR_TX;                 // Instanciate IR transmitter 


// Delays taking into account IRLib runs CPU without a prescaler
// delay(1000000) = 15s --> I believe IR_Send changes CPU prescaler somewhere
// 1000000 / 15000 = 67
#define DELAY_SCALE 67

// Fed into random function
#define IR_TX_INTERVAL_MIN 800      // Min ms intervall to send IR codes on
#define IR_TX_INTERVAL_MAX 1600     // Max ms intervall to send IR codes on

const uint32_t IR_FlagCodes[] = {
                                    0x1010101A,
                                    0x1020202A,
                                    0x1040404A,
                                    0x1080808A,
                                    0x1101010A,
                                    0x1202020A,
                                    0x1404040A
                                 };

////////////////////////////////////////////////// Pixel LED Setup
#define NUM_LEDS 4
#define LED_DATA_PIN 2           // Pixel LED String

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);                       

// Use following two variables together e.g. teamcols_Hue(RED)
enum Colours:uint8_t { BLACK, RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, MAGENTA, WHITE};
const uint32_t teamCols[] =   {  0x000000,   // Black
                                 0xFF0000,   // Red
                                 0xFF1C00,   // Orange
                                 0xFF6F00,   // Yellow
                                 0x00FF0A,   // Green
                                 0x00AAAA,   // Cyan
                                 0x0000FF,   // Blue
                                 0x8F00FF,   // Purple
                                 0xFF002F,   // Magenta
                                 0xFFFFFF    // White
                              };


////////////////////////////////////////////////// Misc
#define BTN_PIN 0
#define DEBOUNCE 100       // (ms) Button debounce time
#define BTN_PRESS_LONG 1000   // (ms) Min time for a 'long' button press
uint8_t myFlagCol;
uint8_t myFlagID;

#define MAX_FLAG_ID    10  // Number of unique flag IDs

enum EEPROM_DATA:uint8_t { DATA_COL, DATA_ID }; // locations in EEPROM that data is saved to


//+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+// Flag Colour & IR Code selection
// See IR_FlagCodes (above) for options
// IR Codes should be unique, but flag colours can be repeated (e.g. one team owns multiple flags)
#define MY_IRCODE PURPLE   // Range 0-7                  
#define MY_COLOUR PURPLE
//+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+/+// 


//////////////////////////////////////////////////
void setup()
{
   randomSeed(analogRead(4));    // Seed random number generator
   pinMode(BTN_PIN, INPUT_PULLUP);

   pixels.begin();

   // Get currently saved flag ID
   myFlagCol = EEPROM.read(DATA_COL);
   myFlagID  = EEPROM.read(DATA_ID);


   // Check for setup
   if (!digitalRead(BTN_PIN))
   {
      flashLED(WHITE, 3, 100, 10);

      // Set colour
      while (setFlagColID(myFlagCol, sizeof(teamCols)))
         fill_solid(teamCols[myFlagCol]);

      // Set ID Num
      while(setFlagColID(myFlagID, MAX_FLAG_ID))
         flashLED_NB(teamCols[myFlagCol], myFlagID, 100, 500);

      // Save the things
      EEPROM.write(DATA_COL, myFlagCol);
      EEPROM.write(DATA_ID, myFlagID);

      // Continue startup
   }


   // Flash LEDs according to flag ID

   // Finish with flag lit up (solid) colour
}
 
//////////////////////////////////////////////////
void loop() 
{
   // NOTE: Calling this seems to break pixel control, delay() and millis() :( 
   // I haven't yet worked out how to get around that
   IR_TX.send(IR_FlagCodes[MY_IRCODE]);

   delay(random(IR_TX_INTERVAL_MIN, IR_TX_INTERVAL_MAX) * DELAY_SCALE);
}

void flashLED(uint8_t flashCol, uint8_t flashNum, uint8_t flashTime, uint8_t pause)
{
   // Flashes pixel LEDs given colour, number of flashes, and timing (ms)
   for (uint8_t i = 0; i < flashNum; ++i)
   {
      fill_solid(flashCol);
      delay(flashTime);
      fill_solid(BLACK);
      delay(flashTime);
   }

   delay(pause);

   return;
}


bool flashLED_NB(uint8_t flashCol, uint8_t flashNum, uint8_t flashTime, uint8_t pause)
{
   // Flashes pixel LEDs given colour, number of flashes, and timing (ms)
   // NON-BLOCKING --> call function a lot!


   static uint8_t i;
   static uint32_t startTime = millis();

   if (i < flashNum*2)
   {
      if (millis() - startTime >= flashTime)
      {
         if (i % 2)
            fill_solid(flashCol);
         else
            fill_solid(BLACK);

         startTime = millis();
         i++;
      }
   }
   else
   {
      if (millis() - startTime >= pause)        // Reset variables
      {
         i = 0;
         startTime = millis();
         return true;
      }
   }


   return false;
}




bool setFlagColID(uint8_t &value, uint8_t max)
{
   // Increments given value, resetting to 0 when max is achieved
   // Returns true if button held for longer then BTN_PRESS_LONG, otherwise returns false

   static bool lastBtnState = true;                // Buttons use pull-up resistors, so high is default state

   // Debounce buttons
   static uint32_t lasttime;                        // used for debouncing
   static uint32_t pressedTime = millis();                     // used to measure button press length

   if (millis() < lasttime)                         // Millis() wrapped around - restart timer
      lasttime = millis();

   if ((lasttime + DEBOUNCE) > millis())            // Debounce timer hasn't elapsed - don't continue
      return false; 

   lasttime = millis();                             // Debouncing complete; record new time & continue


   bool btnState = digitalRead(BTN_PIN);            // Read button state


   if (!btnState && lastBtnState != btnState)      // If button just pressed
   {
      value++;                                     // Increment value
      if (value >= max)                            // Ensure we're within bounds
         value = 0;

      pressedTime = millis();
   }
   else if (!btnState && !lastBtnState && (millis() - pressedTime > BTN_PRESS_LONG))
   {
      // Reset values
      pressedTime = millis();
      lastBtnState = true;
      return true;
   }


   lastBtnState = btnState;                        // Remember button state for next time


   return false;
}



void fill_solid(uint8_t fillcolour)
{
   // Light all the LEDs the same colour & show
   for (uint8_t i = 0; i < NUM_LEDS; ++i)
      pixels.setPixelColor(i, teamCols[fillcolour]);
   pixels.show();

   return;
}

