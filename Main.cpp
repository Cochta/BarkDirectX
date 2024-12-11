#include "SFMLApp.h"

const char* Name = "Samples SFML";

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
	SFMLApp app(Name, Metrics::Width, Metrics::Height);

	app.SetUp();

	app.Run();

	app.TearDown();

	return EXIT_SUCCESS;
}