#include "stdafx.h"
#include "imgui_impl_sdl_gl3.h"
#include "Game.h"
#include "LTimer.h"
#include <iostream>
#include <sstream>
#include "RenderSystem.h"

using namespace std;

const int MAX_FPS = std::numeric_limits<int>::max();
const int SCREEN_TICKS_PER_FRAME = 1000 / MAX_FPS;

Game::Game(Size2D screenSize) : 
	m_screenSize(screenSize), 
	quit(false)
{
	SINGLETON(TaskQueue)->spawnWorkers();
	SINGLETON(TaskQueue)->threadingActive = true;
}

Game::~Game()
{
	SINGLETON(TaskQueue)->waitUntilIdle();
}

bool Game::init() {
	srand(time(NULL));

	//initialize systems
	SINGLETON(RenderSystem)->initialize(m_screenSize);
	SINGLETON(TestSystem)->initialize(m_screenSize);

	//registerSystems
	REGISTER_SYSTEM(RenderSystem);
	REGISTER_SYSTEM_TICK_RATE(TestSystem, 30);

	//create entities
	for (int i = 0; i < 500; i++)
	{
		circles.push_back(new Circle());
	}

	return true;
}

void Game::destroy()
{

}

//** calls update on all game entities*/
void Game::update(float dt)
{
	/*for (int i = 0; i < 50; i++)
	{
		TASK_BEGIN()
		{
			float x;
			for (int y = 0; y < 10000; y++) { x = cosf(1782.45678) * cosf(2179271) * y; }
		}
		TASK_END
		
	}*/

	SINGLETON(SystemManager)->runSystems(dt);
	SINGLETON(TaskQueue)->waitUntilIdle();
	SINGLETON(SyncManager)->DistributeChanges();
}

/** update and render game entities*/
void Game::loop()
{
	LTimer capTimer;//to cap framerate
	SDL_Event event;

	int frameNum = 0;
	float timeSinceLastFrameCheck = 0;

	SINGLETON(SyncManager)->DistributeChanges();
	
	while (!quit) { //game loop

		capTimer.start();
		unsigned int currentTime = LTimer::gameTime();//millis since game started
		float deltaTime = (currentTime - lastTime) / 1000.0;//time since last update
		lastTime = currentTime; //save the curent time for next frame

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSdlGL3_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				quit = true;
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				quit = true;
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_COMMA)
			{
				SINGLETON(TaskQueue)->incrementActiveWorkers();
				std::cout << "Active Workers" << SINGLETON(TaskQueue)->getNumActiveWorkers() << std::endl;
			}
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PERIOD)
			{
				SINGLETON(TaskQueue)->decrementActiveWorkers();
				std::cout << "Active Workers" << SINGLETON(TaskQueue)->getNumActiveWorkers() << std::endl;
			}
			SINGLETON(RenderSystem)->CameraInput(event, deltaTime);
		}

		update(deltaTime);

		int frameTicks = capTimer.getTicks();//time since start of frame
		if (frameTicks < SCREEN_TICKS_PER_FRAME)
		{
			//Wait remaining time before going to next frame
			SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
		}
	}
}


