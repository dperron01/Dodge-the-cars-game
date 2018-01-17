/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <algorithm>
#include <sstream>
#include <iostream>
#include <irrklang/irrKlang.h>
#include <time.h>
#include <Windows.h>
#include <Xinput.h>
using namespace irrklang;

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"
#include "mario.h"
#include "car_level.h"



// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
GameObject		  *Car;
BallObject        *Ball;
Mario			  *mario;
ParticleGenerator *Particles;
PostProcessor     *Effects;
ISoundEngine      *SoundEngine = createIrrKlangDevice();
GLfloat            ShakeTime = 0.0f;
GLfloat            stopTime = 0.0f;
TextRenderer      *Text;
CarLevel cl;
std::vector<GLchar*> level_names;
GLfloat            invincibleTime = 0.0f;
GameObject* icePointer = nullptr;
irrklang::ISound* mainTheme;
irrklang::ISound* iTheme;

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

Game::Game(GLuint width, GLuint height)
	: State(GAME_MENU), Keys(), Width(width), Height(height), Level(0), Lives(3)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
	delete Text;
	SoundEngine->drop();
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("shaders/vertex.vs", "shaders/fragment.fs", nullptr, "sprite");
	ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");
	ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/post_processing.fs", nullptr, "postprocessing");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadTexture("textures/background.jpg", GL_FALSE, "background");
	ResourceManager::LoadTexture("textures/road.jpg", GL_FALSE, "road");
	ResourceManager::LoadTexture("textures/background.jpg", GL_FALSE, "road2");
	ResourceManager::LoadTexture("textures/largebg.jpg", GL_FALSE, "bg2");
	ResourceManager::LoadTexture("textures/awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadTexture("textures/block.png", GL_FALSE, "block");
	ResourceManager::LoadTexture("textures/block_solid.png", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture("textures/paddle.png", GL_TRUE, "paddle");
	ResourceManager::LoadTexture("textures/particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("textures/powerup_speed.png", GL_TRUE, "powerup_speed");
	ResourceManager::LoadTexture("textures/powerup_sticky.png", GL_TRUE, "powerup_sticky");
	ResourceManager::LoadTexture("textures/powerup_increase.png", GL_TRUE, "powerup_increase");
	ResourceManager::LoadTexture("textures/powerup_confuse.png", GL_TRUE, "powerup_confuse");
	ResourceManager::LoadTexture("textures/powerup_chaos.png", GL_TRUE, "powerup_chaos");
	ResourceManager::LoadTexture("textures/powerup_passthrough.png", GL_TRUE, "powerup_passthrough");
	ResourceManager::LoadTexture("textures/mario.png", GL_TRUE, "mario");
	ResourceManager::LoadTexture("textures/car.png", GL_TRUE, "car");
	ResourceManager::LoadTexture("textures/rcar.png", GL_TRUE, "rcar");
	ResourceManager::LoadTexture("textures/rcar2.png", GL_TRUE, "rcar2");
	ResourceManager::LoadTexture("textures/finish.jpg", GL_TRUE, "finish");
	ResourceManager::LoadTexture("textures/win.png", GL_TRUE, "win");
	ResourceManager::LoadTexture("textures/fireball.png", GL_TRUE, "fireball");
	ResourceManager::LoadTexture("textures/deer.png", GL_TRUE, "deer");
	ResourceManager::LoadTexture("textures/star.png", GL_TRUE, "star");
	ResourceManager::LoadTexture("textures/ice.png", GL_TRUE, "ice");
	ResourceManager::LoadTexture("textures/water.png", GL_TRUE, "water");
	ResourceManager::LoadTexture("textures/board.png", GL_TRUE, "bridge");


	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("fireball"), 500);
	Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("fonts/OCRAEXT.TTF", 24);
	// Load levels
	GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height * 0.5);
	GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height * 0.5);
	GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height * 0.5);
	GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height * 0.5);
	
	cl.Load("levels/one.txt", this->Width, this->Height, ROAD_VELOCITY);
	CarLevel lv1;
	CarLevel lv2;
	CarLevel lv3;
	CarLevels2.push_back(lv1);
	CarLevels2.push_back(lv2);
	CarLevels2.push_back(lv3);
	CarLevels2.at(0).Load("levels/1.txt", this->Width, this->Height, ROAD_VELOCITY);
	CarLevels2.at(1).Load("levels/2.txt", this->Width, this->Height, ROAD_VELOCITY + 2.5f);
	CarLevels2.at(2).Load("levels/3.txt", this->Width, this->Height, ROAD_VELOCITY + 5.0f);
	level_names.push_back("levels/1.txt");
	level_names.push_back("levels/2.txt");
	level_names.push_back("levels/3.txt");
	CarLevels.push_back(&cl);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;
	// Configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	glm::vec2 carPos = glm::vec2(this->Width / 2 - CAR_SIZE.x / 2, this->Height - CAR_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	Car = new GameObject(carPos, CAR_SIZE, ResourceManager::GetTexture("rcar2"),CAR_COLOUR);
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

	glm::vec2 marioPos = glm::vec2(this->Width / 2 - MARIO_SIZE.x / 2, this->Height - MARIO_SIZE.y);
	mario = new Mario(marioPos, MARIO_SIZE, INITIAL_MARIO_VELOCITY, ResourceManager::GetTexture("mario"));
	// Audio
	mainTheme = SoundEngine->play2D("audio/cuphead.mp3", GL_TRUE, GL_FALSE, GL_TRUE);

	int k = 2;



	for (GameObject &car : CarLevels2.at(this->Level).cars)
	{
		std::cout << "code: " << car.code << std::endl;
	}






}

void Game::Update(GLfloat dt)
{
	if (stopTime <= 0) {
		// Update objects
		//std::cout << "IN UPDATE\n";
		Ball->Move(dt, this->Width);
		mario->update_jump(dt, MARIO_JUMP_TIME, MARIO_JUMP_VELOCITY);

		for (GameObject &car : CarLevels2.at(this->Level).cars)
		{
			car.Position.y += CarLevels2.at(this->Level).fast + car.Velocity.y;
			car.Position.x += car.Velocity.x;
		}
		CarLevels2.at(this->Level).finish.Position.y += CarLevels2.at(this->Level).fast;
		// Check for collisions
		this->DoCollisions();
		// Update particles
		Particles->Update(dt, *Car, 2, glm::vec2(10, CAR_SIZE.y - 10));
		// Update PowerUps
		this->UpdatePowerUps(dt);
		// Reduce shake time
		if (ShakeTime > 0.0f)
		{
			ShakeTime -= dt;
			if (ShakeTime <= 0.0f)
			{
				Effects->Shake = GL_FALSE;
				//mainTheme->setIsPaused(GL_FALSE);
			}
		}
		if (invincibleTime > 0.0f)
		{
			invincibleTime -= dt;
		}
		else if (invincibleTime <= 0.0f && Car->Color != CAR_COLOUR)
		{
			Car->Color = CAR_COLOUR;
			mainTheme->setIsPaused(GL_FALSE);
			if (iTheme) iTheme->stop();
		}

		if (icePointer != nullptr)
		{
			if (!CheckCollision(*Car, *icePointer))
			{
				icePointer = nullptr;
			}
		}
		// Check loss condition
		if (Ball->Position.y >= this->Height) // Did ball reach bottom edge?
		{
			--this->Lives;
			// Did the player lose all his lives? : Game over
			if (this->Lives == 0)
			{
				this->ResetLevel();
				this->State = GAME_MENU;
			}
			this->ResetPlayer();
		}
		// Check win condition
		if (this->State == GAME_ACTIVE && CarLevels2.at(this->Level).IsCompleted(this->Height))
		{
			if (this->Level < CarLevels2.size() - 1)
			{
				this->Level += 1;
				stopTime = 1.5f;
			}
			else
			{
				this->State = GAME_WIN;
			}
		}
	}
	else
	{
		stopTime -= dt;
	}
}


void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = GL_TRUE;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			this->KeysProcessed[GLFW_KEY_S] = GL_TRUE;
		}
	}
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
			Effects->Chaos = GL_FALSE;
			this->State = GAME_ACTIVE;
			CarLevels2.at(0).Load("levels/one.txt", this->Width, this->Height, ROAD_VELOCITY);
			CarLevels2.at(1).Load("levels/two.txt", this->Width, this->Height, ROAD_VELOCITY + 2.5f);
			CarLevels2.at(2).Load("levels/one.txt", this->Width, this->Height, ROAD_VELOCITY + 5.0f);
			this->Level = 0;
		}
	}
	if (this->State == GAME_ACTIVE && icePointer == nullptr && stopTime <= 0)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// Move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Car->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				Car->Position.x -= velocity;
				mario->Position.x -= velocity+1;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Car->Position.x <= this->Width - Car->Size.x)
			{
				Player->Position.x += velocity;
				mario->Position.x += velocity + 1;
				Car->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_S])
		{
			if (Car->Position.y <= this->Height - Car->Size.y)
			{
				
				Car->Position.y += velocity;
				
			}
		}
		if (this->Keys[GLFW_KEY_W])
		{
			if (Car->Position.y >= 0)
			{
				Car->Position.y -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
		{
			Ball->Stuck = GL_FALSE;
			if (mario->jump_time == 0.0) {
				mario->jump(MARIO_JUMP_TIME);
			}
		}
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(0, &state);


		float normLX = fmaxf(-1, (float)state.Gamepad.sThumbLX / 32767);
		float normLY = fmaxf(-1, (float)state.Gamepad.sThumbLY / 32767);
		float deadzoneX = 0.1f;
		float deadzoneY = 0.1f;

		float leftStickX = (abs(normLX) < deadzoneX ? 0 : normLX);
		float leftStickY = (abs(normLY) < deadzoneY ? 0 : normLY);

		float yy = Car->Position.y - velocity*leftStickY;
		float xx = Car->Position.x + velocity*leftStickX;

		if (xx >= 0 && xx <= this->Width - Car->Size.x)
		{
			Car->Position.x = xx;
		}
		if (yy >= 0 && yy <= this->Height - Car->Size.y)
		{
			Car->Position.y = yy;
		}
	}
}

float bgpos = 0;
float gooby = ROAD_VELOCITY;
void Game::Render()
{
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
	{
		// Begin rendering to postprocessing quad
		Effects->BeginRender();
		// Draw background
		bgpos -= 1;
		//std::cout << "Position: " << bgpos << "Height: " << this->Height << std::endl;
		float test = (-(float)this->Height)/ CarLevels2.at(this->Level).fast;
		if (bgpos == test)
		{
			bgpos = 0;
		}
		bgpos = (float)((int) bgpos);
		float lastgoob = gooby;
		gooby = CarLevels2.at(this->Level).fast;
		if (stopTime > 0)
		{
			gooby = 0;
		}
		
		//float gooby = CarLevels2.at(this->Level).fast;
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(40, 0), glm::vec2(this->Width, this->Height), 0.0f);
		Renderer->DrawSprite(ResourceManager::GetTexture("road"), glm::vec2(0, (0-bgpos)*gooby), glm::vec2(this->Width, this->Height), 0.0f);
		Renderer->DrawSprite(ResourceManager::GetTexture("road"), glm::vec2(0, ((0 - bgpos)*gooby)- this->Height), glm::vec2(this->Width, this->Height), 0.0f);
		// Draw level
		//this->Levels[this->Level].Draw(*Renderer);
		// Draw player
		//Player->Draw(*Renderer);
		
		CarLevels2.at(this->Level).Draw(*Renderer);
		// Draw particles	
		Particles->Draw();
		Car->Draw(*Renderer);
		// Draw PowerUps
		for (PowerUp &powerUp : this->PowerUps)
			if (!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		
		// Draw ball
		//Ball->Draw(*Renderer);
		// End rendering to postprocessing quad
		Effects->EndRender();
		// Render postprocessing quad
		Effects->Render(glfwGetTime());
		// Render text (don't include in postprocessing)
		std::stringstream ss; ss << this->Lives;
		std::stringstream ss2; ss2 << (this->Level + 1);
		Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
		Text->RenderText("Level: " + ss2.str(), this->Width - 125.0f, 5.0f, 1.0f);
	}
	if (stopTime > 0)
	{
		Text->RenderText("LEVEL COMPLETE!", 200.0f, this->Height / 2, 2.0f);
	}
	if (this->State == GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2 + 20.0f, 0.75f);
	}
	if (this->State == GAME_WIN)
	{
		Text->RenderText("You WON!!!", 320.0f, this->Height / 2 - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
	}
}


void Game::ResetLevel()
{
	if (this->Level == 0)this->Levels[0].Load("levels/one.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 1)
		this->Levels[1].Load("levels/two.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 2)
		this->Levels[2].Load("levels/three.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 3)
		this->Levels[3].Load("levels/four.lvl", this->Width, this->Height * 0.5f);

	this->Lives = 3;
}

void Game::ResetPlayer()
{
	// Reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
	// Also disable all active powerups
	Effects->Chaos = Effects->Confuse = GL_FALSE;
	Ball->PassThrough = Ball->Sticky = GL_FALSE;
	Player->Color = glm::vec3(1.0f);
	Ball->Color = glm::vec3(1.0f);
}


// PowerUps
GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type);

void Game::UpdatePowerUps(GLfloat dt)
{
	for (PowerUp &powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f)
			{
				// Remove powerup from list (will later be removed)
				powerUp.Activated = GL_FALSE;
				// Deactivate effects
				if (powerUp.Type == "sticky")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
					{	// Only reset if no other PowerUp of type sticky is active
						Ball->Sticky = GL_FALSE;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "pass-through")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
					{	// Only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = GL_FALSE;
						Ball->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
					{	// Only reset if no other PowerUp of type confuse is active
						Effects->Confuse = GL_FALSE;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
					{	// Only reset if no other PowerUp of type chaos is active
						Effects->Chaos = GL_FALSE;
					}
				}
			}
		}
	}
	// Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
	// Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
		[](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	), this->PowerUps.end());
}

GLboolean ShouldSpawn(GLuint chance)
{
	GLuint random = rand() % chance;
	return random == 0;
}
void Game::SpawnPowerUps(GameObject &block)
{
	if (ShouldSpawn(75)) // 1 in 75 chance
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
	if (ShouldSpawn(15)) // Negative powerups should spawn more often
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
}

void ActivatePowerUp(PowerUp &powerUp)
{
	// Initiate a powerup based type of powerup
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	}
	else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = GL_TRUE;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "pass-through")
	{
		Ball->PassThrough = GL_TRUE;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = GL_TRUE; // Only activate if chaos wasn't already active
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = GL_TRUE;
	}
}

GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
	// Check if another PowerUp of the same type is still active
	// in which case we don't disable its effect (yet)
	for (const PowerUp &powerUp : powerUps)
	{
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return GL_TRUE;
	}
	return GL_FALSE;
}


int cmon = -1;
void Game::DoCollisions()
{


	GameObject* lastCar = nullptr;
	//for (GameObject &car : CarLevels2.at(this->Level).cars)
	for (int i = 0; i < CarLevels2.at(this->Level).cars.size(); i++)
	{
		GameObject &car = CarLevels2.at(this->Level).cars.at(i);
		if (cmon == 0)
		{
			std::cout << "vehicle code: " << car.code << std::endl;
		}
		if (car.code == vehicles::ICE && CheckCollision(*Car, car) && invincibleTime <= 0)
		{
			icePointer = &car;
		}
		else if (car.code == vehicles::STAR && !car.Destroyed && CheckCollision(*Car, car))
		{
			invincibleTime = 8.0f;
			Car->Color = CAR_COLOUR_I;
			car.Destroyed = GL_TRUE;
			mainTheme->setIsPaused(GL_TRUE);
			SoundEngine->play2D("audio/bleep.mp3", GL_FALSE);
			iTheme = SoundEngine->play2D("audio/breakout.mp3", GL_TRUE, GL_FALSE, GL_TRUE);
			//Musichere
		}
		else if (car.code == vehicles::WATER && CheckCollision(*Car, car) && invincibleTime <= 0)
		{
			if (!CheckCollision(*Car, CarLevels2.at(this->Level).cars.at(i + 1)))
			{
				cmon += 1;
				std::cout << lastCar->code << std::endl;
				std::cout << "WATER THING: " << CarLevels2.at(this->Level).cars.at(i + 1).code;
				SoundEngine->play2D("audio/splash.wav", GL_FALSE);
				this->Lives -= 1;
				invincibleTime = 1.5f;
				Car->Color = CAR_COLOUR_I;
				if (this->Lives == 0)
				{
					std::stringstream ss;
					ss << (this->Level - 1);
					CarLevels2.at(this->Level).Load(level_names.at(this->Level), this->Width, this->Height, CarLevels2.at(this->Level).fast);
					this->Lives = 3;
				}
				else
				{
					ShakeTime = 0.1f;
					Effects->Shake = GL_TRUE;
				}
			}
		}
		else if (car.code == 5 && CheckCollision(*Car, car))
		{
			
			
		}
		else if ( CheckCollision(*Car, car) && !car.Destroyed && invincibleTime <= 0)
		{
			SoundEngine->play2D("audio/crash.wav", GL_FALSE);
			car.Destroyed = GL_TRUE;
			this->Lives -= 1;
			invincibleTime = 1.5f;
			Car->Color = CAR_COLOUR_I;
			if (this->Lives == 0)
			{
				std::stringstream ss;
				ss  << (this->Level - 1);
				CarLevels2.at(this->Level).Load(level_names.at(this->Level), this->Width, this->Height, CarLevels2.at(this->Level).fast);
				this->Lives = 3;
			}
			else
			{
				ShakeTime = 0.1f;
				Effects->Shake = GL_TRUE;
			}
		}
		lastCar = &car;
	}
	if (CheckCollision(*Car, CarLevels2.at(this->Level).finish) && !CarLevels2.at(this->Level).finish.Destroyed)
	{
		CarLevels2.at(this->Level).finish.Destroyed = GL_TRUE;
		SoundEngine->play2D("audio/cheer.wav", GL_FALSE);
		this->Lives = 3;
	}
}

GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
	// Collision x-axis?
	GLboolean collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// Collision y-axis?
	GLboolean collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	// Collision only if on both axes
	return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
	// Get center point circle first 
	glm::vec2 center(one.Position + one.Radius);
	// Calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// Get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// Now retrieve vector between center circle and closest point AABB and check if length < radius
	difference = closest - center;

	if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	else
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}

// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	GLfloat max = 0.0f;
	GLuint best_match = -1;
	for (GLuint i = 0; i < 4; i++)
	{
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}