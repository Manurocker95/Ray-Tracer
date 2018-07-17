/***************************************************************************/
/*                                                                         */
/* File:     3ds.cpp                                                       */
/* Author:   bkenwright@xbdev.net                                          */
/* Web:      www.xbdev.net                                                 */
/*                                                                         */
/* Version:  1.0.2                                                         */
/* Date:     9 June 2002                                                   */
/*                                                                         */
/***************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include "3ds.h"

C3DS::C3DS(void)
{
    m_iNumMeshs       = 0;

	m_iNumMaterials   = 0;

	m_iNumAnimObjects = 0;
	m_iKeyFrames      = 0;
}

C3DS::~C3DS(void){}


/***************************************************************************/
/*                                                                         */
/* Some user functions to make the reading of the .3ds file easier         */
/*                                                                         */
/***************************************************************************/
/* Helper Functions that make our parsing of the chunks easier             */
/***************************************************************************/


void C3DS::ReadChunk(stChunk *pChunk)
{
	unsigned short ID   		= 0;
	unsigned int bytesRead		= 0;
	unsigned int bChunkLength	= 0;

	bytesRead = (unsigned int)fread(&ID, 1, 2, m_fp);

	bytesRead += (unsigned int)fread(&bChunkLength, 1, 4, m_fp);

	pChunk->ID  	  = ID;
	pChunk->length	  = bChunkLength;
	pChunk->bytesRead = bytesRead;

}

void C3DS::SkipChunk(stChunk *pChunk)
{
	char* buffer = new char[pChunk->length];

	fread(buffer, 1, pChunk->length - pChunk->bytesRead, m_fp);

	delete[] buffer;
}

/***************************************************************************/
/*                                                                         */
/* Helper Fuction, simply reads in the string from the file pointer, until */
/* a null is reached, then returns how many bytes was read.                */
/*                                                                         */
/***************************************************************************/
int C3DS::GetString(char* pBuffer)
{
	int index = 0;

	char buffer[100] = {0};

	fread(buffer, 1, 1, m_fp);

	while( *(buffer + index++) != 0)
	{
		fread(buffer + index, 1, 1, m_fp);
	}

	strcpy( pBuffer, buffer );

	return (int)(strlen(buffer) + 1);
}

/***************************************************************************/
/*                                                                         */
/* This little function reads the matrial data for our individual object,  */
/* So it determines which face references which material, in our material  */
/* list.                                                                   */
/*                                                                         */
/***************************************************************************/
void C3DS::ReadMeshMaterials(stChunk* Chunk)
{
	// Material Name Where Referencing
	char str[256];
	unsigned int characterlen = GetString(str);
	Chunk->bytesRead += characterlen;

	unsigned short iNumFaces = 0;
	Chunk->bytesRead += (unsigned int)fread(&iNumFaces, 1, 2, m_fp);

	unsigned short *FaceAssignedThisMaterial = new unsigned short[iNumFaces];
	Chunk->bytesRead += (unsigned int)fread(FaceAssignedThisMaterial, 1, 
									iNumFaces*sizeof(unsigned short), m_fp);

	// Determine Which Material It Is In Our List
	int MaterialID = 0;
	for( int cc=0; cc<m_iNumMaterials; cc++)
	{
		if( strcmp( str, m_pMaterials[cc].szName ) == 0 )
			MaterialID = cc;
	}

	stMesh* pMesh = &(m_pMeshs[m_iNumMeshs - 1]);
	for(int i=0; i<iNumFaces; i++)
	{
		int iIndex = FaceAssignedThisMaterial[i];
		pMesh->pFaces[iIndex].MaterialID = MaterialID;
	}

	return;
}

/***************************************************************************/
/*                                                                         */
/* We get all the faces...e.g. Triangle Index's into our vertex array, so  */
/* we can actually put our shape together.                                 */
/*                                                                         */
/***************************************************************************/
void C3DS::ReadMeshFaces(stChunk* Chunk)
{
	unsigned int iNumberFaces = 0; //= Chunk->length - 6;
	Chunk->bytesRead += (unsigned int)fread(&iNumberFaces, 1, 2, m_fp);	

	// Each face is 3 points A TRIANGLE!..WOW
	struct stXFace{ unsigned short p1, p2, p3, visibityflag; };
	stXFace *pFaces = new stXFace[iNumberFaces];

	Chunk->bytesRead += (unsigned int)fread(pFaces, 1, iNumberFaces*sizeof(stXFace), m_fp);

	stMesh* pMesh = &(m_pMeshs[m_iNumMeshs - 1]);
	pMesh->pFaces = new stFace[iNumberFaces];
	pMesh->iNumFaces = iNumberFaces;

	for( unsigned int i=0; i<iNumberFaces; i++ )
	{
		pMesh->pFaces[i].corner[0] = pFaces[i].p1;
		pMesh->pFaces[i].corner[1] = pFaces[i].p2;
		pMesh->pFaces[i].corner[2] = pFaces[i].p3;
	}

	delete[] pFaces;
	

	// Our face material information is a sub-chunk.
	ParseChunk(Chunk);
}

/***************************************************************************/
/*                                                                         */
/* You know all those x,y,z things...yup I mean vertices...well this reads */
/* them all in.                                                            */
/*                                                                         */
/***************************************************************************/
void C3DS::ReadMeshVertices(stChunk* Chunk)
{
	unsigned int iNumberVertices = 0;
	Chunk->bytesRead += (unsigned int)fread(&iNumberVertices, 1, 2, m_fp);	

	// Allocate Memory and dump our vertices to the screen.
	stVert *pVerts = new stVert[iNumberVertices];

	Chunk->bytesRead += (unsigned int)fread( (void*)pVerts, 1, iNumberVertices*sizeof(stVert), m_fp);

	stMesh* pMesh = &(m_pMeshs[m_iNumMeshs - 1]);
	pMesh->pVerts = pVerts;
	pMesh->iNumVerts = iNumberVertices;
	
	// Crazy but true, the z and y need to be swapped around!
	for(int i=0; i< pMesh->iNumVerts; i++)
	{
		pMesh->pVerts[i].x = pMesh->pVerts[i].x;
		float tempz = pMesh->pVerts[i].y;
		pMesh->pVerts[i].y = pMesh->pVerts[i].z;
		pMesh->pVerts[i].z = tempz;
	}

	SkipChunk(Chunk);
}

/***************************************************************************/
/*                                                                         */
/* Well if we have a texture, e.g. coolimage.bmp, then we need to load in  */
/* our texture coordinates...tu and tv.                                    */
/*                                                                         */
/***************************************************************************/
void C3DS::ReadMeshTexCoords(stChunk* Chunk)
{
	unsigned short iNumberVertices = 0;
	Chunk->bytesRead += (unsigned int)fread(&iNumberVertices, 1, 2, m_fp);	

	// Allocate Memory and dump our texture for each vertice to the screen.
	stTex *pTex = new stTex[iNumberVertices];

	Chunk->bytesRead += (unsigned int)fread( (void*)pTex, 1, iNumberVertices*sizeof(stTex), m_fp);

	stMesh* pMesh = &(m_pMeshs[m_iNumMeshs - 1]);
	pMesh->pTexs = pTex;

	for(int i=0; i<iNumberVertices; i++)
	{
		pMesh->pTexs[i].tu = pMesh->pTexs[i].tu;
		pMesh->pTexs[i].tv = 1 - pMesh->pTexs[i].tv;
	}
	
	pMesh->bTextCoords = true;
	
	SkipChunk(Chunk);
}


/***************************************************************************/
/*                                                                         */
/* Read in our objects name...as each object in our 3D world has a name,   */
/* for example Box1, HillMesh...whatever you called your object or object's*/
/* in 3d max before you saved it.                                          */
/*                                                                         */
/***************************************************************************/
void C3DS::GetMeshObjectName(stChunk* Chunk)
{
	// The strange thing is, the next few parts of this chunk represent 
	// the name of the object.  Then we start chunking again.
	char str[256];
	unsigned int characterlen = GetString(str);
	Chunk->bytesRead += characterlen;

	stMesh* pMesh = &(m_pMeshs[m_iNumMeshs - 1]);
	strcpy( pMesh->szMeshName, str );

	ParseChunk(Chunk);
}

// Read in our texture's file name (e.g. coolpic.jpg)
void C3DS::GetTexFileName(stChunk* Chunk)
{
	char str[256];
	Chunk->bytesRead += GetString(str);

	stMaterial* pMat = &(m_pMaterials[m_iNumMaterials-1]);
	strcpy( pMat->szTextureFile, str );

	pMat->bTexFile = true;
}

// Read in our diffuse colour (rgb)
void C3DS::GetDiffuseColour(stChunk* Chunk)
{
	struct stRGB{ unsigned char r, g, b; };
	stRGB DiffColour;

	char ChunkHeader[6];
	Chunk->bytesRead += (unsigned int)fread(ChunkHeader, 1, 6, m_fp);

	Chunk->bytesRead += (unsigned int)fread(&DiffColour, 1, 3, m_fp);	

	stMaterial* pM = &(m_pMaterials[m_iNumMaterials-1]);
	pM->Colour.r = DiffColour.r;
	pM->Colour.g = DiffColour.g;
	pM->Colour.b = DiffColour.b;

	SkipChunk(Chunk);
}

// Ambient (rgb)
void C3DS::GetAmbientColour(stChunk* Chunk)
{
	struct stRGB{ unsigned char r, g, b; };
	stRGB AmbColour;

	char ChunkHeader[6];
	Chunk->bytesRead += (unsigned int)fread(ChunkHeader, 1, 6, m_fp);

	Chunk->bytesRead += (unsigned int)fread(&AmbColour, 1, 3, m_fp);	

	stMaterial* pM = &(m_pMaterials[m_iNumMaterials-1]);
	pM->Ambient.r = AmbColour.r;
	pM->Ambient.g = AmbColour.g;
	pM->Ambient.b = AmbColour.b;

	SkipChunk(Chunk);
}
// Specular (rgb)
void C3DS::GetSpecularColour(stChunk* Chunk)
{
	struct stRGB{ unsigned char r, g, b; };
	stRGB SpecColour;

	char ChunkHeader[6];
	Chunk->bytesRead += (unsigned int)fread(ChunkHeader, 1, 6, m_fp);

	Chunk->bytesRead += (unsigned int)fread(&SpecColour, 1, 3, m_fp);	

	stMaterial* pM = &(m_pMaterials[m_iNumMaterials-1]);
	pM->Specular.r = SpecColour.r;
	pM->Specular.g = SpecColour.g;
	pM->Specular.b = SpecColour.b;

	SkipChunk(Chunk);
}

// Get the materials name, e.g. default-2- etc
void C3DS::GetMaterialName(stChunk* Chunk)
{
	char str[256];
	Chunk->bytesRead += GetString(str);

	stMaterial* pM = &(m_pMaterials[m_iNumMaterials-1]);
	strcpy(pM->szName, str);
}

/***************************************************************************/
/*                             ANIMATION                                   */
/***************************************************************************/

void C3DS::StartEndFrames(stChunk* Chunk)
{
	int dwStartFrame;
	int dwEndFrame;

	Chunk->bytesRead += (unsigned int)fread(&dwStartFrame, 1, 4, m_fp);
	Chunk->bytesRead += (unsigned int)fread(&dwEndFrame, 1, 4, m_fp);

	// Assume for simplisity, that it starts at 0!
	m_iKeyFrames = dwEndFrame;

}

void C3DS::ReadNameOfObjectToAnimate(stChunk* Chunk)
{
	char str[256];
	Chunk->bytesRead += GetString(str);

	stAnimation* pAnim = &(m_pAnimation[m_iNumAnimObjects - 1]);

	strcpy(pAnim->szObjName, str);

	// Skip hierachy
	SkipChunk(Chunk);
}

void C3DS::ReadPivotPoint(stChunk* Chunk)
{
	float vPoint[3];

	Chunk->bytesRead += (unsigned int)fread(&vPoint, 1, 12, m_fp);

	// Skipped upto now...but you could easily add it in later if you wanted :)
}

void C3DS::ReadAnimPos(stChunk* Chunk)
{
	short skip[4];
	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);
	Chunk->bytesRead += (unsigned int)fread(skip, 1, 4*sizeof(short), m_fp);

	short iKeys;
	Chunk->bytesRead += (unsigned int)fread(&iKeys, 1, sizeof(short), m_fp);

	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);


	// Get ready to read it in
	stAnimation* pAnim = &(m_pAnimation[m_iNumAnimObjects - 1]);
	pAnim->iPosKeys = iKeys;

	// Now we now how many keys we are reading in..e.g. 1, 7 and 49.  And we know that
	// there is a totoal of 100 keys.  So its upto us to do the rest.
	// Lets use the simple and best linear interpolation
	// p(t) = p0 + t(p1-p0)
	stPosition* pPosition = new stPosition[ iKeys ];


	short      iFrameNum;
	float      x_pos, y_pos, z_pos;


	for(int jj=0; jj< iKeys; jj++)
	{
		Chunk->bytesRead += (unsigned int)fread( &iFrameNum, 1, sizeof(short),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( skip,       1, 2*sizeof(short), m_fp);
		Chunk->bytesRead += (unsigned int)fread( &x_pos,     1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &y_pos,     1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &z_pos,     1, sizeof(float),   m_fp);

		pPosition[jj].iFrame = iFrameNum;
		pPosition[jj].x = x_pos;
		pPosition[jj].y = z_pos;
		pPosition[jj].z = y_pos;
	}

	pAnim->pPosition = pPosition;

}

void C3DS::ReadAnimRot(stChunk* Chunk)
{
	short skip[4];
	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);
	Chunk->bytesRead += (unsigned int)fread(skip, 1, 4*sizeof(short), m_fp);

	short iKeys;
	Chunk->bytesRead += (unsigned int)fread(&iKeys, 1, sizeof(short), m_fp);

	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);


	// Get ready to read it in
	stAnimation* pAnim = &(m_pAnimation[m_iNumAnimObjects - 1]);
	pAnim->iRotKeys = iKeys;

	// Now we now how many keys we are reading in..e.g. 1, 7 and 49.  And we know that
	// there is a totoal of 100 keys.  So its upto us to do the rest.
	// Lets use the simple and best linear interpolation
	// p(t) = p0 + t(p1-p0)
	stRotation* pRotation = new stRotation[ iKeys ];


	short      iFrameNum;
	float      x_axis, y_axis, z_axis;
	float      rotation_rads;


	for(int jj=0; jj< iKeys; jj++)
	{
		Chunk->bytesRead += (unsigned int)fread( &iFrameNum,    1, sizeof(short),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( skip,          1, 2*sizeof(short), m_fp);
		Chunk->bytesRead += (unsigned int)fread( &rotation_rads,1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &x_axis,       1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &y_axis,       1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &z_axis,       1, sizeof(float),   m_fp);

		pRotation[jj].iFrame = iFrameNum;
		pRotation[jj].rotation_rads = rotation_rads;
		pRotation[jj].axis_x = x_axis;
		pRotation[jj].axis_y = z_axis; // strange but true :)
		pRotation[jj].axis_z = y_axis;

		// Make our axis of chose always positive e.g. 1*2 =1 -1*-1 =1 etc.
		pRotation[jj].axis_x = pRotation[jj].axis_x*pRotation[jj].axis_x;
		pRotation[jj].axis_y = pRotation[jj].axis_y*pRotation[jj].axis_y;
		pRotation[jj].axis_z = pRotation[jj].axis_z*pRotation[jj].axis_z;

		
		if( jj == 0 )
			pRotation[jj].rotation_rads -= 1.57f;

		// Make our angles relative the the absolute begining, not to each prev frame.
		if( jj > 0 )
		{
			float d = z_axis + y_axis + x_axis;
			float old = pRotation[jj-1].rotation_rads;

			pRotation[jj].rotation_rads = old + d*pRotation[jj].rotation_rads;

		}
	}

	pAnim->pRotation = pRotation;
}

void C3DS::ReadAnimScale(stChunk* Chunk)
{

	short skip[4];
	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);
	Chunk->bytesRead += (unsigned int)fread(skip, 1, 4*sizeof(short), m_fp);

	short iKeys;
	Chunk->bytesRead += (unsigned int)fread(&iKeys, 1, sizeof(short), m_fp);

	Chunk->bytesRead += (unsigned int)fread(skip, 1, sizeof(short), m_fp);


	// Get ready to read it in
	stAnimation* pAnim = &(m_pAnimation[m_iNumAnimObjects - 1]);
	pAnim->iScaleKeys = iKeys;

	stScale* pScale = new stScale[ iKeys ];


	short      iFrameNum;
	float      x_scale, y_scale, z_scale;


	for(int jj=0; jj< iKeys; jj++)
	{
		Chunk->bytesRead += (unsigned int)fread( &iFrameNum,  1, sizeof(short),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( skip,        1, 2*sizeof(short), m_fp);
		Chunk->bytesRead += (unsigned int)fread( &x_scale,    1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &y_scale,    1, sizeof(float),   m_fp);
		Chunk->bytesRead += (unsigned int)fread( &z_scale,    1, sizeof(float),   m_fp);

		pScale[jj].iFrame = iFrameNum;
		pScale[jj].x = x_scale;
		pScale[jj].y = y_scale;
		pScale[jj].z = z_scale;
	}

	pAnim->pScale = pScale;

}

/***************************************************************************/
/*                                                                         */
/* If theres a nested sub-chunk, and we know its ID, e.g 0xA000 etc, then  */
/* we can simply add its ID to the switch list, and add a calling sub      */
/* functino which will deal with it.  Else just skip over that Chunk...    */
/* and carry on parsing the rest of our file.                              */
/*                                                                         */
/***************************************************************************/

void C3DS::ParseChunk(stChunk* Chunk)
{
	while(Chunk->bytesRead < Chunk->length)
	{
		stChunk tempChunk = {0};
		ReadChunk(&tempChunk);

		// DEBUG CHUNKS
		//char buf[2000];
		//sprintf(buf, "Chunk ID: %.4x\t Size: %d\n", tempChunk.ID, tempChunk.length);
		//debug_op(buf);

		switch( tempChunk.ID)
		{
		// HEADER OUR ENTRY POINT
		case EDIT3DS: //0x3D3D
			ParseChunk(&tempChunk);
			break;

		// MATERIALS
		case MATERIAL: //0xAFFF
			{
			stMaterial newMaterial;
			m_pMaterials.push_back(newMaterial);
			m_iNumMaterials++;
			}
			ParseChunk(&tempChunk);
			break;
		case MAT_NAME: //0xA000 - sz for hte material name "e.g. default 2"
			GetMaterialName(&tempChunk);
			break;
		case MAT_AMBIENT:
			GetAmbientColour(&tempChunk);
			break;
		case MAT_SPECULAR:
			GetSpecularColour(&tempChunk);
			break;
		case MAT_DIFFUSE:  // Diffuse Colour  //0xA020
			GetDiffuseColour(&tempChunk);
			break;
		case MAT_TEXMAP:  //0xA200 - if there's a texture wrapped to it where here
			ParseChunk(&tempChunk);
			break;
		case MAT_TEXFLNM: // 0xA300 -  get filename of the material
			GetTexFileName(&tempChunk);
			break;

		// OBJECT - MESH'S
		case NAMED_OBJECT: //0x4000
			{
			stMesh newMesh;
			m_pMeshs.push_back(newMesh);
			m_iNumMeshs++;
			GetMeshObjectName(&tempChunk);
			}
			break;
		case OBJ_MESH:     //0x4100
			ParseChunk(&tempChunk);
			break;
		case MESH_VERTICES: //0x4110
			ReadMeshVertices(&tempChunk);
			break;
		case MESH_FACES: //0x4120
			ReadMeshFaces(&tempChunk);
			break;
		case MESH_TEX_VERT: //0x4140
			ReadMeshTexCoords(&tempChunk);
			break;
		case MESH_MATER: //0x4130
			ReadMeshMaterials(&tempChunk);
			break;

		// ANIMATION
		case KEYF3DS:	//0xB000
			ParseChunk(&tempChunk);
			break;
		case ANIM_S_E_TIME:   //0xB008
			StartEndFrames(&tempChunk);
			break;
		case ANIM_OBJ:	//0xB002
			{
				stAnimation newAnimation;
				m_pAnimation.push_back(newAnimation);
				m_iNumAnimObjects++;
			}
			ParseChunk(&tempChunk);
			break;
		case ANIM_NAME:
			ReadNameOfObjectToAnimate(&tempChunk);
			break;
		case ANIM_PIVOT: // 0xB013
			ReadPivotPoint(&tempChunk);
			break;
		case ANIM_POS: // 0xB020
			ReadAnimPos(&tempChunk);
			break;
		case ANIM_ROT: //		0xB021
			ReadAnimRot(&tempChunk);
			break;
		case ANIM_SCALE: // 0xB022
			ReadAnimScale(&tempChunk); 
			break;

		default:
			SkipChunk(&tempChunk);
		}
		
		Chunk->bytesRead += tempChunk.length;
	}
}

/***************************************************************************/
/*                                                                         */
/* Read in .3ds file.                                                      */
/*                                                                         */
/***************************************************************************/

bool C3DS::Create(char* szFileName)
{
	m_fp = fopen(szFileName, "rb");
	if (m_fp == NULL)
		return false;
	
	stChunk Chunk = {0};
	ReadChunk(&Chunk);

	ParseChunk(&Chunk );
	
	fclose(m_fp);

	return true;
}

void C3DS::Release()
{
	int i;
	for(i=0; i<m_iNumMeshs; i++)
	{
		if(m_pMeshs[i].pVerts) delete[] m_pMeshs[i].pVerts;
		if(m_pMeshs[i].pFaces) delete[] m_pMeshs[i].pFaces;
		if(m_pMeshs[i].pTexs ) delete[] m_pMeshs[i].pTexs;
	}

	// Delete any animation resources.
	for(i=0; i<m_iNumAnimObjects; i++)
	{
		if(m_pAnimation[i].pPosition) delete[] m_pAnimation[i].pPosition;
		if(m_pAnimation[i].pRotation) delete[] m_pAnimation[i].pRotation;
		if(m_pAnimation[i].pScale)    delete[] m_pAnimation[i].pScale;
	}
}





// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
/*
   The following functions are used so you can debug, or display the
   raw 3ds information to a file to look for code errors.
*/
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

// Debugging Function to dump raw valeus to a file

void debug_op(char *s)
{
	FILE *fp;
	fp = fopen("t.txt", "a+");
	fprintf(fp, "%s", s);
	fclose(fp);
}

// Debugging Function - Simply put it dumps our data to a file, just to check
// that it has read it all in correctly.

void DisplayRawData(C3DS* pObj)
{
	char buf[500];
	int i;
	//----------------------------------------------------------------
	for(i=0; i<pObj->m_iNumMeshs; i++ )
	{
		C3DS::stMesh* pMesh = &(pObj->m_pMeshs[i]);

		sprintf(buf, "Shape: %s\n", pMesh->szMeshName);
		debug_op(buf);

		sprintf(buf, "iNumFaces: %d\n", pMesh->iNumFaces);
		debug_op(buf);
		for(int cc=0; cc<pMesh->iNumFaces; cc++)
		{
			sprintf(buf, "\t %d, \t %d \t %d\n", 
						pMesh->pFaces[cc].corner[0], pMesh->pFaces[cc].corner[1], pMesh->pFaces[cc].corner[2]);
			debug_op(buf);
		}
	}

	//----------------------------------------------------------------
	for(i=0; i<pObj->m_iNumMeshs; i++ )
	{
		C3DS::stMesh* pMesh = &(pObj->m_pMeshs[i]);


		sprintf(buf, "iNumVertices: %d\n", pMesh->iNumVerts);
		debug_op(buf);
		for(  int cc=0; cc<pMesh->iNumVerts; cc++)
		{
			sprintf(buf, "\t %.2f, \t %.2f \t %.2f\n", 
				pMesh->pVerts[cc].x,pMesh->pVerts[cc].y,pMesh->pVerts[cc].z);
			debug_op(buf);
		}
	}

	//----------------------------------------------------------------
	for(i=0; i<pObj->m_iNumMeshs; i++ )
	{
		C3DS::stMesh* pMesh = &(pObj->m_pMeshs[i]);
		if( pMesh->pTexs != NULL )
		{
			sprintf(buf, "iNumTex: %d\n", pMesh->iNumVerts);
			debug_op(buf);
			for( int  cc=0; cc<pMesh->iNumVerts; cc++)
			{
				sprintf(buf, "\t %.2f, \t %.2f\n", 
					pMesh->pTexs[cc].tu, pMesh->pTexs[cc].tv );
				debug_op(buf);
			}
		}
	}

	//----------------------------------------------------------------
	for(i=0; i<pObj->m_iNumMeshs; i++ )
	{
		C3DS::stMesh* pMesh = &(pObj->m_pMeshs[i]);

		if( pObj->m_iNumMaterials > 0 )
		{
			sprintf(buf, "Material vs Faces: %d\n", pMesh->iNumFaces);
			debug_op(buf);
			for( int cc=0; cc<pMesh->iNumFaces; cc++)
			{
				sprintf(buf, "\t MaterialID: %d", 
					pMesh->pFaces[cc].MaterialID );
				debug_op(buf);

				int ID = pMesh->pFaces[cc].MaterialID;
				sprintf(buf, "\t, Name: %s", pObj->m_pMaterials[ID].szName);
				debug_op(buf);
				
				if( pObj->m_pMaterials[ID].bTexFile )
				{
					sprintf(buf, "\tTex FileName: %s", pObj->m_pMaterials[ID].szTextureFile);
					debug_op(buf);
				}
				debug_op("\n");
			}
		}
	}
	//----------------------------------------------------------------
}



// DEBUG * DEBUG * DEBUG * DEBUG * DEBUG * DEBUG * DEBUG * DEBUG
// Dump animation data to a file.

void DisplayRawAnimationData(C3DS* pObj)
{
	char buf[500];
	debug_op("\nANIMATION DATA\n");

	sprintf(buf, "Srt Frame Assmed 0:0 -  Number of KeyFrame: %d\n", pObj->m_iKeyFrames);			
	debug_op(buf);


	int i;
	for(i=0; i<pObj->m_iNumAnimObjects; i++)
	{
		stAnimation* pAnim = &(pObj->m_pAnimation[i]);
		// SHAPE NAME
		sprintf( buf, "Animated Object Name: %s\n", pAnim->szObjName );
		debug_op( buf );
	
		// POSITION KEYS
		sprintf(buf, "Num Position Key Frames: %d\n", pAnim->iPosKeys);			
		debug_op(buf);
		stPosition* pPos = pAnim->pPosition;
		int jj;
		for(jj=0; jj< pAnim->iPosKeys; jj++)
		{
			sprintf(buf, "\t KeyFrame Num: %d, \t x:%.2f \t y:%.2f \t z:%.2f\n",
								pPos[jj].iFrame, pPos[jj].x, pPos[jj].y, pPos[jj].z);
			debug_op(buf);
		}
		// ROTATION KEYS
		sprintf(buf, "Num Rotation Key Frames: %d\n", pAnim->iRotKeys);			
		debug_op(buf);
		stRotation* pRot = pAnim->pRotation;
		for(jj=0; jj< pAnim->iRotKeys; jj++)
		{
			sprintf(buf, "\t KeyFrame Num: %d, \t x:%.2f \t y:%.2f \t z:%.2f \t angle: %.2f\n",
				pRot[jj].iFrame, pRot[jj].axis_x, pRot[jj].axis_y, pRot[jj].axis_z, pRot[jj].rotation_rads);
			debug_op(buf);
		}

		// SCALE KEYS
		sprintf(buf, "Num Scale Key Frames: %d\n", pAnim->iScaleKeys);			
		debug_op(buf);
		stScale* pScale = pAnim->pScale;
		for(jj=0; jj< pAnim->iScaleKeys; jj++)
		{
			sprintf(buf, "\t KeyFrame Num: %d, \t x:%.2f \t y:%.2f \t z:%.2f\n",
				pScale[jj].iFrame, pScale[jj].x, pScale[jj].y, pScale[jj].z);
			debug_op(buf);
		}
	}
}












