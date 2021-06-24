#ifdef USE_OPENGL
#ifndef _GL_OBJECT3D_H
#define _GL_OBJECT3D_H
#include <memory>
#include <vector>
#include <map>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gl_assets.hpp"
#include "gl_scene.hpp"
using std::shared_ptr;
using std::vector;
using std::map;
using std::string;

class CObject3DGL;
class CMeshGL;
class CSceneGL;
typedef void (*PFNCOBJECT3DGLCB)(int shaderIndex, CObject3DGL* object, CSceneGL* scene, void* data1, void* data2);
typedef void (*PFNCMESHGLCB)(int shaderIndex, CMeshGL* mesh, CSceneGL* scene, void* data1, void* data2);
#define MODEL_MATRIX_NAME "model"

// contains vao, vbo, ebo, textures and shaders for each layer
// also have a pysical component and extra info,such as id, type, status
class CMeshGL
{
protected:
	GLuint m_vao = -1, m_vbo = -1, m_ebo = -1; // bind vao first, then vbo
	GLsizei m_vboCount = 0, m_eboCount = 0;
	GLenum m_drawMode = GL_TRIANGLES;
	// a object can using multi shader for different layers
	// use map is for some of layers which not using seperate shaders
	map<size_t, shared_ptr<CShaderGL>> m_shaders;
	// textures combined with the object will be activate and bind
	map<string, shared_ptr<CTextureGL>> m_textures;
	shared_ptr<void> m_material = nullptr;
	glm::mat4 m_model = glm::mat4(1);
	
public:
	int m_type = 0, m_id = 0, m_status = 0;
	shared_ptr<void> m_pPhysicsRelated = nullptr;
	static const int INDEX_COUNT = 11;
	static const int TEXCOORD_INDEX = 3;
	static const int NORMAL_INDEX = 5;
	static const int TANGENT_INDEX = 8;

protected:
	// this is the implement for user to do somthing before or after draw mesh
	virtual bool beforeDrawMesh(int shaderIndex, CShaderGL* shader);
	virtual bool afterDrawMesh(int shaderIndex, CShaderGL* shader, bool drawed);
public:
	CMeshGL();
	CMeshGL(const glm::mat4& model, const shared_ptr<CShaderGL> shader = nullptr);
	glm::mat4& getModel();
	void setModel(const glm::mat4& model);
	void setDrawMode(GLenum drawMode);

	// VAO VBO EBO
	// No need to reserve the CPU memory, just use glBindBuffer and glMapBuffer
	GLuint getVAO();
	void fillVAO();
	void fillVAO(vector<GLint>& countIndexs);
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
	shared_ptr<void>& getMaterial();
	void setMaterial(shared_ptr<void> material);

	virtual ~CMeshGL();
	// draw the obejct using the m_shaders, or extern shader
	// use pfnMeshSetCallback to set the unifroms by the layer
	virtual void draw(glm::mat4& objectModel,
		int shaderIndex = 0, CShaderGL* shader = nullptr, bool useTextures = true, 
		PFNCMESHGLCB pfnMeshSetCallback=NULL, CSceneGL* scene = NULL, 
		void* data1=NULL, void* data2=NULL);
};

// a 3D object for gl rendering, manage multi meshes
class CObject3DGL
{
protected:
	vector<shared_ptr<CMeshGL>> m_meshes;
	glm::mat4 m_model = glm::mat4(1);
public:
	int m_type = 0, m_id = 0, m_status = 0;
	shared_ptr<void> m_pPhysicsRelated = nullptr;

protected:
	// this is the implement for user to do somthing before or after draw object
    virtual bool beforeDrawObject(int shaderIndex, CShaderGL* shader);
	virtual bool afterDrawObject(int shaderIndex, CShaderGL* shader, bool drawed);
public:
	CObject3DGL();
	CObject3DGL(const glm::mat4& model, shared_ptr<CMeshGL> mesh=nullptr);
	
	vector<shared_ptr<CMeshGL>>& getMeshs();
	void pushMesh(shared_ptr<CMeshGL> mesh);
	void setShader(shared_ptr<CShaderGL> shader, int index = 0);
	bool removeShader(int index = 0);
	glm::mat4& getModel();
	void setModel(const glm::mat4& model);
	// pfnObjectSetCallback, pfnMeshSetCallback is for layer to set values
	virtual void draw(int shaderIndex = 0, CShaderGL* shader = nullptr, bool useTextures = true,
		PFNCOBJECT3DGLCB pfnObjectSetCallback = NULL, 
		PFNCMESHGLCB pfnMeshSetCallback = NULL, CSceneGL* scene=NULL,
		void* data1 = NULL, void* data2 = NULL);
	virtual ~CObject3DGL();
};

// a rectangle unit plane in XOY center
class CPlaneMeshGL:public CMeshGL
{
public:
	// 4 textcoords
	CPlaneMeshGL(const glm::mat4& model=glm::mat4(1), 
		const shared_ptr<CShaderGL> shader = nullptr, 
		GLenum usage = GL_STATIC_DRAW, 
		map<int, glm::vec2> texcoords=map<int, glm::vec2>());
};

// a unit cube in the center
class CCubeMeshGL:public CMeshGL
{
public:
	// 24 textcoords, right, left, top, bottom, back, front
	CCubeMeshGL(const glm::mat4& model = glm::mat4(1),
		const shared_ptr<CShaderGL> shader = nullptr,
		GLenum usage = GL_STATIC_DRAW, 
		map<int, glm::vec2> texcoords = map<int, glm::vec2>());
};

class CSphereMeshGL:public CMeshGL
{
public:
	CSphereMeshGL(const glm::mat4& model = glm::mat4(1),
		const shared_ptr<CShaderGL> shader = nullptr,
		GLenum usage = GL_STATIC_DRAW,
		float stepLatitude = glm::radians(10.f) , 
		float stepLongitude = glm::radians(10.f), 
		glm::vec2 startTexcoord = { 0.f, 0.f }, 
		glm::vec2 endTexcoord = {1.f, 1.f});
};
#endif
#endif