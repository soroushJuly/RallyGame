#pragma once

#include "Common.h"
#include "vertexBufferObject.h"
#include "vertexBufferObjectIndexed.h"
#include "Texture.h"


class CCatmullRom
{
public:
	CCatmullRom(const std::string filename, const std::string filename2);
	~CCatmullRom();

	void CreateCentreline();
	void RenderCentreline();

	void CreateOffsetCurves(const float width);
	void RenderOffsetCurves();

	void CreateTrack();
	void RenderTrack();

	// Create an interpolated path between only 4 points (NOT USED in track making)
	void CreatePath(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
	void RenderPath();

	int CurrentLap(float d); // Return the currvent lap (starting from 0) based on distance along the control curve.

	bool Sample(float d, glm::vec3& p, glm::vec3& up = _dummy_vector); // Return a point on the centreline based on a certain distance along the control curve.

	// Getters
	vector<glm::vec3> GetCenterPoints() { return m_centrelinePoints; }
	vector<glm::vec3> GetLeftPoints() { return m_leftOffsetPoints; }
	vector<glm::vec3> GetRightPoints() { return m_rightOffsetPoints; }
private:

	void SetControlPoints();
	void ComputeLengthsAlongControlPoints();
	void UniformlySampleControlPoints(int numSamples);

	// Using points provided calculate coefficients and return the equation for line
	glm::vec3 Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t);

	GLuint m_vao;


	vector<float> m_distances;
	CTexture m_texture;
	CTexture m_texture1;

	GLuint m_vaoCentreline;
	GLuint m_vaoLeftOffsetCurve;
	GLuint m_vaoRightOffsetCurve;
	GLuint m_vaoTrack;

	static glm::vec3 _dummy_vector;
	// Control points, which are interpolated to produce the centreline points
	vector<glm::vec3> m_controlPoints;		
	vector<glm::vec3> m_controlUpVectors;	// Control upvectors, which are interpolated to produce the centreline upvectors
	vector<glm::vec3> m_centrelinePoints;	// Centreline points
	vector<glm::vec3> m_centrelineUpVectors;// Centreline upvectors

	vector<glm::vec3> m_leftOffsetPoints;	// Left offset curve points
	vector<glm::vec3> m_rightOffsetPoints;	// Right offset curve points


	unsigned int m_vertexCount;				// Number of vertices in the track VBO

	const float SAMPLE_NUM = 200;
	const float TEXT_REPEAT = 2.f;
};
