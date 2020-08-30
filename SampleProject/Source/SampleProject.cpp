// STD Headers

// Engine Headers
#include <ScrewjankEngine.hpp>

class SampleGame : public Screwjank::Game {
public:
	SampleGame() = default;
	~SampleGame() = default;
};

Screwjank::Game* Screwjank::CreateGame() 
{
	return new SampleGame();
}