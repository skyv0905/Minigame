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

    void MobControllerNetwork::Update(float deltaTime)
    {
        if (!gameServices.session.HasNetworkMatch() || transform == nullptr || objectId == 0)
            return;

        const auto worldState = gameServices.network.GetLatestWorldState();
        if (worldState && (!hasAppliedState || worldState->serverTick != lastAppliedServerTick))
        {
            for (std::uint32_t i = 0; i < worldState->mobCount; i++)
            {
                const auto& mob = worldState->mobs[i];
                if (mob.objectId != objectId)
                    continue;

                positionInterpolator.AddSnapshot(worldState->serverTick, Vector2{ Minigame::Network::DecodePosition(mob.positionX), Minigame::Network::DecodePosition(mob.positionY) });
                for (const auto& player : worldState->players)
                {
                    if (player.playerId != mob.targetPlayerId)
                        continue;

                    const int targetDirectionX = static_cast<int>(player.positionX) - static_cast<int>(mob.positionX);
                    if (spriteRenderer && targetDirectionX != 0)
                        spriteRenderer->SetFlipX(targetDirectionX > 0);
                    break;
                }

                lastAppliedServerTick = worldState->serverTick;
                hasAppliedState = true;
                break;
            }
        }

        const Vector2 previous = transform->GetPosition();
        Vector2 position{};
        if (!positionInterpolator.Update(deltaTime, position))
            return;

        transform->SetPosition(position);
        const bool isMoving = position.x != previous.x || position.y != previous.y;
        if (animator && isOnRegen)
        {
            if (!animator->IsFinished())
            {
                return;
            }
            isOnRegen = false;
        }
        if (animator)
            animator->Play(isMoving ? owner.GetName() + "_Move" : owner.GetName() + "_Stand");
    }

    void MobControllerNetwork::SetObjectId(std::uint32_t id)
    {
        objectId = id;
    }

    std::uint32_t MobControllerNetwork::GetObjectId() const
    {
        return objectId;
    }
}
