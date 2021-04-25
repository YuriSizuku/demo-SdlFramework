/*
* A small opengl framework by devseed
* with the concept of scene, multi layer rendering, basic objects, light, camera 
* v0.1
*/
#ifdef USE_OPENGL
#ifndef _GL_SCENE_H
#define _GL_SCENE_H
#include <memory>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include "data_types.hpp"
#include "gl_layers.hpp"
#include "gl_assets.hpp"
#include "gl_object3d.hpp"

using std::shared_ptr;
using std::vector;
using std::string;
class CObject3DGL;
class CLayerGL;

#define VIEW_MATRIX_NAME  "view"
#define PROJECTION_MATRIX_NAME "projection"
GLenum _glCheckError(const char* file, int line);
#ifdef _DEBUG
#define glCheckError() _glCheckError(__FILE__, __LINE__)
#else
#define glCheckError()
#endif

// The light information for rendering lighting
typedef struct Light
{
	glm::vec4 position; // w=0 is direction light, else point or spot light
	glm::vec3 direction; // for direction light or spot light 
	glm::vec3 ambient; 
	glm::vec3 diffuse;
	glm::vec3 specular; 
	glm::vec3 attenuation; // point/spot light attenuation coefficient
	float cutoff, outerCutoff; // cutoff > 0 spot light, else point light
}Light;

// material structure for object with phong rendering
// http://devernay.free.fr/cours/opengl/materials.html
typedef struct MaterialPhong
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
	float alpha;
}MaterialPhong;

// defines the structure of vertex attribution
typedef struct Vertex
{
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::vec3 tangent;
}Vertex;

// a camera struct combined with view and porject matrix
typedef struct Camera
{
	glm::vec3 position = { 0.f,0.f,3.f };
	glm::vec3 angle = { glm::radians(0.f),glm::radians(-90.f),glm::radians(0.f) }; // roll, yaw, pitch
	float fov = glm::radians(45.f), aspect = 16.f / 9.f, zNear = 0.1f, zFar = 100.f;
}Camera;

glm::mat4 CalcView(const Camera& camera);
glm::mat4 CalcProjection(const Camera& camera);
glm::vec3 CalcTangent(const glm::vec3& edge1, const glm::vec3& edge2,
	const glm::vec2& edgeST1, const glm::vec2& edgeST2);
glm::vec3 CalcNormal(const glm::vec3& edge1, glm::vec3& edge2);

// a scene contains multi layers for defered render
class CSceneGL :public CScene<CMapList<shared_ptr<CObject3DGL>>>
{
protected:
	string m_lastShaderDir;
	vector<shared_ptr<CLayerGL>> m_layers;
	vector<Light> m_lights;
	map<string, shared_ptr<CShaderGL>> m_shaders;
	map<string, shared_ptr<CTextureGL>> m_textures; // GLuint m_texture, m_normalMap, m_reflectMap, m_diffuseMap;
	map<string, shared_ptr<CTextureGL>> m_Gbuffer;
	glm::mat4 m_view = glm::mat4(1);
	glm::mat4 m_project = glm::mat4(1);
	Camera m_camera; // camera -> view, project

public:
	CSceneGL();
	CSceneGL(string shaderName, string shaderDir);

	// view, project matrix, shaderName!="" will also update the uniform matrix data
	void setMatrix(const glm::mat4& matrix, string matrixName, string shaderName);
	void setView(const glm::mat4& view);
	void setView(const glm::mat4& view, string shaderName, bool updateMatrix = true);
	void setView(string shaderName = "");
	glm::mat4& getView();
	void setProject(const glm::mat4& project);
	void setProject(const glm::mat4& project, string shaderName, bool updateMatrix = true);
	void setProject(string shaderName = "");
	glm::mat4& getProjection();
	void setCamera(const Camera& camera, bool updateMatrix = true);
	void setCamera(const Camera& camera, string shaderName,
		bool updateMatrix = true, bool updateCamera = true);
	void setCamera(string shaderName = "", bool updateMatrix = true);
	Camera& getCamera();

	// scene asset 
	vector<shared_ptr<CLayerGL>>& getLayers();
	void pushLayer(shared_ptr<CLayerGL> layer);
	vector<Light>& getLights();
	void pushLight(Light light);
	map<string, shared_ptr<CShaderGL>>& getShaders();
	void addShader(string shaderName, string shaderDir); // default.vert, default.frag, default.geom
	void addShader(string shaderName);
	map<string, shared_ptr<CTextureGL>>& getTextures();
	bool addTexture(string textureName, shared_ptr<CTextureGL> texture);
	bool removeTexture(string textureName);

	virtual void render();
	virtual ~CSceneGL();
};
#endif
#endif