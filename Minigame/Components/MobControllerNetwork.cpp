#include "MobControllerNetwork.h"
#include "Animator.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "../GameObject.h"
#include "../GameServices.h"
#include "../GameSession.h"
#include "../Network/NetworkClient.h"

namespace Minigame::Components
{
    MobControllerNetwork::MobControllerNetwork(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void MobControllerNetwork::Awake()
    {
        transform = owner.GetComponent<Transform>();
        animator = owner.GetComponent<Animator>();
        spriteRenderer = owner.GetComponent<SpriteRenderer>();
    }

    void MobControllerNetwork::Start()
    {
        if (!gameServices.session.HasNetworkMatch())
            return;

        if (animator)
            animator->Play("Summon");
    }

    void MobControllerNetwork::Update(float)
    {
        if (!gameServices.session.HasNetworkMatch() || transform == nullptr || objectId == 0)
            return;

        const auto worldState = gameServices.network.GetLatestWorldState();
        if (!worldState || (hasAppliedState && worldState->serverTick == lastAppliedServerTick))
            return;

        for (std::uint32_t i = 0; i < worldState->mobCount; i++)
        {
            const auto& mob = worldState->mobs[i];
            if (mob.objectId != objectId)
                continue;

            const Vector2 previous = transform->GetPosition();
            const Vector2 position{ static_cast<float>(mob.positionX), static_cast<float>(mob.positionY) };
            transform->SetPosition(position);
            const float moveX = position.x - previous.x;
            const float moveY = position.y - previous.y;
            const bool isMoving = moveX != 0.0f || moveY != 0.0f;
            for (std::size_t playerIndex = 0; playerIndex < worldState->players.size(); playerIndex++)
            {
                const auto& player = worldState->players[playerIndex];
                if (player.playerId != mob.targetPlayerId)
                    continue;

                const int targetDirectionX = static_cast<int>(player.positionX) - static_cast<int>(mob.positionX);
                if (spriteRenderer && targetDirectionX != 0.0f)
                    spriteRenderer->SetFlipX(targetDirectionX > 0.0f);
                break;
            }
            if (animator && isOnRegen)
            {
                if (!animator->IsFinished())
                {
                    lastAppliedServerTick = worldState->serverTick;
                    hasAppliedState = true;
                    return;
                }
                isOnRegen = false;
            }
            if (animator)
                animator->Play(isMoving ? owner.GetName() + "_Move" : owner.GetName() + "_Stand");

            lastAppliedServerTick = worldState->serverTick;
            hasAppliedState = true;
            return;
        }
    }

    void MobControllerNetwork::SetObjectId(std::uint32_t id)
    {
        objectId = id;
    }
}
