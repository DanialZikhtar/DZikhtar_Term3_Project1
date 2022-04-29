#include <stdio.h>
#include <string.h>

const int limitX = 360;
const int limitY = 360;

typedef struct Lander
{
    double xPos;
    double yPos;
} Lander;

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

void DEBUG_PrintPos(Lander* pt)
{
    printf("X:%f\nY:%f\n", pt->xPos, pt->yPos);
}

int main()
{
    Lander Lunar;
    Lander* LunarPt = &Lunar;
    resetInstance(LunarPt);

    for(int i = 0; i < 9; i++)
    {
        setXPos(LunarPt, Lunar.xPos + 5);
    }

    DEBUG_PrintPos(LunarPt);

    return 0;
}