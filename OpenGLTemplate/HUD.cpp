#include "HUD.h"
#include "Quad.h"
#include "Shaders.h"
#include "Camera.h"
#include "PlayerPawn.h"

HUD::HUD() {}
HUD::~HUD() {}

void HUD::Initilize()
{
	m_speedometer = make_shared<Quad>();
	m_indicator = make_shared<Quad>();
	m_lapIcon = make_shared<Quad>();
	m_timeIcon = make_shared<Quad>();

	m_speedometer->Create("resources\\textures\\speedometer.png", speedometerHeight, speedometerWidth);
	m_indicator->Create("resources\\textures\\indicator.png", 416 / 5, 200 / 5);
	// time icon and lap icon better have same width
	m_timeIcon->Create("resources\\textures\\stopwatch.png", 25.f, 25.f);
	m_lapIcon->Create("resources\\textures\\flag.png", 18.5f, 25.f);
};

void HUD::Render(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shader, shared_ptr<CCamera> camera, shared_ptr<PlayerPawn> player, int windowWidth, int windowHeight)
{

	/**********************	HUD	****************/
	shader->SetUniform("lightingOn", false);
	glDisable(GL_DEPTH_TEST);

	int width = windowWidth;
	int height = windowHeight;

	float rotation = (player->GetSpeed() / player->GetSpeedLimit()) * M_PI;
	// Speedometer indicator
	modelViewMatrixStack->Push();
	modelViewMatrixStack->SetIdentity();
	modelViewMatrixStack->Translate(glm::vec3(width - speedometerWidth, speedometerHeight - 15, 0));
	modelViewMatrixStack->Rotate(glm::vec3(0, 0, 1), -rotation);
	modelViewMatrixStack->Scale(.4f);
	shader->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shader->SetUniform("matrices.projMatrix",
		camera->GetOrthographicProjectionMatrix());
	m_indicator->Render();
	modelViewMatrixStack->Pop();
	// Speedometer
	modelViewMatrixStack->Push();
	modelViewMatrixStack->SetIdentity();
	modelViewMatrixStack->Translate(glm::vec3(width - speedometerWidth, speedometerHeight, 0));
	shader->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shader->SetUniform("matrices.projMatrix",
		camera->GetOrthographicProjectionMatrix());
	m_speedometer->Render();
	modelViewMatrixStack->Pop();
	// Flag Icon
	modelViewMatrixStack->Push();
	modelViewMatrixStack->SetIdentity();
	modelViewMatrixStack->Translate(glm::vec3(50, height - 30, 0));
	shader->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shader->SetUniform("matrices.projMatrix",
		camera->GetOrthographicProjectionMatrix());
	m_lapIcon->Render();
	modelViewMatrixStack->Pop();
	// Stopwatch Icon
	modelViewMatrixStack->Push();
	modelViewMatrixStack->SetIdentity();
	modelViewMatrixStack->Translate(glm::vec3(50, height - 85, 0));
	shader->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shader->SetUniform("matrices.projMatrix",
		camera->GetOrthographicProjectionMatrix());
	m_timeIcon->Render();
	// glEnable(GL_DEPTH_TEST);
	modelViewMatrixStack->Pop();

	shader->SetUniform("lightingOn", true);
}
