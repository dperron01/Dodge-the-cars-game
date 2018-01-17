/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game_object.h"
#include "game_level.h"
#include "car_level.h"
#include "power_up.h"

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

// Represents the four possible (collision) directions
enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};
// Defines a Collision typedef that represents collision data
typedef std::tuple<GLboolean, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

															   // Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100, 20);

const glm::vec2 MARIO_SIZE(150, 280);

const glm::vec2 CAR_SIZE(50, 100);

const glm::vec3 CAR_COLOUR(255.0 / 255.0, 187.0 / 255.0, 27.0 / 255.0);

const glm::vec3 CAR_COLOUR_I(1 - (255.0 / 255.0), 1 - (187.0 / 255.0), 1 -(27.0 / 255.0));

const float MARIO_JUMP_TIME = 1.0f;
const float MARIO_JUMP_VELOCITY = 5.0f;
const float ROAD_VELOCITY = 5.0f;
// Initial velocity of the player paddle
const GLfloat PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

const glm::vec2 INITIAL_MARIO_VELOCITY(0.0f, -350.0f);
// Radius of the ball object
const GLfloat BALL_RADIUS = 12.5f;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
	// Game state
	GameState              State;
	GLboolean              Keys[1024];
	GLboolean              KeysProcessed[1024];
	GLuint                 Width, Height;
	std::vector<GameLevel> Levels;
	std::vector<CarLevel*> CarLevels;
	std::vector<CarLevel> CarLevels2;
	std::vector<PowerUp>   PowerUps;
	GLuint                 Level;
	GLuint                 Lives;
	// Constructor/Destructor
	Game(GLuint width, GLuint height);
	~Game();
	// Initialize game state (load all shaders/textures/levels)
	void Init();
	// GameLoop
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
	void DoCollisions();
	// Reset
	void ResetLevel();
	void ResetPlayer();
	// Powerups
	void SpawnPowerUps(GameObject &block);
	void UpdatePowerUps(GLfloat dt);
};

#endif