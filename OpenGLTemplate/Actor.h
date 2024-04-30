#pragma once

#include "Common.h"
#include "GameWindow.h"
#include "MatrixStack.h"

class COpenAssetImportMesh;
class CShaderProgram;
class CCamera;

class Actor {
public:
	Actor();
	~Actor();

	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise(const std::string& filePath);
	void Update(double deltaTime);
	void Render(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shaderProgram, std::shared_ptr<CCamera> camera);
	void RenderInstanced(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shaderProgram,
		std::shared_ptr<CCamera> camera, glm::mat4* modelMatrices, int amount);


	COpenAssetImportMesh* GetMesh() const { return m_mesh; }
	const glm::vec3 GetPosition() const { return m_position; }
	const float GetRotationAmountY() const { return m_rotation; }
	void AddPosition(glm::vec3 position) { m_position += position; }
	// Setters
	void SetPosition(glm::vec3 position) { m_position = position; }
	void SetRotation(glm::vec3 axis, float rotationAmount) { m_rotationAxis = axis; m_rotation = rotationAmount; }
	void SetScale(glm::vec3 scale) { m_scale = scale; }
	void SetOrientation(glm::mat4 orientation) { m_orientation = orientation; }

private:
	// Pointers to object mesh. 
	COpenAssetImportMesh* m_mesh;
	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::vec3 m_rotationAxis;
	glm::mat4 m_orientation;

	float m_rotation;
};
