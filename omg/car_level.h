#ifndef CAR_LEVEL_H
#define CAR_LEVEL_H

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "game_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"

enum vehicles{
	CAR, DEER, ICE, STAR, WATER, BRIDGE
};

class CarLevel
{
public:
	// Level state
	std::vector<GameObject> cars;
	GLfloat fast;
	GameObject finish;
	// Constructor
	CarLevel() { }
	// Loads level from file
	void      Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight, float velocity);
	// Render level
	void      Draw(SpriteRenderer &renderer);
	// Check if the level is completed (all non-solid tiles are destroyed)
	GLboolean IsCompleted(int Height);
private:
	// Initialize level from tile data
};






















#endif