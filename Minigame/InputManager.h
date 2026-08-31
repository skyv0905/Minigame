#pragma once
#include <raylib.h>
#include <unordered_map>
#include <vector>

enum class InputAction
{
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,

    Fire,

    Debug,
    Exit
};

class InputManager
{
public:
    void BindKey(InputAction action, KeyboardKey key);

    bool IsPressed(InputAction action) const;
    bool IsDown(InputAction action) const;
    bool IsReleased(InputAction action) const;

    Vector2 GetMoveAxis() const;

private:
    std::unordered_map<InputAction, std::vector<KeyboardKey>> keyBindings;
};