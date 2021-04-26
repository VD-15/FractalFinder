#include "VLFW/VLFW.hpp"
#include "ValkyrieEngine/ValkyrieEngine.hpp"

#include "glad/glad.h"
#include <iostream>

#include "Game.hpp"

using namespace vlk;
using namespace vlfw;

int main()
{
	std::cout << "Hello!" << std::endl;
	VLFWMainArgs args {};
	args.waitForRenderer = true;
	VLFWMain vlfw(args);

	KeyboardMain keyboard;
	MouseMain mouse;

	Monitor* primaryMonitor = Monitor::GetPrimaryMonitor();

	WindowHints hints {};
	hints.title = u8"Fractal Finder";
	hints.contextAPI = ContextAPI::OpenGL;
	hints.contextVersionMajor = 4;
	hints.contextVersionMinor = 3;
	hints.monitor = primaryMonitor;
	hints.raiseStopOnClose = true;
	hints.size = primaryMonitor->GetWorkingArea().size;
	auto window = Component<Window>::Create(0, hints);
	window->MakeContextCurrent();

	if (!gladLoadGLLoader((GLADloadproc)window->GetOpenGLProcessLoader()))
	{
		throw std::runtime_error("Failed to initialize glad");
	}

	game::Game g(window);

	ApplicationArgs appArgs {};
	vlk::Application::Start(appArgs);

	return 0;
}
