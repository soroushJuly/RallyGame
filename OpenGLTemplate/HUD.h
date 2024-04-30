#pragma once

#include "Common.h"
#include "GameWindow.h"
#include "MatrixStack.h"

class Quad;
class CShaderProgram;
class PlayerPawn;
class CCamera;

class HUD
{
public:
	HUD();
	~HUD();

	void Initilize();
	void Update();
	void Render(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shader, shared_ptr<CCamera> camera, shared_ptr<PlayerPawn> player, int windowWidth, int windowHeight);

private:
	shared_ptr<Quad> m_speedometer;
	shared_ptr<Quad> m_indicator;
	shared_ptr<Quad> m_lapIcon;
	shared_ptr<Quad> m_timeIcon;


	float speedometerWidth = 484 / 5;
	float speedometerHeight = 440 / 5;

};