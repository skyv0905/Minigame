#include "Transform.h"

namespace Minigame::Components
{
	void Transform::SetPosition(float x, float y)
	{
		position.x = x;
		position.y = y;
	}

	void Transform::SetPosition(Vector2 pos)
	{
		position = pos;
	}

	void Transform::SetRotation(float angle)
	{
		rotation = angle;
	}

	void Transform::SetScale(float scale)
	{
		this->scale = scale;
	}

	Vector2 Transform::GetPosition() const
	{
		return position;
	}

	float Transform::GetRotation() const
	{
		return rotation;
	}

	float Transform::GetScale() const
	{
		return scale;
	}
}