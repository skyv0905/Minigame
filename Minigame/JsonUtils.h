#pragma once

#include <raylib.h>
#include <nlohmann/json.hpp>

inline void from_json(const nlohmann::json& j, Vector2& v)
{
    v.x = j.at("x").get<float>();
    v.y = j.at("y").get<float>();
}