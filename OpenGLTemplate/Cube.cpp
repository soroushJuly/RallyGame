#include "Cube.h" 

CCube::CCube() {}
CCube::~CCube() { Release(); }

void CCube::Create(string filename, float halfHeight) {
	m_texture.Load(filename);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vbo.Create();
	m_vbo.Bind();
	// Write the code to add interleaved vertex attributes to the VBO 
	//float halfWidth = m_width / 2.0f;
	//float halfHeight = m_height / 2.0f;
	float halfWidth = 1.0f;
	//float halfHeight = halfHeight;
	float halfDepth = 1.0f;
	float textureRepeat = 1.f;

	// Vertex positions
	glm::vec3 planeVertices[24] =
	{
		// Front Side
		glm::vec3(halfWidth, halfHeight, halfDepth),glm::vec3(-halfWidth, halfHeight, halfDepth), glm::vec3(halfWidth, -halfHeight, halfDepth),
		 glm::vec3(-halfWidth, -halfHeight, halfDepth),
		// Back Side
		glm::vec3(-halfWidth, halfHeight, -halfDepth),glm::vec3(halfWidth, halfHeight, -halfDepth), glm::vec3(-halfWidth, -halfHeight, -halfDepth),
		 glm::vec3(halfWidth, -halfHeight, -halfDepth),
		// Left Side
		glm::vec3(-halfWidth, halfHeight, halfDepth),glm::vec3(-halfWidth, halfHeight, -halfDepth), glm::vec3(-halfWidth, -halfHeight, halfDepth),
		 glm::vec3(-halfWidth, -halfHeight, -halfDepth),
		// Right Side
		glm::vec3(halfWidth, halfHeight, -halfDepth),glm::vec3(halfWidth, halfHeight, halfDepth), glm::vec3(halfWidth, -halfHeight, -halfDepth),
		 glm::vec3(halfWidth, -halfHeight, halfDepth),
		// Top Side
		glm::vec3(-halfWidth, halfHeight, -halfDepth),glm::vec3(-halfWidth, halfHeight, halfDepth), glm::vec3(halfWidth, halfHeight, -halfDepth),
		 glm::vec3(halfWidth, halfHeight, halfDepth),
		// Bottom Side
		glm::vec3(halfWidth, -halfHeight, -halfDepth),glm::vec3(halfWidth, -halfHeight, halfDepth), glm::vec3(-halfWidth, -halfHeight, -halfDepth),
		 glm::vec3(-halfWidth, -halfHeight, halfDepth),
	};

	// Texture coordinates
	glm::vec2 planeTexCoords[4] =
	{
		glm::vec2(0.0f, textureRepeat), 
		glm::vec2(0.0f, 0.0f), 
		glm::vec2(textureRepeat, textureRepeat), 
		glm::vec2(textureRepeat, 0.0f)
	};

	// Plane normal
	glm::vec3 planeNormals[6] =
	{
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};

	// Put the vertex attributes in the VBO
	for (unsigned int i = 0; i < sizeof(planeVertices) / sizeof(glm::vec3); i++) {
		m_vbo.AddData(&planeVertices[i], sizeof(glm::vec3));
		m_vbo.AddData(&planeTexCoords[i % 4], sizeof(glm::vec2));
		m_vbo.AddData(&planeNormals[(int)(i / 4)], sizeof(glm::vec3));
	}
	// Upload data to GPU 
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);


	// Set the vertex attribute locations
	GLsizei istride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, istride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, istride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, istride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}
void CCube::Render() {
	glBindVertexArray(m_vao);
	m_texture.Bind();
	// Call glDrawArrays to render each side
	for (int i = 0; i < 6; i++)
	{
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}
}
void CCube::Release() {
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);
	m_vbo.Release();
}
