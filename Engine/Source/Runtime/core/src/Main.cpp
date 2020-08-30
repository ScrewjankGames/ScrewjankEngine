// STD Headers
#include <iostream>

// Engine Headers
#include "core/Game.hpp"

int main(int arc, char** argv) 
{
	auto game = Screwjank::CreateGame();
	game->Run();
	delete game;
}

