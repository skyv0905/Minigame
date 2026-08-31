#pragma once

class GameObject;

namespace Minigame::Components
{
    struct CollisionInfo;

    class Component
    {
    public:
        explicit Component(GameObject& owner) : owner(owner)
        {
        }

        virtual ~Component() = default;

        virtual void Awake() {}
        virtual void Start() {}
        virtual void Update(float deltaTime) {}
        virtual void Draw() {}
        virtual void DrawUI() {}

        GameObject& GetOwner()
        {
            return owner;
        }

        const GameObject& GetOwner() const
        {
            return owner;
        }

        virtual void OnCollisionEnter(const CollisionInfo& info) {}

    protected:
        GameObject& owner;
    };
}