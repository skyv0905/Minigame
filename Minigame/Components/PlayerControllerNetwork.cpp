#include "PlayerControllerNetwork.h"
#include "Transform.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../GameSession.h"
#include "../Network/NetworkClient.h"

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
        if (!worldState || (hasAppliedState && worldState->serverTick == lastAppliedServerTick))
            return;

        for (std::uint32_t i = 0; i < worldState->playerCount; i++)
        {
            const auto& player = worldState->players[i];
            if (player.playerId != playerId)
                continue;

            transform->SetPosition(Vector2{ player.positionX, player.positionY });
            lastAppliedServerTick = worldState->serverTick;
            hasAppliedState = true;
            return;
        }
    }

    void PlayerControllerNetwork::SetPlayerId(std::uint32_t id)
    {
        playerId = id;
    }
}
