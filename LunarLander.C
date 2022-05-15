#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>



//Arbitrary limit for stopping draw function
const int limitX = 720;
const int limitY = 720;




//The player-controlled object.
//Its just a drawer object that prints everytime its moved.
typedef struct Lander
{
    int yPos = 0;
    int xPos = 0;
    int fuel = 500;
    int rotOffset = 0;
    char landerChar = '^';
} Lander;

//Checks if position (y,x) contains a character
bool containChar(int y, int x)
{
    //Empty-case (spacebar or 'allowed' characters)
    if(mvinch(y,x) == 32)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void setLanderPos(Lander* LnPt, int y, int x)
{
    //Check if target destination is empty
    if(containChar(y,x) == true)
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

void rotateLeft(Lander* LnPt)
{
    if(LnPt->landerChar == '>')
    {
        LnPt->landerChar = '^';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else if(LnPt->landerChar == '^')
    {
        LnPt->landerChar = '<';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else
    {
        //Rotation failed
        return;
    }

    return;
}

void rotateRight(Lander* LnPt)
{
    if(LnPt->landerChar == '<')
    {
        LnPt->landerChar = '^';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else if(LnPt->landerChar == '^')
    {
        LnPt->landerChar = '>';
        mvaddch(LnPt->yPos, LnPt->xPos, LnPt->landerChar);
    }
    else
    {
        //Rotation failed
        return;
    }

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
//Drawer should point to bottom left of intended screen space before calling
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
    Lunar.fuel = 500;

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
    halfdelay(1);
    curs_set(0);

    while(1)
    {
        mvprintw(0, COLS - 20, "Fuel: %d", LunarPt->fuel);
        
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
            LunarPt->fuel--;
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

        if(tick % 12 == 0)
        {
            setLanderPos(LunarPt, LunarPt->yPos + 1, LunarPt->xPos);
        }

        refresh();
        tick++;
    }

    printf("%d\n", userInput);

    return 0;
}
