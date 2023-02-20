//////////////////////////////////////////////////////////////////////////////
//////                           ~ CTF-IR-LORA ~                        //////
//////      Code based off multiple example projects                    //////
//////      Brought to gether by S.Eames                                //////
//////      See README for more detail info                             //////
//////////////////////////////////////////////////////////////////////////////


// SETUP DEBUG MESSAGES
// #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG
  #define DPRINT(...)   Serial.print(__VA_ARGS__)   //DPRINT is a macro, debug print
  #define DPRINTLN(...) Serial.println(__VA_ARGS__) //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)                       //now defines a blank line
  #define DPRINTLN(...)                     //now defines a blank line
#endif


// Include the things
#include <RH_RF95.h>          // LoRa Driver
#include <RHReliableDatagram.h>
#include "CommonVars.h"          // Extra local variables
#define FASTLED_INTERNAL         // Stop the annoying messages during compile
#include <FastLED.h>             // Pixel LED
#include <EEPROM.h>              // EEPROM used to store (this) station ID



enum team:uint8_t { TEAM_RED, TEAM_ORANGE, TEAM_YELLOW, TEAM_GREEN, TEAM_BLUE, TEAM_MAGENTA, TEAM_WHITE};



// ARDUINO PIN SETUP
#define RFM95_CS     18          // LoRa pins
#define RFM95_RST    10
#define RFM95_INT    0

#define BVOLT_PIN    A6          // Analog pin for battery voltage measurement
#define BUZZ_PIN     5           // Buzzer
#define LED_DATA_PIN 6           // Pixel LED String

#define IR_REC_PIN   1              // IR Receiver

// BATTERY PARAMETERS
#define LIPO_MINV    9600        // (mV) 9.6V - minimum voltage with a little safety factor
#define LIPO_MAXV    12600       // (mV) 12.6V - fully charged battery 



///////////////////////////////////////////////////////////////////////// PIXEL SETUP
// #include "FastLED.h"
#define NUM_LEDS 20
CRGB leds[NUM_LEDS]; // Define the array of leds


const uint16_t LEDUpdate_Period = 500;
uint32_t LEDUpdate_StartTime;



///////////////////////////////////////////////////////////////////////// FLAG related variables
/* OK, here's how I think it works

(1) Flags transmit their IR code roughly every 3 seconds
(2) Every time a flag is sensed, FlagsSensed is copied (AND) onto FlagsPresent
(3) Every 10 seconds, FlagsPresent is set to equal FlagsSensed, then FlagsSensed is reset to 0


Let's try this again
 * When a flag is sensed
   * A bit for it is set in 'FlagsSensed' --> 
   * FlagsPresent |= FlagsSensed    --> Instantly record it in the master array
 * At the end of every three seconds
   * FlagsPresent = FlagsSensed     --> This flushes out flags that weren't sensed in the previous period
   * FlagsPresent reset to 0        --> Start sensing afress

*/

// #define NUM_FLAGS 7
// #define NUM_TEAMS 8


#define MAX_FLAG_ID    16  // Number of unique flag IDs per colour - note; value loosely limited by number of pixel LEDs to show ID value 
#define MAX_FLAG_COLS  10 // Number of unique flag colours

// uint8_t FlagsPresent = 0b00000000;        
// uint8_t FlagsSensed  = 0b00000000;        

uint16_t FlagsPresent[MAX_FLAG_COLS];        // Used to update LEDs, but lags to change after flag is removed
uint16_t FlagsSensed[MAX_FLAG_COLS];         // Sensed flags are written here, then copied to FlagsPresent


// Team pixel colours
// const uint32_t teamCols[NUM_TEAMS] =   {  
//                                           COL_RED,
//                                           COL_ORANGE,
//                                           COL_YELLOW,
//                                           COL_GREEN,
//                                           COL_CYAN,
//                                           COL_BLUE,
//                                           COL_MAGENTA,
//                                           COL_WHITE
//                                        };

// Use following two variables together e.g. teamcols_Hue(HSV_RED)
enum HSV_Colours:uint8_t { HSV_RED, HSV_ORANGE, HSV_YELLOW, HSV_GREEN, HSV_CYAN, HSV_BLUE, HSV_PURPLE, HSV_MAGENTA };

const uint8_t teamCols_Hue[8] =  { 0, 18, 64, 96, 128, 160, 192, 230 };
#define HSV_SFULL 255
#define HSV_VFULL 255
#define HSV_VDIM  35

// Teams may have multiple flags associated with them but flags have unique IR Codes
// flagCols array sets the colours of each flag; the number refers to the colour from teamCols[]
// Match the order of this array with IR_FlagCodes[] to get the right colour for each flag IRCode
const uint8_t flagCols[NUM_FLAGS] = {0, 1, 2, 3, 4, 5, 6};


// TODO Update this later to be set by hex BCD switch or EEPROM
uint8_t MY_TEAM;        // 'Background' colour to set LEDs on base station

const uint16_t FlagReset_Period = 5000;   // Period (ms) on which to reset FlagsSensed
uint32_t FlagReset_StartTime;

///////////////////////////////////////////////////////////////////////// IR SETUP
#include <IRLibDecodeBase.h>  // First include the decode base
#include <IRLib_P01_NEC.h>    // Now include only the protocols you wish
#include <IRLibCombo.h>       // After all protocols, include this

#include <IRLibRecvPCI.h> 

IRrecvPCI IR_Receiver(IR_REC_PIN);  // Instanciate IR receiver
IRdecode IR_Decoder;          // Instanciate IR decoder

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
#define ADDR_MASTER  48                   // Address of master station

RH_RF95 rf95(RFM95_CS, RFM95_INT);        // Instanciate a LoRa driver
RHReliableDatagram LoRa(rf95, EEPROM.read(0));

#define STATION_DATA_LEN 2
// STATION DATA MAP
//    [0] Current State / Message type
//    [1] Batt %  (set in TX function)


const uint8_t LORA_BUFF_LEN = STATION_DATA_LEN + ceil(NUM_FLAGS/8); // NOTE: ensure this isn't greater than LoRa.maxMessageLength() = 239 bytes
uint8_t LoRa_RecvBuffLen;                    // Set to RH_RF95_MAX_MESSAGE_LEN before each recv

uint8_t LoRa_TX_Buffer[LORA_BUFF_LEN];       // Buffer to transmit over LoRa
uint8_t LoRa_RX_Buffer[RH_RF95_MAX_MESSAGE_LEN];         // Data recieved from LoRa stored here
uint8_t LoRa_msgFrom;                        // Holds ID of station last message was received from

bool ImTheMaster;
bool newLoRaMessage = false;

const uint16_t LoRa_TX_Period = 10000; // Period (ms) on which to reset FlagsSensed
uint32_t LoRa_TX_StartTime;



void setup() 
{
   // Check EEPROM for station ID
   MY_TEAM = EEPROM.read(0) - 49;            // TODO - teams acre current save to EEPROM, use a BCD switch instead

   if (EEPROM.read(0) == ADDR_MASTER)
      ImTheMaster = true;
   else
      ImTheMaster = false;

   //////////////////////////////////////// Buzzer setup
   pinMode(BUZZ_PIN, OUTPUT);
   digitalWrite(BUZZ_PIN, HIGH);


   //////////////////////////////////////// PIXEL SETUP
   FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS); 
   FastLED.setMaxPowerInVoltsAndMilliamps(5,200);                 // Limit total power draw of LEDs to 200mA at 5V
   fill_solid(leds, NUM_LEDS, CRGB::Black);
   
   // Show setup progress
   leds[0] = CRGB::Green;
   FastLED.show();


   //////////////////////////////////////// SERIAL SETUP
   Serial.begin(115200);

   #ifdef DEBUG
      while (!Serial)
      {
         // Show setup progress - blink red while we wait
         leds[1] = CRGB::Red;
         FastLED.show();
         delay(200);

         leds[1] = CRGB::Black;
         FastLED.show();
         delay(200);
      }                       // Wait for serial port to be available

      if (ImTheMaster)
         Serial.println(F("THE MASTER HAS ARRIVED"));
   #endif


   // Show setup progress
   leds[1] = CRGB::Green;
   FastLED.show();

   //////////////////////////////////////// LORA SETUP
   if (!LoRa.init())
   {
      DPRINTLN("LoRa init failed"); 

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
   rf95.setTxPower(5);                    // Setup Power,dBm

   // Serial.println(F("Waiting for radio to setup"));
   delay(500);    // TODO - Why this delay?


   
   //////////////////////////////////////// IR Receiver Setup
   IR_Receiver.enableIRIn(); // Start the receiver

   // Show progress - IR setup complete
   leds[3] = CRGB::Green;
   FastLED.show();

   LEDUpdate_StartTime = millis();


   // Record start time of program fro flag reset timer
   FlagReset_StartTime = millis();



   DPRINTLN(F("Setup completed"));

   // Make a little noise
   digitalWrite(BUZZ_PIN, LOW);
   delay(20);
   digitalWrite(BUZZ_PIN, HIGH); 

   // Let the master know we're here!
   LoRa_TX(ADDR_MASTER);


}

void loop() 
{

   IR_RX();
   LoRa_RX();

   // Return if it's too soon to transmit.
   if ( (LoRa_TX_StartTime + LoRa_TX_Period) <= millis() )
      LoRa_TX(ADDR_MASTER);   // Set to transmit station state roughly every 15 secs (maybe change to 30 later)


   FlagResetTimer();
   updateLEDs();           // Set to update LEDs once a second

}

void LoRa_RX()             
{
   // Checks for complete LoRa message
   // saves into buffer and sets newLoRaMessage to true when full message is received
   // Returns an acknowledge to sender

   
   if (LoRa.available())
   {
      // Wait for a message addressed to us from the client
      LoRa_RecvBuffLen = RH_RF95_MAX_MESSAGE_LEN;                 // Do this before every new receive beause it gets changed in the process

      if (LoRa.recvfromAck(LoRa_RX_Buffer, LoRa_RecvBuffLen, &LoRa_msgFrom))
      {
         #ifdef DEBUG
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
            DPRINTLN(F("sendtoWait failed"));
            
         }
      }
   }

   return;
}

void LoRa_TX(uint8_t toAddr)
{
   // Transmit LoRa message



   // Update station data
   // LoRa_TX_Buffer[0] = NodeState;            // Get current game state
   // LoRa_TX_Buffer[1] = getBattPercent();     // Get current battery level

   // Copy flag flags into TX Buffer
   memcpy ( &LoRa_TX_Buffer + STATION_DATA_LEN, &FlagsPresent, ceil(NUM_FLAGS/8)); // TODO - Fix this
   

   if (LoRa.sendtoWait(LoRa_TX_Buffer, LORA_BUFF_LEN, toAddr))
   {
      // Now wait for a reply from the server

      // Print sent data
      #ifdef DEBUG
         Serial.print(F("Sent: "));
         for (uint8_t i = 0; i < LORA_BUFF_LEN; ++i)
         {
            Serial.print(F(" "));
            Serial.print(LoRa_TX_Buffer[i], HEX);
         }
         Serial.println();
      #endif


      LoRa_RecvBuffLen = RH_RF95_MAX_MESSAGE_LEN;  
      if (LoRa.recvfromAckTimeout(LoRa_RX_Buffer, &LoRa_RecvBuffLen, 2000, &LoRa_msgFrom))
      {
         ;
         #ifdef DEBUG
            Serial.print(F("got reply from : "));
            Serial.println(LoRa_msgFrom, DEC);


            Serial.print(F("Received: "));
            for (uint8_t i = 0; i < LoRa_RecvBuffLen; ++i)
            {
               Serial.print(F(" "));
               Serial.print(LoRa_RX_Buffer[i], HEX);
            }
            Serial.println();
         #endif
      }
      else { DPRINTLN(F("No reply from master station?")); }
   }
   else { DPRINTLN(F("sendtoWait failed")); }


   // Reset timer for transmits
   LoRa_TX_StartTime = millis();

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
      decodeIR(IR_Decoder.value);


      // // Check if received IR code matches one of our team codes
      // for (uint8_t i = 0; i < NUM_FLAGS; ++i)
      // {
      //    if (IR_Decoder.value == IR_FlagCodes[i])
      //    {
      //       // Flag detected!
      //       FlagsSensed |= 1UL << i;      // Record it
      //       FlagsPresent |= FlagsSensed;  // Update present flags

      //       DPRINT(F("Flags Sensed = "));
      //       DPRINTLNFlagsSensed, BIN);
            
      //       // Only one IR code is read at a time, so jump out once we match one
      //       IR_Receiver.enableIRIn();      //Restart IR receiver
      //       return;
      //    }
      // }

      IR_Receiver.enableIRIn();      //Restart IR receiver
   }

   
   return;
}


#define CODE_OFFSET 0x7F      // Offset value used for enoding flag col/ID into IR Code

bool decodeIR(uint32_t code)
{
   // Decodes IR signal (col-col-ID-ID)
   uint8_t IR_RecFlagCol;
   uint8_t IR_RecFlagID;

   uint8_t temp_IR_RecFlagCol;
   uint8_t temp_IR_RecFlagID;

   // Find ID
   IR_RecFlagID = (0xFF & code) + CODE_OFFSET;
   temp_IR_RecFlagID = ((code >> 8) & 0xFF) - CODE_OFFSET;

   if ((IR_RecFlagID != temp_IR_RecFlagID) || (IR_RecFlagID >= MAX_FLAG_ID))      // Error checking
      return false;
   
   // Find Col
   IR_RecFlagCol = ((code >> 16) & 0xFF) + CODE_OFFSET;
   temp_IR_RecFlagCol = ((code >> 24) & 0xFF) - CODE_OFFSET;

   if ((IR_RecFlagCol != temp_IR_RecFlagCol) || (IR_RecFlagCol >= MAX_FLAG_COLS))   // More Error checking
      return false;

   // Save flag to table
   FlagsSensed[IR_RecFlagCol] |= 1UL << IR_RecFlagID;          // Record it
   FlagsPresent[IR_RecFlagCol] |= FlagsSensed[IR_RecFlagCol];  // Update present flags

   DPRINT(F("Flags Sensed = "));
   DPRINTLN(FlagsSensed, BIN);


   return true;
}



void FlagResetTimer()
{
   // Checks to see if timer has elapsed and resets FlagsSensed if it has
   if ( (FlagReset_StartTime + FlagReset_Period) < millis() )
   {
      // Update present flags (and forget those from previous period)
      memcpy ( &FlagsPresent, &FlagsSensed, sizeof(FlagsPresent));  // memcpy(destination, source, size)

      // Reset FlagsSensed
      for (uint8_t i = 0; i < MAX_FLAG_COLS; ++i)
         FlagsSensed[i] = 0;


      // FlagsPresent = FlagsSensed;   // Update present flags
      // FlagsSensed = 0;              // Reset FlagsSensed

      FlagReset_StartTime = millis();  // Reset timer

      DPRINTLN(F("Flags Reset"));

      // DPRINT(F("Flags Sensed = "));
      // DPRINTLN(FlagsSensed, BIN);
      // DPRINT(F("Flags Present = "));
      // DPRINTLN(FlagsPresent, BIN);

   }

   return;
}



void updateLEDs()
{
   // Lights up LEDs according to status of station
   static uint8_t step = 0;
   const uint8_t width = NUM_LEDS / NUM_FLAGS;

   // Return if it's not time to update yet
   if ( (LEDUpdate_StartTime + LEDUpdate_Period) >= millis())
      return;
   else
      LEDUpdate_StartTime = millis();

   // Set dim base colour
   // fill_solid(leds, NUM_LEDS, teamCols[MY_TEAM]);
   // nscale8_video(leds, NUM_LEDS, 5);

   fill_solid(leds, NUM_LEDS, CHSV(teamCols_Hue[MY_TEAM], HSV_SFULL, HSV_VDIM)); 



   // Light up LEDs according to present flags
   int8_t pixelNum = 0;                                  // Current pixel being updated

   for (uint8_t i = 0; i < NUM_FLAGS; ++i)               // Step through flag flags
   {
      if ((FlagsPresent >> i) & 1U)                      // If flag is present...
      {
         for (uint8_t j = 0; j < width; ++j)             // Light LEDs for present flags
         {
            if ( (step + pixelNum) >= NUM_LEDS)          // Loop back to start of pixel chain
               // leds[step + pixelNum++ - NUM_LEDS] = teamCols[flagCols[i]]; 
               leds[step + pixelNum++ - NUM_LEDS] = CHSV(teamCols_Hue[flagCols[i]], HSV_SFULL, HSV_VFULL);
            else
               leds[step + pixelNum++] = CHSV(teamCols_Hue[flagCols[i]], HSV_SFULL, HSV_VFULL); 
         }
      }
   }


   FastLED.show();                                       // Show the things

   // Update step count
   if (++step >= NUM_LEDS)
      step = 0;


   return;
}

