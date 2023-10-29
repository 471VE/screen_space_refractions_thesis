#include "app.h"
#include "logging.h"
#include "../view/camera.h"

/**
* Construct a new App.
* 
* @param width	the width of the window
* @param height the height of the window
*/
App::App(int width, int height)
{
	buildGlfwWindow(width, height);

	graphicsEngine = new Engine(width, height, window);

	scene = new Scene();
}

Camera camera;
uint32_t distance_calculation_mode = 1;

static void on_keyboard_pressed(GLFWwindow* window, int , int, int , int)
{
	camera.resetSpeedVector();
	
  if (glfwGetKey(window, 'W'))
		camera.moveForwards();

	if (glfwGetKey(window, 'S'))
		camera.moveBackwards();

	if (glfwGetKey(window, 'D'))
		camera.moveRight();

	if (glfwGetKey(window, 'A'))
		camera.moveLeft();

	if (glfwGetKey(window, 'E'))
		camera.moveUp();

	if (glfwGetKey(window, 'C'))
		camera.moveDown();

	if (glfwGetKey(window, '1'))
		distance_calculation_mode = 1;

	if (glfwGetKey(window, '2'))
		distance_calculation_mode = 2;
}

/**
* Build the App's window (using glfw)
* 
* @param width		the width of the window
* @param height		the height of the window
* @param debugMode	whether to make extra print statements
*/
void App::buildGlfwWindow(int width, int height)
{
	std::stringstream message;

	//initialize glfw
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//GLFWwindow* glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
	if (window = glfwCreateWindow(width, height, "Renderer", nullptr, nullptr))
	{
		message << "Successfully made a glfw window called \"Renderer\", width: " << width << ", height: " << height;
		vkLogging::Logger::getLogger()->print(message.str());
	}
	else
		vkLogging::Logger::getLogger()->print("GLFW window creation failed");

	glfwSetKeyCallback(window, on_keyboard_pressed);
}

/**
* Start the App's main loop
*/
void App::run()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		graphicsEngine->setDistanceCalculationMode(distance_calculation_mode);
		graphicsEngine->render(scene);

		camera.move(static_cast<float>(glfwGetTime() - lastTime));
		graphicsEngine->updateCameraData(camera);
		calculateFrameRate();
	}
}

/**
* Calculates the App's framerate and updates the window title
*/
void App::calculateFrameRate()
{
	currentTime = glfwGetTime();
	double delta = currentTime - lastTime;

	if (delta >= 1)
	{
		int framerate{ std::max(1, int(numFrames / delta)) };
		std::stringstream title;
		title << "Running at " << framerate << " fps.";
		glfwSetWindowTitle(window, title.str().c_str());
		lastTime = currentTime;
		numFrames = -1;
		frameTime = float(1000.f / framerate);
	}

	++numFrames;
}

/**
* App destructor.
*/
App::~App()
{
	delete graphicsEngine;
	delete scene;
}