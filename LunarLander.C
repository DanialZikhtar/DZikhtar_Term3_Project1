#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>



//Arbitrary limit for stopping draw function
const int limitX = 720;
const int limitY = 720;

double toRad(double theta)
{
    return (theta/180)*3.1415926535;
}


//The player-controlled object.
typedef struct Lander
{
    int yPos = 0;
    int xPos = 0;
    double ySpd = 0;
    double xSpd = 0;
    int angleOffset = 0;

    int fuel = 500;
    int thrustPower = 1;

    char landerChar = '^';
} Lander;

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

//Sets (y,x) of Lander to given (y,x) and redraw it there.
void setLanderPos(Lander* LnPt, int y, int x)
{
    //Check if target destination (and the path inbetween) is empty
        if(isEmpty(y,x) == false)
        {
            printw("Your ship exploded and everyone died :(");

            return;
        }
    
    mvaddch(LnPt->yPos, LnPt->xPos, ' ');
    LnPt->yPos = y;
    LnPt->xPos = x;
    mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);

    return;
}

//Changes the angle offset
void rotateLeft(Lander* LnPt)
{
    if(LnPt->angleOffset > -90)
    {
        LnPt->angleOffset -= 15;
        return;
    }

    return;
}

void rotateRight(Lander* LnPt)
{
    if(LnPt->angleOffset < 90)
    {
        LnPt->angleOffset += 15;
    }

    return;
}

void thrust(Lander* LnPt, double power)
{
    LnPt->ySpd += power*-cos(toRad(LnPt->angleOffset));
    LnPt->xSpd += power*sin(toRad(LnPt->angleOffset));
}

//Redraws ship ASCII based on current rotation (and other parameters?)
void updateLander(Lander* LnPt)
{
    //Rotation checks
    if(LnPt->angleOffset > 60)
    {
        LnPt->landerChar = '>';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else if(LnPt->angleOffset < -60)
    {
        LnPt->landerChar = '<';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else
    {
        LnPt->landerChar = '^';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }

    //Velocity checks
    if(LnPt->ySpd != 0 || LnPt->xSpd != 0)
    {
        setLanderPos(LnPt, LnPt->yPos + round(LnPt->ySpd), LnPt->xPos + round(LnPt->xSpd));
    }
}






//Object that keeps track of a cursor position used for printing characters
typedef struct Drawer
{
    int xPos = 0;
    int yPos = 0;
} Drawer;

void moveDrawPt(Drawer* DrPt, int y, int x)
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
    while(DrPt->xPos < limitX)
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
}







int main()
{
    srand(time(NULL));
    initscr();

    Lander Lunar;
    Lander* LunarPt = &Lunar;
    Lunar.yPos = 0;
    Lunar.xPos = 5;
    Lunar.fuel = 120;
    Lunar.angleOffset = 0;

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

        //DEBUG: Show speed on UI
        for(int i = COLS - 15; i<COLS; i++)
        {
            mvprintw(2, i, " ");
            mvprintw(3, i, " ");
        }
        mvprintw(2, COLS - 15, "ySpd: %0.3f", LunarPt->ySpd);
        mvprintw(3, COLS - 15, "xSpd: %0.3f", LunarPt->xSpd);

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
            
        }
        else if(userInput == 119)    //W Key
        {
            if(LunarPt->fuel > 0)
            {
                thrust(LunarPt, 0.5);
                LunarPt->fuel--;
            }
        }
        else if(userInput == 97)    //A Key
        {
            rotateLeft(LunarPt);
        }
        else if(userInput == 115)    //S Key
        {
            setLanderPos(LunarPt, LunarPt->yPos + 1, LunarPt->xPos);
        }
        else if(userInput == 100)    //D Key
        {
            rotateRight(LunarPt);
        }

        //Every-loop codes
        if(tick % 15240 == 0)
        {
            // setLanderPos(LunarPt, LunarPt->yPos + 1, LunarPt->xPos);
            if(LunarPt->ySpd < 2)
            {
                LunarPt->ySpd += 0.03125;
            }
        }

        if(tick % 45720 == 0)
        {
            updateLander(LunarPt);
        }

        refresh();
        tick++;
    }

    return 0;
}
