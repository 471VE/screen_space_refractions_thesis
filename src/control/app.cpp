#include "app.h"
#include "logging.h"
#include "../view/camera.h"
#include "../preprocessing/preprocessing_common.h"


// Construct a new App.
App::App(int width, int height)
{
	buildGlfwWindow(width, height);
	graphicsEngine = new Engine(width, height, window);
	scene = new Scene();
}

static Camera camera;
static uint32_t distance_calculation_mode = 1;
static uint32_t hammersley_points_num = 100;
static bool recalculate_sh_terms = false;

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

	if (glfwGetKey(window, '3'))
		distance_calculation_mode = 3;

	if (glfwGetKey(window, 'N'))
		recalculate_sh_terms = true;
}


// Build the App's window (using glfw)
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
		vklogging::Logger::getLogger()->print(message.str());
	}
	else
		vklogging::Logger::getLogger()->print("GLFW window creation failed");

	glfwSetKeyCallback(window, on_keyboard_pressed);
}

// Start the App's main loop
void App::run()
{
	std::vector<glm::dvec3> hammersleySequence = construct_hemisphere_hammersley_sequence(hammersley_points_num);
	std::vector<float> sphereShTerms = calculate_sh_terms(hammersleySequence, sphere_width);
	graphicsEngine->setSphereShTerms(sphereShTerms);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (recalculate_sh_terms)
		{
			std::cout << "Enter the number of points in Hammersley sequence: ";
			std::cin >> hammersley_points_num;
			std::vector<glm::dvec3> hammersleySequence = construct_hemisphere_hammersley_sequence(hammersley_points_num);
			std::vector<float> sphereShTerms = calculate_sh_terms(hammersleySequence, sphere_width);
			graphicsEngine->setSphereShTerms(sphereShTerms);
			recalculate_sh_terms = false;
		}

		graphicsEngine->setDistanceCalculationMode(distance_calculation_mode);
		graphicsEngine->render(scene);

		camera.move(static_cast<float>(glfwGetTime() - lastTime));
		graphicsEngine->updateCameraData(camera);
		calculateFrameRate();
	}
}

// Calculates the App's framerate and updates the window title
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

// App destructor.
App::~App()
{
	delete graphicsEngine;
	delete scene;
}