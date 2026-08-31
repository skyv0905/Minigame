#include "Controller.h"
#include "Bullet.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../Scene.h"

namespace Minigame::Components
{
    Controller::Controller(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    Controller::~Controller()
    {
        gameServices.timer.Cancel(fireTimer);
    }

    void Controller::Awake()
    {
        transform = owner.GetComponent<Minigame::Components::Transform>();
        animator = owner.GetComponent<Animator>();
        spriteRenderer = owner.GetComponent<Minigame::Components::SpriteRenderer>();
        health = owner.GetComponent<Minigame::Components::Health>();
    }

    void Controller::Start()
    {
	}

    void Controller::SetBulletPrefab(const std::string& prefabName)
    {
        bulletPrefab = prefabName;
    }

    void Controller::SetMoveSpeed(float speed)
    {
        moveSpeed = speed;
    }

    void Controller::SetBulletSpeed(float speed)
    {
        bulletSpeed = speed;
    }

    void Controller::SetBulletDistance(float dist)
    {
        bulletDistance = dist;
    }

    void Controller::SetBulletTint(Color color)
    {
        bulletTint = color;
    }

    void Controller::SetFireCooldown(float cooldown)
    {
        fireCooldown = cooldown;
    }

    void Controller::SetAttackPower(float power)
    {
        attackPower = power;
    }

    float Controller::GetAttackPower() const
    {
        return attackPower;
    }

    void Controller::Fire()
    {
        if (transform == nullptr)
            return;

        auto* bullet = owner.GetScene().Instantiate(bulletPrefab);
        if (bullet == nullptr)
            return;

        auto* bulletTransform = bullet->GetComponent<Minigame::Components::Transform>();
        if (bulletTransform)
        {
            bulletTransform->SetPosition(transform->GetPosition());
        }

        auto* bulletComponent = bullet->GetComponent<Minigame::Components::Bullet>();
        if (bulletComponent)
        {
			if (owner.ContainsTag("Player"))
			{
				bulletComponent->SetCreatedFromInfo(CreatedFromInfo::Player);
			}
			else if (owner.ContainsTag("Mob"))
			{
				bulletComponent->SetCreatedFromInfo(CreatedFromInfo::Mob);
			}
			bulletComponent->SetAttackPower(attackPower);
            bulletComponent->SetMaxDistance(bulletDistance);
            bulletComponent->SetMoveSpeed(bulletSpeed);
            bulletComponent->SetDirection(forward);
        }

        auto* bulletRenderer = bullet->GetComponent<Minigame::Components::SpriteRenderer>();
        if (bulletRenderer)
        {
            bulletRenderer->SetTint(bulletTint);
        }
    }

    void Controller::OnFireCooldownEnd()
    {
        canFire = true;
        fireTimer = 0;
    }

    void Controller::DisableCollider()
    {
        if (auto* collider = owner.GetComponent<Collider>())
        {
            collider->SetValid(false);
        }
    }

    void Controller::EnableCollider()
    {
        if (auto* collider = owner.GetComponent<Collider>())
        {
            collider->SetValid(true);
        }
    }
}
