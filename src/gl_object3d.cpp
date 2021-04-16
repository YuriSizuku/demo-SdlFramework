#ifdef USE_OPENGL
#include <iostream>
#include "gl_object3d.hpp"
using std::cerr;
using std::endl;

/*CMeshGL start*/
CMeshGL::CMeshGL()
{

}

CMeshGL::~CMeshGL()
{
	if (m_vao != -1) glDeleteVertexArrays(1, &m_vao);
	if (m_vbo != -1) glDeleteBuffers(1, &m_vbo);
	if (m_ebo != -1) glDeleteBuffers(1, &m_ebo);
}

CMeshGL::CMeshGL(const glm::mat4& model, const shared_ptr<CShaderGL> shader)
{
	m_model = model;
	m_shaders[0] = shader;
	glCheckError();
}

glm::mat4& CMeshGL::getModel()
{
	return m_model;
}

void CMeshGL::setModel(const glm::mat4& model)
{
	m_model = model;
}

void CMeshGL::setDrawMode(GLenum drawMode)
{
	m_drawMode = drawMode;
}

GLuint CMeshGL::getVAO()
{
	return m_vao;
}

void CMeshGL::fillVAO(vector<GLint>& countIndexs)
{
	if (m_vbo == -1)
	{
		cerr << "ERROR CMeshGL::getVAO() should gen bind vbo first!" << endl;
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

GLuint CMeshGL::getVBO()
{
	return m_vbo;
}

void CMeshGL::fillVBO(GLsizeiptr size, const GLvoid* data, GLenum usage)
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

GLuint CMeshGL::getEBO()
{
	return m_ebo;
}

void CMeshGL::fillEBO(GLsizeiptr size, const GLvoid* data, GLenum usage)
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

map<size_t, shared_ptr<CShaderGL>>& CMeshGL::getShaders()
{
	return m_shaders;
}

void CMeshGL::setShader(shared_ptr<CShaderGL> shader, int index)
{
	m_shaders[index] = shader;
}

bool CMeshGL::removeShader(int index)
{
	if (m_shaders.find(index) == m_shaders.end())
	{
		cerr << "ERROR CMeshGL::removeShader " << index << " not exist!" << endl;
		return false;
	}
	m_shaders.erase(index);
	return true;
}

map<string, shared_ptr<CTextureGL>>& CMeshGL::getTextures()
{
	return m_textures;
}

bool CMeshGL::addTexture(string textureName, shared_ptr<CTextureGL> texture)
{
	if (m_textures.find(textureName) != m_textures.end())
	{
		cerr << "ERROR CMeshGL::addTexture " << textureName << " already exist!" << endl;
		return false;
	}
	m_textures[textureName] = texture;
	return true;
}

bool CMeshGL::removeTexture(string textureName)
{
	if (m_textures.find(textureName) == m_textures.end())
	{
		cerr << "ERROR CMeshGL::removeTexture " << textureName << " not exist!" << endl;
		return false;
	}
	m_textures.erase(textureName);
	return true;
}

shared_ptr<void>& CMeshGL::getMaterial()
{
	return m_material;
}

void CMeshGL::setMaterial(shared_ptr<void> material)
{
	m_material = material;
}

bool CMeshGL::beforeDrawMesh(int shaderIndex, CShaderGL* shader)
{
	glCheckError();
	return true;
}

bool CMeshGL::afterDrawMesh(int shaderIndex, CShaderGL* shader, bool drawed)
{
	glCheckError();
	return true;
}

void CMeshGL::draw(glm::mat4& objectModel, int shaderIndex, CShaderGL* shader, 
	PFNCMESHGLCB pfnMeshSetCallback, CSceneGL* scene)
{
	bool drawed = false;
	auto currentShader = shader;
	glBindVertexArray(m_vao);
	if (currentShader == nullptr)
	{
		if (m_shaders.find(shaderIndex) != m_shaders.end())
			currentShader = m_shaders[shaderIndex].get();
	}

	// update the model martrix every time, because the shader can be shared
	currentShader->setUniformMat4fv(string(MODEL_MATRIX_NAME), glm::value_ptr(objectModel*m_model));
	currentShader->use();
	for (auto it : m_textures) // active and bind each textures for the object
	{
		it.second->active();
		it.second->bind();
	}
	if (pfnMeshSetCallback) pfnMeshSetCallback(shaderIndex, this, scene);
	if (beforeDrawMesh(shaderIndex, shader))
	{
		if (m_ebo != -1) glDrawElements(m_drawMode, m_eboCount, GL_UNSIGNED_INT, (void*)0);
		else glDrawArrays(m_drawMode, 0, m_vboCount);
		drawed = true;
	}
	for (auto it : m_textures) // unbind all textures
	{
		it.second->unbind();
	}
	afterDrawMesh(shaderIndex, shader, drawed);
	glBindVertexArray(0);
	glUseProgram(0);
}
/*CMeshGL end*/

/*CObject3DGL start*/
CObject3DGL::CObject3DGL()
{

}

CObject3DGL::CObject3DGL(const glm::mat4& model, shared_ptr<CMeshGL> mesh)
{
	m_model = model;
	if (mesh) pushMesh(mesh);
}

CObject3DGL::~CObject3DGL()
{

}

vector<shared_ptr<CMeshGL>>& CObject3DGL::getMeshs()
{
	return m_meshes;
}

void CObject3DGL::pushMesh(shared_ptr<CMeshGL> mesh)
{
	m_meshes.push_back(mesh);
}

void CObject3DGL::setShader(shared_ptr<CShaderGL> shader, int index)
{
	for (auto it : m_meshes)
	{
		it->setShader(shader, index);
	}
}

bool CObject3DGL::removeShader(int index)
{
	for (auto it : m_meshes)
	{
		if(!it->removeShader(index)) return false;
	}
	return true;
}

glm::mat4& CObject3DGL::getModel()
{
	return m_model;
}

void CObject3DGL::setModel(const glm::mat4& model)
{
	m_model = model;
}

bool CObject3DGL::beforeDrawObject(int shaderIndex, CShaderGL* shader)
{
	return true;
}

bool CObject3DGL::afterDrawObject(int shaderIndex, CShaderGL* shader, bool drawed)
{
	return true;
}

void CObject3DGL::draw(int shaderIndex, CShaderGL* shader, 
	PFNCOBJECT3DGLCB pfnObjectSetCallback, 
	PFNCMESHGLCB pfnMeshSetCallback, CSceneGL* scene)
{
	bool drawed = false;
	if (pfnObjectSetCallback) pfnObjectSetCallback(shaderIndex, this, scene);
	if (beforeDrawObject(shaderIndex, shader))
	{
		for (auto it : m_meshes)
		{
			it->draw(m_model, shaderIndex, shader, pfnMeshSetCallback);
		}
		drawed = true;
	}
	afterDrawObject(shaderIndex, shader, drawed);
}
/*CObject3DGL end*/

/*CPlaneMeshGL start*/
CPlaneMeshGL::CPlaneMeshGL( const glm::mat4& model,  
	const shared_ptr<CShaderGL> shader,
	GLenum usage, map<int, glm::vec2>& texcoords):CMeshGL(model, shader)
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
/*CPlaneMeshGL end*/

/*CCubeMeshGL start*/
CCubeMeshGL::CCubeMeshGL(const glm::mat4& model,
	const shared_ptr<CShaderGL> shader,
	GLenum usage, map<int, glm::vec2>& texcoords):CMeshGL(model, shader)
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
/*CCubeMeshGL end*/

/*CSphereMeshGL start*/
/*CSphereMeshGL end*/
#endif