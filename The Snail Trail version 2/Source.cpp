//THE SNAIL TRAIL GAME - Version 2.0 - Instrumented Version plus Automated Input & De-Randomised
// This version 2.0 is used for:
// - running the same game moves repeatedly to allow for comparisons as improvements are made.
// - saving timing data to a file, 'SnailTrailTimes.csv', not just the screen.
// - producing averages for the various timing data.
// - uses key presses previously saved to 'SnailTrailMoves.csv' file for automated game play (see version 1.0).
// Note to watch the game step by step, use a breakpoint in the main game loop and use F5/Continue in the debugger.

// Other changes in this version:
// - Replaced bleeps with text to keep the noise levels down.

bool record_Every_Step(1);	//NEW decide whether to record details of each step played. true/1 = keep details, false/0 = summary only

// A.Oram 2016, originally based on the P.Vacher's skeletal program.
// Comments showing 'NEW' are on lines additional to the original Snail Trail version 0.
// Comments showing 'NEW2' are on lines additional to version 1.0

/*
A snail (the player) moves about the garden leaving a trail of slime that it doesn't like to cross, but this does dissolve in due course.
Some swine has scattered a number of hard-to-see slug pellets around and if the snail slithers over a number of these it dies.
The garden has frogs that leap about trying to make lunch of the snail, but may jump over it by accident. They may incidentally
soak up a slug pellet, thus helping the snail stay alive.
If they land on it or the snail runs into a frog, it's curtains for the snail.
Frogs also have a certain % chance of being carried off by a hungry eagle, so the snail may live to see another day.
In this version there are a number of lettuces to eat - eat them all before being eaten and the snail wins!

This code is certainly snail-like, can you get the frame rate up?


Screenshot of console during gameplay (mono colour!):
___________________________________________________________________________
...THE SNAIL TRAIL...
DATE: 13/11/2015
++++++++++++++++++++++++++++++   TIME: 18:28:23
+             -              +
+                           @+
+         @                  +
+                   -        +   Initialise game= 7.2715us
+                            +   Paint Game=      42.878ms
+                            +
+                            +   Frames/sec=      23.3 at 42.885ms/frame
+ -   -   .....              +
+    M    .   .     -      - +
+         . - .              +
+       &..   .         @    +
+-    -       .              +   TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (@)
+             .              +   TO QUIT USE 'Q'
+    M        .              +
+             .  -           +   MSG:
+         -   .     -        +
+             .        -  -- +   SLITHERED OVER 2 PELLETS SO FAR!
+             .              +
++++++++++++++++++++++++++++++
___________________________________________________________________________

x co-ordinates follow this pattern (y co-ordinates similar):

0,1,2 ...    ...SIZEX-2, SIZEX-1

Garden interior extends 1..SIZEX-2, 1..SIZEY-2.
Walls are at 0 and SIZEX-1, 0 and SIZEY-1.

Annoying bleeps from the PC's speaker announce significant events, check the message shown to see what happened.

*/

//---------------------------------
//include libraries
//include standard libraries
#include <iostream >         //for output and input
#include <iomanip>           //for formatted output in 'cout'
#include <conio.h>           //for getch
#include <fstream>           //for files
#include <string>            //for string
#include "hr_time.h"         //for timers

using namespace std;

//include our own libraries
#include "RandomUtils.h"     //for Seed, Random,
#include "ConsoleUtils.h"    //for Clrscr, Gotoxy, etc.
#include "TimeUtils.h"       //for GetTime, GetDate, etc.

// global constants

// garden dimensions
const int SIZEY(20);					// vertical dimension
const int SIZEX(30);					// horizontal dimension

//constants used for the garden & its inhabitants
const char SNAIL('&');					// snail (player's icon)
const char DEADSNAIL('o');				// just the shell left...
const char GRASS(' ');					// open space, grass
const char WALL('+');                   // garden wall

const char SLIME('.');					// snail produce
const int  SLIMELIFE(25);				// how long slime lasts (in keypresses)

const char PELLET('-');					// should be invisible, but test using a visible character.
const int  NUM_PELLETS(15);				// number of slug pellets scattered about
const int  PELLET_THRESHOLD(5);			// deadly threshold! Slither over this number and you die!

const char LETTUCE('@');				// a lettuce
const char NO_LETTUCE(GRASS);			// guess!
const int  LETTUCE_QUOTA(4);			// how many lettuces you need to eat before you win.

const int  NUM_FROGS(2);
const char FROG('M');
const char DEAD_FROG_BONES('X');		// Dead frogs are marked as such in their 'y' coordinate for convenience
const int  FROGLEAP(4);					// How many spaces do frogs jump when they move
const float  EagleStrike(0.30f);			// There's a 1 in 30 chance of an eagle strike on a frog

// the keyboard arrow codes
const int UP(72);						// up key
const int DOWN(80);						// down key
const int RIGHT(77);					// right key
const int LEFT(75);						// left key

// other command letters
const char QUIT('q');					// end the game
//const char Bleep('\a');					// annoying Bleep
//string Bleeeep("\a\a\a\a");				// very annoying Bleeps
const char Bleep('B');					// NEW2 Silent Version ! annoying Bleep
string Bleeeep("Beep");				// NEW2 Silent Version ! very annoying Bleeps


const int MLEFT(SIZEX + 3);				//define left margin for messages etc (avoiding garden)

//define a few global control constants
int	 snailStillAlive(true);				// snail starts alive!
int  pellets(0);						// number of times snail slimes over a slug pullet
int  lettucesEaten(0);					// win when this reaches LETTUCE_QUOTA
bool fullOfLettuce(false);				// when full and alive snail has won!

int  key, newGame(!QUIT);				// NEW Made global for convenience (see 'main' for original declaration)

CStopWatch	InitTime,
FrameTime,
PaintTime;					// create stopwatchs for timing

// NEW Declarations dealing with instrumentation and saving game data

ofstream ST_Times;	  //NEW
ofstream ST_Moves;	  //NEW
ifstream ST_PlayList; //NEW2	- recorded series of moves.

bool InitTimesAlreadySaved(FALSE); //NEW
char moveResult(0);  //NEW saves result of snail's last  move for output file
char gameEvent(0);  //NEW saves any event such as Frog being eaten by eagle for output to file.
const char WIN('W'); //NEW
const char STUCK('S'); //NEW

double InitTimeTotal(0.); //NEW
double FrameTimeTotal(0.); //NEW
double PaintTimeTotal(0.); //NEW
int GamesPlayed(0);     //NEW
int TotalMovesMade(0);	//NEW


// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int __cdecl main()
{
	//function prototypes

	void initialiseGame(int&, int&, bool&, char[][SIZEX], char[][SIZEX], int[], int[][2], char[][SIZEX]);
	void paintGame(int, string message, char[][SIZEX]);
	void clearMessage(string& message);

	int getKeyPress();
	void analyseKey(string& message, int, int move[2]);
	void moveSnail(char[][SIZEX], int[], int[], int&, string&, char[][SIZEX], char[][SIZEX]);
	void moveFrogs(int[], int[][2], string&, char[][SIZEX], char[][SIZEX]);
	void placeSnail(char[][SIZEX], int[]);
	void dissolveSlime(char[][SIZEX], char[][SIZEX]);
	void showLettuces(char[][SIZEX], char[][SIZEX]);

	int anotherGo(int, int);

	// Timing info
	void showTimes(double, double, double, int, int);
	void saveTimes(double, double, double, int, string); //NEW
	void openFiles(void);	//NEW

	// Local variables
	//arrays that store ...
	char garden[SIZEY][SIZEX];				// the game 'world'
	char slimeTrail[SIZEY][SIZEX];			// lifetime of slime counters overlay
	char lettucePatch[SIZEY][SIZEX];		// remember where the lettuces are planted

	string message;							// various messages are produced on screen

	int  snail[2];							// the snail's current position (x,y)
	int  frogs[NUM_FROGS][2];				// coordinates of the frog contingent n * (x,y)
	int  move[2];							// the requested move direction

	//int  key, newGame(!QUIT);				// start new game by not quitting initially! NEW move to global declaration

	// Now start the game...

	//Seed();									//seed the random number generator
	srand(999);								//NEW2 - seed random number with a constant - ensures the same event sequence occurs
	openFiles();							//NEW - open files for timers and automated gameplay

	// ******************************** Main Game Loop **************************************
	while ((newGame | 0x20) != QUIT)		// keep playing games
	{
		Clrscr();
		InitTimesAlreadySaved = FALSE;	// NEW - only ouptut Init timing once per game

		InitTime.startTimer();

		//initialise garden (incl. walls, frogs, lettuces & snail)
		initialiseGame(pellets, lettucesEaten, fullOfLettuce, slimeTrail, lettucePatch, snail, frogs, garden);

		message = "READY TO SLITHER!? PRESS A KEY...";
		InitTime.stopTimer();

		paintGame(pellets, message, garden);			//display game info, garden & messages

		key = getKeyPress();							//get started or quit game


		// ******************************** Frame Loop **************************************
		while (((key | 0x20) != QUIT) && snailStillAlive && !fullOfLettuce)	//user not bored, and snail not dead or full
		{
			FrameTime.startTimer(); // not part of game

			// ************** code to be timed ***********************************************

			analyseKey(message, key, move);				// get next move from keyboard (//NEW2 or preset file)
			moveSnail(lettucePatch, snail, move, pellets, message, garden, slimeTrail);
			dissolveSlime(garden, slimeTrail);			// remove slime over time from garden
			showLettuces(garden, lettucePatch);			// show remaining lettuces on ground
			placeSnail(garden, snail);					// move snail in garden
			moveFrogs(snail, frogs, message, garden, lettucePatch);	// frogs attempt to home in on snail

			FrameTime.stopTimer(); // you should eventually uncomment this and comment out the identical line 4 lines down

			paintGame(pellets, message, garden);		// display game info, garden & messages
			//clearMessage(message);					// reset message array (NEW moved below, see comment)

			// *************** end of timed section ******************************************

			//FrameTime.stopTimer(); // not part of game

			showTimes(InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), MLEFT, 6);
			// NEW Save performance data outside of game loop
			saveTimes(InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), key, message); //NEW
			clearMessage(message);						// NEW reset message array, moved from above (so we can use it above in saveTimes)
			InitTimesAlreadySaved = TRUE;				// NEW prevent repeated saving of Init timing

			key = getKeyPress();						// display menu & read in next option
		}

		// ******************************** End of Frame  Loop **************************************

		//							If alive...								If dead...
		(snailStillAlive) ? message = "WELL DONE, YOU'VE SURVIVED" : message = "REST IN PEAS.";

		if (!snailStillAlive) garden[snail[0]][snail[1]] = DEADSNAIL;
		paintGame(pellets, message, garden);			//display final game info, garden & message

		newGame = anotherGo(MLEFT, 19);					// play again, or Quit game.

	} 	// ******************************** End of Main Game Loop **************************************

	//NEW Report final frame time (only the paintGame time will be different from last set as frame won't be recalculated).
	saveTimes(InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), key, message); //NEW

	return 0;
} //end main


// FUNCTION DEFINITIONS //////////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//													set game configuration

void initialiseGame(int& pellets, int& Eaten, bool& fullUp, char slimeTrail[][SIZEX], char lettucePatch[][SIZEX],
	int snail[], int frogs[][2], char garden[][SIZEX])
{ //initialise garden & place snail somewhere

	void setGarden(char[][SIZEX]);
	void setSnailInitialCoordinates(int[]);
	void placeSnail(char[][SIZEX], int[]);
	void initialiseSlimeTrail(char[][SIZEX]);
	void initialiseLettucePatch(char[][SIZEX]);
	void showLettuces(char[][SIZEX], char[][SIZEX]);
	void scatterStuff(char[][SIZEX], char[][SIZEX], int[]);
	void scatterFrogs(char[][SIZEX], int[], int[][2]);

	snailStillAlive = true;					// bring snail to life!
	setSnailInitialCoordinates(snail);		// initialise snail position
	setGarden(garden);						// reset the garden
	placeSnail(garden, snail);				// place snail at a random position in garden
	initialiseSlimeTrail(slimeTrail);		// no slime until snail moves
	initialiseLettucePatch(lettucePatch);	// lettuces not been planted yet
	scatterStuff(garden, lettucePatch, snail);	// randomly scatter stuff about the garden (see function for details)
	showLettuces(garden, lettucePatch);		// show lettuces on ground
	scatterFrogs(garden, snail, frogs);		// randomly place a few frogs around

	pellets = 0;							// no slug pellets slithered over yet
	Eaten = 0;								// reset number of lettuces eaten
	fullUp = false;							// snail is hungry again
}

//**************************************************************************
//												randomly drop snail in garden
void setSnailInitialCoordinates(int snail[])
{ //set snail's coordinates inside the garden at random at beginning of game

	snail[0] = Random(SIZEY - 2);		// vertical coordinate in range [1..(SIZEY - 2)]
	snail[1] = Random(SIZEX - 2);		// horizontal coordinate in range [1..(SIZEX - 2)]
}

//**************************************************************************
//						set up garden array to represent grass and walls

void setGarden(char garden[][SIZEX])
{ //reset to empty garden configuration

	for (int col(0); col < SIZEX; ++col)
	{
		for (int row(0); row < SIZEY; ++row)
		{
			garden[row][col] = GRASS;				// grow some 'grass'
			if ((row == 0) || (row == SIZEY - 1))	// insert top or bottom walls where needed
				garden[row][col] = WALL;
			if ((col == 0) || (col == SIZEX - 1))	// insert left & right walls where needed
				garden[row][col] = WALL;
		}
	}
} //end of setGarden

//**************************************************************************
//														place snail in garden
void placeSnail(char garden[][SIZEX], int snail[])
{ //place snail at its new position in garden

	garden[snail[0]][snail[1]] = SNAIL;
} //end of placeSnail

//**************************************************************************
//												slowly dissolve slime trail

void dissolveSlime(char garden[][SIZEX], char slimeTrail[][SIZEX])
{// go through entire slime trail and decrement each item of slime in order

	for (int x = 1; x < SIZEX - 1; x++)
		for (int y = 1; y < SIZEY - 1; y++)
		{
			if (slimeTrail[y][x] <= SLIMELIFE && slimeTrail[y][x] > 0)	// if this bit of slime exists
			{
				slimeTrail[y][x] --;									// then dissolve slime a little.
				if (slimeTrail[y][x] == 0)								// if totally dissolved then
					garden[y][x] = GRASS;								// then remove slime from garden
			}
		}
}

//**************************************************************************
//													show lettuces on garden
void showLettuces(char garden[][SIZEX], char lettucePatch[][SIZEX])
{
	for (int x = 1; x < SIZEX - 1; x++)
		for (int y = 1; y < SIZEY - 1; y++)
			if (lettucePatch[y][x] == LETTUCE) garden[y][x] = LETTUCE;
}

//**************************************************************************
//													paint the game on screen
void paintGame(int pellets, string msg, char garden[][SIZEX])
{ //display game title, messages, snail & other elements on screen

	void showTitle(int, int);
	void showDateAndTime(int, int);
	void showTimingHeadings(int, int);
	void paintGarden(char[][SIZEX]);
	void showOptions(int, int);
	void showMessage(string, int, int);
	void showPelletCount(int, int, int);

	PaintTime.startTimer();	///////// Time this function body

	showTitle(0, 0);				// display game title
	showDateAndTime(MLEFT, 1);		// display system clock
	showTimingHeadings(MLEFT, 5);	// display Timings Heading
	paintGarden(garden);			// display garden contents
	showOptions(MLEFT, 14);			// display menu options available
	showPelletCount(pellets, MLEFT, 19);	// display poisonous moves made so far
	showMessage(msg, MLEFT, 17);	// display status message, if any

	PaintTime.stopTimer(); ///////// Time this function body

} //end of paintGame


//**************************************************************************
//													display garden on screen
void paintGarden(char garden[][SIZEX])
{ //display garden content on screen

	SelectBackColour(clGreen);
	SelectTextColour(clDarkBlue);
	Gotoxy(0, 2);

	for (int row(0); row < SIZEY; ++row)
	{
		for (int col(0); col < SIZEX; ++col)
		{
			cout << garden[row][col];			// display current garden contents 
		}
		cout << endl;
	}
} //end of paintGarden


//**************************************************************************
//															no slime yet!
void initialiseSlimeTrail(char slimeTrail[][SIZEX])
{ // set the whole array to 0

	for (int x = 1; x < SIZEX - 1; x++)			// can't slime the walls
		for (int y = 1; y < SIZEY - 1; y++)
			slimeTrail[y][x] = 0;
}


//**************************************************************************
//															no lettuces yet!
void initialiseLettucePatch(char lettucePatch[][SIZEX])
{ // set the whole array to 0

	for (int x = 1; x < SIZEX - 1; x++)		// can't plant lettuces in walls!
		for (int y = 1; y < SIZEY - 1; y++)
			lettucePatch[y][x] = NO_LETTUCE;
}


//**************************************************************************
//												implement arrow key move
void analyseKey(string& msg, int key, int move[2])
{ //calculate snail movement required depending on the arrow key pressed

	switch (key)		//...depending on the selected key...
	{
	case LEFT:	//prepare to move left
		move[0] = 0; move[1] = -1;	// decrease the X coordinate
		break;
	case RIGHT: //prepare to move right
		move[0] = 0; move[1] = +1;	// increase the X coordinate
		break;
	case UP: //prepare to move up
		move[0] = -1; move[1] = 0;	// decrease the Y coordinate
		break;
	case DOWN: //prepare to move down
		move[0] = +1; move[1] = 0;	// increase the Y coordinate
		break;
	default:  					// this shouldn't happen
		msg = "INVALID KEY";	// prepare error message
		move[0] = 0;			// move snail out of the garden
		move[1] = 0;
	}
}


//**************************************************************************
//			scatter some stuff around the garden (slug pellets and lettuces)

void scatterStuff(char garden[][SIZEX], char lettucePatch[][SIZEX], int snail[])
{
	// ensure stuff doesn't land on the snail, or each other.
	// prime x,y coords with initial random numbers before checking

	for (int slugP = 0; slugP < NUM_PELLETS; slugP++)				// scatter some slug pellets...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == PELLET);	// avoid snail and other pellets

		garden[y][x] = PELLET;										// hide pellets around the garden
	}

	for (int food = 0; food < LETTUCE_QUOTA; food++)				// scatter lettuces for eating...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == PELLET || lettucePatch[y][x] == LETTUCE);	// avoid snail, pellets and other lettucii

		lettucePatch[y][x] = LETTUCE;								// plant a lettuce in the lettucePatch
	}
}


//**************************************************************************
//									some frogs have arrived looking for lunch

void scatterFrogs(char garden[][SIZEX], int snail[], int frogs[][2])
{
	// need to avoid the snail initially (seems a bit unfair otherwise!). Frogs aren't affected by
	// slug pellets, btw, and will absorb them, and they may land on lettuces.

	for (int f = 0; f < NUM_FROGS; f++)					// for each frog passing by ...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));	// prime coords before checking
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == FROG);		// avoid snail and existing frogs

		frogs[f][0] = y;								// store initial positions of frog
		frogs[f][1] = x;
		garden[frogs[f][0]][frogs[f][1]] = FROG;		// put frogs on garden (this may overwrite a slug pellet)
	}
}


//**************************************************************************
//							move the Frogs toward the snail - watch for eagles!

void moveFrogs(int snail[], int frogs[][2], string& msg, char garden[][SIZEX], char lettuces[][SIZEX])
{
	//	Frogs move toward the snail. They jump 'n' positions at a time in either or both x and y
	//	directions. If they land on the snail then it's dead meat. They might jump over it by accident.
	//	They can land on lettuces and slug pellets - in the latter case the pellet is
	//  absorbed harmlessly by the frog (thus inadvertently helping the snail!).
	//	Frogs may also be randomly eaten by an eagle, with only the bones left behind.

	bool eatenByEagle(char[][SIZEX], int[]);

	for (int f = 0; f<NUM_FROGS; f++)
	{
		if ((frogs[f][0] != DEAD_FROG_BONES) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
		{
			// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce though...

			if (lettuces[frogs[f][0]][frogs[f][1]] == LETTUCE)
				garden[frogs[f][0]][frogs[f][1]] = LETTUCE;
			else  garden[frogs[f][0]][frogs[f][1]] = GRASS;

			// work out where to jump to depending on where the snail is...
			// see which way to jump in the Y direction (up and down)

			if (snail[0] - frogs[f][0] > 0)
			{
				frogs[f][0] += FROGLEAP;  if (frogs[f][0] >= SIZEY - 1) frogs[f][0] = SIZEY - 2;
			} // don't go over the garden walls!
			else if (snail[0] - frogs[f][0] < 0)
			{
				frogs[f][0] -= FROGLEAP;  if (frogs[f][0] < 1) frogs[f][0] = 1;
			};

			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[f][1] > 0)
			{
				frogs[f][1] += FROGLEAP;  if (frogs[f][1] >= SIZEX - 1) frogs[f][1] = SIZEX - 2;
			}
			else if (snail[1] - frogs[f][1] < 0)
			{
				frogs[f][1] -= FROGLEAP;  if (frogs[f][1] < 1)	frogs[f][1] = 1;
			};

			if (!eatenByEagle(garden, frogs[f]))						// not gotten by eagle?
			{
				if (frogs[f][0] == snail[0] && frogs[f][1] == snail[1])	// landed on snail? - grub up!
				{
					msg = "FROG GOT YOU!";
					cout << Bleeeep;									// produce a death knell
					snailStillAlive = false;							// snail is dead!
					gameEvent = FROG;									//NEW record this event
				}
				else garden[frogs[f][0]][frogs[f][1]] = FROG;			// display frog on garden (thus destroying any pellet that might be there).
			}
			else {
				msg = "EAGLE GOT A FROG";
				cout << Bleep;											//produce a warning sound
				gameEvent = DEAD_FROG_BONES;							//NEW record this event
			}
		}
	}// end of FOR loop
}

bool eatenByEagle(char garden[][SIZEX], int frog[])
{ //There's a 1 in 'EagleStrike' chance of being eaten

	if (Random(int(EagleStrike * 100)) == int(EagleStrike * 100))
	{
		garden[frog[0]][frog[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
		frog[0] = DEAD_FROG_BONES;								// and mark frog as deceased
		return true;
	}
	else return false;
}

// end of moveFrogs


//**************************************************************************
//											implement player's move command

void moveSnail(char lettucePatch[][SIZEX], int snail[], int keyMove[], int& pellets, string& msg, char garden[][SIZEX], char slimeTrail[][SIZEX])
{
	// move snail on the garden when possible.
	// check intended new position & move if possible...
	// ...depending on what's on the intended next position in garden.

	int targetY(snail[0] + keyMove[0]);
	int targetX(snail[1] + keyMove[1]);
	switch (garden[targetY][targetX]) //depending on what is at target position
	{
	case LETTUCE:		// increment lettuce count and win if snail is full
		garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
		snail[0] += keyMove[0];							//go in direction indicated by keyMove
		snail[1] += keyMove[1];
		lettucePatch[snail[0]][snail[1]] = NO_LETTUCE;	// eat the lettuce
		lettucesEaten++;								// keep a count
		fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
		fullOfLettuce ? msg = "LAST LETTUCE EATEN" : msg = "LETTUCE EATEN";
		fullOfLettuce ? cout << Bleeeep : cout << Bleep;
		//WIN! WIN! WIN!

		if (fullOfLettuce) gameEvent = WIN;				//NEW
		moveResult = LETTUCE;							//NEW record result of move

		break;

	case PELLET:		// increment pellet count and kill snail if > threshold
		garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime lifespan
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		pellets++;
		cout << Bleep;									// produce a warning sound
		if (pellets >= PELLET_THRESHOLD)				// aaaargh! poisoned!
		{
			msg = "TOO MANY PELLETS SLITHERED OVER!";
			cout << Bleeeep;							// produce a death knell
			snailStillAlive = false;					// game over
			gameEvent = DEADSNAIL;						//NEW
		}
		moveResult = PELLET;							//NEW record result of move
		break;

	case FROG:											//	kill snail if it throws itself at a frog!
		garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		msg = "OOPS! ENCOUNTERED A FROG!";
		cout << Bleeeep;								// produce a death knell
		snailStillAlive = false;						// game over
		gameEvent = DEADSNAIL;
		moveResult = FROG;								//NEW record result of move
		break;

	case WALL:											//oops, garden wall
		cout << Bleep;									//produce a warning sound
		msg = "THAT'S A WALL!";
		moveResult = WALL;								//NEW record result of move
		break;											//& stay put

	case GRASS:
	case DEAD_FROG_BONES:								//its safe to move over dead/missing frogs too
		garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime lifespan
		snail[0] += keyMove[0];							//go in direction indicated by keyMove
		snail[1] += keyMove[1];
		moveResult = GRASS;								//NEW record result of move
		break;

	case SLIME:											//Been here before, snail doesn't cross his own slime!
		cout << Bleep;									//produce a warning sound
		msg = "THAT'S SLIME!";
		moveResult = SLIME;								//NEW record result of move
		break;											//& stay put

	default: msg = "TRY A DIFFERENT DIRECTION";
		moveResult = STUCK;								//NEW record result of move
	}
} //end of MoveSnail



//// HELPER AND DISPLAY FUNCTIONS ////////////////////////////////////////////

//**************************************************************************
//											 get control key from player
// This function can be easily altered to save keyboard input to a file.

int getKeyPress()				//NEW2 now altered to read from file
{ //get command from user

	int command;
	char dumpComma;				//NEW2

	ST_PlayList >> command;		//NEW2 Get command from prerecorded list
	ST_PlayList >> dumpComma;	//NEW2 remember to read and dump comma separator

	////read in the selected option
	//command = _getch();  		// to read arrow keys
	//while (command == 224)	// to clear extra info from buffer
	//	command = _getch();

	//ST_Moves << command << ','; // NEW save commands as they're entered, as CSV file.
	return(command);

} //end of getKeyPress

//**************************************************************************
//											display info on screen
void clearMessage(string& msg)
{ //reset message to GRASS
	msg = "";
} //end of clearMessage

//**************************************************************************

void showTitle(int column, int row)
{ //display game title

	Clrscr();
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "...THE SNAIL TRAIL..." << endl;
	SelectBackColour(clWhite);
	SelectTextColour(clRed);

} //end of showTitle

void showDateAndTime(int column, int row)
{ //show current date and time

	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	Gotoxy(column, row);
	cout << "DATE: " << GetDate();
	Gotoxy(column, row + 1);
	cout << "TIME: " << GetTime();
} //end of showDateAndTime

void showOptions(int column, int row)
{ //show game options

	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "TO MOVE USE ARROW KEYS - EAT ALL LETTUCES (" << LETTUCE << ')';
	Gotoxy(column, row + 1);
	cout << "TO QUIT USE 'Q'";
} //end of showOptions

void showPelletCount(int pellets, int column, int row)
{ //display number of pellets slimed over

	SelectBackColour(clBlack);
	SelectTextColour(clWhite);
	Gotoxy(column, row);
	cout << "SLITHERED OVER " << pellets << " PELLETS SO FAR!";
} //end of showPelletCount

void showMessage(string msg, int column, int row)
{ //display auxiliary messages if any

	SelectBackColour(clBlack);
	SelectTextColour(clWhite);
	Gotoxy(column, row);
	cout << "MSG: " << msg;	//display current message
} //end of showMessage

void showTimingHeadings(int column, int row)
{
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "Game Timings:";
} //end of showTimingHeadings

int anotherGo(int column, int row)
{ //show end message and hold output screen

	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "PRESS 'Q' TO QUIT OR ANY KEY TO CONTINUE";
	SelectBackColour(clBlack);
	SelectTextColour(clWhite);

	return (getKeyPress());
} // end of anotherGo


void showTimes(double InitTimeSecs, double FrameTimeSecs, double PaintTimeSecs, int column, int row)
{ // show various times on screen as a measure of performance

#define milli (1000.)
#define micro (1000000.)


	SelectBackColour(clBlack);
	SelectTextColour(clWhite);
	Gotoxy(column, row);
	cout << setprecision(5) << "Initialise game= " << InitTimeSecs * micro << "us";
	Gotoxy(column, row + 1);
	cout << setprecision(5) << "Paint Game=      " << PaintTimeSecs * milli << "ms";
	Gotoxy(column, row + 3);
	cout << setprecision(3) << "Frames/sec=      " << (double) 1.0 / FrameTimeSecs << " at " << setprecision(5) << FrameTimeSecs * milli << "ms/frame";

} // end of showTimes

// NEW functions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void saveTimes(double InitTimeSecs, double FrameTimeSecs, double PaintTimeSecs, int key, string msg)
{ // save timing and other performance data to a file

#define milli (1000.)
#define micro (1000000.)

	if (!InitTimesAlreadySaved)  { // only do this once per game

		if (record_Every_Step)	{ // skip this if we're not saving each step's details
			ST_Times << "\n\nDATE:\t" << GetDate() << "\tTIME:\t" << GetTime()
				<< setprecision(5) << "\nInitialising game took " << InitTimeSecs * micro << "us";
			// Produce headings for rest of data
			ST_Times << "\n\nPaint Game (ms)\t\tMove made & Result\t\t\t\tFrame rate/s\tTime/Frame (ms)\tScreen messages";
		}
		InitTimeTotal += InitTimeSecs;
		GamesPlayed++;
	}

	if (record_Every_Step)	{ // skip this if we're not saving each step's details
		ST_Times << setprecision(5) << "\n\t" << PaintTimeSecs * milli;
		ST_Times << "\t\t\t" << (key == LEFT ? "LEFT " : key == RIGHT ? "RIGHT" : key == UP ? "UP   " : key == DOWN ? "DOWN " : key == QUIT ? "QUIT " : "Wrong")
			<< " to " << (moveResult == GRASS ? "GRASS  " : moveResult == WALL ? "WALL   " : moveResult == PELLET ? "PELLET " : moveResult == LETTUCE ? "LETTUCE" : "STUCK  ")
			<< " and " << (gameEvent == FROG ? "FROG!!    " : gameEvent == DEAD_FROG_BONES ? "FROG EATEN" : gameEvent == WIN ? "WON       " : gameEvent == DEADSNAIL ? "LOSE GAME " : "still OK  ");

		gameEvent = 0; // reset status
		moveResult = 0;
		ST_Times << setprecision(3) << "\t\t\t" << (double) 1.0 / FrameTimeSecs << "\t" << setprecision(5) << FrameTimeSecs * milli;
		ST_Times << "\t\t" << msg; // repeat messages seen on screen
	}

	// Keep running totals
	PaintTimeTotal += PaintTimeSecs;
	FrameTimeTotal += FrameTimeSecs;
	TotalMovesMade++; // for the whole session, may be several games.

	if ((newGame | 0x20) == QUIT) {		// all games played, figure the averages and store

		ST_Times << "\n\nSUMMARY for " << GamesPlayed << " games played, with " << TotalMovesMade << " moves entered.";
		ST_Times << setprecision(6) << "\n\nAverage frames/sec=\t" << (double)TotalMovesMade / FrameTimeTotal
			<< "\nAverage Paint time=\t" << (PaintTimeTotal * milli) / (double)TotalMovesMade << "ms"
			<< "\nAverage Init time=\t" << (InitTimeTotal * micro) / (double)GamesPlayed << "us";

		ST_Times << "\n\n*************************************************************************************************************************";
	};

} // end of save Times


void openFiles()
{ // open the files that will store the times produced and the file with the moves recorded.
	// NB. Files are closed automatically by system when program exits (not good but sufficient here)

	ST_Times.open("SnailTrailTimes.csv", ios::app);
	//ST_Moves.open("SnailTrailMoves.csv", ios::app);		// OLD open for writing moves to initially
	ST_PlayList.open("SnailTrailMoves.csv");				// NEW2 Read moves from previously stored moves

} // end of openFiles

// End of the whole 'SNAIL TRAIL' listing //////////////////////////////////////////////////////////////////////////
