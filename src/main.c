#include <stdio.h>
#include <SDL.h>
#include "constants.h"

//Window
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
int playerX, playerY;

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
	playerX = 0;
	playerY = 0;
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
	playerX += 1;
	playerY += 1;
}

void render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect rect = {playerX, playerY, 20, 20};
	SDL_RenderFillRect(renderer, &rect);

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
