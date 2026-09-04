#include "NetworkInputSender.h"
#include "PlayerController.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../GameSession.h"
#include "../InputManager.h"
#include "../Network/NetworkClient.h"
#include "Network/Packets.h"
#include <algorithm>
#include <cmath>

namespace Minigame::Components
{
    NetworkInputSender::NetworkInputSender(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void NetworkInputSender::Update(float deltaTime)
    {
        if (!gameServices.session.HasNetworkMatch() || gameServices.session.GetGameState() != GameState::GamePlaying || !owner.ContainsTag("LocalPlayer"))
            return;

        if (const auto* controller = owner.GetComponent<Minigame::Components::PlayerController>())
        {
            if (controller->GetFireSequence() > lastSentFireSequence)
            {
                SendFire(*controller);
            }
        }
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
        Vector2 direction = gameServices.input.GetMoveAxis();
        const float lengthSquared = direction.x * direction.x + direction.y * direction.y;
        if (lengthSquared > 1.0f)
        {
            const float inverseLength = 1.0f / std::sqrt(lengthSquared);
            direction.x *= inverseLength;
            direction.y *= inverseLength;
        }

        Minigame::Network::PlayerInputPacket packet{};
        packet.sequence = nextSequence;
        packet.moveX = direction.x;
        packet.moveY = direction.y;

        if (!gameServices.network.SendPacket(packet, Minigame::Network::PacketSendType::Unreliable, Minigame::Network::PacketChannelType::Gameplay))
            return false;

        nextSequence++;
        return true;
    }

    bool NetworkInputSender::SendFire(const PlayerController& controller)
    {
        const auto* transform = owner.GetComponent<Minigame::Components::Transform>();
        if (transform == nullptr)
            return false;

        const Vector2 position = transform->GetPosition();
        const Vector2 direction = controller.GetForward();
        Minigame::Network::PlayerFirePacket packet{};
        packet.fireSequence = controller.GetFireSequence();
        packet.clientTick = gameServices.network.GetEstimatedServerTick();
        packet.positionX = Minigame::Network::EncodePosition(position.x);
        packet.positionY = Minigame::Network::EncodePosition(position.y);
        packet.directionX = Minigame::Network::EncodeDirection(direction.x);
        packet.directionY = Minigame::Network::EncodeDirection(direction.y);

        if (!gameServices.network.SendPacket(packet, Minigame::Network::PacketSendType::Reliable, Minigame::Network::PacketChannelType::Gameplay))
            return false;

        lastSentFireSequence = packet.fireSequence;
        return true;
    }

}
