#include "Animator.h"
#include "../GameObject.h"

namespace Minigame::Components
{
    Animator::Animator(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void Animator::Start()
    {
        spriteRenderer = owner.GetComponent<Minigame::Components::SpriteRenderer>();
        if (currentClip == nullptr && !defaultState.empty())
        {
            Play(defaultState, true);
        }
    }

    void Animator::Update(float deltaTime)
	{
        if (currentClip == nullptr || currentClip->frames.empty() || finished)
        {
            return;
        }

        elapsed += deltaTime;

        while (elapsed >= currentClip->frames[currentFrame].delay)
        {
            elapsed -= currentClip->frames[currentFrame].delay;
            currentFrame++;

            if (currentFrame >= currentClip->frames.size())
            {
                if (currentClip->loop)
                {
                    currentFrame = 0;
                }
                else
                {
                    currentFrame = currentClip->frames.size() - 1;
                    finished = true;
                }
            }

            ApplyCurrentFrame();

            if (finished)
                break;
        }
	}

    void Animator::Play(const std::string& state, bool restart)
    {
        auto it = clips.find(state);
        if (it != clips.end())
        {
            auto* nextClip = &it->second;
            if (nextClip->frames.empty() || (currentClip == nextClip && !restart))
                return;

            currentClip = nextClip;
            currentFrame = 0;
            elapsed = 0.0f;
            finished = false;

            ApplyCurrentFrame();
        }
    }

    void Animator::PlayDefaultState(bool restart)
    {
        Play(defaultState, restart);
    }

    bool Animator::IsFinished() const
    {
        return finished;
    }

    void Animator::SetDefaultState(const std::string& state)
    {
        defaultState = state;
    }

    void Animator::AddClip(const std::string& state, AnimationClip clip)
    {
        clips.insert_or_assign(state, std::move(clip));
    }

    void Animator::ApplyCurrentFrame()
    {
        const auto& frame = currentClip->frames[currentFrame];
        
        if (spriteRenderer)
        {
            spriteRenderer->SetTexture(*frame.texture);
            spriteRenderer->SetOrigin(frame.origin);
        }
    }
}
