///////////////////////////////////////////////////////////////////////// PIXEL SETUP
#include "FastLED.h"
#define NUM_LEDS 12
#define DATA_PIN 3
CRGB leds[NUM_LEDS]; // Define the array of leds

#define COL_RED			0xFF0000
#define COL_ORANGE	0xFF2800
#define COL_YELLOW	0xFF8F00
#define COL_GREEN		0x00FF00
#define COL_BLUE		0x0000FF
#define COL_PURPLE	0xB600FF
#define COL_WHITE		0xFFFF7F
#define COL_BLACK		0x000000



#define NUM_FLAGS 7
bool flagsPresent[NUM_FLAGS] = {0,0,0,0,0,0,0};

#define TEAM_RED 			0
#define TEAM_ORANGE 	1
#define TEAM_YELLOW		2
#define TEAM_GREEN		3
#define TEAM_BLUE			4
#define TEAM_PURPLE		5
#define TEAM_WHITE		6


uint32_t flagCols[NUM_FLAGS] = {COL_RED, COL_ORANGE, COL_YELLOW, COL_GREEN, COL_BLUE, COL_PURPLE, COL_WHITE};


///////////////////////////////////////////////////////////////////////// IR SETUP
#include <IRLibDecodeBase.h> // First include the decode base
#include <IRLib_P01_NEC.h>   // Now include only the protocols you wish
#include <IRLibCombo.h>     // After all protocols, include this

#include <IRLibRecvPCI.h> 

IRrecvPCI myReceiver(2); //create receiver and pass pin number
IRdecode myDecoder;   //create decoder




#define IRCODE_RED			0x1010101A
#define IRCODE_ORANGE		0x1020202A
#define IRCODE_YELLOW		0x1040404A
#define IRCODE_GREEN		0x1080808A
#define IRCODE_BLUE			0x1101010A
#define IRCODE_PURPLE		0x1202020A
#define IRCODE_WHITE		0x1404040A



void setup() 
{
	Serial.begin(9600);
	// delay(2000); while (!Serial); //delay for Leonardo
	myReceiver.enableIRIn(); // Start the receiver
	Serial.println(F("Ready to receive IR signals"));

	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(5,120);

	for (int i = 0; i < NUM_LEDS; ++i)
		leds[i] = COL_WHITE;

	leds[0] = flagCols[TEAM_RED];
	leds[2] = flagCols[TEAM_ORANGE];
	leds[4] = flagCols[TEAM_YELLOW];
	leds[6] = flagCols[TEAM_GREEN];
	leds[8] = flagCols[TEAM_BLUE];
	leds[10] = flagCols[TEAM_PURPLE];

	FastLED.show();
}

void loop() 
{
	//Continue looping until you get a complete signal received
	if (myReceiver.getResults()) 
	{
		myDecoder.decode();           //Decode it
		// myDecoder.dumpResults(true);  //Now print results. Use false for less detail

		switch(myDecoder.value) 
		{
			case IRCODE_RED:
				Serial.println(F("RED"));
				flagsPresent[TEAM_RED] = 1;
				break;
			case IRCODE_ORANGE:
				Serial.println(F("ORANGE"));
				flagsPresent[TEAM_ORANGE] = 1;
				break;
			case IRCODE_YELLOW:
				Serial.println(F("YELLOW"));
				flagsPresent[TEAM_YELLOW] = 1;
				break;
			case IRCODE_GREEN:
				Serial.println(F("GREEN"));
				flagsPresent[TEAM_GREEN] = 1;
				break;
			case IRCODE_BLUE:
				Serial.println(F("BLUE"));
				flagsPresent[TEAM_BLUE] = 1;
				break;
			case IRCODE_PURPLE:
				Serial.println(F("PURPLE"));
				flagsPresent[TEAM_PURPLE] = 1;
				break;			
			case IRCODE_WHITE:
				Serial.println(F("WHITE"));
				flagsPresent[TEAM_WHITE] = 1;
				break;		
			default:
				break;
		} 


		for (int i = 0; i < NUM_FLAGS; ++i)
			Serial.print(flagsPresent[i]);
		Serial.println(F(" "));

		setPixelCols();


		myReceiver.enableIRIn();      //Restart receiver
	}
}

void setPixelCols()
{
	unsigned int parts = 0;
	bool dodgyFive = false;

	for (int i = 0; i < NUM_FLAGS; ++i)
		parts += flagsPresent[i];

	if (parts == 5)
	{
		dodgyFive = true;
		parts++;
	}


	for (int x = 0; x < parts; ++x)
	{
		for (int y = 0; y < NUM_LEDS/parts; ++y)
		{
			if (dodgyFive & x == 5)
				leds[y+x*(NUM_LEDS/parts)] = COL_BLACK;
			else
				leds[y+x*(NUM_LEDS/parts)] = flagCols[getFlagCol(x)];
		}
	}

	FastLED.show();
	return;
}

unsigned int getFlagCol(unsigned int position)
{
	unsigned int count = 0;
	for (int i = 0; i < NUM_FLAGS; ++i)
	{
		if (flagsPresent[i] == 1)
		{
			
			if (count == position)
				return i;

			count++;
		}
	}

	return TEAM_WHITE;
}

