#pragma once

#include "Actor.h"

class PlayerPawn : public Actor {
public:
	PlayerPawn();
	~PlayerPawn();

	void Update(double deltaTime);

	void Break(double deltaTime);
	void Accelerate(double delta);

	bool Intersects(glm::vec3 objectPosition, float radius) const;
	void OnAccident();
	void activateTurbo();

	float GetSpeed() const { return m_speed; }
	float GetSpeedLimit() const { return SPEED_LIMIT; }
	bool GetTurboStatus() const { return isTurbo; }
	bool IsBreaking() const { return isBreaking; };

private:
	float m_speed;
	bool isBreaking;
	const float SPEED_LIMIT = 0.14f;

	double turboTimer = 0.0;
	bool isTurbo = false;
};
