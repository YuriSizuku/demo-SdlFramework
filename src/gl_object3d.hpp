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
#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

GLenum _glCheckError(const char* file, int line);
#ifdef _DEBUG
#define glCheckError() _glCheckError(__FILE__, __LINE__)
#else
#define glCheckError()
#endif

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
	GLenum active(GLenum texture); // < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS -1
	GLenum active(); // return the previous activeIndex
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

class CTextureCubeMapGL : public CTextureGL
{
public:
	CTextureCubeMapGL(GLsizei width, GLsizei height, 
		GLint internalFormat = GL_RGBA);
	void texImage2DI(GLenum i, GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid* data);
	virtual ~CTextureCubeMapGL();
};

typedef struct Light
{

}Light;

typedef struct VertexInfo
{
	glm::vec3 pos;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::vec3 tangent;
}VertexInfo;

// a object3dgl may contains vao, vbo, ebo, textures and a rendering shader
// also have a pysical component and extra info,such as id, type, status
class CObject3DGL
{
protected:
	GLuint m_vao=-1, m_vbo=-1, m_ebo=-1; // bind vao first, then vbo
	GLsizei m_vboCount = 0, m_eboCount = 0;
	GLenum m_drawMode = GL_TRIANGLES;
	shared_ptr<CShaderGL> m_shader = nullptr;
	map<string, shared_ptr<CTextureGL>> m_textures;
	glm::mat4 m_model = glm::mat4(1);
public:
	int m_type = 0, m_id = 0, m_status=0;
	shared_ptr<void*> m_pPhysicsRelated=nullptr;
public:
	static glm::vec3 calcTangent(const glm::vec3 &edge1, const glm::vec3 &edge2,
		const glm::vec2& edgeST1, const glm::vec2& edgeST2);
	static glm::vec3 calcNormal(const glm::vec3& edge1, glm::vec3& edge2);
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
	shared_ptr<CShaderGL> getShader();
	void setShader(shared_ptr<CShaderGL> shader);
	map<string, shared_ptr<CTextureGL>> getTextures();
	bool addTexture(string name, shared_ptr<CTextureGL> texture);
	bool removeTexture(string name);

	virtual ~CObject3DGL();
	virtual void draw();
};

// A scene contains multi objects,  as well as textures, lights to render a frame
class CSceneGL:public CScene<CMapList<shared_ptr<CObject3DGL>>>
{
protected:
	vector<Light> m_lights;
	map<string, shared_ptr<CShaderGL>> m_shaders;
	shared_ptr<CShaderGL> m_currentShader;
	map<string, shared_ptr<CTextureGL>> m_textures; // GLuint m_texture, m_normalMap, m_reflectMap, m_diffuseMap;
	map<string, shared_ptr<CTextureGL>> m_Gbuffer;
	glm::mat4 m_view = glm::mat4(1);
	glm::mat4 m_project = glm::mat4(1);
public:
	CSceneGL();
	CSceneGL(string shaderName, string shaderDir="./shader");
	
	// set get matrix, shaderName!="" will also update the uniform matrix data
	void setView(const glm::mat4& view, string shaderName=""); 
	void setView(string shaderName);
	glm::mat4& getView();
	void setProject(const glm::mat4& project, string shaderName="");
	void setProject(string shaderName);
	glm::mat4& getProject();
	
	// scene asset 
	map<string, shared_ptr<CTextureGL>>& getTextures();
	vector<Light>& getLights();
	map<string, shared_ptr<CShaderGL>>& getShaders();
	void addShader(string programName, string programDir="./shader"); // default.vert, default.frag, default.geom
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