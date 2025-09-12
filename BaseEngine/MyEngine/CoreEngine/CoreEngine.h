#pragma once
#include "pch/pch.h"

namespace FrameWork
{
	class CoreEngine
	{
	public:
		CoreEngine();
		~CoreEngine();

		void Initialize();
		void Update();
		void Destroy();
		void GameLoop();


	private:
		bool game_running;
	};
}
