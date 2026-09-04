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
		const float frameMoveDistance = Vector2Distance(oldPosition, newPosition);
		if (correctingPosition)
		{
			correctionElapsed += deltaTime;
			const float amount = correctionDuration > 0.0f ? Clamp(correctionElapsed / correctionDuration, 0.0f, 1.0f) : 1.0f;
			const Vector2 correction{ correctionOffset.x * amount, correctionOffset.y * amount };
			newPosition.x += correction.x - appliedCorrection.x;
			newPosition.y += correction.y - appliedCorrection.y;
			appliedCorrection = correction;
			correctingPosition = amount < 1.0f;
		}

		transform->SetPosition(newPosition);

		movedDistance += frameMoveDistance;

		if (movedDistance >= maxDistance)
		{
			if (serverAuthoritative && networkObjectId != 0)
			{
				moveSpeed = 0.0f;
			}
			else
			{
				owner.GetScene().DestroyGameObject(owner);
			}
		}
	}

	void Bullet::OnCollisionEnter(const CollisionInfo& info)
	{
		if (serverAuthoritative && !info.other.ContainsTag("Wall"))
			return;

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

	void Bullet::OnServerHit(Vector2 position)
	{
		if (isHit)
			return;

		if (transform)
		{
			transform->SetPosition(position);
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

	void Bullet::StartPositionCorrection(Vector2 targetPosition, float duration)
	{
		const Transform* currentTransform = transform != nullptr ? transform : owner.GetComponent<Transform>();
		if (currentTransform == nullptr)
			return;

		const Vector2 position = currentTransform->GetPosition();
		correctionOffset = Vector2{ targetPosition.x - position.x, targetPosition.y - position.y };
		appliedCorrection = Vector2{ 0.0f, 0.0f };
		correctionElapsed = 0.0f;
		correctionDuration = duration;
		correctingPosition = correctionOffset.x != 0.0f || correctionOffset.y != 0.0f;
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

	void Bullet::SetNetworkObjectId(std::uint32_t id)
	{
		networkObjectId = id;
	}

	std::uint32_t Bullet::GetNetworkObjectId() const
	{
		return networkObjectId;
	}

	void Bullet::SetServerAuthoritative(bool value)
	{
		serverAuthoritative = value;
	}

	void Bullet::SetFireSequence(std::uint32_t sequence)
	{
		fireSequence = sequence;
	}

	std::uint32_t Bullet::GetFireSequence() const
	{
		return fireSequence;
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
