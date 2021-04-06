#ifdef USE_OPENGL
#ifndef _GL_OBJECT3D_H
#define _GL_OBJECT3D_H
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "data_types.hpp"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::shared_ptr;

class CShaderGL
{
protected:
	GLuint m_program;
	vector<GLuint> m_shaders;
public:
	CShaderGL();
	CShaderGL(string vertPath, string fragPath, string geometryPath="");
	void addShaderFile(string path, GLenum shaderType);
	void addShaderSource(string& source, GLenum shaderType);
	void linkProgram();
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

class CObject3DGL
{
protected:
	GLuint m_vao=-1, m_vbo=-1, m_ebo=-1; // bind vao first, then vbo
	GLuint m_program = -1;
	glm::mat4 m_model = glm::mat4(1);
public:
	int m_type = 0, m_id = 0, m_status=0;
	void* m_pPhysicsRelated;
public:
	CObject3DGL();
	CObject3DGL(GLuint program, glm::mat4& m_model);
	GLuint getVAO();
	void fillVAO(); // glVertexAttribPointer
	GLuint getVBO();
	void fillVBO(GLsizeiptr size, const GLvoid* data, GLenum usage);
	GLuint getEBO();
	void fillEBO(GLsizeiptr size, const GLvoid* data, GLenum usage);
	GLuint getProgram();
	void setProgram(GLuint m_program);
	glm::mat4& getModel();
	void setModel(glm::mat4 &model);
	virtual ~CObject3DGL();
	virtual void draw() = 0;
};

// A scene contains multi objects,  as well as textures, lights to render a frame
class CSceneGL:public CScene<CMapList<shared_ptr<CObject3DGL>>>
{
protected:
	vector<Light> m_lights;
	map<string, shared_ptr<CShaderGL>> m_pShaders;
	map<string, shared_ptr<CTextureGL>> m_pTextures; // GLuint m_texture, m_normalMap, m_reflectMap, m_diffuseMap;
	glm::mat4 m_view = glm::mat4(1);
	glm::mat4 m_project = glm::mat4(1);
public:
	CSceneGL();
	void setView(glm::mat4& view);
	glm::mat4& getView();
	void setProject(glm::mat4& project);
	glm::mat4& getProject();
	map<string, CTextureGL*>& getpTextures();
	vector<Light>& getLights();
	map<string, CShaderGL>& getShaders();
	void addShader(string programName="default", string programDir="shader"); // default.vert, default.frag, default.geom
	void addTexture(string textureName, shared_ptr<CTextureGL> texture);
	void removeTexture(string textureName);
	virtual ~CSceneGL();
	virtual void render();
};

class CPlaneGL:public CObject3DGL
{

};

class CCubeGL:public CObject3DGL
{

};

class CSphereGL:public CObject3DGL
{

};
#endif
#endif