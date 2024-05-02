#include "Actor.h"
#include "OpenAssetImportMesh.h"
#include "Camera.h"
#include "Shaders.h"

Actor::Actor()
{
	m_position = glm::vec3(0.f, 0.f, 0.f);
	m_scale = glm::vec3(1.f);
	m_rotationAxis = glm::vec3(0.f, 1.f, 0.f);
	m_rotation = 0.0f;
	m_mesh = NULL;
}

Actor::~Actor()
{
	delete m_mesh;
}

void Actor::Initialise(const std::string& filePath)
{
	m_mesh = new COpenAssetImportMesh;
	m_mesh->Load(filePath);
}

void Actor::Update(double deltaTime)
{
	//m_position += glm::vec3(0.001 * deltaTime, 0.f, 0.001 * deltaTime);
}

void Actor::Render(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shaderProgram, std::shared_ptr<CCamera> camera)
{
	modelViewMatrixStack->Push();
	modelViewMatrixStack->Translate(GetPosition());
	//modelViewMatrixStack->Translate(glm::vec3(1,1,1));
	modelViewMatrixStack->ApplyMatrix(m_orientation);
	modelViewMatrixStack->Rotate(m_rotationAxis, m_rotation);
	modelViewMatrixStack->Scale(m_scale);
	shaderProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shaderProgram->SetUniform("matrices.normalMatrix", camera->ComputeNormalMatrix(modelViewMatrixStack->Top()));
	GetMesh()->Render();
	modelViewMatrixStack->Pop();
}

void Actor::RenderInstanced(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shaderProgram,
	std::shared_ptr<CCamera> camera, glm::mat4* modelMatrices, int amount)
{
	modelViewMatrixStack->Push();
	modelViewMatrixStack->Translate(glm::vec3(0,0,0));
	modelViewMatrixStack->Scale(1);
	//shaderProgram->SetUniform("isInstanced", true);
	shaderProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack->Top());
	shaderProgram->SetUniform("matrices.normalMatrix", camera->ComputeNormalMatrix(modelViewMatrixStack->Top()));
	
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	glBindVertexArray(GetMesh()->m_vao);
	// set attribute pointers for instance transform matrix (4 times vec4)
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 + sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 + 2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 + 3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	//glBindVertexArray(0);
	GetMesh()->RenderInstanced(amount, modelMatrices);
	shaderProgram->SetUniform("isInstanced", false);
	modelViewMatrixStack->Pop();
}
