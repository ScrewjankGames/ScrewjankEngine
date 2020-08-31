#pragma once
// STD Headers

// Library Headers

// Engine Headers

namespace Screwjank {

	class Game {
	public:
		/**
		 * Constructor
		 */
		Game();

		/**
		 * Destructor 
		 */
		virtual ~Game();

		/**
		 * Main game loop
		 */
		void Run();

	protected:
		/** Current frame time */
		float m_DeltaTime;
	};

	// API function externed to allow users to create custom game classes for main
	extern Game* CreateGame();
}