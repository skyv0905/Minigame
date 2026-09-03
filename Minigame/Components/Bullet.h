#pragma once
#include <raylib.h>
#include "Component.h"
#include "PlayerController.h"
#include "Transform.h"
#include "Animator.h"
#include "Collider.h"
#include "../IDGenerator.h"

namespace Minigame::Components
{
	class Bullet : public Component
	{
	public:
		explicit Bullet(GameObject& owner);

		void Awake() override;
		void Start() override;
		void Update(float deltaTime) override;
		void OnCollisionEnter(const CollisionInfo& info) override;

		void SetMaxDistance(float dist);
		void SetMoveSpeed(float speed);
		void SetDirection(Vector2 dir);
		void SetCreatedFrom(GameObjectID id);
		GameObjectID GetCreatedFrom() const;
		void SetAttackPower(float power);
		float GetAttackPower() const;
		void SetNetworkObjectId(std::uint32_t id);
		std::uint32_t GetNetworkObjectId() const;
		void SetServerAuthoritative(bool value);
		void SetFireSequence(std::uint32_t sequence);
		std::uint32_t GetFireSequence() const;

	private:
		Minigame::Components::Transform* transform = nullptr;
		Minigame::Components::Animator* animator = nullptr;

		float movedDistance = 0.0f;
		float maxDistance = 300.0f;
		float moveSpeed = 500.0f;
		float attackPower = 0.0f;
		Vector2 direction{ 0.0f, 0.0f };
		GameObjectID createdFrom = 0;
		std::uint32_t networkObjectId = 0;
		std::uint32_t fireSequence = 0;
		bool serverAuthoritative = false;

		void InitRotation();
		bool rotationInited = false;

		bool isHit = false;
	};
}
