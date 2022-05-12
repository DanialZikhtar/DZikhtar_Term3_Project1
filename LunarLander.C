#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>

//Arbitrary limit for stopping draw function
const int limitX = 360;
const int limitY = 360;

//The player-controlled object.
//Its just a drawer object that prints everytime its moved.
typedef struct Lander
{
    int yPos = 0;
    int xPos = 0;
} Lander;

void setLanderYPos(Lander* LnPt, int y)
{
    if(mvinch(y, LnPt->xPos) != 32)
    {
        printw("Your ship exploded and everyone died :(");

        return;
    }
    
    mvprintw(LnPt->yPos, LnPt->xPos, " ");
    LnPt->yPos = y;
    mvprintw(LnPt->yPos, LnPt->xPos, "*");

    return;
}

void setLanderXPos(Lander* LnPt, int x)
{
    if(mvinch(LnPt->yPos, x) != 32)
    {
        printw("Your ship exploded and everyone died :(");

        return;
    }

    mvprintw(LnPt->yPos, LnPt->xPos, " ");
    LnPt->xPos = x;
    mvprintw(LnPt->yPos, LnPt->xPos, "*");

    return;
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
            if(ranB > 0 && DrPt->yPos > 1)
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

    Drawer D;
    Drawer* DrPt = &D;
    D.yPos = LINES - 1;
    D.xPos = 0;

    int userInput;

    refresh();

    raw();
    noecho();
    curs_set(0);

    while(1)
    {
        userInput = getch();
        if(userInput == 10)         //Enter Key
        {
            drawLevel(DrPt);
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
            setLanderYPos(LunarPt, LunarPt->yPos - 1);
        }
        else if(userInput == 97)    //A Key
        {
            setLanderXPos(LunarPt, LunarPt->xPos - 1);
        }
        else if(userInput == 115)    //S Key
        {
            setLanderYPos(LunarPt, LunarPt->yPos + 1);
        }
        else if(userInput == 100)    //D Key
        {
            setLanderXPos(LunarPt, LunarPt->xPos + 1);
        }

        refresh();
    }

    printf("%d\n", userInput);

    return 0;
}
