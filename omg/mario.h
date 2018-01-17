#ifndef MARIO_H
#define MARIO_H
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "power_up.h"




class Mario : public GameObject
{
public:
	GLfloat jump_time;
	
	// Constructor(s)
	Mario();
	Mario(glm::vec2 pos, glm::vec2 size, glm::vec2 velocity, Texture2D sprite);
	// Moves the ball, keeping it constrained within the window bounds (except bottom edge); returns new position
	glm::vec2 Move(GLfloat dt, GLuint window_width);
	// Resets the ball to original state with given position and velocity
	void      Reset(glm::vec2 position, glm::vec2 velocity);
	void      jump(float jump);
	void      update_jump(float dt, float time, float velocity);
};























#endif