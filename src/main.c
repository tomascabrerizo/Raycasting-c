#include <stdio.h>
#include <SDL.h>
#include "constants.h"

//Map
const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = 
{
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player
{
	float x;
	float y;
	float width;
	float height;
	int turnDirection;//-1 for left, 1 for right
	int walkDirection;//-1 for back, 1 for front
	float rotationAngle;
	float walkSpeed;
	float turnSpeed;
} player;

//Window
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
int ticksLastFrame = 0;

//Function that setup a windows
int initiallizeWindow()
{
	if(SDL_Init("SDL_INIT_EVERYTHING") != 0)
	{
		printf(stderr, "Error initializing SDL\n");
		return FALSE;
	}
	//Create SDL WINDOW
	window = SDL_CreateWindow(
			"C_RAYCAST", 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			WINDOW_WIDTH, 
			WINDOW_HEIGHT,
			SDL_WINDOW_SHOWN);
	if(!window)
	{
		printf(stderr, "Error Creating SDL_WINDOW");
		return FALSE;
	}
	//Create SDL RENDERER
	renderer = SDL_CreateRenderer(window, -1, 0);
	if(!renderer)
	{
		printf(stderr, "Creating SDL_RENDERER");
		return FALSE;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return TRUE;	
}

void destroyWindow()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup()
{
	player.x = WINDOW_WIDTH/2;
	player.y = WINDOW_HEIGHT/2;
	player.width = 5;
	player.height = 5;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI/2;
	player.walkSpeed = 100;
	player.turnSpeed = 45 * (PI/180);
}

void renderMap()
{
	for(int i = 0; i < MAP_NUM_ROWS; i++)
	{
		for(int j = 0; j < MAP_NUM_COLS; j++)
		{
			int tileX = j * TILE_SIZE;
			int tileY = i * TILE_SIZE;
			int tileColor = map[i][j] != 0 ? 255 : 0;
			
			SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);

			SDL_Rect mapTileRect = {
				tileX * MINIMAP_SCALE_FACTOR, 
				tileY * MINIMAP_SCALE_FACTOR, 
				TILE_SIZE * MINIMAP_SCALE_FACTOR, 
				TILE_SIZE * MINIMAP_SCALE_FACTOR};

			SDL_RenderFillRect(renderer, &mapTileRect);
		}
	}
}

void processInput()
{
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_QUIT:
			isGameRunning = FALSE;
		break;
		case SDL_KEYDOWN:
			int keyCode = event.key.keysym.sym;
			if(keyCode == SDLK_ESCAPE)
			{
				isGameRunning = FALSE;
			}
		break;
	}
}

void update()
{
	while(!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH));	
	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
	ticksLastFrame = SDL_GetTicks();
	//TODO:remember to update game objects as a function of deltaTime
}

void render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	renderMap();
	//renderRays();
	//renderPlayer();

	SDL_RenderPresent(renderer);
}

int main(int argc, char* args[])
{
	isGameRunning = initiallizeWindow();
	setup();
	while(isGameRunning)
	{
		processInput();
		update();
		render();
	}
	destroyWindow();
	return 0;
}
