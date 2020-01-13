#include <stdio.h>
#include <limits.h>
#include <SDL.h>
#include "constants.h"
#include "textures.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = 
{
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 1},
	{1, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 1},
	{1, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 1},
	{1, 0, 7, 2, 2, 2, 0, 8, 0, 0, 0, 0, 0, 4, 0, 3, 3, 3, 3, 1},
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

struct Ray
{
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	int wasHitVertical;
	int isRayFacingUp;
	int isRayFacingDown;
	int isRayFacingLeft;
	int isRayFacingRight;
	int wallHitContent;
} rays[NUM_RAYS];

//Global variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
int ticksLastFrame = 0;
//Color buffer pointer
Uint32* colorBuffer = NULL;
SDL_Texture* colorBufferTexture;
Uint32* textures[NUM_TEXTURES];

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
	free(colorBuffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup()
{
	player.x = WINDOW_WIDTH/2;
	player.y = WINDOW_HEIGHT/2;
	player.width = 1;
	player.height = 1;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI/2;
	player.walkSpeed = 100;
	player.turnSpeed = 100 * (PI/180);
	//allocate the total amount of bits to hold the colorBuffer
	colorBuffer = (Uint32*)malloc(sizeof(Uint32) * (Uint32)WINDOW_WIDTH * (Uint32)WINDOW_HEIGHT);
	colorBufferTexture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			WINDOW_WIDTH,
			WINDOW_HEIGHT);
	//load some textures from textures.h
	textures[0] = (Uint32*) REDBRICK_TEXTURE;
	textures[1] = (Uint32*) PURPLESTONE_TEXTURE;
	textures[2] = (Uint32*) MOSSYSTONE_TEXTURE;
	textures[3] = (Uint32*) GRAYSTONE_TEXTURE;
	textures[4] = (Uint32*) COLORSTONE_TEXTURE;
	textures[5] = (Uint32*) BLUESTONE_TEXTURE;
	textures[6] = (Uint32*) WOOD_TEXTURE;
	textures[7] = (Uint32*) EAGLE_TEXTURE;
}
int mapHasWallAt(float x, float y)
{
	if(x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
	{
		return TRUE;
	}
	int mapGridIndexX = floor(x / TILE_SIZE);
	int mapGridIndexY = floor(y / TILE_SIZE);
	return map[mapGridIndexY][mapGridIndexX];
}

void movePlayer(float deltaTime)
{
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	float moveStep = player.walkDirection * player.walkSpeed * deltaTime;
	int newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	int newPlayerY = player.y + sin(player.rotationAngle) * moveStep;
	if(!mapHasWallAt(newPlayerX, newPlayerY))
	{
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
}

void renderPlayer()
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_Rect playerRect = {
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			player.width * MINIMAP_SCALE_FACTOR,
			player.height * MINIMAP_SCALE_FACTOR
	};
	SDL_RenderFillRect(renderer, &playerRect);
	SDL_RenderDrawLine(
			renderer,
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			(player.x + cos(player.rotationAngle) * 40) * MINIMAP_SCALE_FACTOR,
			(player.y + sin(player.rotationAngle) * 40) * MINIMAP_SCALE_FACTOR);
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
				TILE_SIZE * MINIMAP_SCALE_FACTOR
			};
			SDL_RenderFillRect(renderer, &mapTileRect);
		}
	}
}

void renderRays()
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	for(int i = 0; i < NUM_RAYS; i++)
	{
		SDL_RenderDrawLine(
				renderer,
				MINIMAP_SCALE_FACTOR * player.x,
				MINIMAP_SCALE_FACTOR * player.y,
				MINIMAP_SCALE_FACTOR * rays[i].wallHitX,
				MINIMAP_SCALE_FACTOR * rays[i].wallHitY);
	}
}

float normalizeAngle(float angle)
{
	angle = remainder(angle, TWO_PI);
	if(angle < 0)
	{
		angle = TWO_PI + angle;
	}
	return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2)
{
	return sqrt( (x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));
}

void castRay(float rayAngle, int stripId)
{
	//TODO:Find horizontal and vertical hits
	rayAngle = normalizeAngle(rayAngle);
	int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	int isRayFacingUp = !isRayFacingDown;
	int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	int isRayFacingLeft = !isRayFacingRight;
	float yintercept;
	float xintercept;
	float ystep;
	float xstep;
	////////////////////////////
	//Horizontal intersection
	///////////////////////////
	int foundHorzWallHit = FALSE;
	float horzWallHitX = 0;
	float horzWallHitY = 0;
	int horzWallContent = 0;
	//fond the first horizontal intersection
	yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown ? TILE_SIZE : 0;
	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);
	//Calculate the increment xstep and ystep
	ystep = TILE_SIZE;
	ystep *= isRayFacingUp ? -1 : 1;
	xstep = TILE_SIZE / tan(rayAngle);
	xstep *= isRayFacingLeft && xstep > 0 ? -1 : 1;
	xstep *= isRayFacingRight && xstep < 0 ? -1 : 1;
	float nextHorzTouchX = xintercept;
	float nextHorzTouchY = yintercept;
	//increment xstep and ystep until find wall
	while(nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT)
	{
		float xToCheck = nextHorzTouchX;
		float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);
		if(mapHasWallAt(xToCheck, yToCheck))
		{
			horzWallHitX = nextHorzTouchX;
			horzWallHitY = nextHorzTouchY;
			foundHorzWallHit = TRUE;
			horzWallContent = map[(int)floor(yToCheck/TILE_SIZE)][(int)floor(xToCheck/TILE_SIZE)];
			break;
		}
		else
		{
			nextHorzTouchX += xstep;
			nextHorzTouchY += ystep;
		}	
	}
	////////////////////////////
	//Vertical intersection
	///////////////////////////
	int foundVertWallHit = FALSE;
	float vertWallHitX = 0;
	float vertWallHitY = 0;
	int vertWallContent = 0;
	//fond the first horizontal intersection
	xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	xintercept += isRayFacingRight ? TILE_SIZE : 0;
	yintercept = player.y + (xintercept - player.x) * tan(rayAngle);
	//Calculate the increment xstep and ystep
	xstep = TILE_SIZE;
	xstep *= isRayFacingLeft ? -1 : 1;
	ystep = TILE_SIZE * tan(rayAngle);
	ystep *= isRayFacingUp && ystep > 0 ? -1 : 1;
	ystep *= isRayFacingDown && ystep < 0 ? -1 : 1;
	float nextVertTouchX = xintercept;
	float nextVertTouchY = yintercept;
	//increment xstep and ystep until find wall
	while(nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT)
	{
		float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
		float yToCheck = nextVertTouchY;
		if(mapHasWallAt(xToCheck, yToCheck))
		{
			vertWallHitX = nextVertTouchX;
			vertWallHitY = nextVertTouchY;
			foundVertWallHit = TRUE;
			vertWallContent = map[(int)floor(yToCheck/TILE_SIZE)][(int)floor(xToCheck/TILE_SIZE)];
			break;
		}
		else
		{
			nextVertTouchX += xstep;
			nextVertTouchY += ystep;
		}	
	}
	//Calculate both horizontal and vertical hitDistances and choose the smallest one
	float horzHitDistance = foundHorzWallHit 
		? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY) : INT_MAX;  
	float vertHitDistance = foundVertWallHit
		? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY) : INT_MAX;
	if(vertHitDistance < horzHitDistance)
	{
		rays[stripId].distance = vertHitDistance;
		rays[stripId].wallHitX = vertWallHitX;
		rays[stripId].wallHitY = vertWallHitY;
		rays[stripId].wallHitContent = vertWallContent;
		rays[stripId].wasHitVertical = TRUE;
	}
	else
	{
		rays[stripId].distance = horzHitDistance;
		rays[stripId].wallHitX = horzWallHitX;
		rays[stripId].wallHitY = horzWallHitY;
		rays[stripId].wallHitContent = horzWallContent;
		rays[stripId].wasHitVertical = FALSE;
	}
	rays[stripId].rayAngle = rayAngle;
	rays[stripId].isRayFacingDown = isRayFacingDown;
	rays[stripId].isRayFacingUp = isRayFacingUp;
	rays[stripId].isRayFacingLeft = isRayFacingLeft;
	rays[stripId].isRayFacingRight = isRayFacingRight;
}

void castAllRays()
{
	//start first ray subtractig half of FOV
	float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
	for(int stripId = 0; stripId < NUM_RAYS; stripId++)
	{
		castRay(rayAngle, stripId);
		rayAngle += FOV_ANGLE / NUM_RAYS;
	}
}

void processInput()
{
	SDL_Event event;
	SDL_PollEvent(&event);
	int keyCode = event.key.keysym.sym;
	switch (event.type)
	{
		case SDL_QUIT:
			isGameRunning = FALSE;
		break;
		case SDL_KEYDOWN:
			if(keyCode == SDLK_ESCAPE)
				isGameRunning = FALSE;
			if(keyCode == SDLK_UP)
				player.walkDirection = 1;
			if(keyCode == SDLK_DOWN)
				player.walkDirection = -1;
			if(keyCode == SDLK_RIGHT)
				player.turnDirection = 1;
			if(keyCode == SDLK_LEFT)
				player.turnDirection = -1;
		break;
		case SDL_KEYUP:
			if(keyCode == SDLK_UP)
				player.walkDirection = 0;
			if(keyCode == SDLK_DOWN)
				player.walkDirection = 0;
			if(keyCode == SDLK_RIGHT)
				player.turnDirection = 0;
			if(keyCode == SDLK_LEFT)
				player.turnDirection = 0;
		break;
	}
}

void update()
{
	while(!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH));	
	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
	ticksLastFrame = SDL_GetTicks();
	movePlayer(deltaTime);
	castAllRays();
}

void generate3DProjection()
{
	for(int i = 0; i < NUM_RAYS; i++)
	{
		float perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
		float distanceProjPlane = (WINDOW_WIDTH/2) / tan(FOV_ANGLE / 2);
		float projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;
	
		int wallStripHeight = (int)projectedWallHeight;
	
		int wallTopPixel = (WINDOW_HEIGHT/2) - (wallStripHeight/2);
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;
		
		int wallBottomPixel = (WINDOW_HEIGHT/2) + (wallStripHeight/2);
		wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;
		
		//celling color
		for(int y = 0; y < wallTopPixel; y++)
		{
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF000022;
		}
		//floor color
		for(int y = wallBottomPixel; y <  WINDOW_HEIGHT; y++)
		{
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF006600;
		}
		//TEXTURE offsets
		int textureOffsetX;
		//Calculate TextureOffsetX
		if(rays[i].wasHitVertical)
		{
			//offset for the vertical hit
			textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
		}	
		else
		{
			//offset for the horizontal hit
			textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;
		}
		int texNum = rays[i].wallHitContent - 1;
		for(int y = wallTopPixel; y < wallBottomPixel; y++)
		{
			int distanceFormTop = (y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2));
			int textureOffsetY = distanceFormTop * ((float)TEXTURE_HEIGHT/wallStripHeight);
				
			//Set the color of the wall base form the color of the texture
			Uint32 texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
			colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
		}
	}	
}

void clearColorBuffer(Uint32 color)
{
	for(int x = 0; x < WINDOW_WIDTH; x++)
		for(int y = 0; y < WINDOW_HEIGHT; y++)
			colorBuffer[(y * WINDOW_WIDTH) + x] = color;
}

void renderColorBuffer()
{
	SDL_UpdateTexture(
			colorBufferTexture, 
			NULL, 
			colorBuffer, 
			(int)((Uint32)WINDOW_WIDTH * sizeof(Uint32)));
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	generate3DProjection();
	renderColorBuffer();
	clearColorBuffer(0xFF000000);
	
	renderMap();
	renderRays();
	renderPlayer();

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
