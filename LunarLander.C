#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>

const int ScreenLimitY = 720;           //Arbitrary limit for screen space
const int ScreenLimitX = 720;
const int StartY = 4;                   //Start position of lander
const int StartX = 8;
const int MaxLanderYDimension = 3;      //Max size of lander art. Has to be a square that encloses all characters
const int MaxLanderXDimension = 5;
const int rotationAmount = 30;
const int explosionSize = 13;
const int GravityCap = 3;               //Maximum gravity force (yAccel) that the lander can have. (actually just downward force in general, but you can't thrust downwards anyway so)
const double gravAccel = 0.0320;        //Constant downward force
const double airResistance = 0.005;     //Friction
const double LandableYAccel = 1.2;       //Maximum y-accel value for which a landing check succeeds
const double LandableXAccel = 4.0;        //Maximum x-accel value for which a landing check succeeds
static double GameScore = 0;
static double BaseScore = 100;


bool GameActive;
bool LanderPersist = false;             //If true, lander will remain on screen at landing spot after landing (and you can't land there again). Currently buggy :L

//LANDER ARTS
//Note: Thrust calculation is done at midpoint of bottom row. Landing check is done via at least one '_' character overlapping with at least one '_' character on the ground
char LANDER_UPRIGHT[MaxLanderYDimension][MaxLanderXDimension] = 
{ 
    {' ', ' ', ' ', ' ' ,' ' },
    {' ', ' ', 'O', ' ' ,' ' },
    {'_', '/', ' ', '\\','_' }
};

char LANDER_LEFT_TILT[MaxLanderYDimension][MaxLanderXDimension] = 
{ 
    {' ', ' ', ' ', ' ' ,' ' },
    {' ', ' ', 'O', '-' ,'-' },
    {' ', ' ', ' ', '\\',' ' }
};

char LANDER_LEFT_FULL[MaxLanderYDimension][MaxLanderXDimension] = 
{ 
    {' ', ' ', ' ', '/' ,' ' },
    {' ', ' ', 'O', ' ' ,' ' },
    {' ', ' ', ' ', '\\',' ' }
};

char LANDER_RIGHT_TILT[MaxLanderYDimension][MaxLanderXDimension] = 
{ 
    {' ', ' ', ' ', ' ' ,' ' },
    {'-', '-', 'O', ' ' ,' ' },
    {' ', '/', ' ', ' ',' ' }
};

char LANDER_RIGHT_FULL[MaxLanderYDimension][MaxLanderXDimension] = 
{ 
    {' ', '\\', ' ', ' ' ,' ' },
    {' ', ' ', 'O', ' ' ,' ' },
    {' ', '/', ' ', ' ',' ' }
};














//UTILITY FUNCTIONS

double toRad(double theta)
{
    return (theta/180)*3.1415926535;
}

int min(int a, int b)
{
    if(a>b)
    {
        return b;
    }
    else if(a<b)
    {
        return a;
    }
    else 
    {
        return a;
    }
}

//Checks if position (y,x) is empty
bool isEmpty(int y, int x)
{
    //Empty-case (spacebar or 'allowed' characters)
    //Current exceptions
    // ' '
    // '<'
    // '^'
    // '>'
    if(mvinch(y,x) == 32 || mvinch(y,x) == 60 || mvinch(y,x) == 62 || mvinch(y,x) == 94)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//Prints text at to left of screen space
void ContextPrintw(const char* str)
{
    int length = strlen(str);
    int allowedprintspace = COLS - 30;
    for(int i = 0; i<min(length,allowedprintspace); i++)
    {
        mvaddch(0, i, str[i]);
    }
}

void EraseContextPrintw()
{
    for(int i = 0; i<COLS - 30; i++)
    {
        mvaddch(0, i, ' ');
    }
}

//Pauses the whole screen
void GamePause()
{
    ContextPrintw("Game is paused. Press r to restart.");
    GameActive = false;
    nodelay(stdscr, false);
}

//Resumes the whole screen
void GameUnpause()
{
    GameActive = true;
    nodelay(stdscr, true);
}
















//Object that keeps track of a cursor position used for printing characters
typedef struct Drawer
{
    int xPos = 0;
    int yPos = 0;
} Drawer;

void moveDrawHelper(Drawer* DrPt, int y, int x)
{
    DrPt->yPos = y;
    DrPt->xPos = x;
}

//For use with level generation algorthm
void drawGround(Drawer* DrPt)
{
    mvaddch(DrPt->yPos, DrPt->xPos, '_');
    DrPt->xPos++;
}

//For use with level generation algorthm
void drawUpSlope(Drawer* DrPt)
{
    mvaddch(DrPt->yPos, DrPt->xPos, '/');
    DrPt->yPos--;
    DrPt->xPos++;
}

//For use with level generation algorthm
void drawDownSlope(Drawer* DrPt)
{
    DrPt->yPos++;
    mvaddch(DrPt->yPos, DrPt->xPos, '\\');
    DrPt->xPos++;
}

typedef struct lvlchar
{
    int yPos;
    int xPos;
    char ch = ' ';
    double Score = 0;
}lvlchar;

typedef struct Level
{
    int StartYPos;
    int StartXPos;
    lvlchar Layout[ScreenLimitX];
    Level* Right = NULL;
    Level* Left = NULL;
}Level;

void InsertScores(Level* Lvl)
{
    int counter = 0;
    
    int wideness = 0;
    double score = 0;
    for(int i=0;i<ScreenLimitX - 1; i++)
    {
        if(Lvl->Layout[i].ch == '_')
        {
            //Check how wide the pad is, then use that to calculate score
            while(Lvl->Layout[i + wideness].ch == '_')
            {
                wideness++;
            }

            if(wideness <= 5)
            {
                score = 10;
            }
            else if(wideness <= 10)
            {
                score = 9 - 2.132*(wideness/8);
            }
            else if(wideness <= 14)
            {
                score = 6 - 2.653*(wideness/14);
            }
            else if(wideness <= 20)
            {
                score = 2.2- 1.468*(wideness/20);
            }
            else if(wideness > 20)
            {
                score = 1;
            }

            if(score < 1)
            {
                score = 1;
            }

            //Assign score to the pad associated with the wideness
            for(int j = 0; j < wideness; j++)
            {
                Lvl->Layout[i+j].Score = score;
            }
            i += wideness;
            wideness = 0;
        }
    }
}

//The level generation algorithm. Generates a level at DrPt then stores it in Lvl
//Drawer should point to bottom left of intended screen space before calling e.g (LINES - 1, 0) 
void GenerateLevel(Drawer* DrPt, Level* Lvl)
{ 
    Lvl->StartYPos = DrPt->yPos;
    Lvl->StartXPos = DrPt->xPos;
    
    int ranA = rand() % 18;         //Used to draw ground
    int ranB = rand() % 10 - 5;     //Used to draw slopes
    int counter = 0;
    while(DrPt->xPos < ScreenLimitX)
    {
        if(ranA > 0)
        {
            drawGround(DrPt);
            Lvl->Layout[counter].ch = '_';
            Lvl->Layout[counter].yPos = DrPt->yPos;
            Lvl->Layout[counter].xPos = DrPt->xPos - 1;
            counter++;
            ranA--;
        }
        else
        {
            if(ranB > 0 && DrPt->yPos > 5)
            {
                drawUpSlope(DrPt);
                Lvl->Layout[counter].ch = '/';
                Lvl->Layout[counter].yPos = DrPt->yPos + 1;
                Lvl->Layout[counter].xPos = DrPt->xPos - 1;
                counter++;
                ranB--;
            }
            else if(ranB < 0 && DrPt->yPos < LINES - 1)
            {
                drawDownSlope(DrPt);
                Lvl->Layout[counter].ch = '\\';
                Lvl->Layout[counter].yPos = DrPt->yPos;
                Lvl->Layout[counter].xPos = DrPt->xPos - 1;
                counter++;
                ranB++;
                ranB++;
            }
            else
            {
                ranA = rand() % 15;
                ranB = rand() % 24 - 12;
            }
        }
    }

    return;
}

void DrawLevel(Level* Lvl)
{
    int counter = 0;
    
    Drawer DrawerHelper;
    DrawerHelper.yPos = Lvl->StartYPos;
    DrawerHelper.xPos = Lvl->StartXPos;

    while(counter < ScreenLimitX - 1)
    {
        mvaddch(Lvl->Layout[counter].yPos, Lvl->Layout[counter].xPos, Lvl->Layout[counter].ch);
        counter++;
    }
}

//Draws an explosion at (y,x) according to specified dimensions
void DrawExplosion(int y, int x, int size)
{
    Drawer DrawHelper;
    DrawHelper.yPos = y - 1;
    DrawHelper.xPos = x - 1;

    //First Pass
    for(int i = 0; i< 3; i++)
    {
        for(int j = 0; j< 3; j++)
        {          
            mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, '*');
        }
    }

    DrawHelper.yPos = (int) y - size/2;
    DrawHelper.xPos = (int) x - size/2;
    int random;
    int distToCentre;

    //Second Pass
    for(int i = 0; i<size; i++)
    {
        for(int j = 0; j<size; j++)
        {          
            distToCentre = (int) sqrt(pow(((DrawHelper.yPos + i) - y), 2) + pow(((DrawHelper.xPos + j) - x), 2));
            
            random = rand() % size + distToCentre;
            if(random <= 0.55*size)
            {
                mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, '*');
            }
        }
    }


    return;
}

void EraseExplosion(int y, int x, int size)
{
    Drawer DrawHelper;
    DrawHelper.yPos = (int) y - size/2;
    DrawHelper.xPos = (int) x - size/2;

    for(int i = 0; i<size; i++)
    {
        for(int j = 0; j<size; j++)
        {          
            if(mvinch(DrawHelper.yPos + i,DrawHelper.xPos + j) == '*')
            {
                mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, ' ');
            }
        }
    }


    return;
}













//The player-controlled object.
typedef struct Lander
{
    int yPos = 0;
    int xPos = 0;
    double yAccel = 0;
    double xAccel = 0;
    double yVelo = 0;
    double xVelo = 0;
    int angleOffset = 0;

    int fuel = 500;
    double thrustPower = 0.01525;

    bool IsDestroyed = false;
    bool IsLanded = false;

    char landerArt[MaxLanderYDimension][MaxLanderXDimension];
} Lander;

void setLanderArt(Lander* LnPt, char LanderArt[MaxLanderYDimension][MaxLanderXDimension])
{
    for (int i = 0; i < MaxLanderYDimension; i++)
    {
        for (int j = 0; j < MaxLanderXDimension; j++)
        {
            LnPt->landerArt[i][j] = LanderArt[i][j];
        }
    }
}

void thrust(Lander* LnPt, double power)
{
    LnPt->yAccel += power*-cos(toRad(LnPt->angleOffset));
    LnPt->xAccel += power*sin(toRad(LnPt->angleOffset));

    LnPt->fuel--;
}

//Draws the lander art at current position
void drawLanderArt(Lander* LnPt)
{
    Drawer DrawHelper;
    DrawHelper.yPos = LnPt->yPos - MaxLanderYDimension - 1;
    DrawHelper.xPos = LnPt->xPos - ((int) MaxLanderXDimension/2) - 1;
    
    for(int i = 0; i<MaxLanderYDimension; i++)
    {
        for(int j = 0; j<MaxLanderXDimension; j++)
        {
            //Prevents function from erasing non-lander characters. Lander characters can still replace other characters
            while(LnPt->landerArt[i][j] == ' ' && mvinch(DrawHelper.yPos + i, DrawHelper.xPos + j) != 32 && j<MaxLanderXDimension)
            {
                j++;
            }
            
            mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, LnPt->landerArt[i][j]);
        }
    }

    return;
}

void eraseLander(Lander* LnPt)
{
    Drawer DrawHelper;
    DrawHelper.yPos = LnPt->yPos - MaxLanderYDimension - 1;
    DrawHelper.xPos = LnPt->xPos - ((int) MaxLanderXDimension/2) - 1;
    
    for(int i = 0; i<MaxLanderYDimension; i++)
    {
        for(int j = 0; j<MaxLanderXDimension; j++)
        {
            mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, ' ');
        }
    }

    return;
}

//Checks collision by checking if drawing lander at (y,x) would result in conflict
bool checkCollision(Lander* LnPt, int y, int x)
{
    Drawer DrawHelper;
    DrawHelper.yPos = y - MaxLanderYDimension - 1;
    DrawHelper.xPos = x - ((int) MaxLanderXDimension/2) - 1;

    for(int i = 0; i<MaxLanderYDimension; i++)
    {
        for(int j = 0; j<MaxLanderXDimension; j++)
        {
            //If lander character to check is ' ', ignore and skip ahead
            while(LnPt->landerArt[i][j] == ' ' && j<MaxLanderXDimension)
            {
                j++;
            }
            
            if(mvinch(DrawHelper.yPos + i, DrawHelper.xPos + j) != 32 && mvinch(DrawHelper.yPos + i, DrawHelper.xPos + j) != -1)
            {
                return true;
            }
                
        }
    }

    return false;
}

//Special collision check, returns true if all '_' characters on lander overlaps with a '_' on the ground. Lander must have at least one '_' to succeed.
bool checkLandable(Lander* LnPt, int y, int x)
{
    Drawer DrawHelper;
    DrawHelper.yPos = y - MaxLanderYDimension - 1;
    DrawHelper.xPos = x - ((int) MaxLanderXDimension/2) - 1;

    bool HaveLandingGear = false;

    for(int i = 0; i<MaxLanderYDimension; i++)
    {
        for(int j = 0; j<MaxLanderXDimension; j++)
        {            
            if(LnPt->landerArt[i][j] == '_')
            {
                HaveLandingGear = true;
                if(mvinch(DrawHelper.yPos + i, DrawHelper.xPos + j) != '_')
                {
                    return false;
                }

                if(LnPt->yAccel > LandableYAccel && LnPt->xAccel > LandableXAccel)
                {
                    return false;
                }
            }
        }
    }

    if(HaveLandingGear == false)
    {
        return false;
    }

    return true;
}

//Changes the angle offset
void rotateLander(Lander* LnPt, int angle)
{
    int revertValue = LnPt->angleOffset;
    
    LnPt->angleOffset += angle;

    if(LnPt->angleOffset < -90 || LnPt->angleOffset > 90)
    {
        LnPt->angleOffset = revertValue;
    }

    return;
}

//Sets (y,x) of Lander to given (y,x) and redraw it there. Automatically performs collision checks.
void moveLander(Lander* LnPt, int y, int x)
{   
    eraseLander(LnPt);
    if(checkCollision(LnPt, y, x) == true)
    {
        if(checkLandable(LnPt, y, x) == true)
        {  
            LnPt->yPos = y;
            LnPt->xPos = x;
            drawLanderArt(LnPt);
            LnPt->IsLanded = true;
            return;
        }

        LnPt->IsDestroyed = true;
        return;
    }
    LnPt->yPos = y;
    LnPt->xPos = x;
    drawLanderArt(LnPt);

    return;
}

//A level of abstraction controlling the moveLander() function. Use velocity (after accel calculation) to move to a new position.
//This is what should be called to move the lander under physics.
void mover(Lander* LnPt)
{
    int yMove = (int) floor(LnPt->yVelo);
    int xMove = (int) floor(LnPt->xVelo);
    
    moveLander(LnPt, LnPt->yPos + yMove, LnPt->xPos + xMove);
    LnPt->yVelo -= yMove;
    LnPt->xVelo -= xMove;
}

void resetLander(Lander* LnPt)
{  
    LnPt->IsDestroyed = false;
    LnPt->IsLanded = false;
    
    LnPt->yPos = StartY;
    LnPt->xPos = StartX;
    LnPt->yVelo = 0;
    LnPt->xVelo = 0;
    LnPt->yAccel = 0;
    LnPt->xAccel = 0;
    LnPt->angleOffset = 0;

    return;
}

//The ultimate function that controls lander behavior. Calculates properties of LnPt when it interacts with level in Lvl
//Checks for various properties and update the visuals. Also calls nessecary move functions.
void updateLander(Lander* LnPt, Level* Lvl)
{ 
    //Destruct check (see if lander has crashed)
    if(LnPt->IsDestroyed == true)
    {
        DrawExplosion(LnPt->yPos, LnPt->xPos, explosionSize);
        GamePause();
        return;
    }

    //Landed check (see if lander has landed)
    //Functionally the same as destruct check, but I want features added to specifically the landed state later
    if(LnPt->IsLanded == true)
    {
        GameScore += BaseScore*Lvl->Layout[LnPt->xPos].Score;
        GamePause();
        return;
    }

    //Friction calculation
    if(LnPt->yPos >= 0)
    {
        LnPt->yAccel += gravAccel - airResistance;
    }
    else
    {
        LnPt->yAccel += 8*gravAccel;
    }

    if(LnPt->yAccel > GravityCap && LnPt->yPos >= 0)
    {
        LnPt->yAccel = GravityCap;
    }

    if(LnPt->xAccel > 0)
    {
        LnPt->xAccel -= airResistance;
    }
    else if(LnPt->xAccel < 0)
    {
        LnPt->xAccel += airResistance;
    }
    if(LnPt->xAccel < 0.01 && LnPt->xAccel > -0.01)
    {
        LnPt->xAccel = 0;
    }              
    
    //Acceleration and velocity calculation
    LnPt->yVelo += LnPt->yAccel;
    LnPt->xVelo += LnPt->xAccel;

    //Smoothes out abrupt change in direction
    if(LnPt->yAccel < 0.01 && LnPt->yAccel > -0.01)
    {
        LnPt->yVelo = 0.92;
    }
    if(LnPt->xAccel < 0.01 && LnPt->xAccel > -0.01)
    {
        LnPt->xVelo = 0.92;
    }
    
    //Rotation checks
    if(LnPt->angleOffset > 20 && LnPt->angleOffset < 75)
    {
        setLanderArt(LnPt, LANDER_RIGHT_TILT);
    }
    else if(LnPt->angleOffset >= 65)
    {
        setLanderArt(LnPt, LANDER_RIGHT_FULL);
    }
    else if(LnPt->angleOffset < -20 && LnPt->angleOffset > -75)
    {
        setLanderArt(LnPt, LANDER_LEFT_TILT);
    }
    else if(LnPt->angleOffset <= -65)
    {
        setLanderArt(LnPt, LANDER_LEFT_FULL);
    }
    else
    {
        setLanderArt(LnPt, LANDER_UPRIGHT);
    }

    //Velocity checks
    mover(LnPt);
}










int main()
{
    srand(time(NULL));
    initscr();

    Lander Lunar;
    Lander* LunarPt = &Lunar;
    Lunar.yPos = StartY;
    Lunar.xPos = StartX;
    Lunar.fuel = 500;
    Lunar.thrustPower = 0.025;
    Lunar.angleOffset = 0;
    setLanderArt(LunarPt, LANDER_UPRIGHT);

    Drawer D;
    Drawer* DrPt = &D;
    D.yPos = LINES - 1;
    D.xPos = 0;

    Level Level1;
    Level* CurrentLevel = &Level1;

    GenerateLevel(DrPt, CurrentLevel);
    InsertScores(CurrentLevel);

    int userInput;
    int tick = 0;

    refresh();

    raw();
    noecho();
    curs_set(0);
    GameUnpause();

    while(1)
    {
        //User Interface Codes
        for(int i = COLS - 15; i<COLS; i++)
        {
            mvprintw(0, i, " ");
            mvprintw(1, i, " ");
        }
        mvprintw(0, COLS - 15, "Fuel: %d", LunarPt->fuel);
        mvprintw(0, COLS - 30, "yAccel: %0.2f", 10*LunarPt->yAccel);
        mvprintw(0, COLS - 45, "Score: %0.0f", GameScore);
        mvprintw(1, COLS - 15, "Angle: %d", LunarPt->angleOffset);
        mvprintw(1, COLS - 30, "xAccel: %0.2f", 10*LunarPt->xAccel);

        //DEBUG: Show speed & mover component on UI
        // for(int i = COLS - 30; i<COLS; i++)
        // {
        //     mvprintw(2, i, " ");
        //     mvprintw(3, i, " ");
        // }
        // mvprintw(2, COLS - 15, "yVelo: %0.3f", LunarPt->yVelo);
        // mvprintw(3, COLS - 15, "xVelo: %0.3f", LunarPt->xVelo);

        //Button Presses Codes, divided into active (unpaused) and inactive (paused) state
        userInput = getch();
        if(GameActive == true)      //Active state
        {
            if(userInput == 10)         //Enter Key
            {
                
            }
            else if(userInput == 27)    //Escape key
            {
                endwin();
                break;
            }
            else if(userInput == 32)    //Spacebar Key
            {
                ContextPrintw("Test");
            }
            else if(userInput == 119)    //W Key
            {
                if(LunarPt->fuel > 0)
                {
                    thrust(LunarPt, LunarPt->thrustPower);
                }
            }
            else if(userInput == 97)    //A Key
            {
                rotateLander(LunarPt, -rotationAmount);
            }
            else if(userInput == 115)    //S Key
            {
                moveLander(LunarPt, LunarPt->yPos + 1, LunarPt->xPos);
            }
            else if(userInput == 100)    //D Key
            {
                rotateLander(LunarPt, rotationAmount);
            }
            else if(userInput == 114)    //R Key
            {
                
            }
        }
        else if(GameActive == false)        //Inactive state
        {
            if(userInput == 10)         //Enter Key
            {

            }
            else if(userInput == 27)    //Escape key
            {
                endwin();
                break;
            }
            else if(userInput == 32)    //Spacebar Key
            {

            }
            else if(userInput == 114)    //R Key
            {
                EraseContextPrintw();
                EraseExplosion(LunarPt->yPos, LunarPt->xPos, explosionSize);
                if(LanderPersist == false)
                {
                    eraseLander(LunarPt);
                }
                DrawLevel(CurrentLevel);
                resetLander(LunarPt);
                GameUnpause();
            }
        }

        //Every-loop codes
        if(tick % 30480 == 0 && GameActive == true)
        {
            updateLander(LunarPt, CurrentLevel);
        }

        refresh();
        tick++;
    }

    return 0;
}
