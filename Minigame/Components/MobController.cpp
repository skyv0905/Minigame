#include "MobController.h"
#include "Transform.h"
#include "Animator.h"
#include "SpriteRenderer.h"
#include "Bullet.h"
#include "../GameServices.h"
#include "../SoundPlayer.h"
#include "../Scene.h"
#include <raymath.h>

namespace Minigame::Components
{
    MobController::MobController(GameObject& owner, GameServices& gameServices) : Controller(owner, gameServices)
    {
    }

    void MobController::Start()
    {
        Controller::Start();
        canFire = false;
        fireTimer = gameServices.timer.SetTimeout(fireCooldown, [this]()
            {
                OnFireCooldownEnd();
            });

        DisableCollider();
        if (animator)
        {
            animator->Play("Summon");
        }
        isOnRegen = true;
    }

    void MobController::Update(float deltaTime)
    {
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
        }
        if (isHit)
        {
            if (!animator || animator->IsFinished())
            {
                if (health && health->IsDead())
                {
                    owner.GetScene().DestroyGameObject(owner);
                }
                else
                {
                    isHit = false;
                    if (animator)
                    {
                        animator->PlayDefaultState();
                    }
                }
            }
            return;
        }

        if (transform == nullptr)
            return;

        if (target == nullptr)
        {
            target = owner.GetScene().FindGameObjectWithTag("Player");
            if (target == nullptr)
                return;
        }

        auto* targetTransform = target->GetComponent<Minigame::Components::Transform>();
        bool targetDetected = false;

        if (targetTransform && !isHit)
        {
            Vector2 position = transform->GetPosition();
            Vector2 targetPosition = targetTransform->GetPosition();

            Vector2 direction = Vector2Subtract(targetPosition, position);
            float distanceSquare = Vector2LengthSqr(direction);
            targetDetected = distanceSquare <= detectionRangeSquare;
            if (targetDetected)
            {
                if (spriteRenderer)
                {
                    if (direction.x > 0)
                    {
                        spriteRenderer->SetFlipX(true);
                    }
                    else
                    {
                        spriteRenderer->SetFlipX(false);
                    }
                }
                if (animator)
                {
                    animator->Play(owner.GetName() + "_Move");
                }
                if (Vector2LengthSqr(direction) > 0.0f)
                {
                    direction = Vector2Normalize(direction);
                    position.x += direction.x * moveSpeed * deltaTime;
                    position.y += direction.y * moveSpeed * deltaTime;

                    transform->SetPosition(position);

                    forward = direction;
                }
            }
            else
            {
                if (animator)
                {
                    animator->Play(owner.GetName() + "_Stand");
                }
            }
        }

        if (canFire && targetDetected)
        {
            canFire = false;
            fireTimer = gameServices.timer.SetTimeout(fireCooldown, [this]()
                {
                    OnFireCooldownEnd();
                });
            Fire();
        }
    }

    void MobController::OnCollisionEnter(const CollisionInfo& info)
    {
        if (info.other.ContainsTag("Wall") || info.other.ContainsTag("Mob") || info.other.ContainsTag("Player"))
        {
            if (transform == nullptr)
                return;

            Vector2 position = transform->GetPosition();
            position.x += info.collisionDirection.x * info.collisionDepth;
            position.y += info.collisionDirection.y * info.collisionDepth;

            transform->SetPosition(position);
        }
        else if (info.other.ContainsTag("Bullet"))
        {
			auto* bullet = info.other.GetComponent<Bullet>();
			if (bullet && bullet->GetCreatedFromInfo() == CreatedFromInfo::Player)
            {
                isHit = true;
                bool dead = false;
                if (health)
                {
					health->Hit(bullet->GetAttackPower());
                    dead = health->IsDead();
                }
                if (dead)
                {
                    if (animator)
                    {
                        animator->Play(owner.GetName() + "_Die");
                        gameServices.sounds.Play(owner.GetName() + "_Die.mp3");
                    }
                    DisableCollider();
                }
                else
                {
                    if (animator)
                    {
                        animator->Play(owner.GetName() + "_Hit", true);
                    }
                    gameServices.sounds.Play(owner.GetName() + "_Hit.mp3");

                }
            }
        }
    }

    void MobController::SetDetectionRange(float range)
    {
        detectionRangeSquare = range * range;
    }
}
