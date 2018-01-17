#include "car_level.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <time.h>

void CarLevel::Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight, float velocity)
{
	this->fast = velocity;
	// Clear old data
	this->cars.clear();
	// Load from file
	GLuint tileCode;
	CarLevel level;
	std::string line;
	std::ifstream fstream(file);
	float time;
	int vehicle;
	int position;
	float speed;
	if (fstream)
	{
		std::getline(fstream, line);	//First line is a throw away
		while (std::getline(fstream, line)) // Read each line from level file
		{
			std::istringstream sstream(line);
			sstream >> time >> vehicle >> position >> speed;
			//std::cout << time << vehicle << position << speed << "\n";
			if (vehicle == vehicles::CAR)	//Car
			{
				float placex = (position / 8.0)*levelWidth;
				float placey = -60 * time*(velocity + speed);
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(80, 160);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("rcar"), glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX), glm::vec2(0, speed));
				car.code = vehicles::CAR;
				cars.push_back(car);
			}
			else if (vehicle == vehicles::DEER)	//Deer
			{
				float placex = -60*time*speed;
				float placey = -60*time*velocity;
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(120, 120);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("deer"), glm::vec3(1.0f), glm::vec2(speed, 0));
				car.code = vehicles::DEER;
				cars.push_back(car);
				
			}
			else if (vehicle == vehicles::ICE)   //Ice
			{
				float placex = (position / 8.0)*levelWidth;
				float placey = -60 * time*velocity;
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(200, 200);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("ice"), glm::vec3(1.0f), glm::vec2(0, 0));
				car.Destroyed = GL_TRUE;
				car.code = vehicles::ICE;
				cars.push_back(car);
			}
			else if (vehicle == vehicles::STAR)	//STAR
			{
				float placex = (position / 8.0)*levelWidth;
				float placey = -60 * time*velocity;
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(100, 100);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("star"), glm::vec3(1.0f), glm::vec2(0, 0));
				car.code = vehicles::STAR;
				cars.push_back(car);
			}
			else if (vehicle == vehicles::WATER)
			{
				float placex = 0;
				float placey = -60 * time*velocity;
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(800, 600);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("water"), glm::vec3(1.0f), glm::vec2(0, 0));
				car.code = vehicles::WATER;
				cars.push_back(car);
			}
			else if (vehicle == vehicles::BRIDGE)
			{
				float placey = -60 * time*velocity;
				float placex = (position / 8.0)*levelWidth;
				glm::vec2 carPos = glm::vec2(placex, placey);
				glm::vec2 carSize = glm::vec2(212, 600);
				GameObject car(carPos, carSize, ResourceManager::GetTexture("bridge"), glm::vec3(1.0f), glm::vec2(0, 0));
				car.code = vehicles::BRIDGE;
				cars.push_back(car);
			}

		}
		GameObject lastCar = cars.back();
		glm::vec2 fpos = glm::vec2(0, (lastCar.Position.y+(60*(lastCar.Velocity.y+velocity)))-(2*60*velocity));
		glm::vec2 fsize = glm::vec2(levelWidth, levelHeight / 4);
		finish = GameObject(fpos, fsize, ResourceManager::GetTexture("finish"));
		
	}
}

void CarLevel::Draw(SpriteRenderer &renderer)
{
	for (GameObject &car : this->cars)
	{
		if (!car.Destroyed || car.code == vehicles::ICE)
			car.Draw(renderer);
	}
	finish.Draw(renderer);
}

GLboolean CarLevel::IsCompleted(int Height)
{
	if (this->finish.Position.y > Height)
		return GL_TRUE;
	return GL_FALSE;

}