#include "PlayerControllerNetwork.h"
#include "Transform.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../GameSession.h"
#include "../Network/NetworkClient.h"
#include <algorithm>

namespace Minigame::Components
{
    PlayerControllerNetwork::PlayerControllerNetwork(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void PlayerControllerNetwork::Awake()
    {
        transform = owner.GetComponent<Transform>();
    }

    void PlayerControllerNetwork::Update(float deltaTime)
    {
        if (!gameServices.session.HasNetworkMatch() || transform == nullptr || playerId == 0)
            return;

        const auto worldState = gameServices.network.GetLatestWorldState();
        if (worldState && (!hasAppliedState || worldState->serverTick != lastAppliedServerTick))
        {
            for (const auto& player : worldState->players)
            {
                if (player.playerId != playerId)
                    continue;

                const Vector2 serverPosition{ Minigame::Network::DecodePosition(player.positionX), Minigame::Network::DecodePosition(player.positionY) };
                if (owner.ContainsTag("LocalPlayer"))
                {
                    const Vector2 currentPosition = transform->GetPosition();
                    const Vector2 error{ serverPosition.x - currentPosition.x, serverPosition.y - currentPosition.y };
                    const float errorSquared = error.x * error.x + error.y * error.y;
                    if (errorSquared >= SnapDistance * SnapDistance)
                    {
                        transform->SetPosition(serverPosition);
                        pendingCorrection = Vector2{};
                    }
                    else if (errorSquared >= CorrectionDeadZone * CorrectionDeadZone)
                    {
                        pendingCorrection = error;
                    }
                    else
                    {
                        pendingCorrection = Vector2{};
                    }
                }
                else
                    positionInterpolator.AddSnapshot(worldState->serverTick, serverPosition);

                lastAppliedServerTick = worldState->serverTick;
                hasAppliedState = true;
                break;
            }
        }

        if (owner.ContainsTag("LocalPlayer"))
        {
            const float correctionAmount = std::clamp(CorrectionRate * deltaTime, 0.0f, 1.0f);
            const Vector2 appliedCorrection{ pendingCorrection.x * correctionAmount, pendingCorrection.y * correctionAmount };
            Vector2 position = transform->GetPosition();
            position.x += appliedCorrection.x;
            position.y += appliedCorrection.y;
            transform->SetPosition(position);
            pendingCorrection.x -= appliedCorrection.x;
            pendingCorrection.y -= appliedCorrection.y;
            return;
        }

        Vector2 interpolatedPosition{};
        if (positionInterpolator.Update(deltaTime, interpolatedPosition))
            transform->SetPosition(interpolatedPosition);
    }

    void PlayerControllerNetwork::SetPlayerId(std::uint32_t id)
    {
        playerId = id;
    }
}
