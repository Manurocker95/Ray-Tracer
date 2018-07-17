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

#include "Scene.h"


bool Scene::Load (char *filename)
{
	XMLNode tempNode;

	// Open the Scene XML File
	printf ("Loading Scenefile %s...\n", filename);
	XMLNode sceneXML = XMLNode::openFileHelper(filename, "scene");
	if (sceneXML.isEmpty ())
		return false;
	m_Desc = CHECK_ATTR(sceneXML.getAttribute("desc"));
	m_Author = CHECK_ATTR(sceneXML.getAttribute("author"));

	printf ("Description: \n\t%s\n", m_Desc.c_str ());
	printf ("Author: %s\n", m_Author.c_str ());

	// Load the Background
	printf ("Loading Background...\n");
	tempNode = sceneXML.getChildNode("background");
	if (tempNode.isEmpty ())
		return false;
	m_Background.color = ParseColor (tempNode.getChildNode("color"));
	m_Background.ambientLight = ParseColor (tempNode.getChildNode("ambientLight"));

	// Load the Lights
	printf ("Loading Lights...\n");
	tempNode = sceneXML.getChildNode("light_list");
	if (!tempNode.isEmpty ())
	{
		unsigned int numLights = tempNode.nChildNode ("light");
		for (unsigned int n = 0; n < numLights; n++)
		{
			XMLNode tempLightNode = tempNode.getChildNode("light", n);
			if (tempLightNode.isEmpty ())
				return false;
			SceneLight *tempLight = new SceneLight();
			tempLight->color = ParseColor (tempLightNode.getChildNode("color"));
			tempLight->attenuationConstant = atof(CHECK_ATTR(tempLightNode.getChildNode("attenuation").getAttribute ("constant")));
			tempLight->attenuationLinear = atof(CHECK_ATTR(tempLightNode.getChildNode("attenuation").getAttribute ("linear")));
			tempLight->attenuationQuadratic = atof(CHECK_ATTR(tempLightNode.getChildNode("attenuation").getAttribute ("quadratic")));
			tempLight->position = ParseXYZ (tempLightNode.getChildNode("position"));
			m_LightList.push_back (tempLight);
		}
	}

	// Load the Materials
	printf ("Loading Materials...\n");
	tempNode = sceneXML.getChildNode("material_list");
	if (!tempNode.isEmpty ())
	{
		unsigned int numMaterials = tempNode.nChildNode ("material");
		for (unsigned int n = 0; n < numMaterials; n++)
		{
			XMLNode tempMaterialNode = tempNode.getChildNode("material", n);
			if (tempMaterialNode.isEmpty ())
				return false;
			SceneMaterial *tempMaterial = new SceneMaterial();
			tempMaterial->name = CHECK_ATTR(tempMaterialNode.getAttribute("name"));
			tempMaterial->texture = CHECK_ATTR(tempMaterialNode.getChildNode("texture").getAttribute("filename"));
			tempMaterial->diffuse = ParseColor (tempMaterialNode.getChildNode("diffuse"));
			tempMaterial->specular = ParseColor (tempMaterialNode.getChildNode("specular"));
			tempMaterial->shininess = atof(CHECK_ATTR(tempMaterialNode.getChildNode("specular").getAttribute("shininess")));
			tempMaterial->transparent = ParseColor (tempMaterialNode.getChildNode("transparent"));
			tempMaterial->reflective = ParseColor (tempMaterialNode.getChildNode("reflective"));
			tempMaterial->refraction_index = ParseColor (tempMaterialNode.getChildNode("refraction_index"));
			tempMaterial->alpha = atof(CHECK_ATTR(tempMaterialNode.getChildNode("alpha").getAttribute("value")));
			tempMaterial->Fo = atof(CHECK_ATTR(tempMaterialNode.getChildNode("fo").getAttribute("value")));
			tempMaterial->reflectionBias = atof(CHECK_ATTR(tempMaterialNode.getChildNode("bias").getAttribute("reflection")));
			tempMaterial->refractionBias = atof(CHECK_ATTR(tempMaterialNode.getChildNode("bias").getAttribute("refraction")));

			if (tempMaterial->texture != "")
			{
				if (!tempMaterial->LoadTexture ())
					return false;
			}

			m_MaterialList.push_back (tempMaterial);
			m_MaterialMap.insert(std::pair<std::string, SceneMaterial*>(tempMaterial->name, tempMaterial));
		}
	}

	// Load the Objects
	printf ("Loading Objects...\n");
	tempNode = sceneXML.getChildNode("object_list");
	if (!tempNode.isEmpty ())
	{
		unsigned int numObjects = tempNode.nChildNode ();
		for (unsigned int n = 0; n < numObjects; n++)
		{
			XMLNode tempObjectNode = tempNode.getChildNode(n);
			if (tempObjectNode.isEmpty ())
				return false;
			if (!strcasecmp(tempObjectNode.getName (), "sphere"))
			{
				// Load a Sphere
				SceneSphere *tempSphere = new SceneSphere ();
				tempSphere->name = CHECK_ATTR(tempObjectNode.getAttribute("name"));
				tempSphere->material = CHECK_ATTR(tempObjectNode.getAttribute("material"));
				tempSphere->radius = atof(CHECK_ATTR(tempObjectNode.getAttribute("radius")));
				tempSphere->scale = ParseXYZ (tempObjectNode.getChildNode("scale"));
				tempSphere->rotation = ParseXYZ (tempObjectNode.getChildNode("rotation"));
				tempSphere->position = ParseXYZ (tempObjectNode.getChildNode("position"));
				tempSphere->center = ParseXYZ (tempObjectNode.getChildNode("center"));
				m_ObjectList.push_back (tempSphere);
			}
			else if (!strcasecmp(tempObjectNode.getName (), "triangle"))
			{
				// Load a Triangle
				XMLNode vertexNode;
				SceneTriangle *tempTriangle = new SceneTriangle ();
				tempTriangle->name = CHECK_ATTR(tempObjectNode.getAttribute("name"));
				tempTriangle->scale = ParseXYZ (tempObjectNode.getChildNode("scale"));
				tempTriangle->rotation = ParseXYZ (tempObjectNode.getChildNode("rotation"));
				tempTriangle->position = ParseXYZ (tempObjectNode.getChildNode("position"));
				
				// Load Vertex 0
				vertexNode = tempObjectNode.getChildNodeWithAttribute ("vertex", "index", "0");
				tempTriangle->material[0] = CHECK_ATTR(vertexNode.getAttribute("material"));
				tempTriangle->vertex[0] = ParseXYZ (vertexNode.getChildNode("position"));
				tempTriangle->normal[0] = ParseXYZ (vertexNode.getChildNode("normal"));
				tempTriangle->u[0] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("u")));
				tempTriangle->v[0] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("v")));

				// Load Vertex 1
				vertexNode = tempObjectNode.getChildNodeWithAttribute ("vertex", "index", "1");
				tempTriangle->material[1] = CHECK_ATTR(vertexNode.getAttribute("material"));
				tempTriangle->vertex[1] = ParseXYZ (vertexNode.getChildNode("position"));
				tempTriangle->normal[1] = ParseXYZ (vertexNode.getChildNode("normal"));
				tempTriangle->u[1] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("u")));
				tempTriangle->v[1] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("v")));

				// Load Vertex 2
				vertexNode = tempObjectNode.getChildNodeWithAttribute ("vertex", "index", "2");
				tempTriangle->material[2] = CHECK_ATTR(vertexNode.getAttribute("material"));
				tempTriangle->vertex[2] = ParseXYZ (vertexNode.getChildNode("position"));
				tempTriangle->normal[2] = ParseXYZ (vertexNode.getChildNode("normal"));
				tempTriangle->u[2] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("u")));
				tempTriangle->v[2] = atof (CHECK_ATTR(vertexNode.getChildNode("texture").getAttribute("v")));

				m_ObjectList.push_back (tempTriangle);
			}
			else if (!strcasecmp(tempObjectNode.getName (), "model"))
			{
				// Load a Model
				SceneModel *tempModel = new SceneModel ();
				tempModel->filename = CHECK_ATTR(tempObjectNode.getAttribute("filename"));
				if (tempModel->filename == "")
				{
					printf ("No Specified Model filename\n");
					return false;
				}
				tempModel->name = CHECK_ATTR(tempObjectNode.getAttribute("name"));
				std::string material = CHECK_ATTR(tempObjectNode.getAttribute("material"));
				//tempModel->material = tempObjectNode.getAttribute("material");
				tempModel->scale = ParseXYZ (tempObjectNode.getChildNode("scale"));
				tempModel->rotation = ParseXYZ (tempObjectNode.getChildNode("rotation"));
				tempModel->position = ParseXYZ (tempObjectNode.getChildNode("position"));
				

				//TRANSFORM Matrixes similar to Opengl and applied always
				//FIRST rotate, order matters: rotationMatrixes: https://en.wikipedia.org/wiki/Rotation_matrix
				float angleX = tempModel->rotation.x;
				Matrix rotX;
				rotX._11 = rotX._44 = 1.0f;
				rotX._32 = sin(angleX);
				rotX._22 = cos(angleX);
				rotX._23 = -rotX._32;
				rotX._33 = rotX._22;
				//Rotation in Y
				float angleY = tempModel->rotation.y;
				Matrix rotY;
				rotY._22 = rotY._44 = 1.0f;
				rotY._13 = sin(angleY);
				rotY._11 = cos(angleY);
				rotY._31 = -rotY._13;
				rotY._33 = rotY._11;
				//Rotation in Z
				float angleZ = tempModel->rotation.z;
				Matrix rotZ;
				rotZ._33 = rotZ._44 = 1.0f;
				rotZ._21 = sin(angleZ);
				rotZ._11 = cos(angleZ);
				rotZ._12 = -rotZ._21;
				rotZ._22 = rotZ._11;
				//Compute TRS matrix
				Matrix translation,rotTotal,scale, TRS;
				translation = translation.Identity();
				translation._14= tempModel->position.x;
				translation._24 = tempModel->position.y;
				translation._34 = tempModel->position.z;
				translation._44 = 1.0;
				rotTotal = rotX*rotY*rotZ;
				scale._11 = tempModel->scale.x;
				scale._22 = tempModel->scale.y;
				scale._33 = tempModel->scale.z;
				scale._44 = 1.0;
				//Compute TRS matrix
				TRS = translation*rotTotal*scale;

				// Check the file format
				if (tempModel->filename.substr (tempModel->filename.length() - 4, 4) == ".3ds")
				{
					// Load the list of triangles from the .3ds
					C3DS sceneObj;
					if (!sceneObj.Create((char *)tempModel->filename.c_str()))
					{
						printf ("Error loading .3ds file\n");
						return false;
					}

					for (unsigned int obj = 0; obj < (unsigned int)sceneObj.m_iNumMeshs; obj++)
					{
						for (unsigned int n = 0; n < (unsigned int)sceneObj.m_pMeshs[obj].iNumFaces; n++)
						{
							SceneTriangle tempTriangle;

							Vector v1, v2, v3;
							// Vert 1
							v1.x = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[0]].x;
							v1.y = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[0]].y;
							v1.z = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[0]].z;
							// Vert 2
							v2.x = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[1]].x;
							v2.y = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[1]].y;
							v2.z = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[1]].z;
							// Vert 3
							v3.x = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[2]].x;
							v3.y = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[2]].y;
							v3.z = sceneObj.m_pMeshs[obj].pVerts[sceneObj.m_pMeshs[obj].pFaces[n].corner[2]].z;

							//MICAHS Transforms: rotate, then translate, then scale
							//v1 = ((rotTotal*v1) + tempModel->position)*tempModel->scale;
							//v2 = ((rotTotal*v2) + tempModel->position)*tempModel->scale;
							//v3 = ((rotTotal*v3) + tempModel->position)*tempModel->scale;
							v1 = TRS*v1;
							v2 = TRS*v2;
							v3 = TRS*v3;


							// Normal
							Vector normal = (v1 - v2).Cross(v3 - v2).Normalize();

							// Load Vertex 0
							tempTriangle.material[0] = material;
							tempTriangle.vertex[0] = v1;
							tempTriangle.normal[0] = normal;
							// Texture Coords
							if (sceneObj.m_pMeshs[obj].bTextCoords)
							{
								tempTriangle.u[0] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[0]].tu;
								tempTriangle.v[0] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[0]].tv;
							}
							else
							{
								tempTriangle.u[0] = 0.0f;
								tempTriangle.v[0] = 0.0f;
							}

							// Load Vertex 1
							tempTriangle.material[1] = material;
							tempTriangle.vertex[1] = v2;
							tempTriangle.normal[1] = normal;
							// Texture Coords
							if (sceneObj.m_pMeshs[obj].bTextCoords)
							{
								tempTriangle.u[1] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[1]].tu;
								tempTriangle.v[1] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[1]].tv;
							}
							else
							{
								tempTriangle.u[1] = 0.0f;
								tempTriangle.v[1] = 0.0f;
							}

							// Load Vertex 2
							tempTriangle.material[2] = material;
							tempTriangle.vertex[2] = v3;
							tempTriangle.normal[2] = normal;
							// Texture Coords
							if (sceneObj.m_pMeshs[obj].bTextCoords)
							{
								tempTriangle.u[2] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[2]].tu;
								tempTriangle.v[2] = sceneObj.m_pMeshs[obj].pTexs[sceneObj.m_pMeshs[obj].pFaces[n].corner[2]].tv;
							}
							else
							{
								tempTriangle.u[2] = 0.0f;
								tempTriangle.v[2] = 0.0f;
							}

							tempModel->triangleList.push_back (tempTriangle);
						}
					}

					sceneObj.Release();
				}
				else if (tempModel->filename.substr (tempModel->filename.length() - 4, 4) == ".obj")
				{
					// The following code is a modified version of code from the old RayTracer Code rt_trimesh.cpp
					char line[MAX_LINE_LEN];
					char command[MAX_LINE_LEN];
					int position;
					vector<Vector> vertices;
					vector<Vector> normals;

					std::ifstream infile (tempModel->filename.c_str());

					if (infile.fail() )
					{
						printf ("Error loading .obj file\n");
						return false;
					}

					while (infile.good ())
					{
						infile.getline (line, MAX_LINE_LEN);
						ParseOBJCommand (line, MAX_LINE_LEN, command, position);

						if (strcmp (command,"v")==0)
						{
							Vector pos = ParseOBJVector (&(line[position]));
							vertices.push_back (pos);
						}
						else if (strcmp (command,"vn")==0)
						{
							Vector norm = ParseOBJVector (&(line[position]));
							normals.push_back (norm);
						}
						else if (strcmp (command,"f")==0)
						{
							int num = 0; // number of edges
							int v_index[3]; // vertex index
							int n_index[3]; // normal index

							if (!ParseOBJCoords (&(line[position]), num, v_index, n_index))
							{
								printf ("Error parsing faces in .obj file\n");
								return false;
							}

							SceneTriangle tempTriangle;

							// Load Vertex 0
							tempTriangle.material[0] = material;
							tempTriangle.vertex[0] = vertices[v_index[0]];
							tempTriangle.normal[0] = normals[n_index[0]];
							// Texture Coords
							tempTriangle.u[0] = 0.0f;
							tempTriangle.v[0] = 0.0f;

							// Load Vertex 1
							tempTriangle.material[1] = material;
							tempTriangle.vertex[1] = vertices[v_index[1]];
							tempTriangle.normal[1] = normals[n_index[1]];
							// Texture Coords
							tempTriangle.u[1] = 0.0f;
							tempTriangle.v[1] = 0.0f;

							// Load Vertex 2
							tempTriangle.material[2] = material;
							tempTriangle.vertex[2] = vertices[v_index[2]];
							tempTriangle.normal[2] = normals[n_index[2]];
							// Texture Coords
							tempTriangle.u[2] = 0.0f;
							tempTriangle.v[2] = 0.0f;

							

							//MICAHS Transformations, Rotation first around origin, then translate and scale 
						/*	//Rotation
							tempTriangle.vertex[0] = rotTotal*tempTriangle.vertex[0];
							tempTriangle.vertex[1] = rotTotal*tempTriangle.vertex[1];
							tempTriangle.vertex[2] = rotTotal*tempTriangle.vertex[2];
							//normals too			 
							tempTriangle.normal[0] = (rotTotal*tempTriangle.normal[0]).Normalize();
							tempTriangle.normal[1] = (rotTotal*tempTriangle.normal[1]).Normalize();
							tempTriangle.normal[2] = (rotTotal*tempTriangle.normal[2]).Normalize();
							//Second, Translate
							tempTriangle.vertex[0] = tempTriangle.vertex[0] + tempModel->position;
							tempTriangle.vertex[1] = tempTriangle.vertex[1] + tempModel->position;
							tempTriangle.vertex[2] = tempTriangle.vertex[2] + tempModel->position;
							//then ,Scale:
							tempTriangle.vertex[0] = tempTriangle.vertex[0] * tempModel->scale;
							tempTriangle.vertex[1] = tempTriangle.vertex[1] * tempModel->scale;
							tempTriangle.vertex[2] = tempTriangle.vertex[2] * tempModel->scale;
						*/	
							//USING TRS
							tempTriangle.vertex[0] = TRS*tempTriangle.vertex[0];
							tempTriangle.vertex[1] = TRS*tempTriangle.vertex[1];
							tempTriangle.vertex[2] = TRS*tempTriangle.vertex[2];
							//normals too			 
							tempTriangle.normal[0] = (rotTotal*tempTriangle.normal[0]).Normalize();
							tempTriangle.normal[1] = (rotTotal*tempTriangle.normal[1]).Normalize();
							tempTriangle.normal[2] = (rotTotal*tempTriangle.normal[2]).Normalize();
						
							
							tempModel->triangleList.push_back (tempTriangle);
						}
						else
						{
							//printf ("Ignoring command <%s> in obj file\n", command);
						}
					}
					infile.close ();
				}
				else
				{
					printf ("Unsupported file format\n");
					return false;
				}

				m_ObjectList.push_back (tempModel);
			}
			else
			{
				printf ("Unrecognized Node <%s> in <object_list>\n", tempObjectNode.getName ());
				exit (255);
			}
		}
	}

	// Load the Camera
	printf ("Loading Camera...\n");
	tempNode = sceneXML.getChildNode("camera");
	if (tempNode.isEmpty ())
		return false;
	m_Camera.SetFOV (atof(CHECK_ATTR2(tempNode.getAttribute("fieldOfView"), "45.0")));
	m_Camera.SetNearClip (atof(CHECK_ATTR2(tempNode.getAttribute("nearClip"), "0.1")));
	m_Camera.SetFarClip (atof(CHECK_ATTR2(tempNode.getAttribute("farClip"), "100.0")));
	m_Camera.SetPosition (ParseXYZ (tempNode.getChildNode("position")));
	m_Camera.SetTarget (ParseXYZ (tempNode.getChildNode("target")));
	m_Camera.SetUp (ParseXYZ (tempNode.getChildNode("up")));

	printf ("Scene Loaded!\n");

	return true;
}

void Scene::ParseOBJCommand (char *line, int max, char *command, int &position)
{
	int i = 0;
	int j = 0;

	while (i<max && line[i]==' ') i++;
	while (i<max && line[i]!='\0' && line[i]!=' ')
	{
		command[j] = line[i];
		j++; i++;
	}

	command[j] = '\0';
	position = i;
}

Vector Scene::ParseOBJVector (char *str)
{
	int i = 0;
	float x,y,z;
	while (str[i]!='\0' && str[i]==' ') i++;
	x = atof (&(str[i]));
	while (str[i]!='\0' && str[i]!=' ') i++;
	y = atof (&(str[i]));
	while (str[i]!='\0' && str[i]==' ') i++;
	while (str[i]!='\0' && str[i]!=' ') i++;
	z = atof (&(str[i]));
	return Vector (x,y,z);
}

bool Scene::ParseOBJCoords (char *str, int &num, int v_index[3], int n_index[3])
{
	int i = 0;
	num = 0;

	while (str[i]!='\0' && str[i]==' ') i++;
	while (str[i]!='\0')
	{
		while (str[i]!='\0' && str[i]!=' ') i++;
		while (str[i]!='\0' && str[i]==' ') i++;
		num++;
	}
	if (num != 3)
		return false;

	i = 0;
	for (int j=0; j<num; j++)
	{
		while (str[i]==' ') i++;
		v_index[j] = atoi (&(str[i])) - 1;
		while (str[i]!='/' && str[i]!=' ') i++;
		if (str[i] != '/')
			return false;
		i++;
		while (str[i]!='/' && str[i]!=' ') i++;
		if (str[i] != '/')
			return false;
		i++;
		n_index[j] = atoi (&(str[i])) - 1;
		while(str[i]!='\0' && str[i]!=' ') i++;
	}

	return true;
}

