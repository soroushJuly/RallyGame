#include "Quad.h" 

Quad::Quad() {}
Quad::~Quad() { Release(); }

void Quad::Create(string filename, float halfHeight, float halfWidth) {
	m_texture.Load(filename);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vbo.Create();
	m_vbo.Bind();

	float textureRepeat = 1.f;

	// Vertex positions
	glm::vec3 planeVertices[4] =
	{
		// Front Side
		glm::vec3(halfWidth, halfHeight, 0),
		glm::vec3(-halfWidth, halfHeight, 0),
		glm::vec3(halfWidth, -halfHeight, 0),
		glm::vec3(-halfWidth, -halfHeight, 0),
	};

	// Texture coordinates
	glm::vec2 planeTexCoords[4] =
	{
		glm::vec2(textureRepeat, textureRepeat),
		glm::vec2(0.0f, textureRepeat),
		glm::vec2(textureRepeat, 0.0f),
		glm::vec2(0.0f, 0.0f),
	};

	// Plane normal
	glm::vec3 planeNormals[1] =
	{
		glm::vec3(0.0f, 0.0f, -1.0f),
	};
	glm::mat4 transfrom(1.f);
	// Put the vertex attributes in the VBO
	for (unsigned int i = 0; i < sizeof(planeVertices) / sizeof(glm::vec3); i++) {
		m_vbo.AddData(&planeVertices[i], sizeof(glm::vec3));
		m_vbo.AddData(&planeTexCoords[i % 4], sizeof(glm::vec2));
		m_vbo.AddData(&planeNormals[0], sizeof(glm::vec3));
		m_vbo.AddData(&transfrom, sizeof(glm::mat4));
	}
	// Upload data to GPU 
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);


	// Set the vertex attribute locations
	GLsizei istride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, istride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, istride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, istride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	// Transform:
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)32);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + sizeof(glm::vec4)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + 2 * sizeof(glm::vec4)));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + 3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
}
void Quad::Create(float halfHeight, float halfWidth) {
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vbo.Create();
	m_vbo.Bind();

	float textureRepeat = 1.f;

	// Vertex positions
	glm::vec3 planeVertices[4] =
	{
		// Front Side
		glm::vec3(halfWidth, halfHeight, 0),
		glm::vec3(-halfWidth, halfHeight, 0),
		glm::vec3(halfWidth, -halfHeight, 0),
		glm::vec3(-halfWidth, -halfHeight, 0),
	};

	// Texture coordinates
	glm::vec2 planeTexCoords[4] =
	{
		glm::vec2(textureRepeat, textureRepeat),
		glm::vec2(0.0f, textureRepeat),
		glm::vec2(textureRepeat, 0.0f),
		glm::vec2(0.0f, 0.0f),
	};

	// Plane normal
	glm::vec3 planeNormals[1] =
	{
		glm::vec3(0.0f, 0.0f, -1.0f),
	};
	glm::mat4 transfrom(1.f);
	// Put the vertex attributes in the VBO
	for (unsigned int i = 0; i < sizeof(planeVertices) / sizeof(glm::vec3); i++) {
		m_vbo.AddData(&planeVertices[i], sizeof(glm::vec3));
		m_vbo.AddData(&planeTexCoords[i % 4], sizeof(glm::vec2));
		m_vbo.AddData(&planeNormals[0], sizeof(glm::vec3));
		m_vbo.AddData(&transfrom, sizeof(glm::mat4));
	}
	// Upload data to GPU 
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);


	// Set the vertex attribute locations
	GLsizei istride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, istride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, istride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, istride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	// Transform:
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)32);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + sizeof(glm::vec4)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + 2 * sizeof(glm::vec4)));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(32 + 3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
}
void Quad::Render() {
	glBindVertexArray(m_vao);
	m_texture.Bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void Quad::RenderNoTexture() {
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void Quad::Release() {
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);
	m_vbo.Release();
}
