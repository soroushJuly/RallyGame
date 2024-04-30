#include "Common.h"
#include "Arrow.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


CArrow::CArrow()
{}

CArrow::~CArrow()
{
	Release();
}

struct CVertex
{
	glm::vec3 m_position;
	glm::vec2 m_textCoordinate;
	glm::vec3 m_normal;
	glm::mat4 transform = glm::mat4(1.f);
	CVertex() {};
	CVertex(glm::vec3 position, glm::vec2 textCoordinate, glm::vec3 normal) :
		m_position(position), m_textCoordinate(textCoordinate), m_normal(normal)
	{}
};

// Create the plane, including its geometry, texture mapping, normal, and colour
void CArrow::Create(string directory, string filename, float width, float height, float thickness, float textureRepeat)
{

	m_width = width;
	m_height = height;
	m_thickness = thickness;

	// Load the texture
	m_texture.Load(directory + filename);

	m_directory = directory;
	m_filename = filename;

	// Set parameters for texturing using sampler object
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	glm::vec3 vPositions[36] =
	{
		// Front face
		glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * .125f),		glm::vec3(1.f * m_width, .25f * m_height, m_thickness * 0.125f),	glm::vec3(1.f * m_width, -.75f * m_height, m_thickness * 0.125f),
		glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * .125f),		glm::vec3(-1.f * m_width, -.75f * m_height, m_thickness * 0.125f),  glm::vec3(-1.f * m_width, .25f * m_height, m_thickness * 0.125f),
		// Back face
		glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * -.125f),		glm::vec3(1.f * m_width, .25f * m_height, m_thickness * -0.125f),	glm::vec3(1.f * m_width, -0.75f * m_height, m_thickness * -0.125f),
		glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * -.125f),		glm::vec3(-1.f * m_width, -.75f * m_height, m_thickness * -0.125f), glm::vec3(-1.f * m_width, .25f * m_height, m_thickness * -0.125f),
		// Vertices in Rectangle – 0, 1, 6, 7
		glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * 0.125f),		glm::vec3(1.f * m_width, 0.25f * m_height, m_thickness * 0.125f),
		glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * -0.125),		glm::vec3(1.f * m_width, 0.25 * m_height, m_thickness * -0.125),
		// Vertices in Rectangle – 1, 2, 7, 8
		glm::vec3(1.f * m_width, 0.25f * m_height, m_thickness * 0.125f),	glm::vec3(1.f * m_width, -0.75f * m_height, m_thickness * 0.125f),
		glm::vec3(1.f * m_width, 0.25f * m_height, m_thickness * -0.125),	glm::vec3(1.f * m_width, -0.75f * m_height, m_thickness * -0.125f),
		// Vertices in Rectangle – 2, 3, 8, 9
		glm::vec3(1.f * m_width, -0.75f * m_height, m_thickness * 0.125f),	glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * 0.125f),
		glm::vec3(1.f * m_width, -0.75f * m_height, m_thickness * -0.125f), glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * -0.125),
		// Vertices in Rectangle – 3, 4, 9, 10
		glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * 0.125),		glm::vec3(-1.f * m_width, -0.75f * m_height, m_thickness * 0.125),
		glm::vec3(0.f * m_width, 0.f * m_height, m_thickness * -0.125),		glm::vec3(-1.f * m_width, -0.75f * m_height, m_thickness * -0.125),
		// Vertices in Rectangle – 4, 5, 10, 11
		glm::vec3(-1.f * m_width, -0.75f * m_height, m_thickness * 0.125),	glm::vec3(-1.f * m_width, 0.25f * m_height, m_thickness * 0.125),
		glm::vec3(-1.f * m_width, -0.75f * m_height, m_thickness * -0.125), glm::vec3(-1.f * m_width, 0.25f * m_height, m_thickness * -0.125),
		// Vertices in Rectangle – 5, 0, 11, 6
		glm::vec3(-1.f * m_width, 0.25f * m_height, m_thickness * 0.125),	glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * 0.125),
		glm::vec3(-1.f * m_width, 0.25f * m_height, m_thickness * -0.125),	glm::vec3(0.f * m_width, 1.f * m_height, m_thickness * -0.125),
	};
	glm::vec2 vTexCoords[4] =
	{
		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.0f, textureRepeat),
		glm::vec2(textureRepeat, 0.0f),
		glm::vec2(textureRepeat, textureRepeat)
	};

	glm::vec3 vNormals[8] =
	{
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::cross(vPositions[1] - vPositions[0], vPositions[6] - vPositions[0]),
		glm::cross(vPositions[2] - vPositions[1], vPositions[7] - vPositions[1]),
		glm::cross(vPositions[3] - vPositions[2], vPositions[8] - vPositions[2]),
		glm::cross(vPositions[4] - vPositions[3], vPositions[9] - vPositions[3]),
		glm::cross(vPositions[5] - vPositions[4], vPositions[10] - vPositions[4]),
		glm::cross(vPositions[6] - vPositions[5], vPositions[11] - vPositions[5])
	};
	CVertex vVertices[36]
	{
		CVertex(vPositions[0], vTexCoords[3], vNormals[0]),
		CVertex(vPositions[1], vTexCoords[0], vNormals[0]),
		CVertex(vPositions[2], vTexCoords[1], vNormals[0]),
		CVertex(vPositions[3], vTexCoords[2], vNormals[0]),
		CVertex(vPositions[4], vTexCoords[0], vNormals[0]),
		CVertex(vPositions[5], vTexCoords[1], vNormals[0]),
		CVertex(vPositions[6], vTexCoords[3], vNormals[1]),
		CVertex(vPositions[7], vTexCoords[0], vNormals[1]),
		CVertex(vPositions[8], vTexCoords[1], vNormals[1]),
		CVertex(vPositions[9], vTexCoords[2], vNormals[1]),
		CVertex(vPositions[10], vTexCoords[0], vNormals[1]),
		CVertex(vPositions[11], vTexCoords[1], vNormals[1]),

		CVertex(vPositions[12], vTexCoords[1], vNormals[2]),
		CVertex(vPositions[13], vTexCoords[3], vNormals[2]),
		CVertex(vPositions[14], vTexCoords[2], vNormals[2]),
		CVertex(vPositions[15], vTexCoords[0], vNormals[2]),

		CVertex(vPositions[16], vTexCoords[1], vNormals[3]),
		CVertex(vPositions[17], vTexCoords[3], vNormals[3]),
		CVertex(vPositions[18], vTexCoords[2], vNormals[3]),
		CVertex(vPositions[19], vTexCoords[0], vNormals[3]),

		CVertex(vPositions[20], vTexCoords[1], vNormals[4]),
		CVertex(vPositions[21], vTexCoords[3], vNormals[4]),
		CVertex(vPositions[22], vTexCoords[2], vNormals[4]),
		CVertex(vPositions[23], vTexCoords[0], vNormals[4]),

		CVertex(vPositions[24], vTexCoords[1], vNormals[5]),
		CVertex(vPositions[25], vTexCoords[3], vNormals[5]),
		CVertex(vPositions[26], vTexCoords[2], vNormals[5]),
		CVertex(vPositions[27], vTexCoords[0], vNormals[5]),

		CVertex(vPositions[28], vTexCoords[1], vNormals[6]),
		CVertex(vPositions[29], vTexCoords[3], vNormals[6]),
		CVertex(vPositions[30], vTexCoords[2], vNormals[6]),
		CVertex(vPositions[31], vTexCoords[0], vNormals[6]),

		CVertex(vPositions[32], vTexCoords[1], vNormals[7]),
		CVertex(vPositions[33], vTexCoords[3], vNormals[7]),
		CVertex(vPositions[34], vTexCoords[2], vNormals[7]),
		CVertex(vPositions[35], vTexCoords[0], vNormals[7]),
	};

	GLint vIndices[60] = {
		// Front face triangles
		1,0,3, // 1,0,3 vertices
		2,1,3,
		3,5,4, // 3,5,4 vertices
		3,0,5,
		// Back face triangles
		6,7,9, // 6,7,9 vertices
		9,7,8,
		11,9,10,
		11,6,9,
		// Faces connecting the back and front faces
		12,13,14, // 0,1,6 vertices
		13,15,14, // 1,7,6 vertices

		16,17,18, // 1,2,7 vertices
		17,19,18, // 2,8,7 vertices

		20,21,22, // 2,3,8 vertices
		21,23,22, // 3,9,8 vertices

		24,25,26, // 3,4,9 vertices
		25,27,26,

		28,29,30, // 4,5,10 vertices
		29,31,30,

		32,33,34, // 5,0,11 vertices
		33,35,34 // 0,6,11 vertices
	};


	// Use VAO to store state associated with vertices
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Creating and binding a VBO
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	// Pass vertices to the GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), &vVertices[0], GL_STATIC_DRAW);

	// Creating indices VBO
	glGenBuffers(1, &m_vbo_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	// Pass indices to the program
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndices), &vIndices[0], GL_STATIC_DRAW);

	// Set the vertex attribute locations
	GLsizei istride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);
	//GLsizei istride = sizeof(CVertex);

	// Enabling positions - using strides
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, istride, 0);

	// Enabling texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, istride, (const GLvoid*)(sizeof(glm::vec3)));

	// Enabling normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, istride, (const GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

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

// Render the plane as a triangle strip
void CArrow::Render()
{
	glBindVertexArray(m_vao);
	m_texture.Bind();

	glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);
}

// Release resources
void CArrow::Release()
{
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_vbo_indices);
}