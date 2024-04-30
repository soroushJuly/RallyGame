#include "PlayerPawn.h"

PlayerPawn::PlayerPawn() : Actor::Actor()
{
	m_speed = 0.0f;
	isBreaking = false;
	SetScale(glm::vec3(2.4f));
}

PlayerPawn::~PlayerPawn() {}

void PlayerPawn::Accelerate(double delta) {
	isBreaking = false;
	if (!isTurbo)
	{
		m_speed += (0.0002f * delta);
		if (m_speed > SPEED_LIMIT)
			m_speed = SPEED_LIMIT;
	}
	else
	{
		m_speed += 0.001f * delta;
		if (m_speed > SPEED_LIMIT * 1.5f)
			m_speed = SPEED_LIMIT * 1.5f;
	}
}

bool PlayerPawn::Intersects(glm::vec3 objectPosition, float radius) const
{
	float distance = glm::distance(objectPosition, this->GetPosition());
	if (distance < radius)
	{
		return true;
	}
	return false;
}

void PlayerPawn::OnAccident()
{
	m_speed = 0.f;
}

void PlayerPawn::activateTurbo()
{
	isTurbo = true;
	// Turbo will remian active for this amount
	turboTimer = 5000;
}

void PlayerPawn::Break(double deltaTime) {
	isBreaking = true;
	m_speed -= 0.00013f * deltaTime;
	if (m_speed < 0.0000f)
	{
		m_speed = 0.f;
		isBreaking = false;
	}
}

void PlayerPawn::Update(double deltaTime)
{
	turboTimer -= deltaTime;
	if (turboTimer < 0.0)
	{
		isTurbo = false;
	}
	// If we have speed decrease speed
	if (m_speed > 0.0001f)
	{
		m_speed -= 0.0001f * deltaTime;
		if (m_speed < 0.0000f)
		{
			isBreaking = false;
			m_speed = 0.f;
		}
	}
}
