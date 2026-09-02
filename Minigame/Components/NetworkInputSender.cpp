#include "NetworkInputSender.h"
#include "../GameServices.h"
#include "../GameSession.h"
#include "../InputManager.h"
#include "../Network/NetworkClient.h"
#include "Network/Packets.h"
#include <algorithm>

namespace Minigame::Components
{
    NetworkInputSender::NetworkInputSender(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void NetworkInputSender::Update(float deltaTime)
    {
        if (!gameServices.session.HasNetworkMatch())
            return;

        fireBuffered = fireBuffered || gameServices.input.IsPressed(InputAction::Fire) || gameServices.input.IsDown(InputAction::Fire);
        sendAccumulator = std::min(sendAccumulator + deltaTime, SendInterval * MaxSendsPerFrame);

        int sendCount = 0;
        while (sendAccumulator >= SendInterval && sendCount < MaxSendsPerFrame)
        {
            if (!SendInput())
                break;

            sendAccumulator -= SendInterval;
            sendCount++;
        }
    }

    bool NetworkInputSender::SendInput()
    {
        const Vector2 direction = gameServices.input.GetMoveAxis();

        Minigame::Network::PlayerInputPacket packet{};
        packet.sequence = nextSequence;
        packet.moveX = direction.x;
        packet.moveY = direction.y;
        packet.fire = fireBuffered || gameServices.input.IsDown(InputAction::Fire);

        if (!gameServices.network.SendPacket(packet, Minigame::Network::PacketSendType::Unreliable, Minigame::Network::PacketChannelType::Gameplay))
            return false;

        nextSequence++;
        fireBuffered = false;
        return true;
    }
}
