#include "GameObjectFactory.h"
#include "GameObject.h"
#include "GameServices.h"
#include "SceneManager.h"
#include "GameSession.h"
#include "ResourceManager.h"
#include "Components/Transform.h"
#include "Components/SpriteRenderer.h"
#include "Components/Animator.h"
#include "Components/PlayerController.h"
#include "Components/MobController.h"
#include "Components/Health.h"
#include "Components/HealthBar.h"
#include "Components/PlayerMessage.h"
#include "Components/Collider.h"
#include "Components/Bullet.h"
#include "Components/MapBuilder.h"
#include "Components/Button.h"
#include "JsonUtils.h"
#include <fstream>

GameObjectFactory::GameObjectFactory(GameServices& gameServices, SceneManager& sceneManager) : gameServices(gameServices), sceneManager(sceneManager)
{
    std::ifstream file(std::string(GetApplicationDirectory()) + "Data/Prefab/Prefab.json");
    file >> prefabData;

    std::ifstream file2(std::string(GetApplicationDirectory()) + "Data/Resources/Animation.json");
    file2 >> animationData;
}

std::unique_ptr<GameObject> GameObjectFactory::CreatePrefab(Scene& scene, const std::string& name)
{
    if (!prefabData.contains(name))
        return nullptr;

    return Create(scene, prefabData.at(name));
}

std::unique_ptr<GameObject> GameObjectFactory::Create(Scene& scene, const json& gameObjectData)
{
    auto gameObject = std::make_unique<GameObject>(scene, gameObjectData.at("name").get<std::string>());
    gameObject->SetZOrder(gameObjectData.value("zOrder", 0));

    if (gameObjectData.contains("tags"))
    {
        for (const auto& tag : gameObjectData.at("tags"))
        {
            gameObject->AddTag(tag.get<std::string>());
        }
    }

    if (!gameObjectData.value("ignore", false))
    {
        LoadComponents(*gameObject, gameObjectData);
    }

    return gameObject;
}

void GameObjectFactory::LoadComponents(GameObject& gameObject, const json& gameObjectData)
{
    if (!gameObjectData.contains("components"))
        return;

    for (const auto& componentData : gameObjectData.at("components"))
    {
        LoadComponent(gameObject, componentData);
    }
}

void GameObjectFactory::LoadComponent(GameObject& gameObject, const json& componentData)
{
    const std::string type = componentData.at("type").get<std::string>();
    if (type == "Transform")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::Transform>();

        if (componentData.contains("position"))
        {
            component.SetPosition(componentData.at("position").get<Vector2>());
        }

        component.SetRotation(componentData.value("rotation", 0.0f));

        component.SetScale(componentData.value("scale", 1.0f));
    }
    else if (type == "SpriteRenderer")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::SpriteRenderer>();

        if (componentData.contains("texture"))
        {
            if (const auto* texture = gameServices.resources.GetTexture(componentData.at("texture").get<std::string>()))
            {
                component.SetTexture(*texture);
            }
        }

        if (componentData.contains("origin"))
        {
            component.SetOrigin(componentData.at("origin").get<Vector2>());
        }
    }
    else if (type == "Animator")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::Animator>(gameServices);

        if (componentData.contains("states"))
        {
            for (const auto& state : componentData.at("states"))
            {
                auto clip = CreateAnimationClip(state.get<std::string>());
                if (!clip.frames.empty())
                {
                    component.AddClip(state, std::move(clip));
                }
            }

            if (componentData.contains("defaultState"))
            {
                component.SetDefaultState(componentData.at("defaultState").get<std::string>());
            }
        }
    }
    else if (type == "PlayerController")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::PlayerController>(gameServices);

        component.SetMoveSpeed(componentData.value("speed", 100.0f));

        component.SetBulletSpeed(componentData.value("bulletSpeed", 500.0f));

        component.SetBulletDistance(componentData.value("bulletDistance", 300.0f));

        component.SetFireCooldown(componentData.value("fireCooldown", 0.15f));

        component.SetAttackPower(componentData.value("attackPower", 10.0f));

        if (componentData.contains("bulletPrefab"))
        {
            component.SetBulletPrefab(componentData.at("bulletPrefab").get<std::string>());
        }

        if (componentData.contains("bulletTint"))
        {
            const auto& bulletTintData = componentData.at("bulletTint");
            Color bulletTint
            {
                static_cast<unsigned char>(bulletTintData.value("r", 255)),
                static_cast<unsigned char>(bulletTintData.value("g", 255)),
                static_cast<unsigned char>(bulletTintData.value("b", 255)),
                static_cast<unsigned char>(bulletTintData.value("a", 255))
            };

            component.SetBulletTint(bulletTint);
        }
    }
    else if (type == "MobController")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::MobController>(gameServices);

        component.SetMoveSpeed(componentData.value("speed", 100.0f));

        component.SetBulletSpeed(componentData.value("bulletSpeed", 500.0f));

        component.SetBulletDistance(componentData.value("bulletDistance", 300.0f));

        component.SetFireCooldown(componentData.value("fireCooldown", 5.0f));

        component.SetAttackPower(componentData.value("attackPower", 10.0f));

        component.SetDetectionRange(componentData.value("detectionRange", 100.0f));

        if (componentData.contains("bulletPrefab"))
        {
            component.SetBulletPrefab(componentData.at("bulletPrefab").get<std::string>());
        }

        if (componentData.contains("bulletTint"))
        {
            const auto& bulletTintData = componentData.at("bulletTint");
            Color bulletTint
            {
                static_cast<unsigned char>(bulletTintData.value("r", 255)),
                static_cast<unsigned char>(bulletTintData.value("g", 255)),
                static_cast<unsigned char>(bulletTintData.value("b", 255)),
                static_cast<unsigned char>(bulletTintData.value("a", 255))
            };

            component.SetBulletTint(bulletTint);
        }
    }
    else if (type == "Collider")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::Collider>(gameServices);

        if (componentData.contains("size"))
        {
            component.SetSize(componentData.at("size").get<Vector2>());
        }

        if (componentData.contains("offset"))
        {
            component.SetOffset(componentData.at("offset").get<Vector2>());
        }
    }
    else if (type == "Health")
    {
        gameObject.AddComponent<Minigame::Components::Health>(componentData.at("maxHealth").get<float>());
    }
    else if (type == "HealthBar")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::HealthBar>();

        if (componentData.contains("offset"))
        {
            component.SetOffset(componentData.at("offset").get<Vector2>());
        }

        if (componentData.contains("size"))
        {
            component.SetSize(componentData.at("size").get<Vector2>());
        }

        if (componentData.contains("healthColor"))
        {
            const auto& healthColorData = componentData.at("healthColor");
            Color healthColor
            {
                static_cast<unsigned char>(healthColorData.value("r", 255)),
                static_cast<unsigned char>(healthColorData.value("g", 255)),
                static_cast<unsigned char>(healthColorData.value("b", 255)),
                static_cast<unsigned char>(healthColorData.value("a", 255))
            };

            component.SetHealthColor(healthColor);
        }
    }
    else if (type == "PlayerMessage")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::PlayerMessage>();

        const std::string fontKey = componentData.value("font", "Maplestory Bold.ttf");
        const float fontSize = componentData.value("fontSize", 20.0f);
        if (const auto* font = gameServices.resources.GetFont(fontKey, static_cast<int>(fontSize)))
        {
            component.SetFont(*font, fontSize);
        }

        if (componentData.contains("offset"))
        {
            component.SetOffset(componentData.at("offset").get<Vector2>());
        }
    }
    else if (type == "Bullet")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::Bullet>();
    }
    else if (type == "MapBuilder")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::MapBuilder>(gameServices);

        if (componentData.contains("grid"))
        {
            component.SetGrid(componentData.at("grid").get<Vector2>());
        }

        if (componentData.contains("blockSize"))
        {
            component.SetBlockSize(componentData.at("blockSize").get<Vector2>());
        }

        if (componentData.contains("chunkSize"))
        {
            component.SetChunkSize(componentData.at("chunkSize").get<Vector2>());
        }

        if (componentData.contains("chunkPreset"))
        {
            for (auto& chunkPreset : componentData.at("chunkPreset"))
            {
                std::vector<std::string> preset;
                for (int i = 0;; i++)
                {
                    std::string rowIndex = std::to_string(i);
                    if (chunkPreset.contains(rowIndex))
                    {
                        preset.push_back(chunkPreset.at(rowIndex).get<std::string>());
                    }
                    else break;
                }
                component.AddChunkPreset(std::move(preset));
            }
        }

        if (componentData.contains("powerUps"))
        {
            for (auto& powerUp : componentData.at("powerUps"))
            {
                component.AddPowerUpToList(powerUp.get<std::string>());
            }
        }

        if (componentData.contains("stages"))
        {
            const auto& stagesData = componentData.at("stages");
            for (const auto& stageData : stagesData)
            {
                int stage = stageData.value("stage", 0);
                int spawnCount = stageData.value("spawnCount", 0);
                float nextSpawnCooldown = stageData.value("nextSpawnCooldown", 0.0f);
                int powerUpCount = stageData.value("powerUpCount", 0);

                std::vector< Minigame::Components::MobInfo> mobs;
                if (stageData.contains("mobs"))
                {
                    const auto& mobData = stageData.at("mobs");
                    for (const auto& mob : mobData)
                    {
                        mobs.push_back({ mob.value("prefab", ""), mob.value("count", 0) });
                    }
                }
                component.AddStageInfo(stage, { spawnCount, nextSpawnCooldown, powerUpCount, std::move(mobs) });
            }
        }
    }
    else if (type == "Button")
    {
        auto& component = gameObject.AddComponent<Minigame::Components::Button>();

        if (componentData.contains("size"))
        {
            component.SetSize(componentData.at("size").get<Vector2>());
        }

        if (componentData.contains("text"))
        {
            component.SetText(componentData.at("text").get<std::string>());
        }

        if (componentData.contains("font"))
        {
            const std::string fontKey = componentData.value("font", "Maplestory Bold.ttf");
            const float fontSize = componentData.value("fontSize", 32.0f);
            if (const auto* font = gameServices.resources.GetFont(fontKey, static_cast<int>(fontSize)))
            {
                component.SetFont(*font, fontSize);
            }
        }

        if (componentData.contains("onClick"))
        {
            const auto& onClickData = componentData.at("onClick");
            if (onClickData.contains("func"))
            {
                const std::string func = onClickData.at("func").get<std::string>();
                if (func == "SelectScene")
                {
                    int sceneNum = onClickData.value("value", 0);
                    component.SetOnClick([this, sceneNum]()
                        {
                            sceneManager.SelectScene(sceneNum);
                        });
                }
                else if (func == "RestartGame")
                {
                    component.SetOnClick([this]()
                        {
                            gameServices.session.Reset();
                            sceneManager.ReloadCurrentScene();
                        });
                }
                else if (func == "GoToMainMenu")
                {
                    component.SetOnClick([this]()
                        {
                            gameServices.session.Reset();
                            sceneManager.SelectScene(0);
                        });
                }
            }
        }
    }
}

Minigame::Components::AnimationClip GameObjectFactory::CreateAnimationClip(const std::string& state)
{
    Minigame::Components::AnimationClip clip;
    clip.name = state;

    if (!animationData.contains("objects"))
        return clip;

    for (const auto& stateData : animationData.at("objects"))
    {
        if (stateData.value("name", "") != state)
            continue;

        clip.loop = stateData.value("loop", true);

        if (!stateData.contains("frame"))
            return clip;

        for (const auto& frameData : stateData.at("frame"))
        {
            std::string index = "";
            if (frameData.contains("index"))
            {
                const int intIndex = frameData.at("index").get<int>();
                index = std::to_string(intIndex);
            }
            const float delay = frameData.value("delay", 120.0f);

            const std::string textureName = state + (index.empty() ? ".png" : "_" + index + ".png");

            const auto* texture = gameServices.resources.GetTexture(textureName);
            if (!texture)
                continue;

            Minigame::Components::AnimationFrame frame;
            frame.texture = texture;

            frame.delay = delay / 1000.0f;

            if (frameData.contains("origin"))
            {
                frame.origin = frameData.at("origin").get<Vector2>();
            }

            clip.frames.push_back(std::move(frame));
        }

        break;
    }

    return clip;
}
