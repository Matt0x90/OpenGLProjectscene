///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1500; //1000
	const int WINDOW_HEIGHT = 1200; //800
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 10.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
	g_pCamera->MovementSpeed = 20;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive window resize events
	glfwSetFramebufferSizeCallback(window, &ViewManager::Window_Resize_Callback);
	// this callback is used to receive scroll wheel events
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Wheel_Callback);
	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	// this callback is used to receive mouse button press events
	glfwSetMouseButtonCallback(window, &ViewManager::Mouse_Button_Callback);


	// enable blending for supporting transparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}
/***********************************************************
 *  MouseButtonCallback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse button is pressed or released.
 ***********************************************************/
// GLFW_PRESS / GLFW_RELEASE, no mods
void ViewManager::Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods)
{
	// if the right mouse button is pressed
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		// process the mouse movement for the camera
		bool rightButtonPressed = button;
		if (rightButtonPressed && g_pCamera->Zoom != 45.0f)
		{
			g_pCamera->Zoom = 45.0f;
		}
		else if (rightButtonPressed && g_pCamera->Zoom == 45.0f)
		{
			g_pCamera->Zoom = 80.0f;
		}
	}
}

/***********************************************************
 *  Window_Resize_Callback() (New functionality)
 *
 *  This method is automatically called from GLFW whenever
 *  the display window is resized. (by OS or user resize)
 ***********************************************************/
void ViewManager::Window_Resize_Callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/***********************************************************
 *  Mouse_Scroll_Wheel_Callback() (New functionality)
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is used / scrolls.
 *  Up = slow, Down = fast
 ***********************************************************/
void ViewManager::Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// adjust speed of movement at which camera travels
	//g_pCamera->ProcessMouseScroll(static_cast<float>(yOffset));
	// updated to not call MouseScroll since it's tied to Zoom by default.
	g_pCamera->MovementSpeed += static_cast<float>(yOffset);
	if (g_pCamera->MovementSpeed < 1.0f)
		g_pCamera->MovementSpeed = 1.0f;
	if (g_pCamera->MovementSpeed > 45.0f)
		g_pCamera->MovementSpeed = 45.0f;

}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// when the first mouse move event is received, this needs to be recorded so that
	// all subsequent mouse moves can correctly calculate the X position offset and Y
	// position offset for proper operation
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate the X offset and Y offset values for moving the 3D camera accordingly
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

	// set the current positions into the last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// move the 3D camera according to the calculated offsets
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	// process camera panning up and down (New functionality)
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}

	/*******************************************************************/
	// process camera view switching between orthographic and perspective
	/* Multiple views of orthographic projection*/
	/*******************************************************************/
	//front ortho
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		// change to front orthographic projection
		bOrthographicProjection = true;
		// camera settings to show a front orthographic view
		g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
		// y 5.0 position puts it at ground level which doesnt show plane depending on angle of scene
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
		
	}
	// side ortho
	if (glfwGetKey(m_pWindow, GLFW_KEY_I) == GLFW_PRESS)
	{
		// change to side orthographic projection
		bOrthographicProjection = true;
		// change the camera settings to show a side orthographic view
		g_pCamera->Position = glm::vec3(12.0f, 5.0f, 0.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Front = glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	// top ortho
	if (glfwGetKey(m_pWindow, GLFW_KEY_U) == GLFW_PRESS)
	{
		// change to a multi-view orthographic projection
		bOrthographicProjection = true;
		// change the camera settings to show a top orthographic view
		g_pCamera->Position = glm::vec3(0.0f, 16.0f, 2.0f);
		g_pCamera->Up = glm::vec3(-1.0f, 0.0f, 0.0f);
		g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
	}
	// perspective
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		// change to perspective projection
		bOrthographicProjection = false;
		// default perspective view parameters
		g_pCamera->Position = glm::vec3(0.0f, 10.0f, 12.0f);
		g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 80;
		
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	if (bOrthographicProjection == false)
	{
		// perspective projection
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else
	{
		// wasn't really sure what settings to do, this was my attempt that seems identical to the sample code
		// g_pCamera->Zoom can be replaced with 80.0f default fixed zoom level, no RMB zoom allowed if desired.
		projection = glm::ortho(
			-(GLfloat)WINDOW_WIDTH / (2.0f * g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (2.0f * g_pCamera->Zoom),
			-(GLfloat)WINDOW_HEIGHT / (2.0f * g_pCamera->Zoom), (GLfloat)WINDOW_HEIGHT / (2.0f * g_pCamera->Zoom),
			0.1f, 100.0f);
	}
	
	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}