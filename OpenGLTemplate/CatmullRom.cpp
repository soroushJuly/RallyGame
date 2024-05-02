#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>



CCatmullRom::CCatmullRom(const std::string filename, const std::string filename2)
{
	m_vertexCount = 0;
	m_texture.Load(filename);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_texture1.Load(filename2);
	m_texture1.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture1.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture1.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture1.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
}

CCatmullRom::~CCatmullRom()
{
	m_texture.Release();
	m_texture1.Release();
}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return a + b * t + c * t2 + d * t3;
}


void CCatmullRom::SetControlPoints()
{
	// Set control points (m_controlPoints) here, or load from disk
	// First line with up and downs 0 - 11
	m_controlPoints.push_back(glm::vec3(-500, 1.51f, -400));
	m_controlPoints.push_back(glm::vec3(-500, 10.01f, 0));
	m_controlPoints.push_back(glm::vec3(-500, 20.01f, 100));
	m_controlPoints.push_back(glm::vec3(-500, 30.01f, 200));
	m_controlPoints.push_back(glm::vec3(-500, 50.01f, 300));
	m_controlPoints.push_back(glm::vec3(-500, 50.01f, 400));
	m_controlPoints.push_back(glm::vec3(-500, 40.01f, 500));
	m_controlPoints.push_back(glm::vec3(-500, 20.01f, 600));
	m_controlPoints.push_back(glm::vec3(-500, 10.01f, 700));
	m_controlPoints.push_back(glm::vec3(-500, 5.01f, 750));
	m_controlPoints.push_back(glm::vec3(-500, 2.51f, 800));
	m_controlPoints.push_back(glm::vec3(-500, 0.51f, 1000));
	//// Round 12
	m_controlPoints.push_back(glm::vec3(-300, 5.01f, 1300));
	//// Marpich
	m_controlPoints.push_back(glm::vec3(300, 0.51f, 1100));
	m_controlPoints.push_back(glm::vec3(-220, 0.51f, 700));
	m_controlPoints.push_back(glm::vec3(300, 0.51f, 400));
	m_controlPoints.push_back(glm::vec3(0, 0.51f, 100));
	m_controlPoints.push_back(glm::vec3(-100, 0.51f, -100));
	m_controlPoints.push_back(glm::vec3(100, 0.51f, -500));
	m_controlPoints.push_back(glm::vec3(-100, 1.51f, -800));

	// Optionally, set upvectors (m_controlUpVectors, one for each control point as well)
}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int)m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i - 1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M - 1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3& p, glm::vec3& up)
{
	if (d < 0)
		return false;

	int M = (int)m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int)(d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size() - 1; i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;

	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j - 1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));

	return true;
}



// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
	glm::vec3 p, up;

	// Compute the lengths of each segment along the control polygon, and the total length
	ComputeLengthsAlongControlPoints();
	float fTotalLength = m_distances[m_distances.size() - 1];

	// The spacing will be based on the control polygon
	float fSpacing = fTotalLength / numSamples;

	// Call PointAt to sample the spline, to generate the points
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);

	}


	// Repeat once more for truly equidistant points
	m_controlPoints = m_centrelinePoints;
	m_controlUpVectors = m_centrelineUpVectors;
	m_centrelinePoints.clear();
	m_centrelineUpVectors.clear();
	m_distances.clear();
	ComputeLengthsAlongControlPoints();
	fTotalLength = m_distances[m_distances.size() - 1];
	fSpacing = fTotalLength / numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);
	}


}



void CCatmullRom::CreateCentreline()
{
	// Call Set Control Points
	SetControlPoints();
	// Call UniformlySampleControlPoints with the number of samples required
	UniformlySampleControlPoints(SAMPLE_NUM);
	// Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
	glGenVertexArrays(1, &m_vaoCentreline);
	glBindVertexArray(m_vaoCentreline);
	// Create a VBO
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	glm::mat4 transform = glm::mat4(1.f);
	for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
		float t = (float)i / m_centrelinePoints.size();
		glm::vec3 v = Interpolate(
			m_centrelinePoints[i % m_centrelinePoints.size()],
			m_centrelinePoints[(i + 1) % m_centrelinePoints.size()],
			m_centrelinePoints[(i + 2) % m_centrelinePoints.size()],
			m_centrelinePoints[(i + 3) % m_centrelinePoints.size()], t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
		vbo.AddData(&transform, sizeof(glm::mat4));
	}
	// Upload the VBO to the GPU
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	//GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

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

// TODO: should I interpolate these too?
// TODO: If yes should I interpolate the triangle too ?
void CCatmullRom::CreateOffsetCurves(const float width)
{
	// Compute the offset curves, one left, and one right.
	// Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
	for (int i = 0; i < m_centrelinePoints.size(); i++)
	{
		glm::vec3 pNext = m_centrelinePoints[(i + 1) % m_centrelinePoints.size()];
		glm::vec3 N = glm::cross(glm::normalize(pNext - m_centrelinePoints[i]), glm::vec3(0.f, 1.f, 0.f));
		// N assumed to be y axis
		m_leftOffsetPoints.push_back(m_centrelinePoints[i] - (width / 2 * N));
		m_rightOffsetPoints.push_back(m_centrelinePoints[i] + (width / 2 * N));
	}
	// Generate two VAOs called m_vaoLeftOffsetCurve and m_vaoRightOffsetCurve, each with a VBO, and get the offset curve points on the graphics card
	// Note it is possible to only use one VAO / VBO with all the points instead.
	glGenVertexArrays(1, &m_vaoLeftOffsetCurve);
	glBindVertexArray(m_vaoLeftOffsetCurve);
	// Create a VBO
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::mat4 transform = glm::mat4(1.f);
	for (unsigned int i = 0; i < m_leftOffsetPoints.size(); i++) {
		vbo.AddData(&m_leftOffsetPoints[i], sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
		vbo.AddData(&transform, sizeof(glm::mat4));
	}
	// Upload the VBO to the GPU
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	//GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

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


	// Right offset
	glGenVertexArrays(1, &m_vaoRightOffsetCurve);
	glBindVertexArray(m_vaoRightOffsetCurve);
	// Create a VBO
	CVertexBufferObject vbo2;
	vbo2.Create();
	vbo2.Bind();
	for (unsigned int i = 0; i < m_rightOffsetPoints.size(); i++) {
		vbo2.AddData(&m_rightOffsetPoints[i], sizeof(glm::vec3));
		vbo2.AddData(&texCoord, sizeof(glm::vec2));
		vbo2.AddData(&normal, sizeof(glm::vec3));
		vbo2.AddData(&transform, sizeof(glm::mat4));
	}
	// Upload the VBO to the GPU
	vbo2.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	//GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

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

void CCatmullRom::CreateTrack()
{
	// Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
	glGenVertexArrays(1, &m_vaoTrack);
	glBindVertexArray(m_vaoTrack);

	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	// 2 triangles for each point in the path is needed
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	glm::mat4 transform = glm::mat4(1.f);
	for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
		// Add first triangle
		{
			glm::vec2 texCoord(TEXT_REPEAT, 0.0f);
			vbo.AddData(&m_leftOffsetPoints[i], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));

			texCoord = glm::vec2(0.0f, 0.0f);
			vbo.AddData(&m_rightOffsetPoints[i], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));

			texCoord = glm::vec2(TEXT_REPEAT, TEXT_REPEAT);
			vbo.AddData(&m_leftOffsetPoints[(i + 1) % m_centrelinePoints.size()], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));
			m_vertexCount += 3;
		}
		// Add second triangle
		{
			glm::vec2 texCoord(0.0f, 0.0f);
			vbo.AddData(&m_rightOffsetPoints[i], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));

			texCoord = glm::vec2(TEXT_REPEAT, TEXT_REPEAT);
			vbo.AddData(&m_leftOffsetPoints[(i + 1) % m_centrelinePoints.size()], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));

			texCoord = glm::vec2(0.0f, TEXT_REPEAT);
			vbo.AddData(&m_rightOffsetPoints[(i + 1) % m_centrelinePoints.size()], sizeof(glm::vec3));
			vbo.AddData(&texCoord, sizeof(glm::vec2));
			vbo.AddData(&normal, sizeof(glm::vec3));
			vbo.AddData(&transform, sizeof(glm::mat4));
			m_vertexCount += 3;
		}
	}
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::mat4);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

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


void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
	glBindVertexArray(m_vaoCentreline);
	glLineWidth(1);
	glPointSize(4);
	glDrawArrays(GL_POINTS, 0, m_centrelinePoints.size());
	glDrawArrays(GL_LINE_LOOP, 0, m_centrelinePoints.size());
}

void CCatmullRom::RenderOffsetCurves()
{
	// Bind the VAO m_vaoLeftOffsetCurve and render it
	glBindVertexArray(m_vaoLeftOffsetCurve);
	glLineWidth(1);
	glPointSize(4);
	glDrawArrays(GL_POINTS, 0, m_leftOffsetPoints.size());
	glDrawArrays(GL_LINE_LOOP, 0, m_leftOffsetPoints.size());
	// Bind the VAO m_vaoRightOffsetCurve and render it
	glBindVertexArray(m_vaoRightOffsetCurve);
	glLineWidth(1);
	glPointSize(4);
	glDrawArrays(GL_POINTS, 0, m_rightOffsetPoints.size());
	glDrawArrays(GL_LINE_LOOP, 0, m_rightOffsetPoints.size());
}


void CCatmullRom::RenderTrack()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// Bind the VAO m_vaoTrack and render it
	glBindVertexArray(m_vaoTrack);
	m_texture.Bind(0);
	m_texture1.Bind(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CCatmullRom::CreatePath(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
{
	// Use VAO to store state associated with vertices
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	// Create a VBO
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < 100; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = Interpolate(p0, p1, p2, p3, t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}
	// Upload the VBO to the GPU
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CCatmullRom::RenderPath()
{
	glBindVertexArray(m_vao);
	glLineWidth(50);
	glDrawArrays(GL_LINE_STRIP, 0, 100);
}

int CCatmullRom::CurrentLap(float d)
{
	return (int)(d / m_distances.back());
}

glm::vec3 CCatmullRom::_dummy_vector(0.0f, 0.0f, 0.0f);