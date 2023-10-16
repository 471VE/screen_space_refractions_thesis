#pragma once
#include "../config.h"
#include "../view/engine.h"
#include "../model/scene.h"

class App {

private:
	Engine* graphicsEngine;
	GLFWwindow* window;
	Scene* scene;

	double lastTime, currentTime;
	int numFrames;
	float frameTime;

	void buildGlfwWindow(int width, int height);

	void calculateFrameRate();

public:
	App(int width, int height);
	~App();
	void run();
};