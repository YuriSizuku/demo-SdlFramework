#ifdef USE_OPENGL
#include "gl_scene.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
using std::ifstream;
using std::ios;
using std::cout;
using std::cerr;
using std::endl;

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
	return glm::lookAt(camera.position, camera.position + direction, up);
}

glm::mat4 CalcProjection(const Camera& camera)
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

/*CSceneGL start*/
CSceneGL::CSceneGL()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glCheckError();
}

CSceneGL::CSceneGL(string shaderName, string shaderDir) :CSceneGL()
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
	if (updateMatrix) setView(view);
	setMatrix(view, VIEW_MATRIX_NAME, shaderName);
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
	if (updateMatrix) setProject(project);
	setMatrix(project, PROJECTION_MATRIX_NAME, shaderName);
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
		m_project = ::CalcProjection(camera);
	}
}

void CSceneGL::setCamera(const Camera& camera, string shaderName,
	bool updateMatrix, bool updateCamera)
{
	if (updateCamera) m_camera = camera;
	auto view = ::CalcView(camera);
	auto project = ::CalcProjection(camera);
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
	else cout << "loading vertex shader: " << vertPath << endl;
	fin.close();

	fin.open(geometryPath);
	if (fin.fail()) geometryPath = "";
	else cout << "loading geometry shader: " << geometryPath << endl;
	fin.close();

	fin.open(fragPath);
	if (fin.fail()) fragPath = "";
	else cout << "loading fragment shader: " << fragPath << endl;
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
#endif