#ifdef _OS_X_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>	

#elif defined(WIN32)
#include <windows.h>
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"

#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include "Scene.h"
#include "RayTrace.h"

#define MAX_BOUNCES 4
#ifdef INFINITY std::numeric_limits<float>::infinity()
#endif

// -- Main Functions --
// - CalculatePixel - Returns the Computed Pixel for that screen coordinate
Vector RayTrace::CalculatePixel (int screenX, int screenY)
{
   /*
   -- How to Implement a Ray Tracer --

   This computed pixel will take into account the camera and the scene
   and return a Vector of <Red, Green, Blue>, each component ranging from 0.0 to 1.0

   In order to start working on computing the color of this pixel,
   you will need to cast a ray from your current camera position
   to the image-plane at the given screenX and screenY
   coordinates and figure out how/where this ray intersects with 
   the objects in the scene descriptor.
   The Scene Class exposes all of the scene's variables for you 
   through its functions such as m_Scene.GetBackground (), m_Scene.GetNumLights (), 
   etc. so you will need to use those to learn about the World.

   To determine if your ray intersects with an object in the scene, 
   you must calculate where your objects are in 3D-space [using 
   the object's scale, rotation, and position is extra credit]
   and mathematically solving for an intersection point corresponding to that object.

   For example, for each of the spheres in the scene, you can use 
   the equation of a sphere/ellipsoid to determine whether or not 
   your ray from the camera's position to the screen-pixel intersects 
   with the object, then from the incident angle to the normal at 
   the contact, you can determine the reflected ray, and recursively 
   repeat this process capped by a number of iterations (e.g. 10).

   Using the lighting equation & texture to calculate the color at every 
   intersection point and adding its fractional amount (determined by the material)
   will get you a final color that returns to the eye at this point.
   */

   if (screenX == 50 && screenY ==100)
   {
      int kk=0;
   }
   Scene &la_escena = m_Scene;
   Camera &la_camara = la_escena.GetCamera();
   Vector posicion = la_camara.GetPosition();


   if ((screenX < 0 || screenX > Scene::WINDOW_WIDTH - 1) ||
      (screenY < 0 || screenY > Scene::WINDOW_HEIGHT - 1))
   {
      // Off the screen, return black
      return Vector (0.0f, 0.0f, 0.0f);
   }

   Ray ray = get_Ray(screenX, screenY, la_camara, la_escena);

   if (la_escena.supersample)
   {
	   Vector actualDirection = ray.direction;
	   Vector finalColor = Vector(0.0, 0.0, 0.0);
	   Vector shadeColor = Vector(0.0, 0.0, 0.0);
	   float ssaaOffset = 0.002;

	   for (int i = 0; i < 5; i++)
	   {
		   switch (i)
		   {
		   case 0:
			   ray.direction = actualDirection;
		   case 1:
			   ray.direction = actualDirection + Vector(ssaaOffset, ssaaOffset, 0.0);
			   ray.direction = ray.direction.Normalize();
			   break;
		   case 2:
			   ray.direction = actualDirection + Vector(-ssaaOffset, ssaaOffset, 0.0);
			   ray.direction = ray.direction.Normalize();
			   break;
		   case 3:
			   ray.direction = actualDirection + Vector(ssaaOffset, -ssaaOffset, 0.0);
			   ray.direction = ray.direction.Normalize();
			   break;
		   case 4:
			   ray.direction = actualDirection + Vector(-ssaaOffset, -ssaaOffset, 0.0);
			   ray.direction = ray.direction.Normalize();
			   break;
		   }

		   finalColor = finalColor + Shade(la_escena, ray, la_camara);
	   }

	   return finalColor / 5.0f;
   }


   // Until this function is implemented, return white
   return Shade(la_escena, ray, la_camara);
}


// Create a ray from camera to the pixel i,j
Ray & RayTrace::get_Ray(int x, int y, Camera & cam, Scene & scene)
{

	float resX = scene.WINDOW_WIDTH;
	float resY = scene.WINDOW_HEIGHT;
	float FOV = cam.GetFOV();
	float farClip = cam.GetFarClip();

	float aspectRatio = (float)resX / resY;

	// Camera coordinates
	Vector cameraLocalPos(0.0f, 0.0f, 0.0f);
	Vector cameraView = (cam.GetTarget() - cameraLocalPos).Normalize();
	Vector cameraUp = cam.GetUp().Normalize();

	float u = 2.0f * x / resX - 1.0f;
	float v = 2.0f * y / resY - 1.0f;
	float uv = tan((FOV / 2.0f) * M_PI / 180.0f);

	// World coordinates
	Vector worldView = (cam.GetTarget() - cam.GetPosition()).Normalize();
	Vector worldLeft = (worldView.Cross(cameraUp)).Normalize();
	// We use right not left or the screen is swapped horizontally, so changing crossproduct order gives right
	Vector worldUp = (worldLeft.Cross(worldView)).Normalize();


	Vector px = worldLeft * u * uv * farClip * aspectRatio;
	Vector py = worldUp * v * uv * farClip;
	Vector pz = worldView * farClip;

	Vector p = px + py + pz;

	Ray * r = new Ray();
	r->direction = (p-cam.GetPosition()).Normalize(); //p.Normalize();//
	r->position = cam.GetPosition();

	return *r;
}

Vector RayTrace::Shade(Scene & scene, Ray & ray, Camera & cam)
{
	Vector final_color= scene.GetBackground().color;

	Intersection intersection;
	intersection.didHit = false;
	intersection.foundTriangle = false;
	intersection = intersect(scene, ray);

	if (intersection.didHit && ray.level < MAX_BOUNCES)
	{
		Vector reflectionFactor;
		Vector refractFactor;

		if (intersection.intersectionObject->IsSphere())
		{
			SceneTriangle t;

			switch (actualShading)
			{
			case PHONG:
				final_color = Phong(intersection, scene, cam);
				break;
			case COOK_TORRANCE:
				final_color = phongCookTorrance(intersection, scene, cam, t, false);
				break;
			}

			if (intersection.materialList.at(0)->reflective.Magnitude() != 0)
			{
				Ray reflectedRay(intersection.position, Reflect( ray.direction, intersection.direction), ray.level + 1);
				reflectedRay.position = reflectedRay.position + reflectedRay.direction*intersection.materialList.at(0)->reflectionBias;//to not crash with self
				reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->reflective;
				final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor *intersection.materialList.at(0)->reflective;
			/*
				//Send another Ray towards each light
				SceneLight * light;
				for (int i = 0; i < scene.GetNumLights(); i++)
				{
					light = scene.GetLight(i);
					reflectedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
					reflectedRay.position = reflectedRay.position + reflectedRay.direction*0.02f;//to not crash with self
					reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
					final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor *intersection.materialList.at(0)->reflective;

				}
				*/
			}

			if (intersection.materialList.at(0)->transparent.Magnitude() != 0)
			{
				Ray refractedRay(intersection.position, Refract(ray.direction, intersection.direction, intersection.materialList.at(0)->refraction_index), ray.level + 1);
				refractedRay.position = refractedRay.position + refractedRay.direction*intersection.materialList.at(0)->refractionBias;//to not crash with self
				refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
				final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor *intersection.materialList.at(0)->transparent;
			/*
				//Send another Ray towards each light
				SceneLight * light;
				for (int i = 0; i < scene.GetNumLights(); i++)
				{
					light = scene.GetLight(i);
					//refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(ray.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
					refractedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
					refractedRay.position = refractedRay.position + refractedRay.direction*0.02f;//to not crash with self
					refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
					final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor *intersection.materialList.at(0)->transparent;

				}
				*/

			}
		}
		else if (intersection.intersectionObject->IsTriangle())
		{
			SceneTriangle & triangle = static_cast<SceneTriangle&>(*intersection.intersectionObject);

			switch (actualShading)
			{
			case PHONG:
				final_color = PhongBaricentric(intersection, scene, cam, triangle);
				break;
			case COOK_TORRANCE:
				final_color = phongCookTorrance(intersection, scene, cam, triangle, true);
				break;
			}
			
			if (intersection.materialList.at(0)->reflective.Magnitude() != 0 && intersection.materialList.at(1)->reflective.Magnitude() != 0 && intersection.materialList.at(2)->reflective.Magnitude() != 0)
			{
				Ray reflectedRay(intersection.position, Reflect(ray.direction, intersection.direction), ray.level + 1);
				reflectedRay.position = reflectedRay.position + reflectedRay.direction*intersection.materialList.at(0)->reflectionBias;//to not crash with self
				 //reflectionFactor = intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]);//triangle.b[0] + intersection.materialList.at(1)->reflective * triangle.b[1] + intersection.materialList.at(2)->reflective * triangle.b[2];
				reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]));

				final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor*intersection.materialList.at(0)->reflective;
			
				/*
				//Send another Ray towards each light
				SceneLight * light;
				for (int i = 0; i < scene.GetNumLights(); i++)
				{
					light = scene.GetLight(i);
					reflectedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
					reflectedRay.position = reflectedRay.position + reflectedRay.direction*0.02f;//to not crash with self
					reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
					final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor *intersection.materialList.at(0)->reflective;

				}
				*/
			
			}

			if (intersection.materialList.at(0)->transparent.Magnitude() != 0)
			{
				Ray refractedRay(intersection.position, Refract(ray.direction, intersection.direction, intersection.materialList.at(0)->refraction_index), ray.level + 1);
				refractedRay.position = refractedRay.position + refractedRay.direction*intersection.materialList.at(0)->refractionBias;//to not crash with self
				 //refractFactor = intersection.materialList.at(0)->transparent * Vector(triangle.b[0], triangle.b[1], triangle.b[2]) ; //+ intersection.materialList.at(1)->transparent * triangle.b[1] + intersection.materialList.at(2)->transparent * triangle.b[2];
				refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]));
				final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor*intersection.materialList.at(0)->transparent;
			
				/*
				//Send another Ray towards each light
				SceneLight * light;
				for (int i = 0; i < scene.GetNumLights(); i++)
				{
					light = scene.GetLight(i);
					refractedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
					refractedRay.position = refractedRay.position + refractedRay.direction*0.02f;//to not crash with self
					refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
					final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor *intersection.materialList.at(0)->transparent;

				}
				*/
			
			}
			
		}
		else // Model
		{
			if (intersection.foundTriangle)
			{
				SceneModel & model = static_cast<SceneModel&>(*intersection.intersectionObject);
				SceneTriangle & triangle = *model.GetTriangle(intersection.triangleIndex);

				switch (actualShading)
				{
				case PHONG:
					final_color = PhongBaricentric(intersection, scene, cam, triangle);
					break;
				case COOK_TORRANCE:
					final_color = phongCookTorrance(intersection, scene, cam, triangle, true);
					break;
				}
					
				if (intersection.materialList.at(0)->reflective.Magnitude() != 0 && intersection.materialList.at(1)->reflective.Magnitude() != 0 && intersection.materialList.at(2)->reflective.Magnitude() != 0)
				{
					Ray reflectedRay(intersection.position, Reflect(ray.direction, intersection.direction), ray.level + 1);
					reflectedRay.position = reflectedRay.position + reflectedRay.direction*intersection.materialList.at(0)->reflectionBias;//to not crash with self
					//reflectionFactor = intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]);//triangle.b[0] + intersection.materialList.at(1)->reflective * triangle.b[1] + intersection.materialList.at(2)->reflective * triangle.b[2];
					reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]));
					final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor*intersection.materialList.at(0)->reflective;
				
					/*
					//Send another Ray towards each light
					SceneLight * light;
					for (int i = 0; i < scene.GetNumLights(); i++)
					{
						light = scene.GetLight(i);
						reflectedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
						reflectedRay.position = reflectedRay.position + reflectedRay.direction*0.02f;//to not crash with self
						reflectionFactor = CalculateFresnelReflectance(reflectedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
						final_color = final_color + Shade(scene, reflectedRay, cam) * reflectionFactor *intersection.materialList.at(0)->reflective;

					}
					*/
				}

				if (intersection.materialList.at(0)->transparent.Magnitude() != 0)
				{
					Ray refractedRay(intersection.position, Refract(ray.direction, intersection.direction, intersection.materialList.at(0)->refraction_index), ray.level + 1);
					refractedRay.position = refractedRay.position + refractedRay.direction*intersection.materialList.at(0)->refractionBias;//to not crash with self
					 //refractFactor = intersection.materialList.at(0)->transparent * Vector(triangle.b[0], triangle.b[1], triangle.b[2]);//triangle.b[0] + intersection.materialList.at(1)->transparent * triangle.b[1] + intersection.materialList.at(2)->transparent * triangle.b[2];
					refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective * Vector(triangle.b[0], triangle.b[1], triangle.b[2]));

					final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor*intersection.materialList.at(0)->transparent;
			
					/*
					//Send another Ray towards each light
					SceneLight * light;
					for (int i = 0; i < scene.GetNumLights(); i++)
					{
						light = scene.GetLight(i);
						refractedRay = Ray(intersection.position, (light->position - intersection.position).Normalize(), ray.level + 1);
						refractedRay.position = refractedRay.position + refractedRay.direction*0.02f;//to not crash with self
						refractFactor = Vector(1.0, 1.0, 1.0) - CalculateFresnelReflectance(refractedRay.direction, intersection.direction, intersection.materialList.at(0)->refraction_index, intersection.materialList.at(0)->reflective);//intersection.materialList.at(0)->transparent;
						final_color = final_color + Shade(scene, refractedRay, cam) * refractFactor *intersection.materialList.at(0)->transparent;

					}
					*/
				}
				
			}
		}
	}
	//else if ((!intersection.didHit)&&ray.level < MAX_BOUNCES){final_color = scene.GetBackground().color;}

	else if((!intersection.didHit))
	{

		if (ray.level>1 && ray.level < MAX_BOUNCES) {
			final_color = Vector(0.0, 1.0, 0.0);
		//}if (ray.level < 1){
			final_color = scene.GetBackground().color;
		}

	}
	else
	{
		final_color = Vector(0.0, 0.0, 0.0);
	}

	//clamp final outcome
	//final_color = Vector(Clamp(final_color.x, 0.0f, 1.0f), Clamp(final_color.y, 0.0f, 1.0f), Clamp(final_color.z, 0.0f, 1.0f));

	return final_color;
}


// http://paulbourke.net/geometry/reflected/   //theta should be Ri:  ray Ri incident at a point on a surface with normal N
Vector RayTrace::Reflect(Vector theta, Vector normal)
{
	//reflected ray->  Rr = raydirection - 2 * Normal * (raydirection * Normal)
	// Rr = Ri - 2 N(Ri.N)
	return theta - (normal).Scale(2 * theta.Dot(normal));
}

//https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
//http://www.scratchapixel.com/code.php?id=3&origin=/lessons/3d-basic-rendering/introduction-to-ray-tracing
Vector RayTrace::Refract(Vector & rayDir, Vector normal, Vector refractIndex)
{
	// Method 1 from scrathcapixel
	bool inside = false;
	if (rayDir.Dot(normal) > 0) { inside = true; normal = normal * -1.0f; }

	//ASSUMING refraction index is the ssame for rgb, i only acces the .r component
	float ior = refractIndex.x; //1.1<== refractIndex
	float eta = (inside) ? ior : 1.0f / ior; // are we inside or outside the surface? 
	float cosi = -(normal).Dot(rayDir);
	float k = 1.0f - eta * eta * (1.0f - cosi * cosi);

	Vector refrdir = rayDir * eta - normal * (eta *  cosi - sqrt(k));
	refrdir.Normalize();

	if (k< 0.0) {

		refrdir = Vector(0.0f, 0.0f, 0.0f);
	}

	return refrdir;
	

	//Method2 from the paper
	/*
	bool inside = false;
	if (rayDir.Dot(normal) > 0) { inside = true; normal = normal * -1.0f; }

	//ASSUMING refraction index is the ssame for rgb, i only acces the .r component
	float ior = refractIndex.x; //1.1<== refractIndex
	float n = (inside) ? ior : 1.0f / ior; // are we inside or outside the surface? 
	float cosi = -(normal).Dot(rayDir);
	float sinT2 = n * n * (1.0f - cosi * cosi);
	if (sinT2 > 1.0f) { return Vector(0.0f, 0.0f, 0.0f); } //TIR 
	float cosT = sqrt(1.0f - sinT2);
	 
	Vector refrdir = rayDir * n + normal * (n *  cosi - cosT);
	return refrdir;
	*/
	
}


float RayTrace::brdf(Vector l, Vector v, Vector n, SceneMaterial & material) {
	//halfvector
	Vector h = (v - l).Normalize();

	//Fraccion reflejada en funcion del rayo incidente y normal
	float F = material.Fo + (1 - material.Fo) * pow((1 - (l.Dot(n))), 5);


	//tamaño y forma de brillo, Funcion distribucion de normales
	float D = (material.alpha + 2) * pow(n.Dot(n), material.alpha) / (2 * M_PI);

	//Shadowmasking, visible desde direccion de luz y camara
	float G = (1.0 / pow(l.Dot(h), 2));

	//BRDF
	return F * G * D / 4.0;
}

//https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
Vector RayTrace::CalculateFresnelReflectance(Vector & rayDir, Vector normal, Vector refractIndex,Vector objectReflectiveness) {
	
	//METHOD 1
	/*
	bool inside = false;
	if (rayDir.Dot(normal) > 0) { inside = true; normal = normal * -1.0f; }

		//ASSUMING refraction index is the ssame for rgb, i only acces the .r component
		float ior = refractIndex.x; //1.1<== refractIndex

		float n1; //ior from 
		float n2; //ior to

		if (inside) {
			n1 = ior;
			n2 = 1.0f;//air
		}
		else {
			n1 = 1.0f;//air
			n2 = ior;
		}
		
		const float n = n1 / n2;
		const float cosI = -normal.Dot(rayDir);
		const float sinT2 = n*n*(1.0 - cosI*cosI);
		if (sinT2 > 1.0) { return Vector(1.0f, 1.0f, 1.0f); } //TIR
		const float cosT = sqrt(1.0 - sinT2);
		const float rOrth = (n1*cosI - n2*cosT) / (n1*cosI - n2*cosT);
		const float rPar = (n2*cosI - n1*cosT) / (n2*cosI - n1*cosT);
		const float r = (rOrth*rOrth + rPar*rPar) / 2.0f;

		//return Vector(r, r, r);

		return objectReflectiveness + Vector(r, r, r)*(Vector(1.0,1.0,1.0)- objectReflectiveness);
		*/

 //METHOD 2: https://blog.demofox.org/2017/01/09/raytracing-reflection-refraction-fresnel-total-internal-reflection-and-beers-law/
	bool inside = false;
	if (rayDir.Dot(normal) > 0) { inside = true; normal = normal * -1.0f; }

		//ASSUMING refraction index is the ssame for rgb, i only acces the .r component
		float ior = refractIndex.x; //1.1<== refractIndex

		float n1; //ior from 
		float n2; //ior to

		if (inside) {
			n1 = ior;
			n2 = 1.0f;//air
		}
		else {
			n1 = 1.0f;//air
			n2 = ior;
		}

		// Schlick aproximation
		float r0 = (n1 - n2) / (n1 + n2);
		r0 *= r0;
		float cosX = -normal.Dot(rayDir);
		if (n1 > n2)
		{
			float n = n1 / n2;
			float sinT2 = n*n*(1.0 - cosX*cosX);
			// Total internal reflection
			if (sinT2 > 1.0)
				return Vector(1.0,1.0,1.0);
			cosX = sqrt(1.0 - sinT2);
		}
		float x = 1.0 - cosX;
		float ret = r0 + (1.0 - r0)*x*x*x*x*x;

		// adjust reflect multiplier for object reflectivity
		ret = (objectReflectiveness.x + (1.0 - objectReflectiveness.x) * ret);
		return Vector(1.0,1.0,1.0)*ret;
		
	
}

RayTrace::Intersection RayTrace::intersect(Scene & scene, Ray & ray)
{
	Intersection inter;
	inter.didHit = false;
	inter.foundTriangle = false;

	float tmin = INFINITY;
	float intersect_distance = tmin;
	unsigned int numObj = scene.GetNumObjects();

	SceneObject *intshape = new SceneObject();
	int a = -1; //auxiliar variable.

	SceneObject * sceneObj;
	bool theres_intersection = false;

	for (int i = 0; i < numObj; i++)
	{
		sceneObj = scene.GetObjectW(i);

		if (intersectWithShape(*sceneObj, ray, intersect_distance, inter))
		{
			if (intersect_distance >0.02f && intersect_distance < tmin) //this 0.02f helps it get unstuck and creates less black or white pixles.
			{
				tmin = intersect_distance;
				intshape = scene.GetObjectW(i);
				inter.sceneObjectId = i;  //save a copy of the intersection object id in scene, used later in isVisibleFunction to avoid checking against self
				a = 1;
			}
		}
	}

	//Detecting a Correct Intersection.
	if (a == 1)
	{
		inter.didHit = true;
		inter.position = ray.position + ray.direction*tmin;//intersect_distance;
		inter.intersectionObject = intshape;

		if (intshape->IsTriangle())
		{
			SceneTriangle & triangle = static_cast<SceneTriangle&>(*intshape);

			inter.foundTriangle = true;
			inter.triangle = &triangle;

			inter.materialList.push_back(scene.GetMaterialByMap(triangle.material[0]));
			inter.materialList.push_back(scene.GetMaterialByMap(triangle.material[1]));
			inter.materialList.push_back(scene.GetMaterialByMap(triangle.material[2]));

			inter.material = triangle.material[0];	//get material in intersection pt
													//get normal in intersection pt
													//inter.direction = ((triangle.vertex[1] - triangle.vertex[0]).Cross(triangle.vertex[2] - triangle.vertex[0])).Normalize();  
			inter.direction = ((triangle.normal[0] * triangle.b[0]) + (triangle.normal[1] * triangle.b[1]) + (triangle.normal[2] * triangle.b[2])).Normalize();
				
		}

		if (intshape->IsSphere())
		{
			SceneSphere & sphere = static_cast<SceneSphere&>(*intshape);
			inter.material = sphere.material;	//get material in intersection pt
			inter.materialList.push_back(scene.GetMaterialByMap(sphere.material));
			inter.direction = (inter.position - sphere.center).Normalize();   //get normal in intersection pt
		}

		if (intshape->IsModel())
		{
			SceneModel & model = static_cast<SceneModel&>(*intshape);

			if (inter.foundTriangle)
			{
				//inter.direction = ((inter.triangle->vertex[1] - inter.triangle->vertex[0]).Cross(inter.triangle->vertex[2] - inter.triangle->vertex[0])).Normalize();
				inter.direction = ((inter.triangle->normal[0] * inter.triangle->b[0]) + (inter.triangle->normal[1] * inter.triangle->b[1]) + (inter.triangle->normal[2] * inter.triangle->b[2])).Normalize();
				inter.material = inter.triangle->material[0];

				inter.materialList.push_back(scene.GetMaterialByMap(inter.triangle->material[0]));
				inter.materialList.push_back(scene.GetMaterialByMap(inter.triangle->material[1]));
				inter.materialList.push_back(scene.GetMaterialByMap(inter.triangle->material[2]));
			}
			else
			{
				inter.direction = Vector(1, 1, 1);
				inter.material = model.GetTriangle(0)->material[0];
				inter.materialList.push_back(scene.GetMaterialByMap(model.GetTriangle(0)->material[0]));
			}

		}
	}


	return inter;
}


bool RayTrace::intersectWithShape(SceneObject & sceneObj, Ray & ray, float & intersect_distance, Intersection & intersection)
{
	bool theres_intersection = false;

	if (sceneObj.IsTriangle())
	{
		theres_intersection = intersectionRayTriangle(static_cast<SceneTriangle&>(sceneObj), ray, intersect_distance);
	}
	else if (sceneObj.IsSphere())
	{
		theres_intersection = intersectionRaySphere(static_cast<SceneSphere&>(sceneObj), ray, intersect_distance);
	}
	else
	{
		theres_intersection = intersectionRayModel(static_cast<SceneModel&>(sceneObj), ray, intersect_distance, intersection);
	}

	return theres_intersection;
}

//vec3 ray_origen, vec3 ray_direction, vec3 sphere_center, float sphere_radius, vec3& outIntersectionPoint
bool RayTrace::intersectionRaySphere(SceneSphere & s, Ray & ray, float & distance)
{
	Vector ray_origen = ray.position;
	Vector ray_direction = ray.direction;
	float sphere_radius = s.radius;
	Vector sphere_center = s.center;

	//if inside sphere, ray will always intersect
	float diff = abs((ray_origen - sphere_center).Magnitude());
	if (diff <= sphere_radius) {
		distance = sphere_radius - diff;
		return true;
	}

	//Auxiliar operations
	Vector o_c = ray_origen - sphere_center;
	float a = ray_direction.Dot(ray_direction);
	float b = 2.0f * o_c.Dot(ray_direction);
	float c = o_c.Dot(o_c) - pow(sphere_radius, 2);

	//The discriminant of the quadratic ecuation
	float discriminant = b * b - 4.0f * a * c;

	//If discriminant <0, no intersection.
	bool intersectionExists = discriminant >= 0.0f;

	float t1 = (-b + sqrt(discriminant)) / (2.0f * a);
	float t2 = (-b - sqrt(discriminant)) / (2.0f * a);

	float tfinal = 0.0;//INFINITY;

					   //To determine the lowest positive value(menor no - negativo)
	if (t1 > 0.0f && (t2 < 0.0f || t1 < t2)) {
		tfinal = t1;
		distance = tfinal;
	}
	else if (t2 > 0.0f && (t1 < 0.0f || t2 < t1))
	{
		tfinal = t2;
		distance = tfinal;
	}
	else {//Then there is no valid intersection
		  //For example : the sphere is on the same axis as d, but c is behind 0,
		  //both t1 and t2 are negative.See "pruebas con matlab 2" in notes.
		intersectionExists = false;
	}

	return intersectionExists;

}

//vec3 ray_origen, vec3 ray_direction, vec3 triangle_V0, vec3 triangle_V1, vec3 triangle_V2,const int faceType, vec3& outIntersectionPoint, vec3& uvt, bool& triangleFacingOutwards
bool RayTrace::intersectionRayTriangle(SceneTriangle & t, Ray & r, float & distance)
{
	Vector ray_origen = r.position;
	Vector ray_direction = r.direction;

	Vector triangle_V0 = t.vertex[0];
	Vector triangle_V1 = t.vertex[1];
	Vector triangle_V2 = t.vertex[2];

	//Auxiliar Calculations
	Vector T = ray_origen - triangle_V0;   // _s
	Vector E1 = triangle_V1 - triangle_V0;  //_edge1
	Vector E2 = triangle_V2 - triangle_V0;     //_edge2
	Vector F = ray_direction.Cross(E2); //_h
	Vector G = T.Cross(E1); //_q

	Vector s = (T) / (E1.Dot(F));
	Vector r_ = s.Cross(E1);

	Vector n = E1.Cross(E2).Normalize(); // normal of triangle
	bool triangleFacingOutwards = n.Dot(ray_direction)<0.0f ;  //Good if   d--->  <-- - n      

	float OneOverFE1 = 1.0f / F.Dot(E1);

	float u = F.Dot(T)*OneOverFE1;
	float v = G.Dot(ray_direction)*OneOverFE1;

	Vector uvt;

	uvt.x = u;
	uvt.y = v;

	//By default, assume no interaction
	bool intersectionExists = false;

	float dst = INFINITY;

	//Restrictions:
	if (u >= 0.0f && v >= 0.0f && u <= 1.0f && v <= 1.0f && u + v <= 1.0f) {
		dst = G.Dot(E2)*OneOverFE1;

		intersectionExists = (dst > 0.0f);   //t must be positive, this is the only place I set intersectionExists to true

		if (intersectionExists) {
			distance = dst;
			uvt.z = dst;

			// SHIFTED
			t.b[1] = s.Dot(F);
			t.b[2] = r_.Dot(ray_direction);
			t.b[0] = 1.0f - t.b[2] - t.b[1];
			
			/*
			t.b[0] = s.Dot(F);
			t.b[1] = r_.Dot(ray_direction);
			t.b[2] = 1.0f - t.b[0] - t.b[1];
			*/
			//t.Baricentricas = triangle_V0 + E1 * u + E2 * v;
		}

	}

	return intersectionExists;
}

bool RayTrace::intersectionRayModel(SceneModel & model, Ray & r, float & out, Intersection & intersection)
{
	int numTriangles = model.GetNumTriangles();
	

	for (int i = 0; i < numTriangles; i++)
	{
		SceneTriangle * sceneTriangle = model.GetTriangle(i);

		
		//for (int j = 0; j < 3; j++) {
			//translation
			//sceneTriangle->vertex[j] = sceneTriangle->vertex[j] + model.position;
			//Scale
			//sceneTriangle->vertex[j] = sceneTriangle->vertex[j] * model.scale;
			//Rotation
			//todo
		//}
		

		if (intersectionRayTriangle(*sceneTriangle, r, out))
		{
			intersection.foundTriangle = true;
			intersection.triangle = sceneTriangle;
			intersection.triangleIndex = i;
			return true;
		}
	}

	return false;
}

//our clamp function
float RayTrace::Clamp(float value, float low, float high) {
	if (value < low) return low;
	else if (value > high) return high;
	else return value;
}

//https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
Vector RayTrace::Lerp(Vector start, Vector end, Vector percent)
{
	return (start + percent * (end - start));
}

//http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx

Vector RayTrace::fcookTorrance(Vector l, Vector v, Vector n, SceneMaterial & material) {
	//halfvector
	Vector h = (v - l).Normalize();

	//Fraccion reflejada en funcion del rayo incidente y normal
	// Calculate colour at normal incidence
	Vector F0 = ((Vector(1, 1, 1) - material.refraction_index) / ((Vector(1, 1, 1) + material.refraction_index)));// .Abs();
	F0 = F0 * F0;
	F0 = Lerp(material.diffuse, material.reflective, F0);
	
	//use the material value. 
	F0 = Vector(1,1,1)*material.Fo;

	Vector F = F0 + (Vector(1.0f, 1.0f, 1.0f) - F0) * pow((1.0f - (l.Dot(n))), 5);  //h.Dot(v)

	//tamaño y forma de brillo, Funcion distribucion de normales:  alpha is roughness between 0 and 1.  (0 is super shiny)
	float xhn = abs(h.Dot(n)) > 0 ? 1 : 0;
	float mn2 = pow(n.Dot(n), 2);
	float alpha2 = pow(material.alpha, 2);
	float D = alpha2 * xhn / (M_PI*pow(mn2*(alpha2 + ((1 - mn2) / mn2)), 2));

	//Shadowmasking, visible desde direccion de luz y camara
	float G1 = GGX_PartialGeometryTerm(l, n, h, material.alpha);
	float G2 = GGX_PartialGeometryTerm(v, n, h, material.alpha);   //donde wi=l  wo=v

	float G = G1 * G2;

	//BRDF
	return (F * G * D) / (4.0 *l.Dot(n) * v.Dot(n));


}

float RayTrace::GGX_PartialGeometryTerm(Vector w, Vector n, Vector h, float alpha)
{
	float chi = 1.0;//  (w.Dot(n) / (w.Dot(n))) > 0 ? 1 : 0;
	float VoH2 = w.Dot(h);
	VoH2 = VoH2 * VoH2;
	float tan2 = (1.0 - VoH2) / VoH2;
	return (chi * 2.0) / (1.0 + sqrt(1.0 + alpha * alpha * tan2));
}

///eleveates each compenent in the vector to the power
Vector RayTrace::Pow(Vector v, float power) {
	return Vector(pow(v.x, power), pow(v.y, power), pow(v.z, power));
}


//http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx
Vector RayTrace::phongCookTorrance(Intersection inter, Scene & scene, Camera & cam, SceneTriangle & triangle, bool baricentric)
{
	Vector color(0.0f, 0.0f, 0.0f);
	SceneLight * light;

	if (baricentric)
	{
		// Light properties

		// Ambiental
		Vector Ia = scene.GetBackground().ambientLight;
		Vector Ka = scene.GetMaterialByMap(triangle.material[0])->diffuse*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->diffuse*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->diffuse*triangle.b[2];

		// Diffuse
		Vector Kd = Ka;

		// Specular
		float specularPower = scene.GetMaterialByMap(triangle.material[0])->shininess*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->shininess*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->shininess*triangle.b[2];
		Vector Ks = scene.GetMaterialByMap(triangle.material[0])->specular*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->specular*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->specular*triangle.b[2];

		Vector L;
		Vector V;
		Vector N = triangle.normal[0] * triangle.b[0] + triangle.normal[1] * triangle.b[1] + triangle.normal[2] * triangle.b[2];

		Vector ambiental = Ia * Ka;
		Vector diffuse;
		Vector specular;

		for (int i = 0; i < scene.GetNumLights(); i++)
		{
			light = scene.GetLight(i);

			if (IsVisible(scene, *light, inter))
			{

				L = (light->position - inter.position).Normalize();
				V = (cam.GetPosition() - inter.position).Normalize();

				if (inter.foundTriangle && (scene.GetMaterialByMap(triangle.material[0])->HasTexure() && scene.GetMaterialByMap(triangle.material[1])->HasTexure() && scene.GetMaterialByMap(triangle.material[2])->HasTexure()))
				{
					//Mapping texture by barycentric coordinates
					float texU = triangle.u[0] * triangle.b[0] + triangle.u[1] * triangle.b[1] + triangle.u[2] * triangle.b[2];
					float texV = triangle.v[0] * triangle.b[0] + triangle.v[1] * triangle.b[1] + triangle.v[2] * triangle.b[2];

					Vector diffuse1 = inter.materialList[0]->GetTextureColor(texU, texV);
					Vector diffuse2 = inter.materialList[1]->GetTextureColor(texU, texV);
					Vector diffuse3 = inter.materialList[2]->GetTextureColor(texU, texV);

					diffuse = (diffuse1 + diffuse2 + diffuse3);// *Clamp(L.Dot(N), 0.0, 1.0);;
				}
				else
				{
					diffuse = Kd * Clamp(L.Dot(N), 0.0, 1.0);//Clamp(0.0, 1.0, L.Dot(N));
				}
				 
				Vector torrance  =  (fcookTorrance(L, V, triangle.normal[0] * triangle.b[0], *inter.materialList[0])).Clamp(0.0, 1.0) + 
									(fcookTorrance(L, V, triangle.normal[1] * triangle.b[1], *inter.materialList[1])).Clamp(0.0, 1.0) + 
									(fcookTorrance(L, V, triangle.normal[2] * triangle.b[2], *inter.materialList[2])).Clamp(0.0, 1.0);

				//Alternative 1: Clampling shine
				//specular = Ks * pow(Clamp(Reflect(N, L).Dot(V), 0.0, 1.0), specularPower) * torrance;
				//Alt 3:		
				//specular = Ks *torrance;
				//alt 4: No clamp
				specular = Ks * pow(Reflect(L, N).Dot(V), specularPower) * torrance;


				color = color + get_incoming_light(*light, inter) * (ambiental + diffuse + specular);

			}
			else
			{
				color = Vector(0.0, 0.0, 0.0);
			}
		}
	}
	else // normal phong torrance
	{
		// Light properties

		// Ambiental
		Vector Ia = scene.GetBackground().ambientLight;
		Vector Ka = inter.materialList.at(0)->diffuse;

		// Diffuse
		Vector Kd = inter.materialList.at(0)->diffuse;

		// Specular
		float specularPower = inter.materialList.at(0)->shininess;
		Vector Ks = inter.materialList.at(0)->specular;
		float attenuation;
		float LDistance;
		Vector L;
		Vector V;
		Vector N = inter.direction;

		Vector ambiental = Ia * Ka;
		Vector diffuse;
		Vector specular;

		for (int i = 0; i < scene.GetNumLights(); i++)
		{
			light = scene.GetLight(i);

			if (IsVisible(scene, *light, inter))
			{
				L = (light->position - inter.position).Normalize();
				V = (cam.GetPosition() - inter.position).Normalize();  //- light->position

				diffuse = Kd * Clamp(L.Dot(N), 0.0, 1.0); 

		// Alternative0 same as phong		
				//specular = Ks *pow(Clamp(Reflect(N, L).Dot(V), 0.0, 1.0), specularPower) * fcookTorrance(L, V, N, *inter.materialList[0]).Clamp(0.0, 1.0);
		//alternative 1	
				//specular = Ks *Pow(fcookTorrance(L, V, N, *inter.materialList[0]).Clamp(0.0, 1.0),specularPower);//pow(Clamp(Reflect(N, L).Dot(V), 0.0, 1.0), specularPower) * fcookTorrance(L, V, N, *inter.materialList[0]).Clamp(0.0, 1.0);
		//alternative 2
				//specular = (Ks *pow(Reflect(N,L).Dot(V), specularPower) * fcookTorrance(L, V, N, *inter.materialList[0])).Clamp(0.0, 1.0);
		//alt 3:		
				specular = Ks *fcookTorrance(L, V, N, *inter.materialList[0]).Clamp(0.0f, 1.0f);
		//alt 4
				//specular = (Ks *pow(Reflect(L, N).Dot(V), specularPower) * fcookTorrance(L, V, N, *inter.materialList[0]));

			//alt 5
				//specular = Ks * Pow(fcookTorrance(L, V, N, *inter.materialList[0]), specularPower);

			//alt 6
				//specular = Ks * Pow(fcookTorrance(L, V, N, *inter.materialList[0]), specularPower).Clamp(0.0, 1.0);

			//alt 7
				//specular = Ks * fcookTorrance(L, V, N, *inter.materialList[0]);


				color = color + get_incoming_light(*light, inter) * (ambiental + diffuse + specular);

			}
			else
			{
				color = Vector(0.0, 0.0, 0.0);
			}
		}
	}

	return color;
}


Vector RayTrace::Phong(Intersection inter, Scene & scene, Camera & cam) 
{

	Vector color(0.0f, 0.0f, 0.0f);
	SceneLight * light;

	// Light properties

	// Ambiental
	Vector Ia = scene.GetBackground().ambientLight;
	Vector Ka = inter.materialList.at(0)->diffuse;

	// Diffuse
	Vector Kd = inter.materialList.at(0)->diffuse;

	// Specular
	float specularPower = inter.materialList.at(0)->shininess;
	Vector Ks = inter.materialList.at(0)->specular;
	float attenuation;
	float LDistance;
	Vector L;
	Vector V;
	Vector N = inter.direction;

	Vector ambiental = Ia * Ka;
	Vector diffuse;
	Vector specular;

	for (int i = 0; i < scene.GetNumLights(); i++)
	{
		light = scene.GetLight(i);

		if (IsVisible(scene, *light, inter))
		{
			L = (light->position - inter.position).Normalize();
			V = (cam.GetPosition() - light->position).Normalize();

			diffuse = Kd * Clamp(L.Dot(N),0.0, 1.0);
			specular = Ks * pow(Clamp(Reflect(N, L).Dot(V), 0.0, 1.0), specularPower);

			color = color + get_incoming_light(*light, inter) * (ambiental + diffuse + specular);

		}
		else
		{
			color = Vector(0.0, 0.0, 0.0);
		}
	}

	return color;

}

///Including atenuation by distance
Vector  RayTrace::get_incoming_light(SceneLight & light, Intersection intersection)
{
	//float tlight = (light.position - intersection.position).Magnitude();
	//return light.color;//.Scale(1.0 / pow(tlight, 2));

	float dist = (light.position - intersection.position).Magnitude();
	float atenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear*dist + light.attenuationQuadratic*dist*dist);
	return  (light.color)*atenuation;

}

Vector RayTrace::PhongBaricentric(Intersection inter, Scene &scene, Camera &cam, SceneTriangle & triangle)
{
	Vector color(0.0f, 0.0f, 0.0f);
	SceneLight * light;

	// Light properties

	// Ambiental
	Vector Ia = scene.GetBackground().ambientLight;
	Vector Ka = scene.GetMaterialByMap(triangle.material[0])->diffuse*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->diffuse*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->diffuse*triangle.b[2];

	// Diffuse
	Vector Kd = Ka;

	// Specular
	float specularPower = scene.GetMaterialByMap(triangle.material[0])->shininess*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->shininess*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->shininess*triangle.b[2];
	Vector Ks = scene.GetMaterialByMap(triangle.material[0])->specular*triangle.b[0] + scene.GetMaterialByMap(triangle.material[1])->specular*triangle.b[1] + scene.GetMaterialByMap(triangle.material[2])->specular*triangle.b[2];

	Vector L;
	Vector V;
	Vector N = triangle.normal[0] * triangle.b[0] + triangle.normal[1] * triangle.b[1] + triangle.normal[2] * triangle.b[2];

	Vector ambiental = Ia * Ka;
	Vector diffuse;
	Vector specular;

	for (int i = 0; i < scene.GetNumLights(); i++)
	{
		light = scene.GetLight(i);

		if (IsVisible(scene, *light, inter))
		{

			L = (light->position - inter.position).Normalize();
			V = (cam.GetPosition() - light->position).Normalize();

			if (inter.foundTriangle && (scene.GetMaterialByMap(triangle.material[0])->HasTexure() && scene.GetMaterialByMap(triangle.material[1])->HasTexure() && scene.GetMaterialByMap(triangle.material[2])->HasTexure()))
			{
				//Mapping texture by barycentric coordinates
				float texU = triangle.u[0] * triangle.b[0] + triangle.u[1] * triangle.b[1] + triangle.u[2] * triangle.b[2];
				float texV = triangle.v[0] * triangle.b[0] + triangle.v[1] * triangle.b[1] + triangle.v[2] * triangle.b[2];

				Vector diffuse1 = inter.materialList[0]->GetTextureColor(texU, texV);
				Vector diffuse2 = inter.materialList[1]->GetTextureColor(texU, texV);
				Vector diffuse3 = inter.materialList[2]->GetTextureColor(texU, texV);

				diffuse = (diffuse1 + diffuse2 + diffuse3);// *Clamp(L.Dot(N), 0.0, 1.0);;
			}
			else
			{
				diffuse = Kd * Clamp(L.Dot(N), 0.0, 1.0);
			}
	
			specular = Ks * pow(Clamp(Reflect(N, L).Dot(V), 0.0, 1.0), specularPower); //Reflect(L, N)

			color = color + get_incoming_light(*light, inter) * (ambiental + diffuse + specular);

		}
		else
		{
			color = Vector(0.0, 0.0, 0.0);
		}
	}

	return color;
}


///Returns true if there is a direct line (with no intersections) between this point and the light
bool RayTrace::IsVisible(Scene & scene, SceneLight & light, Intersection intersection)
{
	float tlight = (light.position - intersection.position).Magnitude();
	Ray ray(intersection.position, (light.position - intersection.position).Normalize(), 1);
	//tiny offset to remove noise, has to be - in this case.
	ray.position = ray.position - ray.direction*0.02f;

	float intersect_distance = INFINITY;
	unsigned int numObj = scene.GetNumObjects();

	SceneObject * sceneObj;

	for (int i = 0; i < numObj; i++)
	{
		//dont count myself
		if (i == intersection.sceneObjectId) { continue; }

		sceneObj = scene.GetObjectW(i);

		intersectWithShape(*sceneObj, ray, intersect_distance, intersection);

		if (intersect_distance < tlight)
		{
			return false;
		}
	}

	return true;
}
