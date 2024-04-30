#pragma once
#include "Common.h" 
#include "Texture.h" 
#include "VertexBufferObject.h"
// Class for generating a unit cube
class Quad {
public:
	Quad();
	~Quad();
	void Create(string filename, float halfHeight, float halfWidth);
	void Create(float halfHeight, float halfWidth);
	void Render();
	void RenderNoTexture();
	void Release();
private:
	GLuint m_vao;
	CVertexBufferObject m_vbo;
	CTexture m_texture;
};