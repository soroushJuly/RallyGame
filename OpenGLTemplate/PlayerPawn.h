#pragma once

#include "Actor.h"

class PlayerPawn : public Actor {
public:
	PlayerPawn();
	~PlayerPawn();

	void Update(double deltaTime);

	void Brake(double deltaTime);
	void Accelerate(double delta);

	bool Intersects(glm::vec3 objectPosition, float radius) const;
	void OnAccident();
	void activateTurbo();

	float GetSpeed() const { return m_speed; }
	float GetSpeedLimit() const { return SPEED_LIMIT; }
	bool GetTurboStatus() const { return isTurbo; }
	bool IsBraking() const { return isBraking; };

private:
	float m_speed;
	// acceleration of the car
	float a;
	bool isBraking;
	const float SPEED_LIMIT = 0.14f;

	double turboTimer = 0.0;
	bool isTurbo = false;
};
