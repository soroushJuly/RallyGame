#include "Common.h"
#include "Torus.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


CTorus::CTorus()
{
	tubeRadius = 0.f;
}

CTorus::~CTorus()
{
	Release();
}

// Create the plane, including its geometry, texture mapping, normal, and colour
void CTorus::Create(string directory, string filename, float mainRadius, float tubeRadius, float thickness, float textureRepeat)
{
	// Load the texture
	m_texture.Load(directory + filename);

	m_directory = directory;
	m_filename = filename;

	// Set parameters for texturing using sampler object
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	vector<glm::vec3> circleCenters;
	//vertices = std::vector<CVertex>();


	mainSlices = 20;   // Number of stacks forming the central ring
	tubeSlices = 20;     // Number of slices forming the tube

	float ringRadius = mainRadius;  // Radius of the central ring
	this->tubeRadius = tubeRadius;  // Radius of the tube


	// Calculate and cache counts of vertices and indices
	// We need one more slice for the ending of each circle
	const auto numVertices = (mainSlices + 1) * (tubeSlices + 1);
	primitiveRestartIndex = numVertices;
	numIndices = (mainSlices * 2 * (tubeSlices + 1)) + mainSlices - 1;

	float mainSegmentAngleStep = glm::radians(360.0f / float(mainSlices));
	float tubeSegmentAngleStep = glm::radians(360.0f / float(tubeSlices));

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();

	glm::mat4 transfrom = glm::mat4(1.f);

	auto currentMainSegmentAngle = 0.0f;

	// Texture coordinates
	auto mainSegmentTextureStep = 16.0f / float(mainSlices);
	auto tubeSegmentTextureStep = 4.f / float(tubeSlices);
	auto currentMainSegmentTexCoordV = 0.0f;

	for (auto i = 0; i <= mainSlices; i++)
	{
		// Texture
		auto currentTubeSegmentTexCoordU = 0.0f;
		// Calculate sine and cosine of main segment angle
		auto sinMainSegment = sin(currentMainSegmentAngle);
		auto cosMainSegment = cos(currentMainSegmentAngle);
		auto currentTubeSegmentAngle = 0.0f;
		for (auto j = 0; j <= tubeSlices; j++)
		{
			// Calculate sine and cosine of tube segment angle
			auto sinTubeSegment = sin(currentTubeSegmentAngle);
			auto cosTubeSegment = cos(currentTubeSegmentAngle);

			// Calculate vertex position on the surface of torus
			auto surfacePosition = glm::vec3(
				(ringRadius + tubeRadius * cosTubeSegment) * cosMainSegment,
				(ringRadius + tubeRadius * cosTubeSegment) * sinMainSegment,
				tubeRadius * sinTubeSegment);

			vbo.AddData(&surfacePosition, sizeof(glm::vec3));

			// Texture
			auto textureCoordinate = glm::vec2(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV);
			vbo.AddData(&textureCoordinate, sizeof(glm::vec2));

			currentTubeSegmentTexCoordU += tubeSegmentTextureStep;

			// Update current tube angle
			auto normal = glm::normalize(glm::vec3(
				cosMainSegment * cosTubeSegment,
				sinMainSegment * cosTubeSegment,
				sinTubeSegment
			));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transfrom, sizeof(glm::mat4));

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
		// Update texture coordinate of main segment
		currentMainSegmentTexCoordV += mainSegmentTextureStep;
	}

	// Creating indices
	GLuint currentVertexOffset = 0;
	for (auto i = 0; i < mainSlices; i++)
	{
		for (auto j = 0; j <= tubeSlices; j++)
		{
			GLuint vertexIndexA = currentVertexOffset;
			vIndices.push_back(vertexIndexA);
			GLuint vertexIndexB = currentVertexOffset + tubeSlices + 1;
			vIndices.push_back(vertexIndexB);
			currentVertexOffset++;
		}

		// Don't restart primitive, if it's last segment, rendering ends here anyway
		if (i != mainSlices - 1) {
			//vbo_indices.AddData(&primitiveRestartIndex, sizeof(GLuint));
			vIndices.push_back(primitiveRestartIndex);
		}
	}

	vbo.UploadDataToGPU(GL_STATIC_DRAW);

	// Use VAO to store state associated with vertices


	// Creating and binding a VBO
	//glGenBuffers(1, &m_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CVertex), &vertices[0], GL_STATIC_DRAW);
	GLsizei istride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);
	// Creating indices VBO
	glGenBuffers(1, &m_vbo_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	// Pass indices to the program
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(GLuint), &vIndices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, istride, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, istride, (void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);

	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, istride, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

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
void CTorus::Render()
{
	glBindVertexArray(m_vao);
	m_texture.Bind();

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(primitiveRestartIndex);
	glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, 0);
	glDisable(GL_PRIMITIVE_RESTART);

	glBindVertexArray(0);
}

// Release resources
void CTorus::Release()
{
	m_texture.Release();
	glDeleteVertexArrays(1, &m_vao);

	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_vbo_indices);
}