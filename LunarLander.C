#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>

const int ScreenLimitY = 720;   //Arbitrary limit for screen space
const int screenLimitX = 720;
const int MaxLanderYDimension = 3;  //Max size of lander art. Has to be a square that encloses all characters
const int MaxLanderXDimension = 5;
const int rotationAmount = 45;
const int explosionSize = 13;
const double gravAccel = 0.0320;        //Constant downward force
const double airResistance = 0.005;     //Friction

//LANDER ARTS
//Note: Thrust calculation is done at midpoint of bottom row.
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
    mvprintw(DrPt->yPos, DrPt->xPos, "_");
    DrPt->xPos++;
}

//For use with level generation algorthm
void drawUpSlope(Drawer* DrPt)
{
    mvprintw(DrPt->yPos, DrPt->xPos, "/");
    DrPt->yPos--;
    DrPt->xPos++;
}

//For use with level generation algorthm
void drawDownSlope(Drawer* DrPt)
{
    DrPt->yPos++;
    mvprintw(DrPt->yPos, DrPt->xPos, "\\");
    DrPt->xPos++;
}

//The level generation algorithm. Should only be called once per level.
//Drawer should point to bottom left of intended screen space before calling e.g (LINES - 1, 0)
void drawLevel(Drawer* DrPt)
{
    int ranA = rand() % 18;         //Used to draw ground
    int ranB = rand() % 10 - 5;     //Used to draw slopes
    while(DrPt->xPos < screenLimitX)
    {
        if(ranA > 0)
        {
            drawGround(DrPt);
            ranA--;
        }
        else
        {
            if(ranB > 0 && DrPt->yPos > 5)
            {
                drawUpSlope(DrPt);
                ranB--;
            }
            else if(ranB < 0 && DrPt->yPos < LINES - 1)
            {
                drawDownSlope(DrPt);
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
            
            random = rand() % 11 + distToCentre;
            if(random <= 0.55*size)
            {
                mvaddch(DrawHelper.yPos + i, DrawHelper.xPos + j, '*');
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

//Sets (y,x) of Lander to given (y,x) and redraw it there.
void moveLander(Lander* LnPt, int y, int x)
{   
    eraseLander(LnPt);
    LnPt->yPos = y;
    LnPt->xPos = x;
    drawLanderArt(LnPt);

    return;
}


void thrust(Lander* LnPt, double power)
{
    LnPt->yAccel += 1.2*power*-cos(toRad(LnPt->angleOffset));
    LnPt->xAccel += power*sin(toRad(LnPt->angleOffset));

    LnPt->fuel--;
}

void mover(Lander* LnPt)
{
    int yMove = (int) floor(LnPt->yVelo);
    int xMove = (int) floor(LnPt->xVelo);
    
    moveLander(LnPt, LnPt->yPos + yMove, LnPt->xPos + xMove);
    LnPt->yVelo -= yMove;
    LnPt->xVelo -= xMove;
}

//Redraws ship ASCII based on current rotation (and other parameters?)
void updateLander(Lander* LnPt)
{
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
    Lunar.yPos = 0 + MaxLanderYDimension;
    Lunar.xPos = 5;
    Lunar.fuel = 500;
    Lunar.thrustPower = 0.025;
    Lunar.angleOffset = 0;
    setLanderArt(LunarPt, LANDER_UPRIGHT);

    Drawer D;
    Drawer* DrPt = &D;
    D.yPos = LINES - 1;
    D.xPos = 0;

    drawLevel(DrPt);

    int userInput;
    int tick = 0;

    refresh();

    raw();
    noecho();
    nodelay(stdscr, true);
    curs_set(0);

    while(1)
    {
        //User Interface Codes
        for(int i = COLS - 15; i<COLS; i++)
        {
            mvprintw(0, i, " ");
            mvprintw(1, i, " ");
        }
        mvprintw(0, COLS - 15, "Fuel: %d", LunarPt->fuel);
        mvprintw(1, COLS - 15, "Angle: %d", LunarPt->angleOffset);

        //DEBUG: Show speed & mover component on UI
        for(int i = COLS - 30; i<COLS; i++)
        {
            mvprintw(2, i, " ");
            mvprintw(3, i, " ");
        }
        mvprintw(2, COLS - 30, "yAccel: %0.3f yVelo: %0.3f", LunarPt->yAccel, LunarPt->yVelo);
        mvprintw(3, COLS - 30, "xAccel: %0.3f xVelo: %0.3f", LunarPt->xAccel, LunarPt->xVelo);

        //Button Presses Codes
        userInput = getch();
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
            DrawExplosion(LunarPt->yPos, LunarPt->xPos, explosionSize);
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

        //Every-loop codes
        if(tick % 30480 == 0)
        {
            updateLander(LunarPt);

            //Friction calculation
            if(LunarPt->yPos >= 0)
            {
                LunarPt->yAccel += gravAccel - airResistance;
            }
            else
            {
                LunarPt->yAccel += 12*gravAccel;
            }

            if(LunarPt->yAccel > 3 && LunarPt->yPos >= 0)
            {
                LunarPt->yAccel = 3;
            }

            if(LunarPt->xAccel > 0)
            {
                LunarPt->xAccel -= airResistance;
            }
            else if(LunarPt->xAccel < 0)
            {
                LunarPt->xAccel += airResistance;
            }
            if(LunarPt->xAccel < 0.01 && LunarPt->xAccel > -0.01)
            {
                LunarPt->xAccel = 0;
            }
        }

        refresh();
        tick++;
    }

    return 0;
}
