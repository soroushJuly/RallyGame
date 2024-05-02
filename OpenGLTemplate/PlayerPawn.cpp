#include "PlayerPawn.h"

PlayerPawn::PlayerPawn() : Actor::Actor()
{
	m_speed = 0.0f;
	a = 0.f;
	isBraking = false;
	SetScale(glm::vec3(2.4f));
}

PlayerPawn::~PlayerPawn() {}

void PlayerPawn::Accelerate(double delta) {
	isBraking = false;
	a += 0.000002f;
	if (a > 0.000052f)
	{
		a = 0.000052f;
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

void PlayerPawn::Brake(double deltaTime) {
	isBraking = true;
	a -= 0.0000015f;
	if (m_speed < 0.0000f)
	{
		m_speed = 0.f;
		isBraking = false;
	}
}

void PlayerPawn::Update(double deltaTime)
{
	m_speed += a * deltaTime;
	a -= 0.000001f;
	// If we have speed decrease speed
	if (m_speed < 0.0000f)
	{
		isBraking = false;
		m_speed = 0.f;
		a = 0.f;
	}

	turboTimer -= deltaTime;
	if (turboTimer < 0.0)
	{
		isTurbo = false;
	}
	if (!isTurbo)
	{
		if (m_speed > SPEED_LIMIT)
			m_speed = SPEED_LIMIT;
	}
	else
	{
		if (m_speed > SPEED_LIMIT * 1.5f)
			m_speed = SPEED_LIMIT * 1.5f;
	}
}
