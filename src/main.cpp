#include "control/app.h"

int main() {
	std::system("cd src/shaders && python compile_shaders.py");

	App* myApp = new App(1280, 720);

	myApp->run();
	delete myApp;

	return 0;
}