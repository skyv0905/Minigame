#include "PlayerController.h"
#include "Bullet.h"
#include "../GameServices.h"
#include "../InputManager.h"
#include "../SoundPlayer.h"
#include "../Scene.h"
#include <algorithm>

namespace Minigame::Components
{
    PlayerController::PlayerController(GameObject& owner, GameServices& gameServices) : Controller(owner, gameServices)
    {
    }

    void PlayerController::Awake()
    {
        Controller::Awake();
        playerMessage = owner.GetComponent<PlayerMessage>();
    }

    void PlayerController::Start()
    {
        Controller::Start();
    }

    void PlayerController::Update(float deltaTime)
    {
        if (transform == nullptr)
            return;

        Vector2 direction = gameServices.input.GetMoveAxis();
        if (!(direction.x == 0.0f && direction.y == 0.0f))
        {
            Vector2 position = transform->GetPosition();
            position.x += direction.x * moveSpeed * deltaTime;
            position.y += direction.y * moveSpeed * deltaTime;

            transform->SetPosition(position);

            forward = direction;
        }

        if (gameServices.input.IsDown(InputAction::Fire))
        {
            if (canFire)
            {
                canFire = false;
                fireTimer = gameServices.timer.SetTimeout(fireCooldown, [this]()
                    {
                        OnFireCooldownEnd();
                    });
                Fire();
            }
        }
    }

    void PlayerController::OnCollisionEnter(const CollisionInfo& info)
    {
        if (info.other.ContainsTag("Wall") || info.other.ContainsTag("Player") || info.other.ContainsTag("Mob"))
        {
            if (transform == nullptr)
                return;

            Vector2 position = transform->GetPosition();
            position.x += info.collisionDirection.x * info.collisionDepth;
            position.y += info.collisionDirection.y * info.collisionDepth;

            transform->SetPosition(position);
        }
        else if (info.other.ContainsTag("PowerUp"))
        {
            owner.GetScene().DestroyGameObject(info.other);
            OnPowerUpCollected(info.other);
        }
        else if (info.other.ContainsTag("Bullet"))
        {
			auto* bullet = info.other.GetComponent<Bullet>();
			if (bullet && bullet->GetCreatedFromInfo() == CreatedFromInfo::Mob)
            {
                bool dead = false;
                if (health)
                {
					health->Hit(bullet->GetAttackPower());
                    dead = health->IsDead();
                }
                gameServices.sounds.Play("Hit.mp3");
                if (dead)
                {

                }
                else
                {

                }
            }
        }
    }

    void PlayerController::Fire()
    {
        Controller::Fire();
        gameServices.sounds.Play("Jump.mp3");
    }

    void PlayerController::OnPowerUpCollected(const GameObject& powerUp)
    {
        if (powerUp.ContainsTag("incSpeed"))
        {
            moveSpeed *= 1.1f;
            moveSpeed = std::min(moveSpeed, 700.0f);
			if (playerMessage) playerMessage->Show("이동 속도 증가");
        }
        if (powerUp.ContainsTag("incAttackPower"))
        {
            attackPower += 5.0f;
			if (playerMessage) playerMessage->Show("공격력 증가");
        }
        if (powerUp.ContainsTag("incAttackSpeed"))
        {
            fireCooldown *= 0.82f;
            fireCooldown = std::max(fireCooldown, 0.1f);
			if (playerMessage) playerMessage->Show("공격 속도 증가");
        }
        if (powerUp.ContainsTag("incBulletDistance"))
        {
            bulletDistance *= 1.1f;
			if (playerMessage) playerMessage->Show("사거리 증가");
        }
        if (powerUp.ContainsTag("incBulletSpeed"))
        {
            bulletSpeed *= 1.1f;
            bulletSpeed = std::min(bulletSpeed, 900.0f);
			if (playerMessage) playerMessage->Show("발사체 속도 증가");
        }
        if (powerUp.ContainsTag("heal20"))
        {
            if (health)
            {
                health->Heal(0.2f);
				if (playerMessage) playerMessage->Show("체력 20% 회복");
            }
        }
        if (powerUp.ContainsTag("heal40"))
        {
            if (health)
            {
                health->Heal(0.4f);
				if (playerMessage) playerMessage->Show("체력 40% 회복");
            }
        }
        gameServices.sounds.Play("UseShopItem.mp3");
    }
}
