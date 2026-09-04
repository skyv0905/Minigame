#include "Scene.h"
#include <iostream>
#include <algorithm>
#include "MusicPlayer.h"
#include "GameSession.h"
#include "GameObjectFactory.h"
#include "Components/Bullet.h"
#include "Components/Controller.h"
#include "Components/Exp.h"
#include "Components/Health.h"
#include "Components/MapBuilder.h"
#include "Components/PlayerControllerNetwork.h"
#include "Components/MobControllerNetwork.h"
#include "Components/SpriteRenderer.h"
#include "Components/Transform.h"
#include "Components/TextUI.h"
#include "Network/NetworkClient.h"

Scene::Scene(GameServices& gameServices, GameObjectFactory& gameObjectFactory, std::string name, int index) :
    gameServices(gameServices), gameObjectFactory(gameObjectFactory),
    name(name), index(index)
{
}

void Scene::Start()
{
    gameServices.music.Play(bgm);
}

void Scene::Update(float deltaTime)
{
    for (auto& gameObject : gameObjects)
    {
        gameObject->Update(deltaTime);
    }

    CheckCollisions();

    FlushPendingGameObjects();
}

void Scene::Draw()
{
    std::vector<GameObject*> drawOrder;
    drawOrder.reserve(gameObjects.size());

    for (auto& gameObject : gameObjects)
    {
        drawOrder.push_back(gameObject.get());
    }

    std::stable_sort(drawOrder.begin(), drawOrder.end(), [](const auto* a, const auto* b)
        {
            return a->GetZOrder() < b->GetZOrder();
        });

    for (auto* gameObject : drawOrder)
    {
        gameObject->Draw();
    }
    for (auto* gameObject : drawOrder)
    {
        gameObject->DrawUI();
    }
    //DrawText(TextFormat("Scene no: %d", index), 10, 10, 15, BLACK);
}

void Scene::SetBgm(const std::string& bgmName)
{
    bgm = bgmName;
}

const std::string& Scene::GetName() const
{
    return name;
}

int Scene::GetIndex() const
{
    return index;
}

void Scene::AddGameObject(std::unique_ptr<GameObject> gameObject)
{
    if (!gameObject)
        return;

    pendingGameObjects.push_back(std::move(gameObject));
}

void Scene::DestroyGameObject(GameObject& gameObject)
{
    pendingDestroyGameObjects.insert(&gameObject);
}

GameObject* Scene::Instantiate(const std::string& prefabName)
{
    auto gameObject = gameObjectFactory.CreatePrefab(*this, prefabName);
    GameObject* ret = nullptr;
    if (gameObject)
    {
        ret = gameObject.get();
        AddGameObject(std::move(gameObject));
    }

    return ret;
}

GameObject* Scene::FindGameObject(const std::string& name)
{
    for (auto& gameObject : gameObjects)
    {
        if (gameObject->GetName() == name)
        {
            return gameObject.get();
        }
    }

    return nullptr;
}

GameObject* Scene::FindGameObjectWithTag(const std::string& tag)
{
    for (auto& gameObject : gameObjects)
    {
        if (gameObject->ContainsTag(tag))
        {
            return gameObject.get();
        }
    }

    return nullptr;;
}

GameObject* Scene::FindGameObjectByID(GameObjectID id)
{
    auto it = gameObjectsByID.find(id);
    if (it != gameObjectsByID.end())
    {
        return it->second;
    }

    return nullptr;
}

GameObject* Scene::FindNetworkObject(std::uint32_t objectId)
{
    if (objectId <= Minigame::Network::WorldStatePacket::MaxPlayers)
    {
        return FindGameObjectWithTag("Player" + std::to_string(objectId));
    }

    for (auto& gameObject : gameObjects)
    {
        if (gameObject->GetNetworkObjectId() == objectId)
        {
            return gameObject.get();
        }

        auto* controller = gameObject->GetComponent<Minigame::Components::MobControllerNetwork>();
        if (controller && controller->GetObjectId() == objectId)
        {
            return gameObject.get();
        }
    }
    return nullptr;
}

void Scene::ApplyBulletSpawn(const Minigame::Network::BulletSpawnPacket& packet)
{
    GameObject* createdFrom = FindNetworkObject(packet.createdFrom);
    if (createdFrom == nullptr)
        return;

    if (packet.createdFrom == gameServices.network.GetPlayerId())
    {
        for (auto& gameObject : gameObjects)
        {
            auto* bullet = gameObject->GetComponent<Minigame::Components::Bullet>();
            if (bullet && bullet->GetCreatedFrom() == createdFrom->GetID() && bullet->GetFireSequence() == packet.fireSequence && bullet->GetNetworkObjectId() == 0)
            {
                bullet->SetNetworkObjectId(packet.bulletId);
                bullet->SetMoveSpeed(Minigame::Network::DecodePosition(packet.moveSpeed));
                bullet->SetMaxDistance(Minigame::Network::DecodePosition(packet.maxDistance));
                bullet->SetServerAuthoritative(true);
                return;
            }
        }
    }

    GameObject* bulletObject = Instantiate("Bullet");
    if (bulletObject == nullptr)
        return;

    if (auto* transform = bulletObject->GetComponent<Minigame::Components::Transform>())
    {
        transform->SetPosition(Vector2{ Minigame::Network::DecodePosition(packet.positionX), Minigame::Network::DecodePosition(packet.positionY) });
    }

    if (auto* bullet = bulletObject->GetComponent<Minigame::Components::Bullet>())
    {
        bullet->SetCreatedFrom(createdFrom->GetID());
        bullet->SetFireSequence(packet.fireSequence);
        bullet->SetNetworkObjectId(packet.bulletId);
        bullet->SetDirection(Vector2{ static_cast<float>(packet.directionX) / 127.0f, static_cast<float>(packet.directionY) / 127.0f });
        bullet->SetMoveSpeed(Minigame::Network::DecodePosition(packet.moveSpeed));
        bullet->SetMaxDistance(Minigame::Network::DecodePosition(packet.maxDistance));
        bullet->SetServerAuthoritative(true);
    }
    if (const auto* controller = createdFrom->GetComponent<Minigame::Components::Controller>())
    {
        if (auto* renderer = bulletObject->GetComponent<Minigame::Components::SpriteRenderer>())
        {
            renderer->SetTint(controller->GetBulletTint());
        }
    }
}

void Scene::ApplyBulletDestroy(const Minigame::Network::BulletDestroyPacket& packet)
{
    GameObject* createdFrom = FindNetworkObject(packet.createdFrom);
    GameObject* hitObject = packet.hitObjectId == 0 ? nullptr : FindNetworkObject(packet.hitObjectId);
    if (createdFrom && hitObject)
    {
        const bool createdFromPlayer = createdFrom->GetComponent<Minigame::Components::PlayerControllerNetwork>() != nullptr;
        const bool createdFromMob = createdFrom->GetComponent<Minigame::Components::MobControllerNetwork>() != nullptr;
        auto* hitPlayer = hitObject->GetComponent<Minigame::Components::PlayerControllerNetwork>();
        if (hitPlayer && ((createdFromPlayer) || (createdFromMob && packet.hitObjectId == gameServices.network.GetPlayerId())))
        {
            hitPlayer->OnHit();
        }
    }

    for (auto& gameObject : gameObjects)
    {
        auto* bullet = gameObject->GetComponent<Minigame::Components::Bullet>();
        if (bullet && bullet->GetNetworkObjectId() == packet.bulletId)
        {
            DestroyGameObject(*gameObject);
            return;
        }
    }
}

void Scene::ApplyExpChanged(const Minigame::Network::ExpChangedPacket& packet)
{
    GameObject* player = FindNetworkObject(packet.playerId);
    if (player == nullptr)
        return;

    if (auto* exp = player->GetComponent<Minigame::Components::Exp>())
    {
        const bool levelUp = exp->GetLevel() < static_cast<int>(packet.newLevel);
        exp->SetNetworkState(static_cast<int>(packet.newExp), static_cast<int>(packet.newLevel));
        if (levelUp && packet.playerId == gameServices.network.GetPlayerId())
        {
            if (auto* controller = player->GetComponent<Minigame::Components::PlayerControllerNetwork>())
            {
                controller->OnLevelUp();
            }
        }
    }
}

void Scene::ApplyHpChanged(const Minigame::Network::HpChangedPacket& packet)
{
    GameObject* object = FindNetworkObject(packet.objectId);
    if (object == nullptr)
        return;

    if (auto* health = object->GetComponent<Minigame::Components::Health>())
    {
        health->SetCurrentHealth(packet.newHp);
        if (auto* controller = object->GetComponent<Minigame::Components::MobControllerNetwork>())
        {
            packet.newHp <= 0 ? controller->OnDead() : controller->OnHit();
        }
    }
}

void Scene::ApplyPlayerStatsChanged(const Minigame::Network::PlayerStatsChangedPacket& packet)
{
    GameObject* player = FindNetworkObject(packet.playerId);
    if (player == nullptr)
        return;

    if (auto* controller = player->GetComponent<Minigame::Components::Controller>())
        controller->SetNetworkStats(packet);
}

void Scene::ApplyPowerUpCollected(const Minigame::Network::PowerUpCollectedPacket& packet)
{
    GameObject* powerUp = FindNetworkObject(packet.objectId);
    GameObject* player = FindNetworkObject(packet.playerId);
    if (powerUp == nullptr || player == nullptr)
        return;

    if (auto* controller = player->GetComponent<Minigame::Components::PlayerControllerNetwork>())
    {
        controller->OnPowerUpCollected(*powerUp);
    }
}

void Scene::ApplyStageChanged(const Minigame::Network::StageChangedPacket& packet)
{
    for (auto& gameObject : gameObjects)
    {
        if (auto* mapBuilder = gameObject->GetComponent<Minigame::Components::MapBuilder>())
        {
            mapBuilder->ApplyStageChanged(packet);
            return;
        }
    }
}

void Scene::ApplyGameResult(const Minigame::Network::GameResultPacket& packet, std::uint32_t localPlayerId)
{
    std::string resultText = "무승부";
    GameState resultState = GameState::GameOver;
    if (packet.winnerPlayerId != 0)
    {
        const bool won = packet.winnerPlayerId == localPlayerId;
        resultText = won ? "승리!" : "저런...";
        resultState = won ? GameState::GameClear : GameState::GameOver;
    }

    gameServices.session.SetGameState(resultState);
    Instantiate("MainMenuButton");
    if (GameObject* textUIGameObject = Instantiate("TextUI"))
    {
        if (auto* textUI = textUIGameObject->GetComponent<Minigame::Components::TextUI>())
        {
            textUI->SetText(resultText);
        }
    }
}

void Scene::CheckCollisions()
{
    for (size_t i = 0; i < colliders.size(); i++)
    {
        auto* colliderA = colliders[i];
        if (!colliderA || !colliderA->IsValid() || pendingDestroyGameObjects.contains(&colliderA->GetOwner()))
            continue;

        for (size_t j = i + 1; j < colliders.size(); j++)
        {
            if (!colliderA->IsValid()) break;

            auto* colliderB = colliders[j];
            if (!colliderB || !colliderB->IsValid() || pendingDestroyGameObjects.contains(&colliderB->GetOwner()))
                continue;

            Vector2 collisionDirA{};
            float depth = 0.0f;
            if (colliderA->CheckCollision(*colliderB, collisionDirA, depth))
            {
                Vector2 collisionDirB{ collisionDirA.x * -1, collisionDirA.y * -1 };

                auto& gameObjectA = colliderA->GetOwner();
                auto& gameObjectB = colliderB->GetOwner();

                Minigame::Components::CollisionInfo ColliderInfoA{ gameObjectB, collisionDirA, depth };
                Minigame::Components::CollisionInfo ColliderInfoB{ gameObjectA, collisionDirB, depth };

                gameObjectA.OnCollisionEnter(ColliderInfoA);
                gameObjectB.OnCollisionEnter(ColliderInfoB);
            }
        }
    }
}

void Scene::FlushPendingGameObjects()
{
    for (auto& gameObject : pendingGameObjects)
    {
        //std::cout << "Added Game Object: " << gameObject->GetName() << "\n";

        if (auto* collider = gameObject->GetComponent<Minigame::Components::Collider>())
        {
            colliders.push_back(collider);
        }
        gameObjectsByID.emplace(gameObject->GetID(), gameObject.get());

        gameObject->Awake();
        gameObject->Start();
        gameObjects.push_back(std::move(gameObject));
    }
    pendingGameObjects.clear();

    for (auto* gameObject : pendingDestroyGameObjects)
    {
        std::cout << "Removed Game Object: " << gameObject->GetName() << "\n";

        if (auto* collider = gameObject->GetComponent<Minigame::Components::Collider>())
        {
            std::erase(colliders, collider);
        }

        gameObjectsByID.erase(gameObject->GetID());
        std::erase_if(gameObjects, [gameObject](const auto& object)
            {
                return object.get() == gameObject;
            });
    }
    pendingDestroyGameObjects.clear();
}
