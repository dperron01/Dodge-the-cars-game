#include "mario.h"
#include <iostream>

Mario::Mario()
	: GameObject() { }

Mario::Mario(glm::vec2 pos, glm::vec2 size,  glm::vec2 velocity, Texture2D sprite)
	: GameObject(pos, size , sprite, glm::vec3(1.0f), velocity), jump_time(0.0f) { }

void Mario::jump(float jump)
{
	//std::cout << "Jump\n";
	jump_time = jump;
}

void Mario::update_jump(float dt, float time, float velocity)
{

	//std::cout << "Jump is " << jump_time << std::endl;

	if (jump_time > 0.0)
	{
		//std::cout << "hey! " << jump_time;
		if (jump_time >= time/2)
			this->Position.y -= velocity;
		else {
			this->Position.y += velocity;
		}
		jump_time -= dt;
	}
	else
	{
		jump_time = 0.0f;
	}
}
