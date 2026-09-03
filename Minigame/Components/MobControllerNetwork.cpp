#include "MobControllerNetwork.h"
#include "Animator.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "Collider.h"
#include "../Scene.h"
#include "../GameObject.h"
#include "../GameServices.h"
#include "../GameSession.h"
#include "../SoundPlayer.h"
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
        {
            animator->Play("Summon");
        }
    }

    void MobControllerNetwork::Update(float deltaTime)
    {
        if (!gameServices.session.HasNetworkMatch() || transform == nullptr || objectId == 0)
            return;

        /*
        if (isOnRegen)
        {
            if (!animator)
            {
                isOnRegen = false;
                EnableCollider();
            }
            else if (animator && animator->IsFinished())
            {
                animator->PlayDefaultState();
                isOnRegen = false;
                EnableCollider();
            }
            return;
        }*/
        if (isDead)
        {
            if (!animator || animator->IsFinished())
            {
                owner.GetScene().DestroyGameObject(owner);
            }
            return;
        }
        if (isHit)
        {
            if (!animator || animator->IsFinished())
            {
                isHit = false;
                if (animator)
                {
                    animator->PlayDefaultState();
                }
            }
        }

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
                    {
                        spriteRenderer->SetFlipX(targetDirectionX > 0);
                    }
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
        if (animator && !isHit)
        {
            animator->Play(isMoving ? owner.GetName() + "_Move" : owner.GetName() + "_Stand");
        }
    }

    void MobControllerNetwork::SetObjectId(std::uint32_t id)
    {
        objectId = id;
    }

    std::uint32_t MobControllerNetwork::GetObjectId() const
    {
        return objectId;
    }

    void MobControllerNetwork::MarkHit()
    {
        isHit = true;
        if (animator)
        {
            animator->Play(owner.GetName() + "_Hit", true);
            gameServices.sounds.Play(owner.GetName() + "_Hit.mp3");
        }
    }

    void MobControllerNetwork::MarkDead()
    {
        if (isDead)
            return;

        if (animator)
        {
            animator->Play(owner.GetName() + "_Die");
            gameServices.sounds.Play(owner.GetName() + "_Die.mp3");
        }
        DisableCollider();
        isDead = true;
    }

    void MobControllerNetwork::DisableCollider()
    {
        if (auto* collider = owner.GetComponent<Collider>())
        {
            collider->SetValid(false);
        }
    }

    void MobControllerNetwork::EnableCollider()
    {
        if (auto* collider = owner.GetComponent<Collider>())
        {
            collider->SetValid(true);
        }
    }
}
