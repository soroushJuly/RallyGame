#pragma once

#include "Texture.h"
#include "VertexBufferObject.h"
#include "vertexBufferObjectIndexed.h"

//class CVertex;

// Class for generating a xz plane of a given size
class CTorus
{
public:
	CTorus();
	~CTorus();
	void Create(string sDirectory, string sFilename, float mainRadius, float ringRadius, float thickness, float fTextureRepeat);
	void Render();
	void Release();

	void renderSpecial(int segments) const;

private:
	UINT m_vao;
	//CVertexBufferObject m_vbo;
	//vector<CVertex> vertices;
	GLuint m_vbo;
	GLuint m_vbo_indices;
	vector<GLint> vIndices;
	CTexture m_texture;
	string m_directory;
	string m_filename;
	float m_width;
	float m_thickness;
	float m_height;



	int mainSlices;
	int tubeSlices;
	float mainRadius;
	float tubeRadius;

	int numIndices = 0;
	int primitiveRestartIndex = 0;
};