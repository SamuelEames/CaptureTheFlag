/* 
Author: Samuel Eames
Created: 2018-01-20

Code used to intermittently send out IR codes from within capture the flag flags.
Received by base stations to know which flags are at them at any given time.
*/

#include <IRLib.h>

IRsendNEC My_Sender;

unsigned long timerCount;  


//////////////////////////////////////////////////
#define TEAM_WHITE	   // ENTER TEAM HERE!! //
//////////////////////////////////////////////////

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



//////////////////////////////////////////////////
void setup()
{
	// Ensure code is sent straight away
	My_Sender.send(IRCODE);
}
 
//////////////////////////////////////////////////
void loop() 
{
	if (timerCount >= REPEATS)
	{
		My_Sender.send(IRCODE);
		timerCount = 0;
	}
	else
		timerCount++;
}

