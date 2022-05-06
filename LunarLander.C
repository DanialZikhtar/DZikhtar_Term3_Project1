#include <stdio.h>
#include <string.h>
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

void DEBUG_PrintPos(Lander* pt)
{
    printf("X:%f\nY:%f\n", pt->xPos, pt->yPos);
}

int main()
{
    initscr();

    Lander Lunar;
    Lander* LunarPt = &Lunar;
    resetInstance(LunarPt);

    Drawer D;
    Drawer* DrPt = &D;
    D.yPos = LINES - 1;
    D.xPos = 0;

    refresh();

    while(1)
    {
        raw();
        noecho();

        int userInput = getch();
        if(userInput == 10)         //Enter Key
        {

        }
        else if(userInput == 32)    //Spacebar Key
        {
            drawGround(DrPt);
        }
        else if(userInput == 27)    //Escape key
        {
            endwin();
            break;
        }

        refresh();
    }

    printf("%d, %d\n", D.yPos, D.xPos);
    printf("%d\n %d\n", LINES, COLS);

    return 0;
}