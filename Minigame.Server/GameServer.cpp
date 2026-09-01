#include "GameServer.h"
#include <enet/enet.h>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>

namespace Minigame::Server
{
    class GameServer::Impl
    {
    public:
        bool initialized = false;
        std::atomic_bool running = false;

        ENetHost* server = nullptr;
    };

	GameServer::GameServer() : impl(std::make_unique<Impl>())
	{
        if (enet_initialize() == 0)
        {
            impl->initialized = true;

            ENetAddress address{};
            address.host = ENET_HOST_ANY;
            address.port = 5000;

            impl->server = enet_host_create(&address, 2, 2, 0, 0);
        }
	}

    GameServer::~GameServer()
    {
        if (impl->initialized)
        {
            if (impl->server)
            {
                enet_host_destroy(impl->server);
                impl->server = nullptr;
            }
            enet_deinitialize();
            impl->initialized = false;
            std::cout << "Server Closed\n";
        }
    }

	void GameServer::Run()
	{
        if (!impl->initialized)
        {
            std::cerr << "Failed to Initialize ENet\n";
            return;
        }
        if (impl->server == nullptr)
        {
            std::cerr << "Failed to Create Server Host\n";
            return;
        }

        impl->running.store(true);
        std::cout << "Server Started at 5000\n";

        std::thread consoleThread([this]()
            {
                std::string command;

                while (impl->running.load() && std::getline(std::cin, command))
                {
                    if (command == "stop")
                    {
                        Stop();
                        break;
                    }

                    std::cout << "Unknown Command: " << command << '\n';
                }
            });

        while (impl->running.load())
        {
            Loop();
        }

        if (consoleThread.joinable())
        {
            consoleThread.join();
        }
	}

    void GameServer::Stop()
    {
        impl->running.store(false);
    }

	void GameServer::Loop()
	{
        GetPackets();
        Update();
	}

    void GameServer::GetPackets()
    {
        ENetEvent event{};
        while (enet_host_service(impl->server, &event, 10) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "Client Connected\n";
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::cout << "Packed Received: " << event.packet->dataLength << " bytes\n";
                std::string message(reinterpret_cast<const char*>(event.packet->data), event.packet->dataLength);
                std::cout << "Message: " << message << "\n";
                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "Client Disconnected\n";
                break;
            }

            default:
                break;
            }
        }
    }

	void GameServer::Update()
	{
	}
}