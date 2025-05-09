#include "main.h"
#include "model.h"
#include "view.h"
#include "control.h"


int main(int argc, char* argv[])
{
	SDL_Window* main_window;
	init(main_window);

	Model model1("models/sun.obj", glm::vec3(1.0f, 1.0f, 0.0f), 1.0f);

	Model model2("models/mercury.obj", {0.5f, 0.5f, 0.5f}, 0.f);
	Model model3("models/venus.obj", {}, 0.0f);
	Model model4("models/earth.obj", {0.5f, 0.5f, 0.5f}, 0.f);
	Model model5("models/mars.obj", {0.5f, 0.5f, 0.5f}, 0.f);
	Model model6("models/jupiter.obj", {}, 0.f);

	std::vector<Model*> planets = {&model2, &model3, &model4, &model5, &model6};

	Camera camera;
	Control control(camera);

	View view1(model1, camera);
	View view2(model2, camera);
	View view3(model3, camera);
	View view4(model4, camera);
	View view5(model5, camera);
	View view6(model6, camera);

	std::vector<View*> views = {&view1, &view2, &view3, &view4, &view5, &view6};

	for (size_t i = 1; i < views.size(); ++i)
	{
		views[i]->addPointLight(glm::vec3(0, 0, 0), glm::vec3(1.0f, 1.0f, 1.0f), 100);
	}

	const double dt = 10;
	double accumulatedTime = 0.0;
	double lastTime = SDL_GetTicks();
	double currentTime;

	std::vector<float> radiuses = {40.0f, 55.0f, 65.0f, 75.0f, 85.0f};
	std::vector<float> speeds = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
	float speedMultiplier = 0.0001f;

	constexpr size_t planetCount = 5;

	while (true)
	{
		SDL_Event event;
		SDL_PollEvent(&event);
		if (event.type == SDL_WINDOWEVENT_CLOSE)
		{
			break;
		}
		currentTime = SDL_GetTicks();
		accumulatedTime += currentTime - lastTime;
		lastTime = currentTime;

		control.handleInput();

		while (accumulatedTime > dt)
		{
			accumulatedTime -= dt;
		}
		for (size_t i = 0; i < planetCount; ++i)
		{
			float angle = (float)currentTime * speeds[i] * speedMultiplier; 
			glm::vec3 orbitPos = glm::vec3(radiuses[i]* cos(angle), 0.0f, radiuses[i]* sin(angle));
			planets[i]->matrix = glm::translate(glm::mat4(1.0f), orbitPos);
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto& view : views)
		{
			view->render();
		}
		glFlush();
		SDL_GL_SwapWindow(main_window);
	}

	tearDown(main_window);

	return 0;
}
