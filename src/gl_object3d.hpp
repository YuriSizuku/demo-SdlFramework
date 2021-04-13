#ifdef USE_OPENGL
#ifndef _GL_OBJECT3D_H
#define _GL_OBJECT3D_H
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gl_assets.hpp"
#include "gl_scene.hpp"
using std::shared_ptr;
using std::vector;
using std::map;
using std::string;

// contains vao, vbo, ebo, textures and shaders for each layer
// also have a pysical component and extra info,such as id, type, status
class CObject3DGL
{
protected:
	GLuint m_vao = -1, m_vbo = -1, m_ebo = -1; // bind vao first, then vbo
	GLsizei m_vboCount = 0, m_eboCount = 0;
	GLenum m_drawMode = GL_TRIANGLES;
	// a object can using multi shader for different layers
	// use map is for some of layers which not using seperate shaders
	map<size_t, shared_ptr<CShaderGL>> m_shaders;
	map<string, shared_ptr<CTextureGL>> m_textures;
	glm::mat4 m_model = glm::mat4(1);
public:
	int m_type = 0, m_id = 0, m_status = 0;
	shared_ptr<void*> m_pPhysicsRelated = nullptr;
	char* MODEL_MATRIX_NAME = "model";
	static const int INDEX_COUNT = 11;
	static const int TEXCOORD_INDEX = 3;
	static const int NORMAL_INDEX = 5;
	static const int TANGENT_INDEX = 8;

protected:
	// set unifroms and values here
	virtual bool beforeDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader);
	virtual bool afterDrawObject(int shaderIndex, shared_ptr<CShaderGL> shader, bool drawed);
public:
	CObject3DGL();
	CObject3DGL(const glm::mat4& model, const shared_ptr<CShaderGL> shader = nullptr);
	glm::mat4& getModel();
	void setModel(const glm::mat4& model);
	void setDrawMode(GLenum drawMode);

	// VAO VBO EBO
	GLuint getVAO();
	void fillVAO(vector<GLint>& countIndexs = vector<GLint>({ 3,2,3,3 }));
	GLuint getVBO();
	void fillVBO(GLsizeiptr size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW);
	GLuint getEBO();
	void fillEBO(GLsizeiptr size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW);

	// shader, texture
	map<size_t, shared_ptr<CShaderGL>>& getShaders();
	void setShader(shared_ptr<CShaderGL> shader, int index = 0);
	bool removeShader(int index = 0);
	map<string, shared_ptr<CTextureGL>>& getTextures();
	bool addTexture(string name, shared_ptr<CTextureGL> texture);
	bool removeTexture(string name);

	virtual ~CObject3DGL();
	// draw the obejct using the m_shaders, or extern shader
	virtual void draw(int shaderIndex = 0, shared_ptr<CShaderGL> shader = nullptr);
};

// a rectangle unit plane in XOY center
class CPlaneGL:public CObject3DGL
{
public:
	// 4 textcoords
	CPlaneGL(const glm::mat4& model=glm::mat4(1), 
		const shared_ptr<CShaderGL> shader = nullptr, 
		GLenum usage = GL_STATIC_DRAW, 
		map<int, glm::vec2>& texcoords = map<int, glm::vec2>());
};

// a unit cube in the center
class CCubeGL:public CObject3DGL
{
public:
	// 24 textcoords, right, left, top, bottom, back, front
	CCubeGL(const glm::mat4& model = glm::mat4(1),
		const shared_ptr<CShaderGL> shader = nullptr,
		GLenum usage = GL_STATIC_DRAW, 
		map<int, glm::vec2>& texcoords = map<int, glm::vec2>());
};

class CSphereGL:public CObject3DGL
{

};
#endif
#endif