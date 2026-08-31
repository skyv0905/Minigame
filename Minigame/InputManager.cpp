#include "InputManager.h"
#include <raymath.h>

void InputManager::BindKey(InputAction action, KeyboardKey key)
{
    keyBindings[action].push_back(key);
}

bool InputManager::IsPressed(InputAction action) const
{
    auto it = keyBindings.find(action);

    if (it == keyBindings.end())
        return false;

    for (KeyboardKey key : it->second)
    {
        if (IsKeyPressed(key))
            return true;
    }

    return false;
}

bool InputManager::IsDown(InputAction action) const
{
    auto it = keyBindings.find(action);

    if (it == keyBindings.end())
        return false;

    for (KeyboardKey key : it->second)
    {
        if (IsKeyDown(key))
            return true;
    }

    return false;
}

bool InputManager::IsReleased(InputAction action) const
{
    auto it = keyBindings.find(action);

    if (it == keyBindings.end())
        return false;

    for (KeyboardKey key : it->second)
    {
        if (IsKeyReleased(key))
            return true;
    }

    return false;
}

Vector2 InputManager::GetMoveAxis() const
{
    Vector2 direction = { 0.0f, 0.0f };

    if (IsDown(InputAction::MoveLeft))
        direction.x -= 1.0f;

    if (IsDown(InputAction::MoveRight))
        direction.x += 1.0f;

    if (IsDown(InputAction::MoveUp))
        direction.y -= 1.0f;

    if (IsDown(InputAction::MoveDown))
        direction.y += 1.0f;

    if (direction.x != 0.0f || direction.y != 0.0f)
        direction = Vector2Normalize(direction);

    return direction;
}