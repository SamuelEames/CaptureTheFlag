/*
	LoRaNow Simple Gateway

	This code receives messages form the node and sends a message back.

	created 01 04 2019
	by Luiz H. Cassettari
*/

#include <LoRaNow.h>
#include <WS2812B.h>
#include <SPI.h>

// // NRF Port
// #define LORA_PIN_SS     PB0
// #define LORA_PIN_DIOO   PA15

// GPIO Pins
#define LORA_PIN_SS     PA4
#define LORA_PIN_DIOO   PA1


///////////////////////////////////////////////////////////////////////// PIXEL SETUP
#define NUM_LEDS 24
WS2812B strip = WS2812B(NUM_LEDS);


///////////////////////////////////////////////////////////////////////// FLAG INFO
#define NUM_FLAGS 7
bool flagsPresent[NUM_FLAGS] = {0,0,0,0,0,0,0};

#define TEAM_RED 		0
#define TEAM_ORANGE 	1
#define TEAM_YELLOW		2
#define TEAM_GREEN		3
#define TEAM_BLUE		4
#define TEAM_PURPLE		5
#define TEAM_WHITE		6


// uint32_t flagCols[NUM_FLAGS] = {COL_RED, COL_ORANGE, COL_YELLOW, COL_GREEN, COL_BLUE, COL_PURPLE, COL_WHITE};


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
	Serial.begin(115200);
	while (!Serial.available());

	Serial.println("This is a LoRaNow Simple Node");

	// LoRaNow.setFrequencyCN(); // Select the frequency 486.5 MHz - Used in China
	// LoRaNow.setFrequencyEU(); // Select the frequency 868.3 MHz - Used in Europe
	// LoRaNow.setFrequencyUS(); // Select the frequency 904.1 MHz - Used in USA, Canada and South America
	LoRaNow.setFrequencyAU(); // Select the frequency 917.0 MHz - Used in Australia, Brazil and Chile

	// LoRaNow.setFrequency(frequency);
	// LoRaNow.setSpreadingFactor(sf);
	LoRaNow.setPins(LORA_PIN_SS, LORA_PIN_DIOO);

	LoRaNow.setId(1);

	// LoRaNow.setPinsSPI(sck, miso, mosi, ss, dio0); // Only works with ESP32

	if (!LoRaNow.begin()) 
	{
		Serial.println("LoRa init failed. Check your connections.");
		while (true);
	}

	LoRaNow.onMessage(onMessage);
	LoRaNow.onSleep(onSleep);
	LoRaNow.showStatus(Serial);


	// Pixel setup
	strip.begin();// Sets up the SPI
	strip.show();// Clears the strip, as by default the strip data is set to all LED's off.
	strip.setBrightness(8);

	// IR Setup
	myReceiver.enableIRIn(); // Start the receiver
	Serial.println(F("Ready to receive IR signals"));


}

void loop() 
{
	LoRaNow.loop();
	// Serial.println("Send Message");
	// LoRaNow.print("HelloWorld! ");
	// LoRaNow.send();

	// delay(1000);

	colorWipe(strip.Color(0, 255, 0), 100); // Green
	// colorWipe(strip.Color(255, 0, 0), 200); // Red
	// colorWipe(strip.Color(0, 0, 255), 200); // Blue

	// delay(500);
}

void onMessage(uint8_t *buffer, size_t size)
{
	Serial.print("Receive Message: ");
	Serial.write(buffer, size);
	Serial.println();
	Serial.println();
}

void onSleep()
{
	Serial.println("Sleep");
	delay(5000); // "kind of a sleep"
	Serial.println("Send Message");
	LoRaNow.print("LoRaNow Node Message ");
	LoRaNow.print(millis());
	LoRaNow.send();
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) 
{
	for(uint16_t i=0; i<strip.numPixels(); i++) 
	{
		strip.setPixelColor(i, c);
		strip.show();
		// delay(wait);
	}
}