#include "SDL.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "string.h"
#include <stack>
#include <cmath>
#include <random>

#define PI 3.14159265359

#define RENDER_WALLS 0x01
#define RENDER_SHADED 0x02

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

static std::random_device rd;
static std::mt19937 engine(rd());
static std::uniform_int_distribution<int> directionsDist(0, 3);
static std::uniform_int_distribution<int> colorComponentDist(0, 255);
static std::uniform_int_distribution<int> colorDist(0, 0xffffff);

struct Coordinates
{
    uint32 X;
    uint32 Y;
};

struct Cell
{
    Coordinates position;
    uint32 distFromStart;
    uint32 visited;
    Cell *neighbours[4];
};

struct Maze
{
    uint32 width;
    uint32 height;
    Coordinates start;
    uint32 maxDistance;
    Cell **cells;
};

struct RGBcolor
{
    uint8 red;
    uint8 green;
    uint8 blue;
};

int randomDir_usingRand() {
    return rand()%4;
}

int randomDir_uniform() {
    return directionsDist(engine);
}

int randomDir_weird() {
    return (rand()%2 + rand()%2 + rand()%2 + rand()%2)%4;
}

int randomDir_horizontal(int range) {
    int dir = rand()%range;

    int ret;
    if(dir < 1)
        ret = 0;
    if(dir > 0 && dir < range/2)
        ret = 1;
    if(dir > range/2-1 && dir < range/2+1)
        ret = 2;
    if(dir > range/2 && dir < range)
        ret = 3;

    return ret;
}

int randomDir_vertical(int range) {
    int dir = rand()%range;

    int ret;
    if(dir < 1)
        ret = 1;
    if(dir > 0 && dir < range/2)
        ret = 0;
    if(dir > range/2-1 && dir < range/2+1)
        ret = 3;
    if(dir > range/2 && dir < range)
        ret = 2;

    return ret;
}

int randomDir_changing(int prevDir) {
    int ret = 0;
    switch(prevDir)
    {
    case 0:
    case 2:
        ret = randomDir_horizontal(100);
        break;
    case 1:
    case 3:
        ret = randomDir_vertical(100);
        break;
    }

    return ret;
}

void process_distanceFromStart(Maze &maze)
{
    uint32 distance = 0, maxDistance = 0;

    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            maze.cells[X][Y].visited = 0;
        }
    }
    std::stack<Cell *> backtrack;
    backtrack.push(&maze.cells[maze.start.X][maze.start.Y]);

    while(!backtrack.empty())
    {
        Cell *cursor = backtrack.top();
        maze.cells[cursor->position.X][cursor->position.Y].visited++;

        bool unvisitedNeighbours = false;
        for (int i = 0; i < 4; ++i)
        {
            Cell *neighbour = cursor->neighbours[i];
            if(neighbour != NULL)
            {
                unvisitedNeighbours |= (maze.cells[neighbour->position.X][neighbour->position.Y].visited == 0);
            }
        }

        if(unvisitedNeighbours)
        {
            int direction;
            Cell *next = NULL;
            while(next == NULL)
            {
                direction = rand() % 4;
                next = cursor->neighbours[direction];
            }

            if(maze.cells[next->position.X][next->position.Y].visited == 0)
            {
                backtrack.push(next);
                maze.cells[next->position.X][next->position.Y].visited++;
                cursor->distFromStart = distance++;
                if(distance > maxDistance)
                {
                    maxDistance = distance;
                }
                cursor = next;
            }
        }
        else
        {
            if(backtrack.size()>1) {
                cursor->distFromStart = distance--;
                cursor = backtrack.top();
            }
            backtrack.pop();
        }
    }

    maze.maxDistance = maxDistance;
}

void generate_recursiveBacktrack(Maze &maze, int (*randomDir)())
{
    std::stack<Coordinates> backtrack;
    Coordinates cursor = {};
    cursor.X = rand() % maze.height;
    cursor.Y = rand() % maze.width;

    maze.start = cursor;

    backtrack.push(cursor);
    maze.cells[cursor.X][cursor.Y].visited++;
    while(!backtrack.empty())
    {
        bool unvisitedNeighbours = false;

        if(cursor.X < maze.height-1)
            unvisitedNeighbours |= (!maze.cells[cursor.X+1][cursor.Y].visited &&
                                    cursor.X+1 < maze.height);
        if(cursor.X > 0)
            unvisitedNeighbours |= (!maze.cells[cursor.X-1][cursor.Y].visited &&
                                    cursor.X-1 < maze.height);
        if(cursor.Y < maze.width-1)
            unvisitedNeighbours |= (!maze.cells[cursor.X][cursor.Y+1].visited &&
                                    cursor.Y+1 < maze.width);
        if(cursor.Y > 0)
            unvisitedNeighbours |= (!maze.cells[cursor.X][cursor.Y-1].visited &&
                                    cursor.Y-1 < maze.width);

        if(unvisitedNeighbours)
        {
            Coordinates next = cursor;
            int direction = 0;

            while(  next.X >= maze.height ||
                    next.Y >= maze.width ||
                    maze.cells[next.X][next.Y].visited )
            {
                direction = randomDir();
                next = cursor;

                switch(direction)
                {
                case 0:
                    next.X--;
                    break;
                case 1:
                    next.Y++;
                    break;
                case 2:
                    next.X++;
                    break;
                case 3:
                    next.Y--;
                    break;
                }
            }

            backtrack.push(next);
            maze.cells[next.X][next.Y].visited++;
            maze.cells[cursor.X][cursor.Y].neighbours[direction] = &maze.cells[next.X][next.Y];
            maze.cells[next.X][next.Y].neighbours[(direction+2)%4] = &maze.cells[cursor.X][cursor.Y];
            cursor = next;
        }
        else
        {
            cursor = backtrack.top();
            backtrack.pop();
        }
    }
}

void generate_binaryTree(Maze &maze)
{
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            if(X != 0 || Y != 0)
            {
                Coordinates target;
                target.X = X;
                target.Y = Y;

                int direction = (rand() % 2 + 3)%4; // Yields either 0 or 3
                if(X == 0) direction = 3;
                if(Y == 0) direction = 0;

                switch(direction)
                {
                case 0:
                    target.X--;
                    break;
                case 3:
                    target.Y--;
                    break;
                }

                maze.cells[X][Y].neighbours[direction] = &maze.cells[target.X][target.Y];
                maze.cells[target.X][target.Y].neighbours[(direction+2)%4] = &maze.cells[X][Y];
            }
        }
    }
}

void buildMaze(Maze &maze, int (*randomDir)())
{
    maze.cells = (Cell **)malloc(sizeof(Cell*)*maze.height);

    // Creating all of the cells with closed doors
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        maze.cells[X] = (Cell *)malloc(sizeof(Cell)*maze.width);
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            Cell dummy = {};
            dummy.distFromStart = 0;
            dummy.visited = 0;
            dummy.position.X = X;
            dummy.position.Y = Y;
            for(int N = 0; N < 4; ++N)
            {
                dummy.neighbours[N] = NULL;
            }

            maze.cells[X][Y] = dummy;
        }
    }

    // Generate the maze
    generate_recursiveBacktrack(maze, randomDir);

    process_distanceFromStart(maze);
}

void destroyMaze(Maze &maze)
{
    for(uint32 i = 0; i < maze.height; i++)
    {
        free(maze.cells[i]);
    }
    free(maze.cells);
}

void SDL_DrawCircle(SDL_Surface *surface, Coordinates &center, int R, uint32 colour)
{
    int X = 0;
    int Y = R;
    int d = R-1;
    uint32 *buffer = (uint32 *)surface->pixels;

    while(Y >= X)
    {
        *(buffer+ (center.X + X)*surface->w + (center.Y + Y)) = colour;
        *(buffer+ (center.X + Y)*surface->w + (center.Y + X)) = colour;
        *(buffer+ (center.X - X)*surface->w + (center.Y + Y)) = colour;
        *(buffer+ (center.X - Y)*surface->w + (center.Y + X)) = colour;
        *(buffer+ (center.X + X)*surface->w + (center.Y - Y)) = colour;
        *(buffer+ (center.X + Y)*surface->w + (center.Y - X)) = colour;
        *(buffer+ (center.X - X)*surface->w + (center.Y - Y)) = colour;
        *(buffer+ (center.X - Y)*surface->w + (center.Y - X)) = colour;

        if(d >= 2*X)
        {
            d = d-2*X-1;
            X++;
        }
        else if(d < 2*(R-Y))
        {
            d = d+2*Y-1;
            Y--;
        }
        else
        {
            d = d+2*(Y-X-1);
            Y--;
            X++;
        }
    }
}

void SDL_DrawLine(SDL_Surface *surface, Coordinates A, Coordinates B, uint32 colour)
{
    uint32 *buffer = (uint32 *)surface->pixels;
    int X = A.X;
    int Y = A.X;
    int W = B.X - A.X;
    int H = B.Y - A.Y;
    int dX1 = 0-(W<0)+(W>0);
    int dX2 = dX1;
    int dY1 = 0-(H<0)+(H>0);
    int dY2 = 0;

    int longest = abs(W);
    int shortest = abs(H);
    if(longest < shortest)
    {
        longest = abs(H);
        shortest = abs(W);
        dY2 = 0-(H<0)+(H>0);
        dX2 = 0;
    }

    int numerator = longest/2;
    for(int i = 0; i <= longest; i++)
    {
        *(buffer+ (X)*surface->w + (Y)) = colour;
        numerator += shortest;
        if(numerator>longest)
        {
            numerator -= longest;
            X += dX1;
            Y += dY1;
        }
        else
        {
            X += dX2;
            Y += dY2;
        }
    }
}

void SDL_DrawOrientedLine(SDL_Surface *surface,
                          Coordinates O,
                          double angle,
                          int start,
                          int length,
                          uint32 colour)
{
    Coordinates A = {}, B = {};
    double sine = sin(angle);
    double cosine = cos(angle);

    A.X = O.X + round(sine*start);
    A.Y = O.Y + round(cosine*start);
    B.X = O.X + round(sine*(start+length));
    B.Y = O.Y + round(cosine*(start+length));

    SDL_DrawLine(surface, A, B, colour);
}

void renderMaze_Walls(SDL_Surface *buffer, Maze &maze)
{ 
    uint32 BLACK = 0x00000000;
    uint32 WHITE = 0xffffffff;

    uint8 *row = (uint8 *)buffer->pixels;
    uint8 *nextRow = row + buffer->pitch;
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        uint32 *pixel = (uint32 *)row;
        uint32 *nextPixel = (uint32 *)nextRow;
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            *pixel++ = BLACK;
            *pixel++ = (maze.cells[X][Y].neighbours[0] == NULL) ? BLACK : WHITE;
            *nextPixel++ = (maze.cells[X][Y].neighbours[3] == NULL) ? BLACK : WHITE;
            *nextPixel++ = WHITE;
        }
        *pixel++ = BLACK;
        *nextPixel++ = (maze.cells[X][maze.width-1].neighbours[1] == NULL) ? BLACK : WHITE;

        row += buffer->pitch*2;
        nextRow += buffer->pitch*2;
    }
}

void renderMaze_WallsShaded(SDL_Surface *buffer, Maze &maze, RGBcolor startColor, RGBcolor maxColor)
{    
    uint32 BLACK = 0x00000000;
    uint32 COLOUR = 0xffffffff;

    uint32 maxDistance = maze.maxDistance;

    uint8 *row = (uint8 *)buffer->pixels;
    uint8 *nextRow = row + buffer->pitch;
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        uint32 *pixel = (uint32 *)row;
        uint32 *nextPixel = (uint32 *)nextRow;
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            double coeff = (double)maze.cells[X][Y].distFromStart / (double)maxDistance;
            RGBcolor cellColor;
            cellColor.red = (startColor.red + (uint8)(coeff * (maxColor.red - startColor.red)))%256;
            cellColor.green = (startColor.green + (uint8)(coeff * (maxColor.green - startColor.green)))%256;
            cellColor.blue = (startColor.blue + (uint8)(coeff * (maxColor.blue - startColor.blue)))%256;

            COLOUR = ((cellColor.red << 24) | (cellColor.green << 16) | (cellColor.blue << 8));

            *pixel++ = BLACK;
            *pixel++ = (maze.cells[X][Y].neighbours[0] == NULL) ? BLACK : COLOUR;
            *nextPixel++ = (maze.cells[X][Y].neighbours[3] == NULL) ? BLACK : COLOUR;
            *nextPixel++ = COLOUR;
        }
        *pixel++ = BLACK;
        *nextPixel++ = (maze.cells[X][maze.width-1].neighbours[1] == NULL) ? BLACK : COLOUR;

        row += buffer->pitch*2;
        nextRow += buffer->pitch*2;
    }
}

void renderMaze_Shaded(SDL_Surface *buffer, 
                       Maze &maze, RGBcolor startColor, RGBcolor maxColor)
{
    uint32 maxDistance = maze.maxDistance;

    uint8 *row = (uint8 *)buffer->pixels;
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        uint32 *pixel = (uint32 *)row;
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            double coeff = (double)maze.cells[X][Y].distFromStart / (double)maxDistance;

            RGBcolor cellColor;
            cellColor.red = (startColor.red + (uint8)(coeff * (maxColor.red - startColor.red)))%256;
            cellColor.green = (startColor.green + (uint8)(coeff * (maxColor.green - startColor.green)))%256;
            cellColor.blue = (startColor.blue + (uint8)(coeff * (maxColor.blue - startColor.blue)))%256;

            *pixel++ = ((cellColor.red << 24) | (cellColor.green << 16) | (cellColor.blue << 8));
        }
        row += buffer->pitch;
    }
}

void renderMaze_TwoShaded(SDL_Surface *buffer, 
                          Maze &maze, 
                          RGBcolor colors[4], 
                          uint32 gradiantThreshold)
{
    uint32 maxDistance = maze.maxDistance;

    uint8 *row = (uint8 *)buffer->pixels;
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        uint32 *pixel = (uint32 *)row;
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            RGBcolor* startColor;
            RGBcolor* maxColor;

            uint32 distanceFromStart = maze.cells[X][Y].distFromStart;
            double coeff = 0.0f;
            if(distanceFromStart < gradiantThreshold)
            {
                coeff = (double)distanceFromStart / 
                        (double)gradiantThreshold;
                startColor = colors;
                maxColor = colors + 1;
            }
            else
            {
                coeff = (double)(distanceFromStart - gradiantThreshold) / 
                        (double)(maxDistance - gradiantThreshold);
                startColor = colors + 2;
                maxColor = colors + 3;
            }

            RGBcolor cellColor;
            cellColor.red = (startColor->red +
                             (uint8)(coeff * (maxColor->red - startColor->red)))%256;
            cellColor.green = (startColor->green +
                               (uint8)(coeff * (maxColor->green - startColor->green)))%256;
            cellColor.blue = (startColor->blue +
                              (uint8)(coeff * (maxColor->blue - startColor->blue)))%256;

            *pixel++ = ((cellColor.red << 24) | (cellColor.green << 16) | (cellColor.blue << 8));
        }
        row += buffer->pitch;
    }
}

void renderGradiant(SDL_Surface *buffer)
{
    uint8 *row = (uint8 *)buffer->pixels;
    for(int Y = 0;
        Y < buffer->h;
        ++Y)
    {
        uint32 *pixel = (uint32 *)row;
        for(int X = 0;
            X < buffer->w;
            ++X)
        {
            uint8 red = X;
            uint8 green = Y;
            uint8 blue = 0;
            uint8 alpha = 0;

            *pixel++ = ((red << 24) | (green << 16) | (blue << 8) | alpha);
        }
        row += buffer->pitch;
    }
}

bool AreStringsEqual(const char* str1, const char* str2)
{
    bool areEqual = false;

    if(strlen(str1) == strlen(str2))
    {
        areEqual = true;
        for(uint32 i = 0; i < strlen(str1); i++)
        {
            if(str1[i] != str2[i])
            {
                areEqual = false;
                break;
            }
        }
    }

    return areEqual;
}


// NOTE(samu); rawMaxColor is AA RR GG BB formated
inline RGBcolor intToRGBColor(uint32 value)
{
    RGBcolor result = {};

    result.blue = (value & 0x000000ff);
    result.green = (value & 0x0000ff00) >> 8;
    result.red = (value & 0x00ff0000) >> 16;

    return result;
}

inline void MakeRandomColor(RGBcolor* color)
{
    uint32 value = colorDist(engine);
    *color = intToRGBColor(value);
}

inline int FindLastDot(const char* filename)
{
    int lastDotIndex = -1;
    for(int i = 0; i < (int)strlen(filename); i++)
    {
        if(filename[i] == '.')
        {
            lastDotIndex = i;
        }
    }

    return lastDotIndex;
}

int main (int argc, char* argv[]) {

/*
    TODO (samu): command line options
        Done* -R [walls|shaded] : 
                    chose the way the maze is going to be displayed
                    both can be selected by calling the option twice
        Done* -c <n> [<color1> .. <colorn> | random] : colorpicking (n between 1 and 4)
        Done* -b <n>: batch generation
        * -v : verbose
 */

    srand(time(NULL));

    int mazeWidth = 50;
    int mazeHeight = 50;
    const char* filename = "maze.bmp";

    uint8 renderType = 0;
    int (*randomDirFunction)() = randomDir_uniform;

    int mazeCount = 1;

    bool randomColor = true;
    int colorCount = 2;
    RGBcolor colors[4] = {};
    colors[1] = intToRGBColor(0xffffff);

    if(argc < 4) {
        printf("usage: aMAZEd <mazeWidth> <mazeHeight> <fileName>");
        return 0;
    }
    if(argc >= 4) {
        for(int i = 0; i < argc; i++)
        {
            if(AreStringsEqual(argv[i], "-R"))
            {
                i++;
                uint8 param = 0;

                if(AreStringsEqual(argv[i], "walls"))
                {
                    param = RENDER_WALLS;
                }

                if(AreStringsEqual(argv[i], "shaded"))
                {
                    param = RENDER_SHADED;
                }

                renderType |= param;
            }

            if(AreStringsEqual(argv[i], "-c"))
            {
                i++;
                colorCount = atoi(argv[i++]);
                if(colorCount > 4)
                {
                    printf("Must specify a max of 4 colors\n");
                    return 1;
                }
                
                if(AreStringsEqual(argv[i], "random"))
                {
                    randomColor = true;
                }
                else
                {
                    randomColor = false;

                    for(int j = 0; j < colorCount; j++)
                    {
                // TODO(samu): This currently supports only 0x000000 formated input
                        int rawColor = strtol(argv[i + j], nullptr, 0);
                        colors[j] = intToRGBColor(rawColor);
                    }

                    i += colorCount-1;
                }
                
                if(colorCount == 3)
                {
                    colors[3] = colors[2];
                    colors[2] = colors[1];
                }
            }

            if(AreStringsEqual(argv[i], "-d"))
            {
                i++;

                if(AreStringsEqual(argv[i], "uniform"))
                {
                    randomDirFunction = randomDir_uniform;
                }
                else if(AreStringsEqual(argv[i], "rand"))
                {
                    randomDirFunction = randomDir_usingRand;
                }
            }

            if(AreStringsEqual(argv[i], "-b"))
            {
                i++;

                mazeCount = atoi(argv[i]);
            }
        }
    }

    if(renderType == 0)
    {
        renderType = 3;
    }

    mazeWidth = atoi(argv[1]);
    mazeHeight = atoi(argv[2]);
    filename = argv[3];

    Maze maze = {};
    maze.width = mazeWidth;
    maze.height = mazeHeight;

    SDL_Init(SDL_INIT_VIDEO);

    uint32 mazeSurfaceWidth = maze.width;
    uint32 mazeSurfaceHeight = maze.height;
    if(renderType & RENDER_WALLS)
    {
        mazeSurfaceWidth = maze.width*2 + 1;
        mazeSurfaceHeight = maze.height*2 + 1;
    }

#if 1
    for(int i = 0; i < mazeCount; i++)
    {
        SDL_Surface* mazeSurface = {};
        mazeSurface = SDL_CreateRGBSurface(0,
                                           mazeSurfaceWidth,
                                           mazeSurfaceHeight,
                                           32,
                                           0xff000000,
                                           0x00ff0000,
                                           0x0000ff00,
                                           0x000000ff);

        if(randomColor)
        {
            for(int j = 0; j < colorCount; j++)
            {
                MakeRandomColor(&colors[j]);
            }
        }
        printf("Building maze %d..\n", i);
        buildMaze(maze, randomDirFunction);
        printf("Maze built\n");

        printf("Rendering the maze.. \n");
        if((renderType & RENDER_WALLS) != 0)
        {
            if((renderType & RENDER_SHADED) != 0)
            {
                renderMaze_WallsShaded(mazeSurface, maze, colors[0], colors[1]);
            }
            else
            {
                renderMaze_Walls(mazeSurface, maze);
            }
        }
        else if((renderType & RENDER_SHADED) != 0)
        {
            switch(colorCount)
            {
                case 1:
                case 2:
                {
                    renderMaze_Shaded(mazeSurface, maze, colors[0], colors[1]); 
                } break;
                case 3:
                {
                    renderMaze_TwoShaded(mazeSurface, 
                                         maze, 
                                         colors, 
                                         maze.maxDistance / 2);
                } break;
                case 4:
                {
                    renderMaze_TwoShaded(mazeSurface, 
                                         maze, 
                                         colors,
                                         maze.maxDistance / 2);
                } break;
            }
        }

        printf("Maze rendered\n");
        printf("Saving the maze to a file..\n");

        char filenameArray[512] = "";
        if(mazeCount > 1)
        {
            char* tmpBuffer = (char*)filename;

            int lastDotIndex = FindLastDot(filename);
            if(lastDotIndex >= 0)
            {
                tmpBuffer[lastDotIndex] = '\0';
            }

            sprintf(filenameArray, "%s%d.bmp", tmpBuffer, i);
        }
        else
        {
            strcpy(filenameArray, filename);
        }

        if(SDL_SaveBMP(mazeSurface, filenameArray))
        {
            printf("Image couldn't be saved : \n%s\n", SDL_GetError());
        }
        printf("Maze saved\n\n");

        destroyMaze(maze);
        SDL_FreeSurface(mazeSurface);
    }

#else
#if 0
    colors[0].blue = 0xff;
    colors[0].green = 0x00;
    colors[0].red = 0x00;

    colors[1].blue = 0xff;
    colors[1].green = 0x88;
    colors[1].red = 0x00;

    colors[2].blue = 0x00;
    colors[2].green = 0xff;
    colors[2].red = 0x00;

    colors[3].blue = 0x20;
    colors[3].green = 0x50;
    colors[3].red = 0xaa;
#else
    MakeRandomColor(&colors[0]);
    MakeRandomColor(&colors[1]);
    MakeRandomColor(&colors[2]);
    MakeRandomColor(&colors[3]);
#endif

    buildMaze(maze, randomDirFunction);

    mazeSurface = SDL_CreateRGBSurface(0,
                                    mazeWidth,
                                    mazeHeight,
                                    32,
                                    0xff000000,
                                    0x00ff0000,
                                    0x0000ff00,
                                    0x000000ff);

    uint32 gradiantThreshold = (maze.maxDistance / 2);
    renderMaze_TwoShaded(mazeSurface, maze, colors, gradiantThreshold);
    SDL_SaveBMP(mazeSurface, "test_twoshaded.bmp");
#endif

    SDL_Quit();

    return 0;
}
