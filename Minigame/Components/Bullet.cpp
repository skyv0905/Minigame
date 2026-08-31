#include "Bullet.h"
#include "../GameObject.h"
#include "../Scene.h"
#include <raymath.h>

namespace Minigame::Components
{
	Bullet::Bullet(GameObject& owner) : Component(owner)
	{
	}

	void Bullet::Awake()
	{
		transform = owner.GetComponent<Minigame::Components::Transform>();
		animator = owner.GetComponent<Animator>();
	}

	void Bullet::Start()
	{
		InitRotation();
	}

	void Bullet::Update(float deltaTime)
	{
		if (isHit)
		{
			if (!animator || animator->IsFinished())
			{
				owner.GetScene().DestroyGameObject(owner);
			}
			return;
		}

		if (transform == nullptr)
			return;

		Vector2 newPosition = transform->GetPosition();
		Vector2 oldPosition = newPosition;
		newPosition.x += direction.x * moveSpeed * deltaTime;
		newPosition.y += direction.y * moveSpeed * deltaTime;

		transform->SetPosition(newPosition);

		movedDistance += Vector2Distance(oldPosition, newPosition);

		if (movedDistance >= maxDistance)
		{
			owner.GetScene().DestroyGameObject(owner);
		}
	}

	void Bullet::OnCollisionEnter(const CollisionInfo& info)
	{
		if (isHit || info.other.ContainsTag("Bullet") || info.other.ContainsTag("PowerUp"))
		{
			return;
		}

		GameObject* from = owner.GetScene().FindGameObjectByID(createdFrom);
		if (from &&
			((from->ContainsTag("Player") && info.other.ContainsTag("Player")) ||
			(from->ContainsTag("Mob") && info.other.ContainsTag("Mob"))))
		{
			return;
		}

		moveSpeed = 0.0f;
		isHit = true;

		if (animator)
		{
			animator->Play("Bullet_Hit", true);
		}

		if (auto* collider = owner.GetComponent<Collider>())
		{
			collider->SetValid(false);
		}
	}

	void Bullet::SetMaxDistance(float dist)
	{
		maxDistance = dist;
	}

	void Bullet::SetMoveSpeed(float speed)
	{
		moveSpeed = speed;
	}

	void Bullet::SetDirection(Vector2 dir)
	{
		direction = dir;
		InitRotation();
	}

	void Bullet::SetCreatedFrom(GameObjectID id)
	{
		createdFrom = id;
	}

	GameObjectID Bullet::GetCreatedFrom() const
	{
		return createdFrom;
	}

	void Bullet::SetAttackPower(float power)
	{
		attackPower = power;
	}

	float Bullet::GetAttackPower() const
	{
		return attackPower;
	}

	void Bullet::InitRotation()
	{
		if (transform && direction != Vector2Zero() && !rotationInited)
		{
			auto angle = Vector2Angle(Vector2{ -1.0f, 0.0f }, direction) * RAD2DEG;
			transform->SetRotation(angle);
			rotationInited = true;
		}
	}
}
