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
#define LORA_PIN_SS     PA3 //PA4
#define LORA_PIN_DIOO   PA1


// PIXEL SETUP
#define NUM_LEDS 24
WS2812B strip = WS2812B(NUM_LEDS);


// IR Receiver Setup
#include <Arduino.h>
#define IRMP_INPUT_PIN 4 // PA4
#define BLINK_13_LED_IS_ACTIVE_LOW // The LED on the BluePill is active LOW
#define IRMP_PROTOCOL_NAMES 1 // Enable protocol number mapping to protocol strings - needs some FLASH. Must before #include <irmp*>

#include <irmpSelectMain15Protocols.h>  // This enables 15 main protocols
#include <irmp.c.h>

IRMP_DATA irmp_data[1];

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)


void setup() 
{
	Serial.begin(115200);
	while (!Serial.available());
	Serial.println("Ready");

	// // LoRaNow.setFrequencyCN(); // Select the frequency 486.5 MHz - Used in China
	// // LoRaNow.setFrequencyEU(); // Select the frequency 868.3 MHz - Used in Europe
	// // LoRaNow.setFrequencyUS(); // Select the frequency 904.1 MHz - Used in USA, Canada and South America
	LoRaNow.setFrequencyAU(); // Select the frequency 917.0 MHz - Used in Australia, Brazil and Chile

	// // LoRaNow.setFrequency(frequency);
	// // LoRaNow.setSpreadingFactor(sf);
	LoRaNow.setPins(LORA_PIN_SS, LORA_PIN_DIOO);

	LoRaNow.setId(1);

	// // LoRaNow.setPinsSPI(sck, miso, mosi, ss, dio0); // Only works with ESP32

	if (!LoRaNow.begin()) 
	{
		Serial.println("LoRa init failed. Check your connections.");
		while (true);
	}

	LoRaNow.onMessage(onMessage);
	LoRaNow.onSleep(onSleep);
	LoRaNow.showStatus(Serial);


	// strip.begin();// Sets up the SPI
	// strip.show();// Clears the strip, as by default the strip data is set to all LED's off.
	// strip.setBrightness(8);

	irmp_init();
}

void loop() 
{
	
	// Serial.println("Send Message");
	// LoRaNow.print("HelloWorld! ");
	// LoRaNow.send();

	// delay(1000);

	// colorWipe(strip.Color(0, 255, 0), 100); // Green
	// colorWipe(strip.Color(255, 0, 0), 200); // Red
	// colorWipe(strip.Color(0, 0, 255), 200); // Blue

	// delay(500);

	if (irmp_get_data(&irmp_data[0])) 
		irmp_result_print(&irmp_data[0]);

	LoRaNow.loop();
}

void onMessage(uint8_t *buffer, size_t size)
{
	Serial.print("Receive Message: ");
	Serial.write(buffer, size);
	Serial.println();
	Serial.println();
}

uint32_t startTime = 0;

void onSleep()
{
	// static uint16_t startTime = millis();
	if ((millis() - startTime) > 5000)
	{
		Serial.println("Sleep");
		// delay(5000); // "kind of a sleep"
		Serial.println("Send Message");
		LoRaNow.print("LoRaNow Node Message ");
		LoRaNow.print(millis());
		LoRaNow.send();

		startTime = millis();
	}


	if (irmp_get_data(&irmp_data[0])) 
	{
		/*
		* Skip repetitions of command
		*/
		// if (!(irmp_data[0].flags & IRMP_FLAG_REPETITION)) 
		// {
		// 	/*
		// 	* Here data is available and is no repetition -> evaluate IR command
		// 	*/
		// 	switch (irmp_data[0].command) 
		// 	{
		// 		case 0x48:
		// 		irmp_blink13(false);
		// 		digitalWrite(LED_BUILTIN, HIGH);
		// 		break;
		// 		case 0x0B:
		// 		irmp_blink13(false);
		// 		digitalWrite(LED_BUILTIN, LOW);
		// 		break;
		// 		default:
		// 		irmp_blink13(true);
		// 		break;
		// 	}
		// }
		irmp_result_print(&irmp_data[0]);
	}
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