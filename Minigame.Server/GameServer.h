#pragma once
#include <memory>

namespace Minigame::Server
{
	class GameServer
	{
	public:
		GameServer();
		~GameServer();

		void Run();
		void Stop();

	private:
		class Impl;
		std::unique_ptr<Impl> impl;

		void Loop();
		void GetPackets();
		void Update();
	};
}