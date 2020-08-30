#pragma once

// STD Headers

// Library Headers

// Engine Headers

namespace Screwjank {

	class Game {
	public:
		Game() = default;
		virtual ~Game() = default;

		void Run();

	protected:
		float m_FrameTime;
	};

	extern Game* CreateGame();
}