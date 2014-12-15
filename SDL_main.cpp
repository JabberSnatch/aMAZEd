#include "SDL.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <stack>
#include <cmath>

#define PI 3.14159265359

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Coordinates
{
    int X;
    int Y;
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

int randomDir_uniform() {
    return rand() % 4;
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

        /*for(uint32 i = 0; i < maze.height; i++) {
            for (uint32 j = 0; j < maze.width; j++) {
                printf("%d ", visitedCells[i][j]);
            }
            printf("\n");
        }
        printf("\n");*/
    }

    maze.maxDistance = maxDistance;
}

void generate_recursiveBacktrack(Maze &maze)
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
        unvisitedNeighbours |= (!maze.cells[cursor.X+1][cursor.Y].visited && cursor.X+1 < maze.height);
        if(cursor.X > 0)
        unvisitedNeighbours |= (!maze.cells[cursor.X-1][cursor.Y].visited && cursor.X-1 < maze.height);
        if(cursor.Y < maze.width-1)
        unvisitedNeighbours |= (!maze.cells[cursor.X][cursor.Y+1].visited && cursor.Y+1 < maze.width);
        if(cursor.Y > 0)
        unvisitedNeighbours |= (!maze.cells[cursor.X][cursor.Y-1].visited && cursor.Y-1 < maze.width);

        if(unvisitedNeighbours)
        {
            Coordinates next = cursor;
            int direction = 0;

            while(  next.X >= maze.height ||
                    next.Y >= maze.width ||
                    maze.cells[next.X][next.Y].visited )
            {
                direction = randomDir_uniform();
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

void buildMaze(Maze &maze)
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
            for(int N = 0;
                N < 4;
                ++N)
            {
                dummy.neighbours[N] = NULL;
                dummy.distFromStart = 0;
                dummy.visited = 0;
                dummy.position.X = X;
                dummy.position.Y = Y;
            }
            maze.cells[X][Y] = dummy;
        }
    }

    // Generate the maze
    generate_recursiveBacktrack(maze);

    process_distanceFromStart(maze);
}

void destroyMaze(Maze &maze)
{
    for(int i = 0; i < maze.height; i++)
    {
        free(maze.cells[i]);
    }
    free(maze.cells);
    maze = {};
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

void renderMaze_Shaded(SDL_Surface *buffer, Maze &maze, RGBcolor startColor, RGBcolor maxColor)
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

int main (int argc, char** argv) {

/*
    TODO (samu): command line options
        * -R [walls|shaded] : chose the way the maze is going to be displayed
        * -r : random
        * -c <start> <end> : colorpicking
        * -b <nb>: batch generation
        * -v : verbose
 */

    bool randomCol = false;
    if(argc < 4) {
        printf("usage: aMAZEd <mazeWidth> <mazeHeight> <fileName>");
        return 0;
    }
    if(argc > 4) {
        randomCol = true;
    }

    srand(time(NULL));

    int mazeWidth = atoi(argv[1]);
    int mazeHeight = atoi(argv[2]);
    const char* filename = argv[3];

#if 1
    for(int i = 0; i < 200; i++)
    {
        char filename [50] = "";
        sprintf(filename, "wallpapers\\%d.bmp", i);


    SDL_Surface* mazeSurface;

    Maze maze = {};
    maze.width = mazeWidth;
    maze.height = mazeHeight;

    printf("-%d:\n", i);
    printf("Building the maze..\n");
    buildMaze(maze);
    printf("Maze built\n");

    RGBcolor startColor;
    RGBcolor maxColor;
    if(randomCol)
    {
        startColor.red = rand() % 255;
        startColor.green = rand() % 255;
        startColor.blue = rand() % 255;

        maxColor.red = rand() % 255;
        maxColor.green = rand() % 255;
        maxColor.blue = rand() % 255;
    }
    else
    {
        startColor.red = 0x00;
        startColor.green = 0x56;
        startColor.blue = 0x1b;

        maxColor.red = 0x8a;
        maxColor.green = 0x33;
        maxColor.blue = 0x24;
    }

    SDL_Init(SDL_INIT_VIDEO);

//    mazeSurface = SDL_CreateRGBSurface(0,
//                                  mazeWidth * 2 + 1,
//                                  mazeHeight * 2 + 1,
//                                  32,
//                                  0xff000000,
//                                  0x00ff0000,
//                                  0x0000ff00,
//                                  0x000000ff);
//    renderMaze_Walls(mazeSurface, maze);
//    //SDL_SaveBMP(mazeSurface, "walls.bmp");
//
//    renderMaze_WallsShaded(mazeSurface, maze, startColor, maxColor);
    //SDL_SaveBMP(mazeSurface, "walls_shaded.bmp");

    mazeSurface = SDL_CreateRGBSurface(0,
                                  mazeWidth,
                                  mazeHeight,
                                  32,
                                  0xff000000,
                                  0x00ff0000,
                                  0x0000ff00,
                                  0x000000ff);
    printf("Rendering the maze.. \n");
    renderMaze_Shaded(mazeSurface, maze, startColor, maxColor);
    printf("Maze rendered\n");
    //SDL_SaveBMP(mazeSurface, "shade.bmp");

    printf("Saving the maze to a file..\n");
    if(SDL_SaveBMP(mazeSurface, filename))
    {
        printf("Image couldn't be saved : \n%s\n", SDL_GetError());
    }
    printf("Maze saved\n\n");

    destroyMaze(maze);
    SDL_FreeSurface(mazeSurface);
    }

#else
    mazeSurface = SDL_CreateRGBSurface(0,
                                  1001,
                                  1001,
                                  32,
                                  0xff000000,
                                  0x00ff0000,
                                  0x0000ff00,
                                  0x000000ff);

    Coordinates A = {}, B = {}, C = {};
    A.X = 50;
    A.Y = 20;
    B.X = 99;
    B.Y = 50;
    C.X = 4000;
    C.Y = 4000;

    // TODO(samu): Make SDL_DrawLine commutative
    //SDL_DrawLine(mazeSurface, A, B);
    //SDL_DrawLine(mazeSurface, C, A);
    //SDL_DrawLine(mazeSurface, B, C);
    //SDL_DrawCircle(mazeSurface, C, 3000, 0xaaaaaa00);

    Coordinates O = {};
    O.X = 500;
    O.Y = 500;
    uint8 red = 0x00;
    uint8 blue = 0xff;
    uint8 green = 0x00;
    int step = floor(255.0f / 85.0f);
    for(int R = 0; R < 255; R++)
    {
        if(R < 85){
            blue -= step;
            green += step;
        }
        else if(R < 170){
            green -= step;
            red += step;
        }
        else {
            red -= step;
            blue += step;
        }
        SDL_DrawCircle(mazeSurface, O, R+10, ((red << 24) | (green << 16) | (blue << 8)));
    }
    SDL_SaveBMP(mazeSurface, "test_line.bmp");
#endif

    SDL_Quit();

    return 0;
}
