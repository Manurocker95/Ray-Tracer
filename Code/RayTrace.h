/*
  15-462 Computer Graphics I
  Assignment 3: Ray Tracer
  C++ RayTracer Class
  Author: rtark
  Oct 2007

  NOTE: This is the file you will need to begin working with
		You will need to implement the RayTrace::CalculatePixel () function

  This file defines the following:
	RayTrace Class
*/
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "Utils.h"
#include "Ray.h"
#include "pic.h"
/*
	RayTrace Class - The class containing the function you will need to implement

	This is the class with the function you need to implement
*/
class RayTrace
{

public:

	struct Intersection
	{
		bool didHit = false;
		Vector position;
		Vector direction;
		Vector emission;
		string material;

		SceneObject * intersectionObject;
		SceneTriangle * triangle;
		int triangleIndex;
		bool foundTriangle;

		std::vector<SceneMaterial*> materialList;

		int sceneObjectId;
	};

public:

	enum SHADING
	{
		PHONG,
		COOK_TORRANCE
	};

	SHADING actualShading;

public:

	/* - Scene Variable for the Scene Definition - */
	Scene m_Scene;

	// -- Constructors & Destructors --
	RayTrace (void) { actualShading = SHADING::PHONG; }
	~RayTrace (void) {}

	// -- Main Functions --
	// - CalculatePixel - Returns the Computed Pixel for that screen coordinate
   Vector CalculatePixel (int screenX, int screenY);

public:

	Ray & get_Ray(int x, int y, Camera & cam, Scene & scene);
	Vector Shade(Scene & scene, Ray & ray, Camera & cam);
	
	Intersection intersect(Scene & scene, Ray & ray);
	
	bool intersectWithShape(SceneObject & sceneObj, Ray & ray, float & intersect_distance, Intersection & inter);
	bool intersectionRayTriangle(SceneTriangle & t, Ray & ray, float & distance);
	bool intersectionRaySphere(SceneSphere & s, Ray & ray, float & distance);
	bool intersectionRayModel(SceneModel & m, Ray & ray, float & out, Intersection & inter);
	
	bool IsVisible(Scene & scene, SceneLight & light, Intersection intersection);

	Vector Reflect(Vector theta, Vector normal);
	Vector Refract(Vector & rayDir, Vector normal, Vector refractIndex);
	float brdf(Vector l, Vector v, Vector n, SceneMaterial & material);
	Vector CalculateFresnelReflectance(Vector & rayDir, Vector normal, Vector refractIndex, Vector objectReflectiveness);

	Vector phongCookTorrance(Intersection inter, Scene & scene, Camera & cam, SceneTriangle & triangle, bool baricentric = false);
	Vector fcookTorrance(Vector l, Vector v, Vector n, SceneMaterial & material);
	float GGX_PartialGeometryTerm(Vector w, Vector n, Vector h, float alpha);

	float Clamp(float value, float low, float high);
	Vector Lerp(Vector start, Vector end, Vector percent);
	///eleveates each compenent in the vector to the power
	Vector RayTrace::Pow(Vector v, float power);

public:
	Vector get_incoming_light(SceneLight & light, Intersection intersection);
	Vector Phong(Intersection inter, Scene & scene, Camera & cam);
	Vector PhongBaricentric(Intersection inter, Scene & scene, Camera & cam, SceneTriangle & triangle);

};

