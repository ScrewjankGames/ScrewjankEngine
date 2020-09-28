// STD Headers

// Engine Headers
#include <ScrewjankEngine.hpp>

class SampleGame : public sj::Game
{
  public:
    SampleGame() = default;
    ~SampleGame() = default;
};

sj::Game* sj::CreateGame()
{
    return new SampleGame();
}
