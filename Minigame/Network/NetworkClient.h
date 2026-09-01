#pragma once
#include <memory>
#include <string>

namespace Minigame::Network
{
	class NetworkClient
	{
	public:
		NetworkClient();
		~NetworkClient();

		NetworkClient(const NetworkClient&) = delete;
		NetworkClient& operator=(const NetworkClient&) = delete;

		bool Connect(const std::string& address, unsigned short port);
		void Disconnect();
		bool IsConnected() const;

		void Update(float deltaTime);
		void Test();

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
