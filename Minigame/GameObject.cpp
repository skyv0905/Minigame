#include "GameObject.h"
#include "Components/Collider.h"

GameObject::GameObject(Scene& scene, const std::string& name) : scene(scene), name(name)
{
}

void GameObject::Awake()
{
    for (auto& component : components)
    {
        component->Awake();
    }
}

void GameObject::Start()
{
    for (auto& component : components)
    {
        component->Start();
    }
}

void GameObject::Update(float deltaTime)
{
    for (auto& component : components)
    {
        component->Update(deltaTime);
    }
}

void GameObject::Draw()
{
    for (auto& component : components)
    {
        component->Draw();
    }
}

void GameObject::DrawUI()
{
    for (auto& component : components)
    {
        component->DrawUI();
    }
}

const std::string& GameObject::GetName() const
{
    return name;
}

int GameObject::GetZOrder() const
{
    return zOrder;
}

void GameObject::SetZOrder(int zOrder)
{
    this->zOrder = zOrder;
}

Scene& GameObject::GetScene()
{
    return scene;
}

const Scene& GameObject::GetScene() const
{
    return scene;
}

void GameObject::OnCollisionEnter(const Minigame::Components::CollisionInfo& info)
{
    for (auto& component : components)
    {
        component->OnCollisionEnter(info);
    }
}

void GameObject::AddTag(const std::string& tag)
{
    tags.insert(tag);
}

bool GameObject::ContainsTag(const std::string& tag) const
{
    return tags.contains(tag);
}
