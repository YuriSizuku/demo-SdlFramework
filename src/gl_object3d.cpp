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
	// roll, yaw, pitch
	glm::vec3 direction( // the camera look at direction
		cos(camera.angle.p) * cos(camera.angle.y),
		sin(camera.angle.p),
		cos(camera.angle.p) * sin(camera.angle.y));
	glm::vec3 up(sin(camera.angle.r), cos(camera.angle.r), 0.f); // angle start is 90
	return glm::lookAt(camera.pos, camera.pos + direction, up);
}

glm::mat4 CalcProject(const Camera& camera)
{
	return glm::perspective(camera.fov, camera.aspect, camera.zNear, camera.zFar);
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

/*CLayerGL start*/
CLayerGL::CLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader,
	shared_ptr<CTextureGL> outFrameBuffer):
m_scene(scene), m_layerShader(layerShader), m_outFrameBuffer(outFrameBuffer)
{
	m_layerIndex = scene.getLayers().size();
}

CLayerGL::~CLayerGL()
{

}

void CLayerGL::setInFramebuffer(shared_ptr<CTextureGL> inFrameBuffer)
{
	m_inFrameBuffer = inFrameBuffer;
}

shared_ptr<CTextureGL> CLayerGL::getOutFrameBuffer()
{
	return m_outFrameBuffer;
}

void CLayerGL::drawSceneObjects(shared_ptr<CShaderGL> shader)
{
	auto m_objects = m_scene.getObjects();
	bool drawed = false;
	if (beforeDrawLayer())
	{
		for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				(*it2)->draw(m_layerIndex, shader);
			}
		}
		drawed = true;
	}
	afterDrawLayer(drawed);
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
	drawSceneObjects(m_layerShader);
}
/*CLayerGL end*/

/*CLayerHudGL start*/
CLayerHudGL::CLayerHudGL(CSceneGL& scene, 
	shared_ptr<CShaderGL> hudShader, GLint hudViewPort[4]) :CLayerGL(scene, hudShader)
{
	glGetIntegerv(GL_VIEWPORT, m_orgViewport);
	if (hudViewPort)
	{
		memcpy(m_hudViewPort, hudViewPort, sizeof(GLint) * 4);
	}
	else
	{
		GLint unit = m_orgViewport[3] / 80;
		m_hudViewPort[2] = 6*unit;
		m_hudViewPort[3] = 6*unit;
		m_hudViewPort[0] = m_orgViewport[2] - m_hudViewPort[2] - 2*unit;
		m_hudViewPort[1] = m_orgViewport[3] - m_hudViewPort[2] - 2*unit;
	}
}

CLayerHudGL::~CLayerHudGL()
{

}

void CLayerHudGL::drawHud()
{
	drawSceneObjects(m_layerShader);
}

void CLayerHudGL::draw()
{
	glViewport(m_hudViewPort[0], m_hudViewPort[1], m_hudViewPort[2], m_hudViewPort[3]);
	drawHud();
	glViewport(m_orgViewport[0], m_orgViewport[1], m_orgViewport[2], m_orgViewport[3]);
}
/*CLayerHudGL end*/

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

void CSceneGL::setMatrix(const glm::mat4& matrix, string matrixName, string shaderName)
{
	if (shaderName != "")
	{
		if (m_shaders.find(shaderName) == m_shaders.end())
		{
			cerr << "ERROR CSceneGL::setView program " << shaderName << " not exist" << endl;
			return;
		}
		m_shaders[shaderName]->setUniformMat4fv(matrixName, glm::value_ptr(matrix));
	}
	else
	{
		for (auto it : m_shaders)
		{
			it.second->setUniformMat4fv(matrixName, glm::value_ptr(matrix));
		}
	}
}

void CSceneGL::setView(const glm::mat4& view)
{
	m_view = view;
}

void CSceneGL::setView(const glm::mat4& view, string shaderName, bool updateMatrix)
{
	if(updateMatrix) setView(view);
	setMatrix(view, "view", shaderName);
}

void CSceneGL::setView(string shaderName)
{
	setView(m_view, shaderName, false);
}

glm::mat4& CSceneGL::getView()
{
	return m_view;
}

void CSceneGL::setProject(const glm::mat4& project)
{
	m_project = project;
}

void CSceneGL::setProject(const glm::mat4& project, string shaderName, bool updateMatrix)
{
	if(updateMatrix) setProject(project);
	setMatrix(project, "project", shaderName);
}

void CSceneGL::setProject(string shaderName)
{
	setProject(m_project, shaderName, false);
}

glm::mat4& CSceneGL::getProject()
{
	return m_project;
}

void CSceneGL::setCamera(const Camera& camera, bool updateMatrix)
{
	m_camera = camera;
	if (updateMatrix)
	{
		m_view = ::CalcView(camera);
		m_project = ::CalcProject(camera);
	}
}

void CSceneGL::setCamera(const Camera& camera, string shaderName, 
	bool updateMatrix, bool updateCamera)
{
	if (updateCamera) m_camera = camera;
	auto view = ::CalcView(camera);
	auto project = ::CalcProject(camera);
	setView(view, shaderName, updateMatrix);
	setProject(project, shaderName, updateMatrix);
}

void CSceneGL::setCamera(string shaderName, bool updateMatrix)
{
	setCamera(m_camera, shaderName, updateMatrix, false);
}

Camera& CSceneGL::getCamera()
{
	return m_camera;
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
	m_lastShaderDir = shaderDir;
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

void CSceneGL::addShader(string shaderName)
{
	addShader(shaderName, m_lastShaderDir);
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
	if (m_layers.size() <= 0)
	{
		cerr << "ERROR CSceneGL::render has no layers" << endl;
		return;
	}
	glCheckError();
	shared_ptr<CTextureGL> inFrameBuffer = nullptr;
	for (auto layer : m_layers)
	{
		if (layer == nullptr) continue;
		layer->setInFramebuffer(inFrameBuffer);
		layer->draw();
		inFrameBuffer = layer->getOutFrameBuffer();
	}
	
}
/*CSceneGL end*/

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
	currentShader->setUniformMat4fv("model", glm::value_ptr(m_model));
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

/*CLayerHudAttitude start*/
CLayerHudAttitude::CLayerHudAttitude(CSceneGL& scene,
	shared_ptr<CShaderGL> attitudeShader,
	GLint hudViewPort[4]) :CLayerHudGL(scene, attitudeShader, hudViewPort)
{
	if (!m_layerShader) 
	{
		cerr << "CLayerHudAttitude::drawHud layer shader is NULL" << endl;
		return;
	}
	GLfloat vbo_buf[] = { // positon, texcoord(color rrrggg  bbbaaa)
		0.f, 0.f, 0.f, 999000.f, 000999.f, 1.f, 0.f, 0.f, 999000.f, 000999.f,
		0.f, 0.f, 0.f, 000999.f, 000999.f, 0.f, 1.f, 0.f, 000999.f, 000999.f,
		0.f, 0.f, 0.f, 000000.f, 999999.f, 0.f, 0.f, 1.f, 000000.f, 999999.f
	};
	m_attitude.setDrawMode(GL_LINES);
	m_attitude.fillVBO(sizeof(vbo_buf), vbo_buf);
	m_attitude.fillVAO(vector<GLint>({ 3, 2 }));
}

void CLayerHudAttitude::drawHud()
{
	auto view = glm::mat4(glm::mat3(m_scene.getView()));
	view[3] = glm::vec4(0.f, 0.f, -3.f, 1.f); //fix the camera to(0,0,3)
	auto project = glm::perspective(glm::radians(30.f), 1.2f, 0.1f, 100.f);
	if (!m_layerShader)
	{
		cerr << "CLayerHudAttitude::drawHud layer shader is NULL" << endl;
		return;
	}
	m_layerShader->setUniformMat4fv("view", glm::value_ptr(view));
	m_layerShader->setUniformMat4fv("project", glm::value_ptr(project));
	glLineWidth(2.f);
	m_attitude.draw(0, m_layerShader);
	glLineWidth(1.f);
}

CLayerHudAttitude::~CLayerHudAttitude()
{

}
/*CLayerHudAttitude end*/

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