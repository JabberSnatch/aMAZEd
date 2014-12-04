#include "SDL.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <stack>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Coordinates
{
    uint32 X;
    uint32 Y;
};

struct Cell
{
    Coordinates position;
    uint32 distFromStart;
    Cell *neighbours[4];
};

struct Maze
{
    uint32 width;
    uint32 height;
    Coordinates start;
    Cell **cells;
};

struct RGBcolor
{
    uint8 red;
    uint8 green;
    uint8 blue;
};

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
                dummy.position.X = X;
                dummy.position.Y = Y;
            }
            maze.cells[X][Y] = dummy;
        }
    }

    // Generate the maze
    printf("Generating the maze..\n");
    uint32 visitedCells[maze.height][maze.width];
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            visitedCells[X][Y] = 0;
        }
    }
    std::stack<Coordinates> backtrack;
    Coordinates cursor = {};
    cursor.X = rand() % maze.height;
    cursor.Y = rand() % maze.width;

    maze.start = cursor;

    backtrack.push(cursor);
    visitedCells[cursor.X][cursor.Y]++;
    while(!backtrack.empty())
    {
        bool unvisitedNeighbours = false;
        unvisitedNeighbours |= (!visitedCells[cursor.X+1][cursor.Y] && cursor.X+1 < maze.height);
        unvisitedNeighbours |= (!visitedCells[cursor.X-1][cursor.Y] && cursor.X-1 < maze.height);
        unvisitedNeighbours |= (!visitedCells[cursor.X][cursor.Y+1] && cursor.Y+1 < maze.width);
        unvisitedNeighbours |= (!visitedCells[cursor.X][cursor.Y-1] && cursor.Y-1 < maze.width);

        if(unvisitedNeighbours)
        {
            Coordinates next = cursor;
            int direction = 0;

            while(  next.X >= maze.height ||
                    next.Y >= maze.width ||
                    visitedCells[next.X][next.Y] )
            {
                direction = rand() % 4;
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
            visitedCells[next.X][next.Y]++;
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
    printf("Maze generated\n");
}

uint32 process_distanceFromStart(Maze &maze)
{
    uint32 distance = 0, maxDistance = 0;

    uint32 visitedCells[maze.height][maze.width];
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            visitedCells[X][Y] = 0;
        }
    }
    std::stack<Cell *> backtrack;
    backtrack.push(&maze.cells[maze.start.X][maze.start.Y]);

    while(!backtrack.empty())
    {
        Cell *cursor = backtrack.top();
        visitedCells[cursor->position.X][cursor->position.Y]++;

        bool unvisitedNeighbours = false;
        for (int i = 0; i < 4; ++i)
        {
            Cell *neighbour = cursor->neighbours[i];
            if(neighbour != NULL)
            {
                unvisitedNeighbours |= (visitedCells[neighbour->position.X][neighbour->position.Y] == 0);
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

            if(visitedCells[next->position.X][next->position.Y] == 0)
            {
                backtrack.push(next);
                visitedCells[next->position.X][next->position.Y]++;
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

    return maxDistance;
}

void renderMaze_Walls(SDL_Surface *buffer, Maze &maze)
{
    uint32 BLACK = 0x00000000;
    uint32 WHITE = 0xffffffff;

    uint8 *row = (uint8 *)buffer->pixels;
    for(uint32 X = 0;
        X < maze.height;
        ++X)
    {
        uint32 *pixel = (uint32 *)row;
        // Run three times through each row of cells, to render each row of pixels

        // TODO(samu): merge the two loops
        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            *pixel++ = BLACK;
            *pixel++ = (maze.cells[X][Y].neighbours[0] == NULL) ? BLACK : WHITE;
        }
        *pixel++ = BLACK;

        row += buffer->pitch;

        for(uint32 Y = 0;
            Y < maze.width;
            ++Y)
        {
            *pixel++ = (maze.cells[X][Y].neighbours[3] == NULL) ? BLACK : WHITE;
            *pixel++ = WHITE;
        }
        *pixel++ = (maze.cells[X][maze.width-1].neighbours[1] == NULL) ? BLACK : WHITE;

        row += buffer->pitch;
    }
}

void renderMaze_Shaded(SDL_Surface *buffer, Maze &maze, RGBcolor startColor, RGBcolor maxColor)
{
    uint32 maxDistance = process_distanceFromStart(maze);

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
    if(argc < 4) {
        printf("usage: aMAZEd <mazeWidth> <mazeHeight> <fileName>");
        return 0;
    }

    srand(time(NULL));

    int mazeWidth = atoi(argv[1]);
    int mazeHeight = atoi(argv[2]);
    const char* filename = argv[3];

    SDL_Surface* mazeSurface;

    Maze maze = {};
    maze.width = mazeWidth;
    maze.height = mazeHeight;
    buildMaze(maze);

    RGBcolor startColor;
    startColor.red = rand() % 255;
    startColor.green = rand() % 255;
    startColor.blue = rand() % 255;

    RGBcolor maxColor;
    maxColor.red = rand() % 255;
    maxColor.green = rand() % 255;
    maxColor.blue = rand() % 255;

    /*
    RGBcolor startColor;
    startColor.red = 0xff;
    startColor.green = 0xff;
    startColor.blue = 0xff;

    RGBcolor maxColor;
    maxColor.red = 0x00;
    maxColor.green = 0x00;
    maxColor.blue = 0x00;
    */

    SDL_Init(SDL_INIT_VIDEO);

    mazeSurface = SDL_CreateRGBSurface(0,
                                  mazeWidth * 2 + 1,
                                  mazeHeight * 2 + 1,
                                  32,
                                  0xff000000,
                                  0x00ff0000,
                                  0x0000ff00,
                                  0x000000ff);

    renderMaze_Walls(mazeSurface, maze);
    SDL_SaveBMP(mazeSurface, "walls.bmp");

    mazeSurface = SDL_CreateRGBSurface(0,
                                  mazeWidth,
                                  mazeHeight,
                                  32,
                                  0xff000000,
                                  0x00ff0000,
                                  0x0000ff00,
                                  0x000000ff);
    renderMaze_Shaded(mazeSurface, maze, maxColor, startColor);
    SDL_SaveBMP(mazeSurface, "shade.bmp");

    SDL_SaveBMP(mazeSurface, filename);

    atexit(SDL_Quit);

    return 0;
}
