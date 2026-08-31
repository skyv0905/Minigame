#include "Collider.h"
#include "Transform.h"
#include "../GameServices.h"
#include "../GameObject.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    Vector2 Rotate(Vector2 value, float radians)
    {
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return Vector2
        {
            value.x * cosine - value.y * sine,
            value.x * sine + value.y * cosine
        };
    }

    float Dot(Vector2 a, Vector2 b)
    {
        return a.x * b.x + a.y * b.y;
    }
}

namespace Minigame::Components
{
    Collider::Collider(GameObject& owner, GameServices& gameServices) : Component(owner), gameServices(gameServices)
    {
    }

    void Collider::Start()
    {
        transform = owner.GetComponent<Minigame::Components::Transform>();
    }

    void Collider::DrawUI()
    {
        if (!transform || !valid || !gameServices.debugMode)
            return;

        const auto corners = GetCorners();
        for (size_t i = 0; i < corners.size(); ++i)
        {
            DrawLineEx(corners[i], corners[(i + 1) % corners.size()], 2.0f, RED);
        }
    }

    Rectangle Collider::GetBounds() const
    {
        const auto corners = GetCorners();
        float minX = corners[0].x;
        float maxX = corners[0].x;
        float minY = corners[0].y;
        float maxY = corners[0].y;

        for (size_t i = 1; i < corners.size(); ++i)
        {
            minX = std::min(minX, corners[i].x);
            maxX = std::max(maxX, corners[i].x);
            minY = std::min(minY, corners[i].y);
            maxY = std::max(maxY, corners[i].y);
        }

        return Rectangle{ minX, minY, maxX - minX, maxY - minY };
    }

    std::array<Vector2, 4> Collider::GetCorners() const
    {
        if (!transform)
            return {};

        const Vector2 position = transform->GetPosition();
        const float scale = transform->GetScale();
        const float radians = transform->GetRotation() * DEG2RAD;
        const std::array<Vector2, 4> localCorners
        {
            Vector2{ offset.x * scale, offset.y * scale },
            Vector2{ (offset.x + size.x) * scale, offset.y * scale },
            Vector2{ (offset.x + size.x) * scale, (offset.y + size.y) * scale },
            Vector2{ offset.x * scale, (offset.y + size.y) * scale }
        };

        std::array<Vector2, 4> corners;
        for (size_t i = 0; i < corners.size(); ++i)
        {
            const Vector2 rotated = Rotate(localCorners[i], radians);
            corners[i] = Vector2{ position.x + rotated.x, position.y + rotated.y };
        }
        return corners;
    }

    bool Collider::CheckCollision(const Collider& other, Vector2& direction, float& depth) const
    {
        if (!transform || !other.transform)
            return false;

        const auto cornersA = GetCorners();
        const auto cornersB = other.GetCorners();
        const std::array<Vector2, 4> axes
        {
            Vector2{ cornersA[1].y - cornersA[0].y, cornersA[0].x - cornersA[1].x },
            Vector2{ cornersA[3].y - cornersA[0].y, cornersA[0].x - cornersA[3].x },
            Vector2{ cornersB[1].y - cornersB[0].y, cornersB[0].x - cornersB[1].x },
            Vector2{ cornersB[3].y - cornersB[0].y, cornersB[0].x - cornersB[3].x }
        };

        depth = std::numeric_limits<float>::max();
        Vector2 minimumAxis{};

        for (Vector2 axis : axes)
        {
            const float length = std::sqrt(Dot(axis, axis));
            if (length == 0.0f)
                return false;
            axis = Vector2{ axis.x / length, axis.y / length };

            float minA = Dot(cornersA[0], axis);
            float maxA = minA;
            float minB = Dot(cornersB[0], axis);
            float maxB = minB;
            for (size_t i = 1; i < 4; ++i)
            {
                minA = std::min(minA, Dot(cornersA[i], axis));
                maxA = std::max(maxA, Dot(cornersA[i], axis));
                minB = std::min(minB, Dot(cornersB[i], axis));
                maxB = std::max(maxB, Dot(cornersB[i], axis));
            }

            const float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap <= 0.0f)
                return false;
            if (overlap < depth)
            {
                depth = overlap;
                minimumAxis = axis;
            }
        }

        const Vector2 centerA
        {
            (cornersA[0].x + cornersA[2].x) * 0.5f,
            (cornersA[0].y + cornersA[2].y) * 0.5f
        };
        const Vector2 centerB
        {
            (cornersB[0].x + cornersB[2].x) * 0.5f,
            (cornersB[0].y + cornersB[2].y) * 0.5f
        };
        const Vector2 centerDelta{ centerB.x - centerA.x, centerB.y - centerA.y };
        if (Dot(centerDelta, minimumAxis) > 0.0f)
            minimumAxis = Vector2{ -minimumAxis.x, -minimumAxis.y };

        direction = minimumAxis;
        return true;
    }

    void Collider::SetSize(Vector2 size)
    {
        this->size = size;
    }

    void Collider::SetOffset(Vector2 offset)
    {
        this->offset = offset;
    }

    bool Collider::IsTrigger() const
    {
        return isTrigger;
    }

    void Collider::SetTrigger(bool trigger)
    {
        isTrigger = trigger;
    }

    void Collider::SetValid(bool valid)
    {
        this->valid = valid;
    }

    bool Collider::IsValid() const
    {
        return valid;
    }
}
