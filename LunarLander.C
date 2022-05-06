#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>

const int limitX = 360;
const int limitY = 360;


typedef struct Lander
{
    double xPos;
    double yPos;
} Lander;

typedef struct Drawer
{
    int xPos = 0;
    int yPos = 0;
} Drawer;

void setXPos(Lander* pt, double X)
{
    pt->xPos = X;
    return;
}

void setYPos(Lander* pt, double Y)
{
    pt->yPos = Y;
    return;
}

void resetInstance(Lander* pt)
{
    pt->xPos = 0;
    pt->yPos = 0;

    return;
}

void drawGround(Drawer* DrPt)
{
    mvprintw(DrPt->yPos, DrPt->xPos, "_");
    DrPt->xPos++;
}

void drawUpSlope(Drawer* DrPt)
{
    mvprintw(DrPt->yPos, DrPt->xPos, "/");
    DrPt->yPos--;
    DrPt->xPos++;
}

void drawDownSlope(Drawer* DrPt)
{
    DrPt->yPos++;
    mvprintw(DrPt->yPos, DrPt->xPos, "\\");
    DrPt->xPos++;
}

void DEBUG_PrintPos(Lander* pt)
{
    printf("X:%f\nY:%f\n", pt->xPos, pt->yPos);
}

void drawLevel(Drawer* DrPt)
{
    int ranA = rand() % 18;
    int ranB = rand() % 10 - 5;
    while(DrPt->xPos < limitX)
    {
        if(ranA > 0)
        {
            drawGround(DrPt);
            ranA--;
        }
        else
        {
            if(ranB > 0)
            {
                drawUpSlope(DrPt);
                ranB--;
            }
            else if(ranB < 0)
            {
                drawDownSlope(DrPt);
                ranB++;
            }
            else
            {
                ranA = rand() % 18;
                ranB = rand() % 10 - 5;
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
    resetInstance(LunarPt);

    Drawer D;
    Drawer* DrPt = &D;
    D.yPos = LINES - 1;
    D.xPos = 0;

    int userInput;

    refresh();

    while(1)
    {
        raw();
        noecho();

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
            drawUpSlope(DrPt);
        }
        else if(userInput == 97)    //A Key
        {
            
        }
        else if(userInput == 115)    //S Key
        {
            drawDownSlope(DrPt);
        }
        else if(userInput == 100)    //D Key
        {
            drawGround(DrPt);
        }

        refresh();
    }

    printf("%d\n", userInput);

    return 0;
}
