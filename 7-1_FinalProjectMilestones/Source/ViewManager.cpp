///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    
#include <glm/gtc/matrix_transform.hpp>


float gSpeedScale = 5.0f;
float gLookSpeed = 500.0f;

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
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
	g_pCamera->Position = glm::vec3(0.0f, 12.0f, 25.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.2f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
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
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
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

	glfwSetCursorPosCallback(window, ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, ViewManager::Mouse_Scroll_Callback);

}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is used for adjusting the speed of the
 * scene movement based on the mouse scroll function
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* /*window*/, double /*xOffset*/, double yOffset)
{
	// Each scroll "notch" changes speed by ~15%
	constexpr float STEP = 0.50f;       // fixed amount to change per scroll
	constexpr float MIN_SPEED = 0.05f;
	constexpr float MAX_SPEED = 20.0f;

	if (yOffset > 0.0) {
		gSpeedScale += STEP * static_cast<float>(yOffset);
	}
	else if (yOffset < 0.0) {
		gSpeedScale += STEP * static_cast<float>(yOffset);  // yOffset is negative, so this subtracts
	}

	// Clamp
	if (gSpeedScale < MIN_SPEED) gSpeedScale = MIN_SPEED;
	if (gSpeedScale > MAX_SPEED) gSpeedScale = MAX_SPEED;
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// sets the rotation speed of the camera for the arrows
	float rotationSpeed = 50.0f;

	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime * gSpeedScale);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime * gSpeedScale);
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime * gSpeedScale);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime * gSpeedScale);
	}

	// process camera panning up and down
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime * gSpeedScale);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime * gSpeedScale);
	}

	float dx = 0.0f, dy = 0.0f;

	if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT) == GLFW_PRESS) dx -= gLookSpeed * gDeltaTime * gSpeedScale; // look left
	if (glfwGetKey(m_pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) dx += gLookSpeed * gDeltaTime * gSpeedScale; // look right

	// optional: look up/down with arrows
	if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN) == GLFW_PRESS) dy += gLookSpeed * gDeltaTime * gSpeedScale; // look down
	if (glfwGetKey(m_pWindow, GLFW_KEY_UP) == GLFW_PRESS) dy -= gLookSpeed * gDeltaTime * gSpeedScale; // look up

	if (dx != 0.0f || dy != 0.0f) {
		g_pCamera->ProcessMouseMovement(dx, dy);
	}

	// Orth - Persp View
	bool tDown = (glfwGetKey(m_pWindow, GLFW_KEY_T) == GLFW_PRESS);
	bool gDown = (glfwGetKey(m_pWindow, GLFW_KEY_G) == GLFW_PRESS);

	// Change mode on press, and only print when the mode actually changes
	static bool lastOrtho = bOrthographicProjection;

	if (tDown) bOrthographicProjection = true;   // ORTHO
	if (gDown) bOrthographicProjection = false;  // PERSPECTIVE

	if (bOrthographicProjection != lastOrtho) {
		std::cout << (bOrthographicProjection ? "View Set to Orthographic (2D)\n"
			: "View Set to Perspective (3D)\n");
		lastOrtho = bOrthographicProjection;
		
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_PRESS) {
		// Front view
		g_pCamera->Position = glm::vec3(0.0f, 12.0f, 25.0f);
		g_pCamera->Front = glm::normalize(glm::vec3(0.0f, -0.2f, -1.0f));
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_PRESS) {
		// Window view
		g_pCamera->Position = glm::vec3(0.0f, 22.0f, 12.0f);
		g_pCamera->Front = glm::normalize(glm::vec3(0.0f, -0.1f, -1.0f));
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_3) == GLFW_PRESS) {
		// Overview
		g_pCamera->Position = glm::vec3(15.0f, 21.0f, 25.0f);  
		g_pCamera->Front = glm::normalize(glm::vec3(-0.3f, -0.4f, -1.0f));  
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
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

	float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	if (!bOrthographicProjection) {
		// Perspective
		projection = glm::perspective(glm::radians(g_pCamera->Zoom),
			aspect,
			0.1f, 100.0f);
	}
	else {
		// Orthographic
		const float halfHeight = 25.0f;       // tune to your scene scale
		const float halfWidth = halfHeight * aspect;
		projection = glm::ortho(-halfWidth, halfWidth,
			-halfHeight, halfHeight,
			1.0f, 100.0f);
	}

	// define the current projection matrix
	//projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

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
	m_pShaderManager->setMat4Value("view", view);
	m_pShaderManager->setMat4Value("projection", projection);
	m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);


}