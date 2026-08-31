#include "Controller.h"
#include "Bullet.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include "../Scene.h"
#include <format>

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

    float Controller::GetFinalMoveSpeed() const
    {
        return std::min(moveSpeed * moveSpeedMultiplier / 100.0f, 700.0f);
    }

    float Controller::GetFinalBulletSpeed() const
    {
        return std::min(bulletSpeed * bulletSpeedMultiplier / 100.0f, 900.0f);
    }

    float Controller::GetFinalBulletDistance() const
    {
        return bulletDistance * bulletDistanceMultiplier / 100.0f;
    }

    float Controller::GetFinalAttackPower() const
    {
        return attackPower * attackPowerMultiplier / 100.0f;
    }

    float Controller::GetFireCooldown() const
    {
        return fireCooldown;
    }

    std::string Controller::GetMoveSpeedDetail() const
    {
        return moveSpeedMultiplier == 100 ? ""
            : std::format("( {:.0f} x {:.2f} )", moveSpeed, moveSpeedMultiplier / 100.0f);
    }

    std::string Controller::GetBulletSpeedDetail() const
    {
        return bulletSpeedMultiplier == 100 ? ""
            : std::format("( {:.0f} x {:.2f} )", bulletSpeed, bulletSpeedMultiplier / 100.0f);
    }

    std::string Controller::GetBulletDistanceDetail() const
    {
        return bulletDistanceMultiplier == 100 ? ""
            : std::format("( {:.0f} x {:.2f} )", bulletDistance, bulletDistanceMultiplier / 100.0f);
    }

    std::string Controller::GetAttackPowerDetail() const
    {
        return attackPowerMultiplier == 100 ? ""
            : std::format("( {:.1f} x {:.2f} )", attackPower, attackPowerMultiplier / 100.0f);
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
            bulletComponent->SetCreatedFrom(owner.GetID());
			bulletComponent->SetAttackPower(GetFinalAttackPower());
            bulletComponent->SetMaxDistance(GetFinalBulletDistance());
            bulletComponent->SetMoveSpeed(GetFinalBulletSpeed());
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
