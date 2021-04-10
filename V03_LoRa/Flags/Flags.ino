/* 
Author: Samuel Eames
Created: 2018-01-20

Code used to intermittently send out IR codes from within capture the flag flags.
Received by base stations to know which flags are at them at any given time.
*/

/* BATTERY NOTES
 * Currently setup to run on A23 battery
 * A23 batteries have a capacity around 55mAh
 * System draws about 23mA in the main loop
 * This works out to be roughly 2hrs 23mins run time
 * I would have liked if this was longer, but oh well... I'll try and fix this for rev 2!

Battery Life = Battery Capacity in mAh / Load Current in mAh
*/ 


#include <IRLib.h>
#include <Adafruit_NeoPixel.h>

IRsendNEC IR_TX;						// Instanciate IR transmitter 


// Delays taking into account IRLib runs CPU without a prescaler
// delay(1000000) = 15s --> I believe IR_Send changes CPU prescaler somewhere
// 1000000 / 15000 = 67
#define DELAY_SCALE 67

// Fed into random function
#define IR_TX_INTERVAL_MIN 800		// Min ms intervall to send IR codes on
#define IR_TX_INTERVAL_MAX 1600		// Max ms intervall to send IR codes on

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
#define NUM_LEDS 5
#define LED_DATA_PIN 2				// Pixel LED String

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);								

// Use following two variables together e.g. teamcols_Hue(RED)
enum Colours:uint8_t { RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, MAGENTA };
const uint32_t teamCols[] = 	{	0xFF0000,	// Red
											0xFF1C00,	// Orange
											0xFF6F00,	// Yellow
											0x00FF0A,	// Green
											0x00AAAA,	// Cyan
											0x0000FF,	// Blue
											0x8F00FF,	// Purple
											0xFF002F		// Magenta
										};

////////////////////////////////////////////////// Flag COlour & IR Code selection
// See IR_FlagCodes (above) for options
// IR Codes should be unique, but flag colours can be repeated (e.g. one team owns multiple flags)
#define MY_IRCODE 1					
const uint8_t MYCOL = RED;


//////////////////////////////////////////////////
void setup()
{
	// Seed random number generator
	randomSeed(analogRead(4));

	// Setup LEDs
	pixels.begin();
	fill_solid(MYCOL);

	delay(1000);

	// Turn off again to save battery power
	pixels.setPixelColor(0, pixels.Color(0,0,0));
	pixels.show();
}
 
//////////////////////////////////////////////////
void loop() 
{
	// NOTE: Calling this seems to break pixel control, delay() and millis() :( 
	// I haven't yet worked out how to get around that
	IR_TX.send(IR_FlagCodes[MY_IRCODE]);

	delay(random(IR_TX_INTERVAL_MIN, IR_TX_INTERVAL_MAX) * DELAY_SCALE);
}


void fill_solid(uint8_t fillcolour)
{
	// Light all the LEDs the same colour & show
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		pixels.setPixelColor(i, teamCols[fillcolour]);
	pixels.show();

	return;
}

