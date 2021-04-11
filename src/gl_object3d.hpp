#ifdef USE_OPENGL
#ifndef _GL_OBJECT3D_H
#define _GL_OBJECT3D_H
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include "data_types.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::shared_ptr;

GLenum _glCheckError(const char* file, int line);
#ifdef _DEBUG
#define glCheckError() _glCheckError(__FILE__, __LINE__)
#else
#define glCheckError()
#endif

typedef struct Light
{

}Light;

typedef struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::vec3 tangent;
}Vertex;

// a camera struct combined with view and porject matrix
typedef struct Camera
{
	glm::vec3 pos = {0.f,0.f,3.f};
	glm::vec3 angle = {glm::radians(0.f),glm::radians(-90.f),glm::radians(0.f) }; // roll, yaw, pitch
	float fov = glm::radians(45.f), aspect=16.f/9.f, zNear=0.1f, zFar=100.f;
}Camera;

glm::mat4 CalcView(const Camera& camera);
glm::mat4 CalcProject(const Camera& camera);
glm::vec3 CalcTangent(const glm::vec3& edge1, const glm::vec3& edge2,
	const glm::vec2& edgeST1, const glm::vec2& edgeST2);
glm::vec3 CalcNormal(const glm::vec3& edge1, glm::vec3& edge2);

class CSceneGL;

class CShaderGL
{
protected:
	GLuint m_programID = -1;
	vector<GLuint> m_shadersID;
public:
	CShaderGL();
	CShaderGL(string vertPath, string fragPath, string geometryPath="");
	void addShaderFile(string path, GLenum shaderType);
	void addShaderSource(string& source, GLenum shaderType);
	void linkProgram();
	GLuint getProgram();
	
	// get, set uniform, return location
	GLint getUniformLocation(string uniformName);
	GLint getUniformBlockIndex(string uniformName);
	GLint setUniform4fv(string uniformName, const GLfloat* data);
	GLint setUniform4fv(string uniformName, GLsizei i, const GLfloat* data);
	GLint setUniform4fv(string uniformName, GLsizei i, GLsizei count, const GLfloat* data);
	GLint setUniformMat4fv(string uniformName, const GLfloat* data);
	GLint setUniformBlock(string uniformName,
		GLintptr offset, GLsizei size, const void* data);
	void use();
	virtual ~CShaderGL();
};

class CModelLoader
{

};

class CTextureGL
{
protected:
	GLuint m_texture = -1;
	GLenum m_target = GL_TEXTURE_2D;
	GLenum m_aciveIndex = GL_TEXTURE0;
public:
	CTextureGL(GLenum target);
	GLuint getTexture();
	GLenum getTarget();
	GLenum getActiveIndex();
	void getTexImage(GLint level, GLenum format, GLenum type, void* pixels);
	GLenum active(GLenum aciveIndex); // < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS -1
	GLenum active(); // return the previous activeIndex
	void bind();
	virtual ~CTextureGL();
};

class CTexture2DGL : public CTextureGL
{
public:
	CTexture2DGL();
	void texImage2D(GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border, 
		GLenum format, GLenum type, const GLvoid* data);
	void texSubImage2D( GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid* data);
	virtual ~CTexture2DGL();
};

class CTexture3DGL :public CTextureGL
{
public:
	CTexture3DGL(GLenum target);
	void texImage3D(GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLsizei depth, 
		GLint border, GLenum format, GLenum type, const GLvoid* data);
	void texSubImage3D(GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth,
		GLenum format, GLenum type, const GLvoid* data);
	virtual ~CTexture3DGL();
};

class CTextureCubeGL : public CTextureGL
{
public:
	CTextureCubeGL(GLsizei width, GLsizei height, 
		GLint internalFormat = GL_RGBA);
	void texImage2DI(GLenum i, GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid* data);
	virtual ~CTextureCubeGL();
};

// a object3dgl may contains vao, vbo, ebo, textures and shaders for each layer
// also have a pysical component and extra info,such as id, type, status
class CObject3DGL
{
protected:
	GLuint m_vao=-1, m_vbo=-1, m_ebo=-1; // bind vao first, then vbo
	GLsizei m_vboCount = 0, m_eboCount = 0;
	GLenum m_drawMode = GL_TRIANGLES;
	// a object can using multi shader for different layer
	// use map is about someof the layer may not need to use the object
	map<size_t, shared_ptr<CShaderGL>> m_shaders; 
	map<string, shared_ptr<CTextureGL>> m_textures;
	glm::mat4 m_model = glm::mat4(1);
public:
	int m_type = 0, m_id = 0, m_status=0;
	shared_ptr<void*> m_pPhysicsRelated=nullptr;
protected:
	// set unifroms and values here
	virtual bool beforeDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader);
	virtual bool afterDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader, bool drawed);
public:
	CObject3DGL();
	CObject3DGL(const glm::mat4& model, const shared_ptr<CShaderGL> shader=nullptr); 
	glm::mat4& getModel();
	void setModel(const glm::mat4& model);
	void setDrawMode(GLenum drawMode);

	// VAO VBO EBO
	GLuint getVAO();
	void fillVAO(vector<GLint>& countIndex =vector<GLint>({3,2,3,3}));
	GLuint getVBO();
	void fillVBO(GLsizeiptr size, const GLvoid* data, GLenum usage=GL_STATIC_DRAW);
	GLuint getEBO();
	void fillEBO(GLsizeiptr size, const GLvoid* data, GLenum usage=GL_STATIC_DRAW);
	
	// shader, texture
	map<size_t, shared_ptr<CShaderGL>>&  getShaders();
	void setShader(shared_ptr<CShaderGL> shader, int index=0);
	bool removeShader(int index = 0);
	map<string, shared_ptr<CTextureGL>>& getTextures();
	bool addTexture(string name, shared_ptr<CTextureGL> texture);
	bool removeTexture(string name);

	virtual ~CObject3DGL();
	virtual void draw(int shaderIndex=0, shared_ptr<CShaderGL> shader=nullptr);
};

// use multi layer for defered rendering, inFrameBuffer -> outFrameBuffer
class CLayerGL
{
protected:
	CSceneGL& m_scene;
	size_t m_layerIndex;
	shared_ptr<CTextureGL> m_inFrameBuffer, m_outFrameBuffer; // in Frame is the from last
protected:
	void _draw(shared_ptr<CShaderGL> shader);
	virtual bool beforeDrawLayer(); // set unifroms and values here
	virtual bool afterDrawLayer(bool drawed);
public:
	CLayerGL(CSceneGL& scene, shared_ptr<CTextureGL> outFrameBuffer = nullptr);
	void setInFramebuffer(shared_ptr<CTextureGL> inFrameBuffer);
	shared_ptr<CTextureGL> getOutFrameBuffer();
	virtual ~CLayerGL();
	virtual void draw();
};

// generate the shadow map by every light, point light, direction light
// combine all light shadow positions to shadowMapTexture (in clip space)
class CShadowMapLayerGL : public CLayerGL
{
private:
	shared_ptr<CTextureGL> m_shadowMapTexture;
public:
	void genLightShadowMap(Light* light);
	CShadowMapLayerGL(CSceneGL& scene, shared_ptr<CTextureGL> shadowMapTexture); 
	virtual ~CShadowMapLayerGL();
	virtual void draw();
};

// Dynamic Environment Mapping: viewing to each direction (without shadow)
// generate the enviroment map(cube map) by randering every object in 6 directions
class CEnviromentLayerGL : public CLayerGL
{
private:
	shared_ptr<CTextureCubeGL> m_enviromentMapTexture;
public:
	CEnviromentLayerGL(CSceneGL& scene, shared_ptr<CTextureCubeGL> m_enviromentMapTexture);
	virtual ~CEnviromentLayerGL();
	virtual void draw();
};

// blend all textures(such as shallow, reflect, sky box) for defered rendering  
class CBlendLayerGL :public CLayerGL
{
private:
	vector<shared_ptr<CTextureGL>> m_textures;
public:
	CBlendLayerGL(CSceneGL& scene, vector<shared_ptr<CTextureGL>>& textures);
	virtual ~CBlendLayerGL();
	virtual void draw();
};

// use the debug shader for output
class CDebugLayerGL : public CLayerGL
{
private:
	shared_ptr<CShaderGL> m_normalShader;
public:
	CDebugLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> normalShader);
	virtual ~CDebugLayerGL();
	virtual void draw();
};

// generate a cube in the light position
class CDebugLightLayerGL : public CDebugLayerGL
{
public:
	CDebugLightLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> lightShader);
	virtual ~CDebugLightLayerGL();
	virtual void draw();
};

// A scene contains multi layers for defered render
class CSceneGL:public CScene<CMapList<shared_ptr<CObject3DGL>>>
{
protected:
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
	CSceneGL(string shaderName, string shaderDir="./shader");
	
	// view, project matrix, shaderName!="" will also update the uniform matrix data
	void setMatrix(const glm::mat4& matrix, string matrixName, string shaderName);
	void setView(const glm::mat4& view); 
	void setView(const glm::mat4& view, string shaderName, bool updateMatrix = true);
	void setView(string shaderName="");
	glm::mat4& getView();
	void setProject(const glm::mat4& project);
	void setProject(const glm::mat4& project, string shaderName, bool updateMatrix = true);
	void setProject(string shaderName="");
	glm::mat4& getProject();
	void setCamera(const Camera& camera, bool updateMatrix = true);
	void setCamera(const Camera& camera, string shaderName,
		bool updateMatrix = true, bool updateCamera=true);
	void setCamera(string shaderName = "", bool updateMatrix = true);
	Camera& getCamera();
	
	// scene asset 
	vector<shared_ptr<CLayerGL>>& getLayers();
	void pushLayer(shared_ptr<CLayerGL> layer);
	vector<Light>& getLights();
	void pushLight(Light light);
	map<string, shared_ptr<CShaderGL>>& getShaders();
	void addShader(string shaderName, string shaderDir="./shader"); // default.vert, default.frag, default.geom
	map<string, shared_ptr<CTextureGL>>& getTextures();
	bool addTexture(string textureName, shared_ptr<CTextureGL> texture);
	bool removeTexture(string textureName);

#if(0)
	// defered render
	virtual void createGbuffer();
	virtual void renderEnviromentMap();
	virtual void renderShadowMap();
	virtual void renderBlendShadowMap();
	virtual void renderBlendEnviromentMap();
#endif
	virtual void render();
	virtual ~CSceneGL();
};

class CPlaneGL:public CObject3DGL
{
public:
	CPlaneGL(const glm::mat4& model=glm::mat4(1), 
		const shared_ptr<CShaderGL> shader = nullptr, 
		GLenum usage=GL_STATIC_DRAW);
};

class CCubeGL:public CObject3DGL
{
public:
	CCubeGL(const glm::mat4& model = glm::mat4(1),
		const shared_ptr<CShaderGL> shader = nullptr,
		GLenum usage = GL_STATIC_DRAW);
};

class CSphereGL:public CObject3DGL
{

};
#endif
#endif