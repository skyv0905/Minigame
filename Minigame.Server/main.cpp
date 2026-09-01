#include <enet/enet.h>
#include <iostream>

int main()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet\n";
        return 1;
    }

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = 5000;

    ENetHost* server = enet_host_create(&address, 2, 2, 0, 0);

    if (server == nullptr)
    {
        std::cerr << "Failed to create Server Host\n";
        enet_deinitialize();
        return 1;
    }

    std::cout << "Server Started at 5000\n";

    bool running = true;
    ENetEvent event{};

    while (running)
    {
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Client Connected\n";
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Packed Received: " << event.packet->dataLength << " bytes\n";
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Client Disconnected\n";
                break;

            default:
                break;
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    
    return 0;
}
