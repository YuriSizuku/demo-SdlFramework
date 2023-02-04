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

void CMeshGL::fillVAO()
{
	auto countIndexes = vector<GLint>({ 3,2,3,3 });
	fillVAO(countIndexes);
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
	GLint sum = 0, position = 0;
	for (auto obj : countIndexs) sum += obj;
	for (size_t i = 0; i < countIndexs.size(); i++)
	{
		glVertexAttribPointer(i, countIndexs[i], GL_FLOAT, GL_FALSE, sum * sizeof(GLfloat), (void*)(position * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		position += countIndexs[i];
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

void CMeshGL::draw(glm::mat4& objectModel, int shaderIndex, CShaderGL* shader, bool useTextures, 
	PFNCMESHGLCB pfnMeshSetCallback, CSceneGL* scene, void* data1, void* data2)
{
	if (m_vao == -1)
	{
		cerr << "ERROR CMeshGL::draw, vao is not initialed" << endl;
		return;
	}

	bool drawed = false;
	auto currentShader = shader;
	glBindVertexArray(m_vao);
	string str;
	if (currentShader == nullptr)
	{
		if (m_shaders.find(shaderIndex) != m_shaders.end())
		{
			currentShader = m_shaders[shaderIndex].get();
		}
		if (currentShader == nullptr)
		{
			cerr << "ERROR CMeshGL::draw, no shader for rendering" << endl;
		}
	}
	// update the model martrix every time, because the shader can be shared
	str = string(MODEL_MATRIX_NAME);
	currentShader->setUniformMat4fv(str, glm::value_ptr(objectModel*m_model));
	currentShader->use();
	if (useTextures)
	{
		for (std::pair<string, shared_ptr<CTextureGL>> it : m_textures) // active and bind each textures for the object
		{
			it.second->active();
			it.second->bind();
		}
	}

	if (pfnMeshSetCallback) pfnMeshSetCallback(shaderIndex, this, scene, data1, data2);
	if (beforeDrawMesh(shaderIndex, shader))
	{
		if (m_ebo != -1) glDrawElements(m_drawMode, m_eboCount, GL_UNSIGNED_INT, (void*)0);
		else glDrawArrays(m_drawMode, 0, m_vboCount);
		drawed = true;
	}
	if (useTextures)
	{
		for (std::pair<string, shared_ptr<CTextureGL>> it : m_textures) // unbind all textures
		{
			it.second->unbind();
		}
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

void CObject3DGL::draw(int shaderIndex, CShaderGL* shader, bool useTextures, 
	PFNCOBJECT3DGLCB pfnObjectSetCallback, 
	PFNCMESHGLCB pfnMeshSetCallback, CSceneGL* scene, void* data1, void* data2)
{
	bool drawed = false;
	if (pfnObjectSetCallback) pfnObjectSetCallback(shaderIndex, this, scene, data1, data2);
	if (beforeDrawObject(shaderIndex, shader))
	{
		for (shared_ptr<CMeshGL>& it : m_meshes)
		{
			it->draw(m_model, shaderIndex, shader, useTextures, pfnMeshSetCallback, scene, data1, data2);
		}
		drawed = true;
	}
	afterDrawObject(shaderIndex, shader, drawed);
}
/*CObject3DGL end*/

/*CPlaneMeshGL start*/
CPlaneMeshGL::CPlaneMeshGL(const glm::mat4& model,  
	const shared_ptr<CShaderGL> shader,
	GLenum usage, map<int, glm::vec2> texcoords):CMeshGL(model, shader)
{
	GLfloat vbo_buf[]= { // vec3 position, vec2 texcoord, vec3 normal, vec3 tangent
		 0.5f,  0.5f, 0, 1.f, 0,   0, 0, 1.f, 1.f, 0, 0,
		-0.5f,  0.5f, 0, 0,   0,   0, 0, 1.f, 1.f, 0, 0,
	    -0.5f, -0.5f, 0, 0,   1.f, 0, 0, 1.f, 1.f, 0, 0,
		 0.5f, -0.5f, 0, 1.f, 1.f, 0, 0, 1.f, 1.f, 0, 0,
	};
	for (std::pair<int, glm::vec2> it: texcoords)
	{
		if (it.first > 4) continue;
		memcpy(&vbo_buf[INDEX_COUNT * it.first + TEXCOORD_INDEX],
			glm::value_ptr(it.second), 2 * sizeof(GLfloat));
	}

	GLuint ebo_buf[] = { 0, 1, 2, 2, 3, 0 };
	auto p0 = glm::make_vec3(&vbo_buf[0]);
	auto t0 = glm::make_vec2(&vbo_buf[TEXCOORD_INDEX]);
	auto p1 = glm::make_vec3(&vbo_buf[INDEX_COUNT]);
	auto t1 = glm::make_vec2(&vbo_buf[INDEX_COUNT + TEXCOORD_INDEX]);
	auto p2 = glm::make_vec3(&vbo_buf[INDEX_COUNT *2]);
	auto t2 = glm::make_vec2(&vbo_buf[INDEX_COUNT *2+ TEXCOORD_INDEX]);
	auto delta_p0 = p1 - p0;
	auto delta_p1 = p2 - p1;
	auto delta_t0 = t1 - t0;
	auto delta_t1 = t2 - t1;
	auto tangent = ::CalcTangent(delta_p0, delta_p1, delta_t0, 
		delta_t1);
	auto normal = ::CalcNormal(delta_p0, delta_p1);
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
	GLenum usage, map<int, glm::vec2> texcoords):CMeshGL(model, shader)
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
			{ 1.f, 0.f },
			{ 0.f, 0.f },
			{ 0.f, 1.f },
			{ 1.f, 1.f}
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
CSphereMeshGL::CSphereMeshGL(const glm::mat4& model,
	const shared_ptr<CShaderGL> shader, GLenum usage,
	float stepLatitude, float stepLongitude,
	glm::vec2 startTexcoord , glm::vec2 endTexcoord)
	:CMeshGL(model, shader)
{
	int vn = static_cast<int>(glm::radians(180.f) / stepLatitude);
	int un = static_cast<int>(glm::radians(360.f) / stepLongitude);
	int vertexs_count = 2 + (vn+1-2)*un;
	int ebo_count = 2 * 3 * un + (vn+1-2)*6*un;
	Vertex* vertexs = new Vertex[vertexs_count];
	GLint* ebo_buf = new GLint[ebo_count];

	// calculate vbo
	vertexs[0].position = { 0.f, 0.5f, 0.f }; // north polar
	vertexs[0].texcoord = {(startTexcoord.s + endTexcoord.s)/2.f, startTexcoord.t};
	vertexs[0].normal = { 0.f, 1.f, 0.f }; 
	vertexs[0].tangent = { 1.f, 0.f, 0.f };
	vertexs[vertexs_count - 1].position = { 0.f, -0.5f, 0.f }; // south polar
	vertexs[vertexs_count - 1].texcoord = { (startTexcoord.s + endTexcoord.s) / 2.f, endTexcoord.t};
	vertexs[vertexs_count - 1].normal = { 0.f, -1.f, 0.f };
	vertexs[vertexs_count - 1].tangent = { -1.f, 0.f, 0.f };
	for (int i = 1; i < vn; i++) // latitude without polar
	{
		for (int j = 0; j < un; j++) // longitude cycle
		{
			int idx = 1 + (i - 1) * un + j;
			float theta = j * stepLongitude;
			float phi = glm::radians(90.f) - i * stepLatitude;
			vertexs[idx].normal = { cos(phi) * cos(theta), sin(phi), cos(phi) * sin(theta)};
			vertexs[idx].tangent = { -cos(phi) * sin(theta), 0.f, cos(phi) * cos(theta) };
			vertexs[idx].position = vertexs[idx].normal / 2.f;
			vertexs[idx].texcoord = {
				startTexcoord.s + (endTexcoord.s - startTexcoord.s) * (j * stepLongitude / glm::radians(360.f)),
				startTexcoord.t + (endTexcoord.t - startTexcoord.t) * (i * stepLatitude / glm::radians(180.f)) };
		}
	}

	// calculate ebo
	int point_offset = 1;
	int ebo_offset = 0;
	for (int j = 0; j < un; j++) // layer 0(north polar)->1, triangle
	{
		ebo_buf[ebo_offset] = 0;
		ebo_buf[ebo_offset + 1] = point_offset + 1;
		ebo_buf[ebo_offset + 2] = point_offset ;
		if (j == un - 1) // for loop
		{
			ebo_buf[ebo_offset + 1] -= un;
		}
		point_offset++;
		ebo_offset += 3;
	}
	for (int i = 2; i < vn; i++) // layer 1 -> n-1, rectangle
	{
		for (int j = 0;j < un; j++)
		{
			ebo_buf[ebo_offset] = point_offset - un;
			ebo_buf[ebo_offset + 1] = point_offset - un + 1;
			ebo_buf[ebo_offset + 2] = point_offset  + 1;
			ebo_buf[ebo_offset + 3] = point_offset  + 1;
			ebo_buf[ebo_offset + 4] = point_offset  ;
			ebo_buf[ebo_offset + 5] = point_offset - un;
			if (j == un - 1) // for loop
			{
				ebo_buf[ebo_offset + 1] -= un;
				ebo_buf[ebo_offset + 2] -= un;
				ebo_buf[ebo_offset + 3] -= un;
			}
			point_offset++;
			ebo_offset += 6;
		}
	}
	point_offset -= un;
	for (int j = 0; j < un; j++) // layer n-1 -> n(south polar), triangle
	{
		ebo_buf[ebo_offset] = point_offset;
		ebo_buf[ebo_offset + 1] = point_offset + 1;
		ebo_buf[ebo_offset + 2] = vertexs_count - 1;
		if (j == un - 1) // for loop
		{
			ebo_buf[ebo_offset + 1] -= un;
		}
		point_offset++;
		ebo_offset += 3;
	}
	
	fillVBO(vertexs_count * sizeof(Vertex), vertexs);
	fillEBO(ebo_count * sizeof(GLint), ebo_buf);
	fillVAO();
	delete[] vertexs;
	delete[] ebo_buf;
	glCheckError();
}
/*CSphereMeshGL end*/
#endif