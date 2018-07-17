
#pragma once

#include "Utils.h" 

class Ray
{
public:
	
	Vector position;
	Vector direction;
	int level;

public:

	Ray() 
	{ 
		this->level = 0; 
		this->position = Vector(0, 0, 0); 
		this->direction = Vector(0, 1, 0); 
	};

	Ray(Vector _position, Vector _direction, int level) 
	{ 
		this->position = _position;
		this->direction = _direction;
		this->level = level;
	};

};