#ifdef USE_OPENGL
#include <fstream>
#include<sstream>
#include<string.h>
#include "gl_object3d.hpp"

#define LOGBUF_SIZE 5024
using std::ifstream;
using std::stringstream;
using std::to_string;
using std::ios;
using std::cerr;

/*Util functions start*/
GLenum _glCheckError(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error = "OPENGL ERROR";
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

glm::mat4 CalcView(const Camera& camera)
{
	return glm::mat4(1);
}

glm::vec3 CalcTangent(const glm::vec3& edge1, const glm::vec3& edge2,
	const glm::vec2& edgeST1, const glm::vec2& edgeST2)
{
	auto detST = edgeST1.s * edgeST2.t - edgeST2.s * edgeST1.t;
	return glm::vec3(
		edgeST2.t * edge1.x - edgeST1.t * edge1.x,
		edgeST2.t * edge1.y - edgeST1.t * edge1.y,
		edgeST2.t * edge1.z - edgeST1.t * edge1.z) / detST;
}

glm::vec3 CalcNormal(const glm::vec3& edge1, glm::vec3& edge2)
{
	return glm::cross(edge1, edge2);
}
/*Util functions end*/

/*CShaderGL start*/
CShaderGL::CShaderGL()
{
	m_programID = glCreateProgram();
	if (!m_programID)
	{
		cerr << "ERROR CShaderGL::CShaderGL() create program failed" << endl;
		return;
	}
	glCheckError();
}

CShaderGL::CShaderGL(string vertPath, string fragPath, string geometryPath):CShaderGL()
{
	if (vertPath != "") addShaderFile(vertPath, GL_VERTEX_SHADER);
	if (geometryPath != "") addShaderFile(geometryPath, GL_GEOMETRY_SHADER);
	if (fragPath != "") addShaderFile(fragPath, GL_FRAGMENT_SHADER);
	linkProgram();
}

void CShaderGL::addShaderFile(string path, GLenum shaderType)
{
	ifstream fin(path);
	stringstream source;
	if (fin.fail())
	{
		cerr << "ERROR CShaderGL::addShaderFile open "<<path<<" failed" << endl;
		return;
	}
	char ch;
	while (source && fin.get(ch)) source.put(ch);
	addShaderSource(source.str(), shaderType);
	fin.close();
}

void CShaderGL::addShaderSource(string& source, GLenum shaderType)
{
	auto shader = glCreateShader(shaderType);
	m_shadersID.push_back(shader);
	const char* csource = source.c_str();
	GLint status;
	glShaderSource(shader, 1, &csource, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetShaderInfoLog(shader, LOGBUF_SIZE, NULL, logbuf);
		cerr<<logbuf<<endl;
	}
	glAttachShader(m_programID, shader);
}

void CShaderGL::linkProgram()
{
	glLinkProgram(m_programID);
	GLint status;
	glGetProgramiv(m_programID, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetProgramInfoLog(m_programID, LOGBUF_SIZE, NULL, logbuf);
		cerr<<logbuf<< endl;
	}
}

GLuint CShaderGL::getProgram()
{
	return m_programID;
}

GLint CShaderGL::getUniformLocation(string uniformName)
{
	auto location = glGetUniformLocation(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformLocation uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::getUniformBlockIndex(string uniformName)
{
	auto location = glGetUniformBlockIndex(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformBlockIndex uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::setUniform4fv(string uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUniform4fv(location, 1, data);
	return location;
}

GLint CShaderGL::setUniform4fv(string uniformName, GLsizei i, const GLfloat* data)
{
	return setUniform4fv(uniformName, i, 1, data);
}

GLint CShaderGL::setUniform4fv(string uniformName, GLsizei i, GLsizei count, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName + "[" + to_string(i) + "]");
	glUseProgram(m_programID);
	glUniform4fv(location, count, data);
	return location;
}

GLint CShaderGL::setUniformMat4fv(string uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniformMatrix4fv(location, 1, false, data);
	return location;
}

GLint CShaderGL::setUniformBlock(string uniformName,
	GLintptr offset, GLsizei size, const void* data)
{
	glUseProgram(m_programID);
	GLuint block = -1;
	glGenBuffers(1, &block);
	glBindBuffer(GL_UNIFORM_BUFFER, block);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	auto block_index = getUniformBlockIndex(uniformName);
	glBindBufferBase(GL_UNIFORM_BUFFER, block_index, block);
	void* pBuf = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT);
	memcpy(pBuf, data, size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &block);
	return block_index;
}

void CShaderGL::use()
{
	glUseProgram(m_programID);
}

CShaderGL::~CShaderGL()
{
	for (auto shader : m_shadersID)
	{
		glDeleteShader(shader);
	}
	glDeleteProgram(m_programID);
}
/*CShaderGL end*/

/*CObject3DGL start*/
CObject3DGL::CObject3DGL()
{

}

CObject3DGL::~CObject3DGL()
{
	if(m_vao!=-1) glDeleteVertexArrays(1, &m_vao);
	if(m_vbo!=-1) glDeleteBuffers(1, &m_vbo);
	if(m_ebo!=-1) glDeleteBuffers(1, &m_ebo);
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

void CObject3DGL::fillVAO(vector<GLint>& countIndex)
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
	GLint sum = 0, pos=0;
	for (auto obj : countIndex) sum += obj;
	for (size_t i = 0; i < countIndex.size(); i++)
	{
		glVertexAttribPointer(i, countIndex[i], GL_FLOAT, GL_FALSE, sum *sizeof(GLfloat), (void*)(pos*sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		pos += countIndex[i];
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

map<size_t, shared_ptr<CShaderGL>>&  CObject3DGL::getShaders()
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

bool CObject3DGL::beforeDrawObject(int shaderIndex)
{
	glCheckError();
	return true;
}

bool CObject3DGL::afterDrawObject(int shaderIndex, bool drawed)
{
	glCheckError();
	return true;
}


void CObject3DGL::draw(int shaderIndex)
{
	bool drawed = false;
	glBindVertexArray(m_vao);
	if (m_shaders.find(shaderIndex)!=m_shaders.end())
	{
		// update the model martrix every time, because the shader can be shared
		m_shaders[shaderIndex]->setUniformMat4fv("model", glm::value_ptr(m_model));
		m_shaders[shaderIndex]->use();
	}
	if (beforeDrawObject(shaderIndex))
	{
		if (m_ebo != -1) glDrawElements(m_drawMode, m_eboCount, GL_UNSIGNED_INT, (void*)0);
		else glDrawArrays(m_drawMode, 0, m_vboCount);
		drawed = true;
	}
	afterDrawObject(shaderIndex, drawed);
	glBindVertexArray(0);
	glUseProgram(0);
}

/*CObject3DGL end*/

/*CLayerGL start*/
CLayerGL::CLayerGL(CSceneGL& scene):m_scene(scene)
{
	m_layerIndex = scene.getLayers().size();
}

CLayerGL::~CLayerGL()
{

}

bool CLayerGL::beforeDrawLayer()
{
	return true;
}

bool CLayerGL::afterDrawLayer(bool drawed)
{
	return true;
}

void CLayerGL::draw()
{
	auto m_objects = m_scene.getObjects();
	bool drawed = false;
	if (beforeDrawLayer())
	{
		for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				(*it2)->draw(m_layerIndex);
			}
		}
		drawed = true;
	}
	afterDrawLayer(drawed);
}
/*CLayerGL end*/

/*CSceneGL start*/
CSceneGL::CSceneGL()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glCheckError();
}

CSceneGL::CSceneGL(string shaderName, string shaderDir):CSceneGL()
{
	addShader(shaderName, shaderDir);
}

CSceneGL::~CSceneGL()
{

}

void CSceneGL::setView(const glm::mat4& view, string shaderName)
{
	m_view = view;
	if (shaderName != "")
	{
		setView(shaderName);
	}
}

void CSceneGL::setView(string shaderName)
{
	if (m_shaders.find(shaderName) == m_shaders.end())
	{
		cout << "ERROR CSceneGL::setView program " << shaderName << " not exist" << endl;
		return;
	}
	m_shaders[shaderName]->setUniformMat4fv("view", glm::value_ptr(m_view));
}

glm::mat4& CSceneGL::getView()
{
	return m_view;
}

void CSceneGL::setProject(const glm::mat4& project, string shaderName)
{
	m_project = project;
	if (shaderName != "")
	{
		setProject(shaderName);
	}
}

void CSceneGL::setProject(string shaderName)
{
	if (m_shaders.find(shaderName) == m_shaders.end())
	{
		cerr << "ERROR CSceneGL::setProject program " << shaderName << " not exist" << endl;
		return;
	}
	m_shaders[shaderName]->setUniformMat4fv("project", glm::value_ptr(m_project));
}

glm::mat4& CSceneGL::getProject()
{
	return m_project;
}

map<string, shared_ptr<CTextureGL>>& CSceneGL::getTextures()
{
	return m_textures;
}

bool CSceneGL::addTexture(string textureName, shared_ptr<CTextureGL> texture)
{
	if (m_textures.find(textureName) != m_textures.end())
	{
		cerr << "ERROR CSceneGL::addTexture " << textureName << " already exist!" << endl;
		return false;
	}
	m_textures[textureName] = texture;
	return true;
}

vector<shared_ptr<CLayerGL>>& CSceneGL::getLayers()
{
	return m_layers;
}

void CSceneGL::pushLayer(shared_ptr<CLayerGL> layer)
{
	m_layers.push_back(layer);
}

vector<Light>& CSceneGL::getLights()
{
	return m_lights;
}

void CSceneGL::pushLight(Light light)
{
	m_lights.push_back(light);
}

map<string, shared_ptr<CShaderGL>>& CSceneGL::getShaders()
{
	return m_shaders;
}

void CSceneGL::addShader(string shaderName, string shaderDir)
{
	if (m_shaders.find(shaderName) != m_shaders.end())
	{
		cout << "ERROR CSceneGL::addShader" << shaderName << " already added!" << endl;
	}
	string path = shaderDir + "/" + shaderName;
	string vertPath = path + ".vert";
	string fragPath = path + ".frag";
	string geometryPath = path + ".geom";
	ifstream fin;

	// check shader file exist
	fin.open(vertPath, ios::in);
	if (fin.fail()) vertPath = "";
	else cout << "loading vertex shader: "<<vertPath<<endl;
	fin.close();
	
	fin.open(geometryPath);
	if (fin.fail()) geometryPath = "";
	else cout << "loading geometry shader: " << geometryPath << endl;
	fin.close();

	fin.open(fragPath);
	if (fin.fail()) fragPath = "";
	else cout << "loading fragment shader: "<<fragPath<<endl;
	fin.close();
	
	// add shaders 
	m_shaders[shaderName] = shared_ptr<CShaderGL>(new 
		CShaderGL(vertPath, fragPath, geometryPath));
}

bool CSceneGL::removeTexture(string textureName)
{
	if (m_textures.find(textureName) == m_textures.end())
	{
		cerr << "ERROR CSceneGL::removeTexture " << textureName << " not exist!" << endl;
		return false;
	}
	m_textures.erase(textureName);
	return true;
}

void CSceneGL::render()
{
	glCheckError();
	for (auto layer : m_layers)
	{
		layer->draw();
	}
}
/*CSceneGL end*/

/*CPlaneGL start*/
CPlaneGL::CPlaneGL( const glm::mat4& model,  
	const shared_ptr<CShaderGL> shader,
	GLenum usage):CObject3DGL(model, shader)
{
	const int EACH_COUNT = 11;
	GLfloat vbo_buf[]= { // vec3 pos, vec2 texcoord, vec3 normal, vec3 tangent
		 0.5f,  0.5f, 0, 0,   0,   0, 0, 1.f, 1.f, 0, 0,
		-0.5f,  0.5f, 0, 1.f, 0,   0, 0, 1.f, 1.f, 0, 0,
	    -0.5f, -0.5f, 0, 1.f, 1.f, 0, 0, 1.f, 1.f, 0, 0,
		 0.5f, -0.5f, 0, 0,   1.f, 0, 0, 1.f, 1.f, 0, 0,
	};
	GLuint ebo_buf[] = { 0, 1, 2, 2, 3, 0 };
	auto p0 = glm::make_vec3(&vbo_buf[0]);
	auto t0 = glm::make_vec2(&vbo_buf[3]);
	auto p1 = glm::make_vec3(&vbo_buf[EACH_COUNT]);
	auto t1 = glm::make_vec2(&vbo_buf[EACH_COUNT+3]);
	auto p2 = glm::make_vec3(&vbo_buf[EACH_COUNT*2]);
	auto t2 = glm::make_vec2(&vbo_buf[EACH_COUNT*2+3]);
	auto tangent = ::CalcTangent(p1 - p0, p2 - p1, t1 - t0, t2 - t1);
	auto normal = ::CalcNormal(p1 - p0, p2 - p1);
	for (int i = 0; i < 4; i++)
	{
		memcpy((GLfloat*)vbo_buf + EACH_COUNT * i + 5, 
			glm::value_ptr(normal), 3 * sizeof(GLfloat));
		memcpy((GLfloat*)vbo_buf + EACH_COUNT * i + 8,
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
	GLenum usage) :CObject3DGL(model, shader)
{
	const int EACH_COUNT = 11;
	GLfloat vbo_buf[EACH_COUNT * 36] = {
	     0.5f,  0.5f, 0.5f, 0,   0,   0, 0, 0, 0, 0, 0,
		-0.5f,  0.5f, 0.5f, 1.f, 0,   0, 0, 0, 0, 0, 0,
		-0.5f, -0.5f, 0.5f, 1.f, 1.f, 0, 0, 0, 0, 0, 0,
		-0.5f, -0.5f, 0.5f, 1.f, 1.f, 0, 0, 0, 0, 0, 0,
		 0.5f, -0.5f, 0.5f, 0,   1.f, 0, 0, 0, 0, 0, 0,
		 0.5f,  0.5f, 0.5f, 0,   0,   0, 0, 0, 0, 0, 0,
	};
	GLint face_ebo_buf[] = { 0,1,2,2,3,0 };

	glm::vec2 texcoords[4] = { 
		{ 0.f,0.f }, 
		{ 1.f, 0.f }, 
		{ 1.f, 1.f }, 
		{0.f, 1.f} 
	};
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
		auto edge1 = points[1] - points[0];
		auto edge2 = points[2] - points[1];
		auto edgeST1 = texcoords[1] - texcoords[0];
		auto edgeST2 = texcoords[2] - texcoords[1];
		auto normal = ::CalcNormal(edge1, edge2);
		auto tangent = ::CalcTangent(edge1, edge2, edgeST1, edgeST2);

		for (int j = 0; j < 6; j++)
		{
			memcpy(&vbo_buf[(i * 6 + j) * EACH_COUNT], 
				glm::value_ptr(points[face_ebo_buf[j]]), 3 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * EACH_COUNT + 3],
				glm::value_ptr(texcoords[face_ebo_buf[j]]), 2 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * EACH_COUNT + 5],
				glm::value_ptr(normal), 3 * sizeof(GLfloat));
			memcpy(&vbo_buf[(i * 6 + j) * EACH_COUNT + 8],
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