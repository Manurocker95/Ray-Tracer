/***************************************************************************/
/*                                                                         */
/* File:     3ds.h                                                         */
/* Author:   bkenwright@xbdev.net                                          */
/* Web:      www.xbdev.net                                                 */
/*                                                                         */
/* Version:  1.0.2                                                         */
/* Date:     9 June 2002                                                   */
/*                                                                         */
/***************************************************************************/
/* 
   What we have here is a totally self contained 3DS file loader, that is 
   easy to use and extremely powerful.

   Using it?
   <1> #include "3ds.h"
   <2> C3DS myObj;
   <3> myObj.Create("cube.3ds");

   <4> Access the 3D data through the m_Object member..vertices etc
       myObj.m_Object.m_iNumMaterials;
	   myObj.m_Object.m_iNumMeshs;
	   .. etc

   <5> myObj.Release(); // Tidy up when you've finished.

*/
/***************************************************************************/

#include <stdio.h>

#include <vector>
using namespace std ;



//>----- Entry point (Primary Chunk at the start of the file ----------------
#define		PRIMARY			0x4D4D

//>----- Main Chunks --------------------------------------------------------
#define		EDIT3DS			0x3D3D  // Start of our actual objects
#define		KEYF3DS			0xB000  // Start of the keyframe information

//>----- General Chunks -----------------------------------------------------
#define		VERSION			0x0002
#define		MESH_VERSION	0x3D3E
#define		KFVERSION		0x0005
#define		COLOR_F			0x0010
#define		COLOR_24		0x0011
#define		LIN_COLOR_24	0x0012
#define		LIN_COLOR_F		0x0013
#define		INT_PERCENTAGE	0x0030
#define		FLOAT_PERC		0x0031
#define		MASTER_SCALE	0x0100
#define		IMAGE_FILE		0x1100
#define		AMBIENT_LIGHT	0X2100

//>----- Object Chunks -----------------------------------------------------
#define		NAMED_OBJECT	0x4000
#define		OBJ_MESH		0x4100
#define		MESH_VERTICES	0x4110
#define		VERTEX_FLAGS	0x4111
#define		MESH_FACES		0x4120
#define		MESH_MATER		0x4130
#define		MESH_TEX_VERT	0x4140
#define		MESH_XFMATRIX	0x4160
#define		MESH_COLOR_IND	0x4165
#define		MESH_TEX_INFO	0x4170
#define		HEIRARCHY		0x4F00


//>----- Material Chunks ---------------------------------------------------
#define		MATERIAL		0xAFFF
#define		MAT_NAME		0xA000
#define		MAT_AMBIENT		0xA010
#define		MAT_DIFFUSE		0xA020
#define		MAT_SPECULAR	0xA030
#define		MAT_SHININESS	0xA040
#define		MAT_FALLOFF		0xA052
#define		MAT_EMISSIVE	0xA080
#define		MAT_SHADER		0xA100
#define		MAT_TEXMAP		0xA200
#define		MAT_TEXFLNM		0xA300

#define		OBJ_LIGHT		0x4600
#define		OBJ_CAMERA		0x4700

//>----- KeyFrames Chunks --------------------------------------------------
#define		ANIM_HEADER		0xB00A
#define		ANIM_OBJ		0xB002
#define     ANIM_S_E_TIME   0xB008

#define		ANIM_NAME		0xB010
#define     ANIM_PIVOT      0xB013
#define		ANIM_POS		0xB020
#define		ANIM_ROT		0xB021
#define		ANIM_SCALE		0xB022

class stPosition
{
public:
	int   iFrame;
	float x, y, z;
};
class stScale
{
public:
	int   iFrame;
	float x, y, z;
};
class stRotation
{
public:
	int   iFrame;
	float rotation_rads;
	float axis_x, axis_y, axis_z;
};

class stAnimation
{
public:
	char          szObjName[256];
	int           iPosKeys;
	stPosition*   pPosition;   // stPosition
	int           iRotKeys;
	stRotation*   pRotation;   // stRotation
	int           iScaleKeys;
	stScale*      pScale;      // stScale
	
	stAnimation()
	{
		iPosKeys = 0;
		pPosition = NULL;
		iRotKeys = 0;
		pRotation = NULL;
		iScaleKeys = 0;
		pScale = NULL;
		szObjName[0]='\0';
	}
};





class C3DS
{
	class Color
	{
	public:
		unsigned short r,g,b;
		Color (void) : r(0), g(0), b(0)
		{}
	};
public:

	// MATERIAL INFORMATION
	class stMaterial
	{
	public:
		char szName[256];
		Color Colour;
		Color Ambient;
		Color Specular;
		bool bTexFile;
		char szTextureFile[256];
		stMaterial()
		{
			bTexFile = false;
			szName[0]='\0';
			szTextureFile[0]='\0';
		};
	};

	// MESH INFORMATION
	class stVert
	{
	public:
		float x, y, z;
	};

	class stFace
	{
	public:
	   // 3 Sides of a triangle make a face.
		unsigned int corner[3];
		int MaterialID;
		stFace()
		{
			MaterialID = 0;
		}
	};
	class stTex
	{
	public:
		float tu, tv;
	};

	class stMesh
	{
	public:
		char         szMeshName[256];
		int          iNumVerts;
		stVert*      pVerts;
		int          iNumFaces;
		stFace*      pFaces;
		bool         bTextCoords;
		stTex*       pTexs;

		stMesh()
		{
			iNumVerts   = 0;
			pVerts      = NULL;
			iNumFaces   = 0;
			pFaces      = NULL;
			pTexs       = NULL;
			bTextCoords = false;
			szMeshName[0]='\0';
		}
		
	};


protected:
	class stChunk
	{
	public:
		unsigned short ID;
		unsigned int length;
		unsigned int bytesRead;
	};

public:
	C3DS(void);
	~C3DS(void);

	bool Create(char* szFileName);
	void Release();
public:

	int                m_iNumMeshs;
	vector<stMesh>     m_pMeshs;

	int                m_iNumMaterials;
	vector<stMaterial> m_pMaterials;

	// Animation stuff is saved in these two parts.
	int                   m_iNumAnimObjects;
	vector<stAnimation>   m_pAnimation;
	int                   m_iKeyFrames;

protected:

	FILE* m_fp;

	void ParseChunk        (stChunk* Chunk);
	void GetMaterialName   (stChunk* Chunk);
	void GetDiffuseColour  (stChunk* Chunk);
	void GetAmbientColour  (stChunk* Chunk);
	void GetSpecularColour  (stChunk* Chunk);
	void GetTexFileName    (stChunk* Chunk);
	void GetMeshObjectName (stChunk* Chunk);
	void ReadMeshTexCoords (stChunk* Chunk);
	void ReadMeshVertices  (stChunk* Chunk);
	void ReadMeshFaces     (stChunk* Chunk);
	void ReadMeshMaterials (stChunk* Chunk);
	int GetString          (char* pBuffer);
	void SkipChunk         (stChunk *pChunk);
	void ReadChunk         (stChunk *pChunk);


	// Animation
	void StartEndFrames(stChunk* Chunk);
	void ReadNameOfObjectToAnimate(stChunk* Chunk);
	void ReadPivotPoint(stChunk* Chunk);
	void ReadAnimPos(stChunk* Chunk);
	void ReadAnimRot(stChunk* Chunk);
	void ReadAnimScale(stChunk* Chunk);

};




// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// Debugging Function
void debug_op(char *s);
void DisplayRawData(C3DS* pObj);
void DisplayRawAnimationData(C3DS* pObj);




