/*
  15-462 Computer Graphics I
  Assignment 3: Ray Tracer
  C++ Normal Renderer Class
  Author: rtark
  Oct 2007

  NOTE: You do not need to edit this file for this assignment but may do so

  This file defines the following:
	NormalRenderer Class

  This is the "Normal Renderer" which draws the descriptor scene with normal
	OpenGL calls in real-time rather than ray-tracing
	This can be a good reference in learning to access the scene objects
*/

#ifndef NORMALRENDERER_H
#define NORMALRENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Utils.h"

/*
	NormalRenderer Class - The class that will render the scene as normal objects in real-time

	This class renders the scene when the RayTrace class isn't rendering
*/
class NormalRenderer
{
	/* - Scene Variable Pointer for the Scene Definition - */
	Scene *m_pScene;

	// - SetRenderStates - Sets the OpenGL Render States
	void SetRenderStates (void)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		glEnable(GL_LIGHT3);
		glEnable(GL_LIGHT4);
		glEnable(GL_LIGHT5);
		glEnable(GL_LIGHT6);
		glEnable(GL_LIGHT7);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE);
	}

	// - UnsetRenderStates - Sets the OpenGL Render States Back
	void UnsetRenderStates (void)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT3);
		glDisable(GL_LIGHT4);
		glDisable(GL_LIGHT5);
		glDisable(GL_LIGHT6);
		glDisable(GL_LIGHT7);
		glDisable(GL_COLOR_MATERIAL);
	}
public:
	// -- Constructors & Destructors --
	NormalRenderer (void) : m_pScene (NULL) {}
	~NormalRenderer (void) {}

	// - SetScene - Sets the scene variable for the Renderer to draw the scene
	void SetScene (Scene *scn) { m_pScene = scn; }

	// - RenderScene - Draws the scene with OpenGL calls
	void RenderScene (void)
	{
		if (m_pScene)
		{
			// Prepare Render States
			SetRenderStates ();

			// Get the Background
			SceneBackground sceneBg = m_pScene->GetBackground ();
			glClearColor(sceneBg.color.x, sceneBg.color.y, sceneBg.color.z, 0);
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat *)&sceneBg.ambientLight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Get the Lights (OpenGL will only support up to 8)
			unsigned int numLights = m_pScene->GetNumLights ();
			for (unsigned int n = 0; n < numLights && n < 8; n++)
			{
				SceneLight *sceneLight = m_pScene->GetLight (n);
		
				glLightfv(GL_LIGHT0 + n, GL_SPECULAR, (GLfloat *)&sceneLight->color);
				glLightf (GL_LIGHT0 + n, GL_CONSTANT_ATTENUATION, sceneLight->attenuationConstant);
				glLightf (GL_LIGHT0 + n, GL_LINEAR_ATTENUATION, sceneLight->attenuationLinear);
				glLightf (GL_LIGHT0 + n, GL_QUADRATIC_ATTENUATION, sceneLight->attenuationQuadratic);
				glLightfv(GL_LIGHT0 + n, GL_DIFFUSE, (GLfloat *)&sceneLight->color);
				glLightfv(GL_LIGHT0 + n, GL_POSITION, (GLfloat *)&sceneLight->position);
			}

			// Get the Camera
			Camera sceneCam = m_pScene->GetCamera ();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective (sceneCam.GetFOV (), (GLdouble)Scene::WINDOW_WIDTH/Scene::WINDOW_HEIGHT, sceneCam.GetNearClip (), sceneCam.GetFarClip ());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			gluLookAt (sceneCam.position.x, sceneCam.position.y, sceneCam.position.z,
						sceneCam.target.x, sceneCam.target.y, sceneCam.target.z,
						sceneCam.up.x, sceneCam.up.y, sceneCam.up.z);

			// Get the Objects and Render them
			unsigned int numObjs = m_pScene->GetNumObjects ();
			for (unsigned int n = 0; n < numObjs; n++)
			{
				SceneObject *sceneObj = m_pScene->GetObject (n);

				// Store the current Transformation Matrix
				glPushMatrix ();

				// Is is a Sphere?
				if (sceneObj->IsSphere ())
				{
					// Set the Material
					SceneMaterial *sceneMat = m_pScene->GetMaterial (((SceneSphere *)sceneObj)->material);
					glColor3fv ((GLfloat *)&sceneMat->diffuse);
					glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat *)&sceneMat->specular);
					glMaterialf(GL_FRONT, GL_SHININESS, sceneMat->shininess);

					// Set Transformations
					Vector translate = ((SceneSphere *)sceneObj)->position + ((SceneSphere *)sceneObj)->center;
					glTranslatef (translate.x, translate.y, translate.z);
					glRotatef (((SceneSphere *)sceneObj)->rotation.x, 1.0f, 0.0f, 0.0f);
					glRotatef (((SceneSphere *)sceneObj)->rotation.y, 0.0f, 1.0f, 0.0f);
					glRotatef (((SceneSphere *)sceneObj)->rotation.z, 0.0f, 0.0f, 1.0f);
					glScalef (((SceneSphere *)sceneObj)->scale.x, ((SceneSphere *)sceneObj)->scale.y, ((SceneSphere *)sceneObj)->scale.z);

					// Draw the Sphere
					glutSolidSphere (((SceneSphere *)sceneObj)->radius, 20, 20);
				}
				else if (sceneObj->IsTriangle ())
				{
					// Set Transformations
					Vector translate = ((SceneTriangle *)sceneObj)->position;
					glTranslatef (translate.x, translate.y, translate.z);
					glRotatef (((SceneTriangle *)sceneObj)->rotation.x, 1.0f, 0.0f, 0.0f);
					glRotatef (((SceneTriangle *)sceneObj)->rotation.y, 0.0f, 1.0f, 0.0f);
					glRotatef (((SceneTriangle *)sceneObj)->rotation.z, 0.0f, 0.0f, 1.0f);
					glScalef (((SceneTriangle *)sceneObj)->scale.x, ((SceneTriangle *)sceneObj)->scale.y, ((SceneTriangle *)sceneObj)->scale.z);

					// Specular Properties set before glBegin ()
					SceneMaterial *sceneMat = m_pScene->GetMaterial (((SceneTriangle *)sceneObj)->material[0]);
					glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat *)&sceneMat->specular);
					glMaterialf(GL_FRONT, GL_SHININESS, sceneMat->shininess);

					// Draw the Triangle and set materials for each vertex as well
					glBegin (GL_POLYGON);

					sceneMat = m_pScene->GetMaterial (((SceneTriangle *)sceneObj)->material[0]);
					glColor3fv ((GLfloat *)&sceneMat->diffuse);
					glTexCoord2f (((SceneTriangle *)sceneObj)->u[0], ((SceneTriangle *)sceneObj)->v[0]);
					glNormal3f (((SceneTriangle *)sceneObj)->normal[0].x, ((SceneTriangle *)sceneObj)->normal[0].y, ((SceneTriangle *)sceneObj)->normal[0].z);
					glVertex3f (((SceneTriangle *)sceneObj)->vertex[0].x, ((SceneTriangle *)sceneObj)->vertex[0].y, ((SceneTriangle *)sceneObj)->vertex[0].z);

					sceneMat = m_pScene->GetMaterial (((SceneTriangle *)sceneObj)->material[1]);
					glColor3fv ((GLfloat *)&sceneMat->diffuse);
					glTexCoord2f (((SceneTriangle *)sceneObj)->u[1], ((SceneTriangle *)sceneObj)->v[1]);
					glNormal3f (((SceneTriangle *)sceneObj)->normal[1].x, ((SceneTriangle *)sceneObj)->normal[1].y, ((SceneTriangle *)sceneObj)->normal[1].z);
					glVertex3f (((SceneTriangle *)sceneObj)->vertex[1].x, ((SceneTriangle *)sceneObj)->vertex[1].y, ((SceneTriangle *)sceneObj)->vertex[1].z);

					sceneMat = m_pScene->GetMaterial (((SceneTriangle *)sceneObj)->material[2]);
					glColor3fv ((GLfloat *)&sceneMat->diffuse);
					glTexCoord2f (((SceneTriangle *)sceneObj)->u[2], ((SceneTriangle *)sceneObj)->v[2]);
					glNormal3f (((SceneTriangle *)sceneObj)->normal[2].x, ((SceneTriangle *)sceneObj)->normal[2].y, ((SceneTriangle *)sceneObj)->normal[2].z);
					glVertex3f (((SceneTriangle *)sceneObj)->vertex[2].x, ((SceneTriangle *)sceneObj)->vertex[2].y, ((SceneTriangle *)sceneObj)->vertex[2].z);

					glEnd ();
				}
				else if (sceneObj->IsModel ())
				{
					// Set Transformations
		//			Vector translate = ((SceneModel *)sceneObj)->position;
		//			glTranslatef (translate.x, translate.y, translate.z);
		//			glRotatef (((SceneModel *)sceneObj)->rotation.x, 1.0f, 0.0f, 0.0f);
		//			glRotatef (((SceneModel *)sceneObj)->rotation.y, 0.0f, 1.0f, 0.0f);
		//			glRotatef (((SceneModel *)sceneObj)->rotation.z, 0.0f, 0.0f, 1.0f);
		//			glScalef (((SceneModel *)sceneObj)->scale.x, ((SceneModel *)sceneObj)->scale.y, ((SceneModel *)sceneObj)->scale.z);
		
					// Set the Material (For a model, the material is uniform throughout all triangles
					SceneMaterial *sceneMat = m_pScene->GetMaterial (((SceneModel *)sceneObj)->GetTriangle (0)->material[0]);
					glColor3fv ((GLfloat *)&sceneMat->diffuse);
					glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat *)&sceneMat->specular);
					glMaterialf(GL_FRONT, GL_SHININESS, sceneMat->shininess);

					// Draw the Model
					glBegin (GL_TRIANGLES);

					unsigned int numTris = ((SceneModel *)sceneObj)->GetNumTriangles ();
					for (unsigned int n = 0; n < numTris; n++)
					{
						SceneTriangle *sceneTri = ((SceneModel *)sceneObj)->GetTriangle (n);

						glTexCoord2f (sceneTri->u[0], sceneTri->v[0]);
						glNormal3f (sceneTri->normal[0].x, sceneTri->normal[0].y, sceneTri->normal[0].z);
						glVertex3f (sceneTri->vertex[0].x, sceneTri->vertex[0].y, sceneTri->vertex[0].z);

						glTexCoord2f (sceneTri->u[1], sceneTri->v[1]);
						glNormal3f (sceneTri->normal[1].x, sceneTri->normal[1].y, sceneTri->normal[1].z);
						glVertex3f (sceneTri->vertex[1].x, sceneTri->vertex[1].y, sceneTri->vertex[1].z);

						glTexCoord2f (sceneTri->u[2], sceneTri->v[2]);
						glNormal3f (sceneTri->normal[2].x, sceneTri->normal[2].y, sceneTri->normal[2].z);
						glVertex3f (sceneTri->vertex[2].x, sceneTri->vertex[2].y, sceneTri->vertex[2].z);
					}

					glEnd ();
				}
				else
				{
					printf ("Unrecognized Object...\n");
				}

				// Restore the Transformation Matrix
				glPopMatrix ();
			}

			// Restore Render States
			UnsetRenderStates ();
		}
	}
};

#endif // NORMALRENDERER_H
