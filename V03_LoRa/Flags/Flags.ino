/* 
Author: Samuel Eames
Created: 2018-01-20

Code used to intermittently send out IR codes from within capture the flag flags.
Received by base stations to know which flags are at them at any given time.
*/

/* 
// Note - currently setup to run on A23 battery
 * A23 batteries have a capacity around 55mAh
 * System draws about 23mA in the main loop
 * This works out to be roughly 2hrs 23mins run time
 * I would have liked if this was longer, but oh well... I'll try and fix this for rev 2!

Battery Life = Battery Capacity in mAh / Load Current in mAh
*/ 


#include <IRLib.h>
#include <FastLED.h>					// Pixel LED

IRsendNEC My_Sender;

uint32_t timer_IR;  
uint32_t timer_LED;  
#define LED_PERIOD 2000

//////////////////////////////////////////////////
#define TEAM_BLUE	   // ENTER TEAM HERE!! //
//////////////////////////////////////////////////
// TODO - Update this section to be more general like in BaseStation doc where multiple flags can have the same colour, etc
#ifdef TEAM_RED
	#define REPEATS 	610000
	#define IRCODE 	0x1010101A
#endif

#ifdef TEAM_ORANGE
	#define REPEATS 	590000
	#define IRCODE 	0x1020202A
#endif

#ifdef TEAM_YELLOW
	#define REPEATS 	530000
	#define IRCODE 	0x1040404A
#endif

#ifdef TEAM_GREEN
	#define REPEATS 	470000
	#define IRCODE 	0x1080808A
#endif

#ifdef TEAM_BLUE
	#define REPEATS 	430000
	#define IRCODE 	0x1101010A
#endif

#ifdef TEAM_PURPLE
	#define REPEATS 	410000
	#define IRCODE 	0x1202020A
#endif

#ifdef TEAM_WHITE
	#define REPEATS 	670000
	#define IRCODE 	0x1404040A
#endif

const uint32_t IR_FlagCodes[7] = {
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
CRGB leds[NUM_LEDS]; // Define the array of leds
#define LED_DATA_PIN 2				// Pixel LED String


// Use following two variables together e.g. teamcols_Hue(HSV_RED)
enum HSV_Colours:uint8_t { HSV_RED, HSV_ORANGE, HSV_YELLOW, HSV_GREEN, HSV_CYAN, HSV_BLUE, HSV_PURPLE, HSV_MAGENTA };
const uint8_t teamCols_Hue[8] =	{ 0, 18, 64, 96, 128, 160, 192, 230 };

#define HSV_SFULL	255
#define HSV_VFULL 255
#define HSV_VDIM	35

const uint8_t HSV_MYCOL = teamCols_Hue[HSV_RED];



//////////////////////////////////////////////////
void setup()
{
	// Setup LEDs
	FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS); 
	// FastLED.setMaxPowerInVoltsAndMilliamps(5,200); 						// Limit total power draw of LEDs to 200mA at 5V
	fill_solid(leds, NUM_LEDS, CHSV(HSV_MYCOL, HSV_SFULL, HSV_VFULL)); 
	FastLED.show();

	delay(1000);

	fill_solid(leds, NUM_LEDS, CRGB::Black); 
	FastLED.show();

	// Ensure code is sent straight away
	My_Sender.send(IRCODE);


}
 
//////////////////////////////////////////////////
void loop() 
{
	// TODO - update this to be more random on time intervals
	if (timer_IR >= REPEATS)
	{
		My_Sender.send(IRCODE);
		timer_IR = 0;

	}
	else
		timer_IR++;
}

