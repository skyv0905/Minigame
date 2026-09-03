#include "PlayerController.h"
#include "Bullet.h"
#include "TextUI.h"
#include "../GameServices.h"
#include "../GameSession.h"
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
        playerMessage = owner.GetComponent<Minigame::Components::PlayerMessage>();
        exp = owner.GetComponent<Minigame::Components::Exp>();
        if (exp)
        {
            exp->SetOnLevelUp([this](int level)
                {
                    OnLevelUp(level);
                });
        }
    }

    void PlayerController::Start()
    {
        Controller::Start();
    }

    void PlayerController::Update(float deltaTime)
    {
        if (gameServices.session.GetGameState() != GameState::GamePlaying)
            return;

        if (gameServices.session.HasNetworkMatch() && !owner.ContainsTag("LocalPlayer"))
            return;

        if (isDead)
            return;

        if (transform == nullptr)
            return;

        Vector2 direction = gameServices.input.GetMoveAxis();
        if (!(direction.x == 0.0f && direction.y == 0.0f))
        {
            const float finalMoveSpeed = GetFinalMoveSpeed();
            Vector2 position = transform->GetPosition();
            position.x += direction.x * finalMoveSpeed * deltaTime;
            position.y += direction.y * finalMoveSpeed * deltaTime;

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
        if (gameServices.session.GetGameState() != GameState::GamePlaying)
            return;

        if (isDead)
            return;

        if (gameServices.session.HasNetworkMatch() && (info.other.ContainsTag("Player") || info.other.ContainsTag("Mob")))
        {
            //return;
        }

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
            if (auto* bullet = info.other.GetComponent<Bullet>())
            {
                if (auto* bulletFrom = owner.GetScene().FindGameObjectByID(bullet->GetCreatedFrom()))
                {
                    if (bulletFrom->ContainsTag("Mob"))
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
                            OnDeath();
                        }
                    }
                }
            }
        }
    }

    void PlayerController::OnLevelUp(int level)
    {
        float m = level % 5 == 0 ? 2.0f : 1.0f;
        attackPower += 5.0f * m;
        moveSpeed += 10.0f * m;
        bulletDistance += 5.0f * m;
        bulletSpeed += 10.0f * m;
        gameServices.sounds.Play("LevelUp.mp3");
    }

    void PlayerController::Fire()
    {
        Controller::Fire();
        gameServices.sounds.Play("Jump.mp3");
    }

    void PlayerController::OnDeath()
    {
        if (isDead)
            return;

        isDead = true;
        DisableCollider();

        owner.GetScene().Instantiate("RestartButton");
        owner.GetScene().Instantiate("MainMenuButton");
        if (GameObject* textUIGameObject = owner.GetScene().Instantiate("TextUI"))
        {
            if (TextUI* textUI = textUIGameObject->GetComponent<TextUI>())
            {
                textUI->SetText("저런...");
            }
        }

        gameServices.session.SetGameState(GameState::GameOver);
    }

    void PlayerController::OnPowerUpCollected(const GameObject& powerUp)
    {
        if (powerUp.ContainsTag("incSpeed"))
        {
            moveSpeedMultiplier += 25;
			if (playerMessage) playerMessage->Show("이동 속도 증가");
        }
        if (powerUp.ContainsTag("incAttackPower"))
        {
            attackPowerMultiplier += 5;
			if (playerMessage) playerMessage->Show("공격력 증가");
        }
        if (powerUp.ContainsTag("incAttackSpeed"))
        {
            if (fireCooldown > 0.1f)
            {
                fireCooldown *= 0.82f;
                fireCooldown = std::max(fireCooldown, 0.1f);
                attackPowerMultiplier -= 5;
                attackPowerMultiplier = std::max(attackPowerMultiplier, 10);
                if (playerMessage) playerMessage->Show("공격 속도 증가");
            }
            else
            {
                if (playerMessage) playerMessage->Show("최대 공격 속도");
            }
        }
        if (powerUp.ContainsTag("incBulletDistance"))
        {
            bulletDistanceMultiplier += 25;
			if (playerMessage) playerMessage->Show("사거리 증가");
        }
        if (powerUp.ContainsTag("incBulletSpeed"))
        {
            bulletSpeedMultiplier += 25;
			if (playerMessage) playerMessage->Show("발사체 속도 증가");
        }
        if (powerUp.ContainsTag("heal1"))
        {
            if (health)
            {
                health->Heal(0.15f);
				if (playerMessage) playerMessage->Show("체력 15% 회복");
            }
        }
        if (powerUp.ContainsTag("heal2"))
        {
            if (health)
            {
                health->Heal(0.3f);
				if (playerMessage) playerMessage->Show("체력 30% 회복");
            }
        }
        gameServices.sounds.Play("UseShopItem.mp3");
    }
}
