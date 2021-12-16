// The C++20 modules are used as part of learning, with no expectations of better performance
// It is recommended by Microsoft to use modules over headers as much as possible in the new projects
//https://docs.microsoft.com/en-us/cpp/cpp/modules-cpp?view=msvc-160

import GameEngine;

int main(int argc, char* argv[])
{
	GameEngine yamaEngine;
	yamaEngine.InitEngine(argc, argv);

	yamaEngine.GameLoop();

	yamaEngine.Cleanup();
}