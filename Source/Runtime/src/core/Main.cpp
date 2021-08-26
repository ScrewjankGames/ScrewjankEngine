// STD Headers
#include <iostream>

// Engine Headers
#include <core/Game.hpp>

int main(int arc, char** argv)
{
    auto game = sj::CreateGame();
    game->Start();
    delete game;
}
