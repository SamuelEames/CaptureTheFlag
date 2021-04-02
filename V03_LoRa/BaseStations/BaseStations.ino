//////////////////////////////////////////////////////////////////////////////
//////							 		~ CTF-IR-LORA ~								//////
////// 		Code based off multiple example projects 							//////
//////		Brought to gether by S.Eames											//////
//////		See README for more detail info										//////
//////////////////////////////////////////////////////////////////////////////


// Include the things
#include <RH_RF95.h>					// LoRa Driver
#include <RHReliableDatagram.h>
#include "CommonVars.h"				// Extra local variables
#include <FastLED.h>					// Pixel LED
#include <EEPROM.h>					// EEPROM used to store (this) station ID

#define debugMSG	1				// Uncomment to get debug / error messages in console


enum team:uint8_t { TEAM_RED, TEAM_ORANGE, TEAM_YELLOW, TEAM_GREEN, TEAM_BLUE, TEAM_MAGENTA, TEAM_WHITE};



// ARDUINO PIN SETUP
#define RFM95_CS 		18				// LoRa pins
#define RFM95_RST 	10
#define RFM95_INT 	0

#define BVOLT_PIN		A6				// Analog pin for battery voltage measurement
#define BUZZ_PIN		5				// Buzzer
#define LED_DATA_PIN 6				// Pixel LED String

#define IR_REC_PIN	1					// IR Receiver

// BATTERY PARAMETERS
#define LIPO_MINV  	9600		//(mV) 9.6V - minimum voltage with a little safety factor
#define LIPO_MAXV 	12600 	//(mV) 12.6V - fully charged battery 



///////////////////////////////////////////////////////////////////////// PIXEL SETUP
#include "FastLED.h"
#define NUM_LEDS 20
CRGB leds[NUM_LEDS]; // Define the array of leds


const uint16_t LEDUpdate_Period = 500;
uint16_t LEDUpdate_StartTime;



///////////////////////////////////////////////////////////////////////// FLAG related variables
/* OK, here's how I think it works

(1) Flags transmit their IR code roughly every 3 seconds
(2) Every time a flag is sensed, FlagsSensed is copied (AND) onto FlagsPresent
(3) Every 10 seconds, FlagsPresent is set to equal FlagsSensed, then FlagsSensed is reset to 0

*/

#define NUM_FLAGS 7

uint8_t FlagsPresent = 0b00000000; 			// Used to update LEDs, but lags to change after flag is removed
uint8_t FlagsSensed  = 0b00000000;			// Sensed flags are written here, then copied to FlagsPresent

// Colours associated with flags (match order with IR_FlagCodes array below)
const uint32_t flagCols[NUM_FLAGS] = {COL_RED, COL_ORANGE, COL_YELLOW, COL_GREEN, COL_BLUE, COL_MAGENTA, COL_WHITE};

// TODO Update this later to be set by hex BCD switch or EEPROM
uint8_t MY_TEAM; 			// 'Background' colour to set LEDs on base station

const uint16_t FlagReset_Period = 10000;	// Period (ms) on which to reset FlagsSensed
uint32_t FlagReset_StartTime;

///////////////////////////////////////////////////////////////////////// IR SETUP
#include <IRLibDecodeBase.h>	// First include the decode base
#include <IRLib_P01_NEC.h>		// Now include only the protocols you wish
#include <IRLibCombo.h>     	// After all protocols, include this

#include <IRLibRecvPCI.h> 

IRrecvPCI IR_Receiver(IR_REC_PIN);	// Instanciate IR receiver
IRdecode IR_Decoder;				//	Instanciate IR decoder

const uint32_t IR_FlagCodes[NUM_FLAGS] = {
															0x1010101A,
															0x1020202A,
															0x1040404A,
															0x1080808A,
															0x1101010A,
															0x1202020A,
															0x1404040A
														};


///////////////////////////////////////////////////////////////////////// LoRa SETUP
#define RF95_FREQ 915.0
#define ADDR_MASTER 	48							// Address of master station

RH_RF95 rf95(RFM95_CS, RFM95_INT);			// Instanciate a LoRa driver
RHReliableDatagram LoRa(rf95, EEPROM.read(0));

#define STATION_DATA_LEN 2
// STATION DATA MAP
//		[0] Current State / Message type
// 	[1] Batt %	(set in TX function)


const uint8_t LORA_BUFF_LEN = STATION_DATA_LEN + NUM_FLAGS; // NOTE: ensure this isn't greater than LoRa.maxMessageLength() = 239 bytes
uint8_t LoRa_RecvBuffLen; 							// Set to RH_RF95_MAX_MESSAGE_LEN before each recv

uint8_t LoRa_TX_Buffer[LORA_BUFF_LEN]; 		// Buffer to transmit over LoRa
uint8_t LoRa_RX_Buffer[RH_RF95_MAX_MESSAGE_LEN];			// Data recieved from LoRa stored here
uint8_t LoRa_msgFrom;								// Holds ID of station last message was received from

bool ImTheMaster;
bool newLoRaMessage = false;





void setup() 
{
	// Check EEPROM for station ID
	MY_TEAM = EEPROM.read(0) - 48;

	if (EEPROM.read(0) == ADDR_MASTER)
		ImTheMaster = true;
	else
		ImTheMaster = false;

	//////////////////////////////////////// Buzzer setup
	pinMode(BUZZ_PIN, OUTPUT);
	digitalWrite(BUZZ_PIN, HIGH);


	//////////////////////////////////////// PIXEL SETUP
	FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS); 
	FastLED.setMaxPowerInVoltsAndMilliamps(5,200); 						// Limit total power draw of LEDs to 200mA at 5V
	fill_solid(leds, NUM_LEDS, COL_BLACK);
	
	// Show setup progress
	leds[0] = CRGB::Green;
	FastLED.show();


	//////////////////////////////////////// SERIAL SETUP
	Serial.begin(115200);

	#ifdef debugMSG
		while (!Serial)
		{
			// Show setup progress - blink red while we wait
			leds[1] = CRGB::Red;
			FastLED.show();
			delay(200);

			leds[1] = CRGB::Black;
			FastLED.show();
			delay(200);
		} 								// Wait for serial port to be available

		if (ImTheMaster)
			Serial.println(F("THE MASTER HAS ARRIVED"));
	#endif


	// Show setup progress
	leds[1] = CRGB::Green;
	FastLED.show();

	//////////////////////////////////////// LORA SETUP
	if (!LoRa.init())
	{
		#ifdef debugMSG
			Serial.println("LoRa init failed");
		#endif	

		// Show progress - failed LoRa init
		leds[2] = CRGB::Red;
		FastLED.show();

		// Make a little noise
		digitalWrite(BUZZ_PIN, LOW);
		delay(50);
		digitalWrite(BUZZ_PIN, HIGH);
		delay(50);
		digitalWrite(BUZZ_PIN, LOW);
		delay(50);
		digitalWrite(BUZZ_PIN, HIGH);
		delay(500);
	}
	else
	{
		// Show progress - passed LoRa init
		leds[2] = CRGB::Green;
		FastLED.show();
	}

	rf95.setFrequency(RF95_FREQ);
	rf95.setTxPower(20); 							// Setup Power,dBm

	// Serial.println(F("Waiting for radio to setup"));
	delay(500); 	// TODO - Why this delay?


	
	//////////////////////////////////////// IR Receiver Setup
	IR_Receiver.enableIRIn(); // Start the receiver

	// Show progress - IR setup complete
	leds[3] = CRGB::Green;
	FastLED.show();

	LEDUpdate_StartTime = millis();


	// Record start time of program fro flag reset timer
	FlagReset_StartTime = millis();



	#ifdef debugMSG
		Serial.println(F("Setup completed"));
	#endif

	// Make a little noise
	digitalWrite(BUZZ_PIN, LOW);
	delay(100);
	digitalWrite(BUZZ_PIN, HIGH);

}

void loop() 
{

	IR_RX();
	LoRa_RX();

	FlagResetTimer();
	// setPixelCols();
	updateLEDs();

}

void LoRa_RX()					
{
	// Checks for complete LoRa message
	// saves into buffer and sets newLoRaMessage to true when full message is received
	// Returns an acknowledge to sender

	
	if (LoRa.available())
	{
		// Wait for a message addressed to us from the client
		LoRa_RecvBuffLen = RH_RF95_MAX_MESSAGE_LEN;						// Do this before every new receive beause it gets changed in the process

		if (LoRa.recvfromAck(LoRa_RX_Buffer, LoRa_RecvBuffLen, &LoRa_msgFrom))
		{
			#ifdef debugMSG
				Serial.print(F("got request from : "));
				Serial.println(LoRa_msgFrom, DEC);

				Serial.print(F("Received: "));
				for (uint8_t i = 0; i < LoRa_RecvBuffLen; ++i)
				{
					Serial.print(F(" "));
					Serial.print(LoRa_RX_Buffer[i], HEX);
				}
				Serial.println();
			#endif


			newLoRaMessage = true;

			// Send a VERY SHORT reply back to the originator client
			if (!LoRa.sendtoWait(&LoRa_msgFrom, 1, LoRa_msgFrom))
			{
				;
				#ifdef debugMSG
					Serial.println(F("sendtoWait failed"));
				#endif
			}
		}
	}

	return;
}


void IR_RX()
{
	// Checks for complete IR message
	// Logs which team message was received from

	if (IR_Receiver.getResults()) 
	{
		IR_Decoder.decode();           //Decode message
		// IR_Decoder.dumpResults(true);  //Dump results


		// Check if received IR code matches one of our team codes
		for (uint8_t i = 0; i < NUM_FLAGS; ++i)
		{
			if (IR_Decoder.value == IR_FlagCodes[i])
			{
				// Flag detected!
				FlagsSensed |= 1UL << i;		// Record it
				FlagsPresent |= FlagsSensed;	// Update present flags

				#ifdef debugMSG
					Serial.print(F("Flags Sensed = "));
					Serial.println(FlagsSensed, BIN);

				#endif


				// setPixelCols();
				// updateLEDs();
			}
		}


		IR_Receiver.enableIRIn();      //Restart IR receiver
	}
}

void setPixelCols()
{
	uint8_t numFlags = 0;
	bool dodgyFive = false;

	fill_solid(leds, NUM_LEDS, COL_BLACK); // TODO - set this to base team dim colour

	// Determine number of flags present
	for (uint8_t i = 0; i < NUM_FLAGS; ++i)
	{
		if ((FlagsPresent >> i) & 1U)
			numFlags++;
	}


	if (numFlags == 5)
	{
		dodgyFive = true;
		numFlags++;
	}


	for (int x = 0; x < numFlags; ++x)
	{
		for (int y = 0; y < NUM_LEDS/numFlags; ++y)
		{
			if (dodgyFive & x == 5)
				leds[y+x*(NUM_LEDS/numFlags)] = COL_BLACK; // TODO set to base team dim colour
			else
				leds[y+x*(NUM_LEDS/numFlags)] = flagCols[getTeamCol(x)];
		}
	}

	FastLED.show();
	return;
}

uint8_t getTeamCol(uint8_t position)
{
	uint8_t count = 0;
	for (uint8_t i = 0; i < NUM_FLAGS; ++i)
	{
		if ((FlagsPresent >> i) & 1U)
		{
			if (count == position)
				return i;

			count++;
		}
	}

	return TEAM_WHITE;
}



void FlagResetTimer()
{
	// Checks to see if timer has elapsed and resets FlagsSensed if it has
	if ( (FlagReset_StartTime + FlagReset_Period) < millis() )
	{
		FlagsPresent = FlagsSensed;	// Update present flags
		FlagsSensed = 0;					// Reset FlagsSensed

		FlagReset_StartTime = millis();	// Reset timer

		

		#ifdef debugMSG
			Serial.println(F("Flags Reset"));

			Serial.print(F("Flags Sensed = "));
			Serial.println(FlagsSensed, BIN);
			Serial.print(F("Flags Present = "));
			Serial.println(FlagsPresent, BIN);
		#endif

		// setPixelCols();
	}

	return;
}



void updateLEDs()
{
	// Lights up LEDs according to status of station
	static uint8_t step = 0;
	const uint8_t width = NUM_LEDS / NUM_FLAGS;
	uint8_t nextPixel;

	// Return if it's not time to update yet
	if ( (LEDUpdate_StartTime + LEDUpdate_Period) >= millis())
		return;
	else
		LEDUpdate_StartTime = millis();

	Serial.println(F("Updating LEDs!"));

	// Set dim base colour
	fill_solid(leds, NUM_LEDS, getTeamCol(MY_TEAM));
	fadeLightBy(leds, NUM_LEDS, 200);


	int8_t pixelNum = 0;



	// Light up LEDs according to present flags
	for (uint8_t i = 0; i < NUM_FLAGS; ++i)					// Step through flag flags
	{
		if ((FlagsPresent >> i) & 1U)								// If flag is present...
		{
			for (uint8_t j = 0; j < width; ++j)					// Light LEDs for present flags
			{
				if ( (step + pixelNum) >= NUM_LEDS)				// Loop back to start of pixel chain
					leds[step + pixelNum++ - NUM_LEDS] = flagCols[i];	
				else
					leds[step + pixelNum++] = flagCols[i];	
			}
		}
	}





	FastLED.show();

	// Update step count
	if (++step >= NUM_LEDS)
		step = 0;


	return;
}

