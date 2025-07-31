///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ================
// This file contains the implementation of the `SceneManager` class, which is 
// responsible for managing the preparation and rendering of 3D scenes. It 
// handles textures, materials, lighting configurations, and object rendering.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Load, bind, and manage textures in OpenGL.
// - Define materials and lighting properties for 3D objects.
// - Manage transformations and shader configurations.
// - Render complex 3D scenes using basic meshes.
//
// NOTE: This implementation leverages external libraries like `stb_image` for 
// texture loading and GLM for matrix and vector operations.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (auto& textureID : m_textureIDs)
	{
		textureID.tag = "/0"; 
		textureID.ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// clear up allocated memory
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// destroy all loaded textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
	m_loadedTextures = 0; // reset counter if utilized in future changes.
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  LoadSceneTextures()
 *
*  This method is used for preparing the 3D scene by loading
*  the textures in memory to be used for mapping to objects.
*  Up to 16 textures can be loaded per scene.
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;
	// load the textures from image files
	// walls, trim, floor, and ceiling textures
	bReturn = CreateGLTexture("textures/floor.png", "floor");
	bReturn = CreateGLTexture("textures/wallpaper.jpg", "wallpaper");
	bReturn = CreateGLTexture("textures/ceiling.jpg", "ceiling");
	// soda textures
	bReturn = CreateGLTexture("textures/soda1.png", "soda1");
	bReturn = CreateGLTexture("textures/soda2.png", "soda2"); // red
	bReturn = CreateGLTexture("textures/sodatop.png", "soda_top");
	// arcade
	bReturn = CreateGLTexture("textures/tekken.jpg", "tekken");
	bReturn = CreateGLTexture("textures/arcade2.png", "arcade2");
	bReturn = CreateGLTexture("textures/coinslot.png", "coin_slot");
	bReturn = CreateGLTexture("textures/test2.png", "test"); 
	bReturn = CreateGLTexture("textures/testt.jpg", "testt");
	bReturn = CreateGLTexture("textures/yellow.png", "yellow");
	// lamp
	bReturn = CreateGLTexture("textures/linen.jpg", "linen");
	// everything
	bReturn = CreateGLTexture("textures/leather.jpg", "leather");
	bReturn = CreateGLTexture("textures/metal2.jpg", "metal2");
	bReturn = CreateGLTexture("textures/aluminum.png", "aluminum");
	// after texture image data is loaded, they need to be bound to texture slots.
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the scene
	LoadSceneTextures();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPrismMesh();
	//customized Torus, I lowered the radius to make it thinner
	//and lowered segments as well since it's such a small part repeated
	m_basicMeshes->LoadTorusMesh(1, 0.06f, 24, 8);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// render objects in the scene
	RenderWalls();
	RenderSoda();
	RenderLamp();
	RenderChair();
	RenderArcade();
}

/***********************************************************
 * RenderWalls()
 *
 * This method is used for rendering the walls, floor,
 * trim, and ceiling of the 3D scene.
 ************************************************************/
void SceneManager::RenderWalls()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/******************************************************************/
	// Floor
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 16.0f);
	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 6.0f);
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	// draw the shader texture for floor
	SetShaderTexture("floor");
	//SetShaderColor(0.51f,0.28f,0.086f,1);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// Ceiling
	scaleXYZ = glm::vec3(20.0f, 1.0f, 16.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 28.0f, 6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("ceiling");
	m_basicMeshes->DrawPlaneMesh();
	
	/****************************************************************/
	// Center Wall
	scaleXYZ = glm::vec3(20.0f, 1.0f, 14.0f);
	XrotationDegrees = 90.0f; //rotate X to position wall
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 14.0f, -10.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.62f, 0.455f, 0.278f, 1);
	SetShaderTexture("wallpaper");
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// Right side Wall
	scaleXYZ = glm::vec3(16.0f, 1.0f, 14.0f);
	XrotationDegrees = 90.0f; //rotate x & y to fit
	YrotationDegrees = -90.0f; // should face inward (ensuring it's the proper direction when applying textures later)
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(20.0f, 14.0f, 6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.62f, 0.455f, 0.278f, 1);
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// Left side Wall
	scaleXYZ = glm::vec3(16.0f, 1.0f, 14.0f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f; //y is +90 for left side, -90 for right side
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-20.0f, 14.0f, 6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.62f, 0.455f, 0.278f, 1);
	m_basicMeshes->DrawPlaneMesh();
}

/**************************************************
 * RenderSoda()
 *
 * This method is used for rendering the soda can
 * object in the 3D scene.
 **************************************************/
void SceneManager::RenderSoda()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	// Base of soda can
	scaleXYZ = glm::vec3(0.8f, 0.4f, 0.8f); //using .8 as the scaling radius
	XrotationDegrees = 180.0f; //flipped upside down
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// positioned in scene near where it is in the image
	positionXYZ = glm::vec3(-8.0f, 0.4f, 4.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.396f, 0.341f, 0.275f, 1); //silver base
	SetShaderTexture("aluminum");
	m_basicMeshes->DrawTaperedCylinderMesh(true, false, true); //no bottom
	/****************************************************************/
	// Body of soda can
	scaleXYZ = glm::vec3(0.8f, 2.0f, 0.8f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f; // rotating to face the camera to see the texture
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-8.0f, 0.4f, 4.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("soda1");
	SetTextureUVScale(-1.0f, 1.0f); // flip the texture
	//SetShaderColor(0.427f, 0.039f, 0.0f, 1.0); //red body
	m_basicMeshes->DrawCylinderMesh();
	SetTextureUVScale(1.0f, 1.0f); // reset the texture UV scale to default
	/****************************************************************/
	// Body top - half sphere
	scaleXYZ = glm::vec3(0.8f, 0.3f, 0.8f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-8.0f, 2.4f, 4.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("soda2");
	//SetShaderColor(0.427f, 0.039f, 0.0f, 1); //red body
	//SetShaderMaterial("red_body");
	m_basicMeshes->DrawHalfSphereMesh();
	/****************************************************************/
	// Flat cylinder for top of lid - texture contains the tab
	scaleXYZ = glm::vec3(0.6f, 0.03f, 0.6f); //scaled x & z smaller to fit
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-8.0f, 2.67f, 4.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("soda_top");
	//SetShaderColor(0.396f, 0.341f, 0.275f, 1); //silver
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// Torus for lid rim
	scaleXYZ = glm::vec3(0.6f, 0.6f, 1.0f); //scaled x & y smaller to fit -- z adjusts thickness
	XrotationDegrees = 90.0f; //rotated flat against lid
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-8.0f, 2.71f, 4.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("aluminum");
	//SetShaderColor(0.500f, 0.410f, 0.350f, 1); //bright silver
	m_basicMeshes->DrawTorusMesh();
}

/***********************************************************
 * RenderLamp()
 *
 * This method is used for rendering the lamp object.
 **********************************************************/
void SceneManager::RenderLamp()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	// flat cylinder base
	scaleXYZ = glm::vec3(2.7f, 0.3f, 2.7f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 0.0f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.300f, 0.082f, 0.039f, 1);
	SetShaderTexture("leather");
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// tapered cylinder base piece
	scaleXYZ = glm::vec3(0.7f, 0.5f, 0.7f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 0.3f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.252f, 0.082f, 0.039f, 1);
	SetShaderTexture("metal2");
	m_basicMeshes->DrawTaperedCylinderMesh();
	/****************************************************************/
	// elongated cylinder pole
	scaleXYZ = glm::vec3(0.3f, 15.0f, 0.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 0.8f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.302f, 0.082f, 0.039f, 1);
	SetShaderTexture("leather");
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// socket for bulb
	scaleXYZ = glm::vec3(0.3f, 0.7f, 0.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 15.8f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.396f, 0.341f, 0.275f, 1); //silver
	SetShaderTexture("metal2");
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// metal switch on side of socket
	scaleXYZ = glm::vec3(0.05f, 0.3f, 0.05f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f; //rotate to be angled as a switch
	positionXYZ = glm::vec3(14.8f, 16.2f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.141f, 0.102f, 0.039f, 1); //brown/black
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// bulb
	scaleXYZ = glm::vec3(0.5f, 1.2f, 0.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 16.5f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.500f, 0.410f, 0.350f, 1); //bright silver
	SetShaderTexture("aluminum");
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// torus metal hoop (meant to support shade)
	scaleXYZ = glm::vec3(1.0f, 1.4f, 1.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 17.7f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.141f, 0.102f, 0.039f, 1); //brown/black
	SetShaderTexture("metal2");
	m_basicMeshes->DrawTorusMesh();
	/****************************************************************/
	// top emblem on hoop (sphere)
	scaleXYZ = glm::vec3(0.15f, 0.25f, 0.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 19.4f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.141f, 0.102f, 0.039f, 1); //brown/black
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	// lamp shade
	scaleXYZ = glm::vec3(2.7f, 3.7f, 2.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 15.8f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.941f, 0.675f, 0.341f, 1); //beige
	SetShaderTexture("linen");
	m_basicMeshes->DrawTaperedCylinderMesh(false, false, true);
	//drawtop = false, drawbottom = false, drawsides = true (for empty lamp shade)
	/****************************************************************/
	// Torus connecting hoop to shade
	scaleXYZ = glm::vec3(1.4f, 0.4f, 0.8f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 19.0f, -5.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.141f, 0.102f, 0.039f, 1); //brown/black
	SetShaderTexture("metal2");
	m_basicMeshes->DrawTorusMesh();
}
void SceneManager::RenderChair()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	// Four Cylinder legs (front leg)
	scaleXYZ = glm::vec3(0.2f, 6.0f, 0.2f);
	XrotationDegrees = -9.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.0f, 6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1); //black for legs
	SetShaderTexture("metal2");
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// right leg
	scaleXYZ = glm::vec3(0.2f, 6.0f, 0.2f); //scaled to fit within pop tab
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 9.0f; //tilt left
	positionXYZ = glm::vec3(3.0f, 0.0f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1); //black for legs
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// back leg
	scaleXYZ = glm::vec3(0.2f, 6.0f, 0.2f);
	XrotationDegrees = 9.0f; //tilt backwards (toward cam)
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1);
	m_basicMeshes->DrawCylinderMesh();
	///****************************************************************/
	// left leg
	scaleXYZ = glm::vec3(0.2f, 6.0f, 0.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -9.0f; //tilt right
	positionXYZ = glm::vec3(-3.0f, 0.0f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1);
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	// Torus Foot Ring
	/****************************************************************/
	scaleXYZ = glm::vec3(2.25f, 2.25f, 3.6f);
	XrotationDegrees = 90.0f; //rotate to make it flat
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 3.0f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1);
	m_basicMeshes->DrawTorusMesh();
	/****************************************************************/
	// Cylinder Seat Cushion
	/****************************************************************/
	// Stool seat
	scaleXYZ = glm::vec3(2.5f, 0.7f, 2.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 5.95f, 3.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.102f, 0.082f, 0.039f, 1);
	SetShaderTexture("leather");
	m_basicMeshes->DrawCylinderMesh();
}

void SceneManager::RenderArcade()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	// Box base
	scaleXYZ = glm::vec3(9.0f, 9.1f, 7.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 4.6f, -6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.245f, 0.063f, 0.012f, 1);  //change back to black
	SetShaderTexture("test");
	//SetShaderTexture("arcade");
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// coin slot decal overlay for box base
	scaleXYZ = glm::vec3(3.0f, 1.0f, 3.0f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 5.0f, -2.495f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("coin_slot");
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// thin box plane for console control prism
	scaleXYZ = glm::vec3(9.0f, 1.7f, 10.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 10.0f, -4.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	SetShaderTexture("testt");
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Prism for controls
	scaleXYZ = glm::vec3(10.0f, 9.0f, 1.5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -90.0f;
	ZrotationDegrees = -90.0f;
	positionXYZ = glm::vec3(0.0f, 11.6f, -4.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.245f, 0.063f, 0.012f, 1);
	SetShaderTexture("test");
	// wrapping test
	int textureSlot = FindTextureSlot("test"); // get texture slot
	GLuint textureID = FindTextureID("test"); // get ID
	glActiveTexture(GL_TEXTURE0 + textureSlot); // 0 + active slot = active slot.
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	SetTextureUVScale(2.0f, 2.0f);
	m_basicMeshes->DrawPrismMesh();
	// reset parameters for next texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	SetTextureUVScale(1.0f, 1.0f);
	/****************************************************************/
	// Prisms for screen box
	scaleXYZ = glm::vec3(3.0f, 9.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(0.0f, 12.35f, -7.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	SetShaderTexture("arcade2");
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/
	// Prisms for screen box
	scaleXYZ = glm::vec3(1.6f, 9.0f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -188.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(0.0f, 13.495f, -7.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/
	// Screen Box
	scaleXYZ = glm::vec3(9.0f, 5.5f, 5.1f);
	XrotationDegrees = -1.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 16.635f, -7.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.245f, 0.063f, 0.012f, 1);
	SetShaderTexture("test");
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Plane for screen -- emit light & add texture later
	scaleXYZ = glm::vec3(3.1f, 1.0f, 2.4f);
	XrotationDegrees = 89.0f;// rotate flat to face camera
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 16.67f, -4.4f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.094f, 0.267f, 0.369f, 1); //blue
	SetShaderTexture("tekken");
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	// Top of Arcade Machine
	scaleXYZ = glm::vec3(9.0f, 2.5f, 7.0f);
	XrotationDegrees = -1.0f;//slightly tilted
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 20.65f, -6.1f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	SetShaderTexture("testt");
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Trim/Decal for Arcade Machine
	/****************************************************************/
	// right side top trim
	scaleXYZ = glm::vec3(0.5f, 0.6f, 5.5f);
	XrotationDegrees = 89.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(4.20f, 16.68f, -4.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	SetShaderTexture("arcade2");
	m_basicMeshes->DrawBoxMesh();
	// left side top trim
	scaleXYZ = glm::vec3(0.5f, 0.6f, 5.5f);
	XrotationDegrees = 89.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.20f, 16.68f, -4.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	m_basicMeshes->DrawBoxMesh();
	// control panel trim left side
	scaleXYZ = glm::vec3(0.5f, 0.6f, 5.4f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.20f, 11.88f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1); //dark red
	m_basicMeshes->DrawBoxMesh();
	// control panel trim right side
	scaleXYZ = glm::vec3(0.5f, 0.6f, 5.4f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(4.20f, 11.88f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1);
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	// Control Panel
	/****************************************************************/
	// Joystick base
	scaleXYZ = glm::vec3(1.0f, 0.15f, 1.0f); // y = thickness/height
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.20f, 11.45f, -1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.012f, 1); //dark red
	m_basicMeshes->DrawCylinderMesh();
	// Joystick rod
	scaleXYZ = glm::vec3(0.1f, 0.8f, 0.1f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.20f, 11.60f, -1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.396f, 0.341f, 0.275f, 1); //silver
	SetShaderTexture("aluminum");
	m_basicMeshes->DrawCylinderMesh();
	// Joystick sphere
	scaleXYZ = glm::vec3(0.4f, 0.4f, 0.4f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.20f, 12.7f, -1.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.427f, 0.039f, 0.0f, 1); //red
	SetShaderTexture("soda2");
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	// Button base left
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.5f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(1.3f, 11.35f, -1.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.445f, 0.063f, 0.012f, 1);
	SetShaderTexture("yellow"); // yellow for all button bases & buttons
	m_basicMeshes->DrawCylinderMesh();
	// button base right
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.5f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(3.0f, 11.35f, -1.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.463f, 0.012f, 1);
	m_basicMeshes->DrawCylinderMesh();
	// button base center
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.5f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(2.2f, 11.75f, -2.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.412f, 1);
	m_basicMeshes->DrawCylinderMesh();
	/* button tops */
	// buttton top left
	scaleXYZ = glm::vec3(0.3f, 0.2f, 0.3f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(1.30f, 11.43f, -1.15f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.627f, 0.039f, 0.0f, 1);
	m_basicMeshes->DrawHalfSphereMesh();
	// button top right
	scaleXYZ = glm::vec3(0.3f, 0.2f, 0.3f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(3.0f, 11.43f, -1.15f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.663f, 0.012f, 1);
	m_basicMeshes->DrawHalfSphereMesh();
	// button top center
	scaleXYZ = glm::vec3(0.3f, 0.2f, 0.3f);
	XrotationDegrees = 17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(2.2f, 11.83f, -2.45f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.145f, 0.063f, 0.612f, 1);
	m_basicMeshes->DrawHalfSphereMesh();
}
