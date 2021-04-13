#ifdef USE_OPENGL
#include <iostream>
#include "gl_object3d.hpp"
using std::cerr;
using std::endl;

/*CObject3DGL start*/
CObject3DGL::CObject3DGL()
{

}

CObject3DGL::~CObject3DGL()
{
	if (m_vao != -1) glDeleteVertexArrays(1, &m_vao);
	if (m_vbo != -1) glDeleteBuffers(1, &m_vbo);
	if (m_ebo != -1) glDeleteBuffers(1, &m_ebo);
}

CObject3DGL::CObject3DGL(const glm::mat4& model, const shared_ptr<CShaderGL> shader)
{
	m_model = model;
	m_shaders[0] = shader;
	glCheckError();
}

glm::mat4& CObject3DGL::getModel()
{
	return m_model;
}

void CObject3DGL::setModel(const glm::mat4& model)
{
	m_model = model;
}

void CObject3DGL::setDrawMode(GLenum drawMode)
{
	m_drawMode = drawMode;
}

GLuint CObject3DGL::getVAO()
{
	return m_vao;
}

void CObject3DGL::fillVAO(vector<GLint>& countIndexs)
{
	if (m_vbo == -1)
	{
		cerr << "ERROR CObject3DGL::getVAO() should gen bind vbo first!" << endl;
		return;
	}
	if (m_vao == -1)
	{
		glGenVertexArrays(1, &m_vao);
	}

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	if (m_ebo != -1) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	GLint sum = 0, pos = 0;
	for (auto obj : countIndexs) sum += obj;
	for (size_t i = 0; i < countIndexs.size(); i++)
	{
		glVertexAttribPointer(i, countIndexs[i], GL_FLOAT, GL_FALSE, sum * sizeof(GLfloat), (void*)(pos * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		pos += countIndexs[i];
	}

	glBindVertexArray(0);
	if (m_ebo != -1) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint CObject3DGL::getVBO()
{
	return m_vbo;
}

void CObject3DGL::fillVBO(GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	if (m_vbo == -1)
	{
		glGenBuffers(1, &m_vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
	m_vboCount = size / sizeof(GL_FLOAT);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint CObject3DGL::getEBO()
{
	return m_ebo;
}

void CObject3DGL::fillEBO(GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	if (m_ebo == -1)
	{
		glGenBuffers(1, &m_ebo);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
	m_eboCount = size / sizeof(GLuint);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

map<size_t, shared_ptr<CShaderGL>>& CObject3DGL::getShaders()
{
	return m_shaders;
}

void CObject3DGL::setShader(shared_ptr<CShaderGL> shader, int index)
{
	m_shaders[index] = shader;
}

bool CObject3DGL::removeShader(int index)
{
	if (m_shaders.find(index) == m_shaders.end())
	{
		cerr << "ERROR CObject3DGL::removeShader " << index << " not exist!" << endl;
		return false;
	}
	m_shaders.erase(index);
	return true;
}

map<string, shared_ptr<CTextureGL>>& CObject3DGL::getTextures()
{
	return m_textures;
}

bool CObject3DGL::addTexture(string textureName, shared_ptr<CTextureGL> texture)
{
	if (m_textures.find(textureName) != m_textures.end())
	{
		cerr << "ERROR CObject3DGL::addTexture " << textureName << " already exist!" << endl;
		return false;
	}
	m_textures[textureName] = texture;
	return true;
}

bool CObject3DGL::removeTexture(string textureName)
{
	if (m_textures.find(textureName) == m_textures.end())
	{
		cerr << "ERROR CObject3DGL::removeTexture " << textureName << " not exist!" << endl;
		return false;
	}
	m_textures.erase(textureName);
	return true;
}

bool CObject3DGL::beforeDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader)
{
	glCheckError();
	return true;
}

bool CObject3DGL::afterDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader, bool drawed)
{
	glCheckError();
	return true;
}

void CObject3DGL::draw(int shaderIndex, shared_ptr<CShaderGL> shader)
{
	bool drawed = false;
	shared_ptr<CShaderGL> currentShader = shader;
	glBindVertexArray(m_vao);
	if (currentShader == nullptr)
	{
		if (m_shaders.find(shaderIndex) != m_shaders.end())
			currentShader = m_shaders[shaderIndex];
	}

	// update the model martrix every time, because the shader can be shared
	currentShader->setUniformMat4fv(MODEL_MATRIX_NAME, glm::value_ptr(m_model));
	currentShader->use();
	if (beforeDrawObject(shaderIndex, shader))
	{
		if (m_ebo != -1) glDrawElements(m_drawMode, m_eboCount, GL_UNSIGNED_INT, (void*)0);
		else glDrawArrays(m_drawMode, 0, m_vboCount);
		drawed = true;
	}
	afterDrawObject(shaderIndex, shader, drawed);
	glBindVertexArray(0);
	glUseProgram(0);
}
/*CObject3DGL end*/

/*CPlaneGL start*/
CPlaneGL::CPlaneGL( const glm::mat4& model,  
	const shared_ptr<CShaderGL> shader,
	GLenum usage, map<int, glm::vec2>& texcoords):CObject3DGL(model, shader)
{
	GLfloat vbo_buf[]= { // vec3 pos, vec2 texcoord, vec3 normal, vec3 tangent
		 0.5f,  0.5f, 0, 0,   0,   0, 0, 1.f, 1.f, 0, 0,
		-0.5f,  0.5f, 0, 1.f, 0,   0, 0, 1.f, 1.f, 0, 0,
	    -0.5f, -0.5f, 0, 1.f, 1.f, 0, 0, 1.f, 1.f, 0, 0,
		 0.5f, -0.5f, 0, 0,   1.f, 0, 0, 1.f, 1.f, 0, 0,
	};
	for (std::pair<int, glm::vec2> it: texcoords)
	{
		if (it.first > 4) continue;
		memcpy(&vbo_buf[INDEX_COUNT * it.first + TEXCOORD_INDEX],
			glm::value_ptr(it.second), 3 * sizeof(GLfloat));
	}

	GLuint ebo_buf[] = { 0, 1, 2, 2, 3, 0 };
	auto p0 = glm::make_vec3(&vbo_buf[0]);
	auto t0 = glm::make_vec2(&vbo_buf[TEXCOORD_INDEX]);
	auto p1 = glm::make_vec3(&vbo_buf[INDEX_COUNT]);
	auto t1 = glm::make_vec2(&vbo_buf[INDEX_COUNT + TEXCOORD_INDEX]);
	auto p2 = glm::make_vec3(&vbo_buf[INDEX_COUNT *2]);
	auto t2 = glm::make_vec2(&vbo_buf[INDEX_COUNT *2+ TEXCOORD_INDEX]);
	auto tangent = ::CalcTangent(p1 - p0, p2 - p1, t1 - t0, t2 - t1);
	auto normal = ::CalcNormal(p1 - p0, p2 - p1);
	for (int i = 0; i < 4; i++)
	{
		memcpy((GLfloat*)vbo_buf + INDEX_COUNT * i + NORMAL_INDEX,
			glm::value_ptr(normal), 3 * sizeof(GLfloat));
		memcpy((GLfloat*)vbo_buf + INDEX_COUNT * i + TANGENT_INDEX,
			glm::value_ptr(tangent), 3 * sizeof(GLfloat));
	}
	fillVBO(sizeof(vbo_buf), vbo_buf, usage);
	fillEBO(sizeof(ebo_buf), ebo_buf, usage);
	fillVAO();
	glCheckError();
}
/*CPlaneGL end*/

/*CCubeGL start*/
CCubeGL::CCubeGL(const glm::mat4& model,
	const shared_ptr<CShaderGL> shader,
	GLenum usage, map<int, glm::vec2>& texcoords):CObject3DGL(model, shader)
{
	GLfloat vbo_buf[INDEX_COUNT * 36];
	GLint face_ebo_buf[] = { 0,1,2,2,3,0 };

	glm::vec3 points[4];
	for (int i = 0; i < 6; i++) // 6 faces
	{
		switch (i)
		{ // different sequence determin the different direction
		case 0: // right, GL_TEXTURE_CUBE_MAP_POSITIVE_X
			points[0] = { 0.5f, 0.5f, -0.5f };
			points[1] = { 0.5f, 0.5f, 0.5f };
			points[2] = { 0.5f, -0.5f, 0.5f };
			points[3] = { 0.5f, -0.5f, -0.5f };
			break;
		case 1: // left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X
			points[3] = { -0.5f, 0.5f, -0.5f };
			points[2] = { -0.5f, 0.5f, 0.5f };
			points[1] = { -0.5f, -0.5f, 0.5f };
			points[0] = { -0.5f, -0.5f, -0.5f };
			break;
		case 2: // top, GL_TEXTURE_CUBE_MAP_POSITIVE_Y
			points[0] = { 0.5f, 0.5f, -0.5f };
			points[1] = { -0.5f, 0.5f, -0.5f };
			points[2] = { -0.5f, 0.5f, 0.5f };
			points[3] = { 0.5f,  0.5f, 0.5f };
			break;
		case 3: // bottom, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
			points[3] = { 0.5f, -0.5f, -0.5f };
			points[2] = { -0.5f, -0.5f, -0.5f };
			points[1] = { -0.5f, -0.5f, 0.5f };
			points[0] = { 0.5f, -0.5f, 0.5f };
			break;

		case 4: // back, GL_TEXTURE_CUBE_MAP_POSITIVE_Z
			points[0] = { 0.5f, 0.5f, 0.5f };
			points[1] = { -0.5f, 0.5f, 0.5f };
			points[2] = { -0.5f, -0.5f, 0.5f };
			points[3] = { 0.5f, -0.5f, 0.5f };
			break;
		case 5: // front, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
			points[3] = { 0.5f, 0.5f, -0.5f };
			points[2] = { -0.5f, 0.5f, -0.5f };
			points[1] = { -0.5f, -0.5f, -0.5f };
			points[0] = { 0.5f, -0.5f, -0.5f };
			break;
		}

		glm::vec2 _texcoords[4] = {
			{ 0.f,0.f },
			{ 1.f, 0.f },
			{ 1.f, 1.f },
			{0.f, 1.f}
		};
		for (int j = 0; j < 4; j++)
		{
			if (texcoords.find(i * 4 + j) == texcoords.end()) continue;
			_texcoords[j] = texcoords[i * 4 + j];
		}

		auto edge1 = points[1] - points[0];
		auto edge2 = points[2] - points[1];
		auto edgeST1 = _texcoords[1] - _texcoords[0];
		auto edgeST2 = _texcoords[2] - _texcoords[1];
		auto normal = ::CalcNormal(edge1, edge2);
		auto tangent = ::CalcTangent(edge1, edge2, edgeST1, edgeST2);

		for (int j = 0; j < 6; j++)
		{
			memcpy(&vbo_buf[(i * 6 + j) * INDEX_COUNT],
				glm::value_ptr(points[face_ebo_buf[j]]), 3 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * INDEX_COUNT + TEXCOORD_INDEX],
				glm::value_ptr(_texcoords[face_ebo_buf[j]]), 2 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * INDEX_COUNT + NORMAL_INDEX],
				glm::value_ptr(normal), 3 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * INDEX_COUNT + TANGENT_INDEX],
				glm::value_ptr(tangent), 3 * sizeof(GLfloat));
		}
	}

	fillVBO(sizeof(vbo_buf), vbo_buf);
	fillVAO();
	glCheckError();
}
/*CCubeGL end*/

/*CSphereGL start*/
/*CSphereGL end*/
#endif