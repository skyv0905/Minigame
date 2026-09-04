#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <string>
#include <utility>
#include <cstdint>

#include "Components/Component.h"
#include "IDGenerator.h"

class Scene;
struct CollisionInfo;

class GameObject
{
public:
    GameObject(Scene& scene, GameObjectID id, const std::string& name);
    virtual ~GameObject() = default;

    virtual void Awake();
    virtual void Start();
    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void DrawUI();

    const std::string& GetName() const;
    GameObjectID GetID() const;
    std::uint32_t GetNetworkObjectId() const;
    void SetNetworkObjectId(std::uint32_t objectId);
    int GetZOrder() const;
    void SetZOrder(int zOrder);
    Scene& GetScene();
    const Scene& GetScene() const;

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args);
    template<typename T>
    T* GetComponent();

    void OnCollisionEnter(const Minigame::Components::CollisionInfo& info);

    void AddTag(const std::string& tag);
    bool ContainsTag(const std::string& tag) const;

private:
    Scene& scene;
    GameObjectID id;
    std::uint32_t networkObjectId = 0;
    std::string name;
    int zOrder = 0;

    std::vector<std::unique_ptr<Minigame::Components::Component>> components;
    std::unordered_set<std::string> tags;
};

template<typename T, typename... Args>
T& GameObject::AddComponent(Args&&... args)
{
    auto component = std::make_unique<T>(*this, std::forward<Args>(args)...);

    T& reference = *component;

    components.push_back(std::move(component));

    return reference;
}

template<typename T>
T* GameObject::GetComponent()
{
    for (auto& component : components)
    {
        if (auto result = dynamic_cast<T*>(component.get()))
        {
            return result;
        }
    }

    return nullptr;
}
