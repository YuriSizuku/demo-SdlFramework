#ifdef USE_OPENGL
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm.hpp>
#include "data_types.hpp"
#ifndef _GL_OBJECT3D_H
#define _GL_OBJECT3D_H
using std::vector;
using std::map;
using std::string;
class CShaderGL
{

};

class CModelLoader
{

};

typedef struct Light
{

}Light;

typedef struct VertexInfo
{
	GLfloat x, y, z;
	GLfloat tex_x, tex_y;
	GLfloat norm_x, norm_y, norm_z;
	GLfloat tan_x, tan_y, tan_z;
}VertexInfo;

class COject3DGL
{
protected:
	GLuint m_vao, m_vbo, m_ebo;
	GLuint m_texure, m_normalMap, m_reflectMap;
	vector<GLubyte> m_vboBuffer;
	vector<GLubyte> m_eboBuffer;
public:
	int m_type = 0, m_id = 0, m_status=0;
	glm::mat4 m_model;
	void* m_pPhysicsRelated;
public:
	COject3DGL();
	virtual ~COject3DGL();
	virtual void draw() = 0;
};

// A scene contains multi objects,  as well as textures, lights to render a frame
class CSceneGL:public CScene<CMapList<COject3DGL*>>
{
protected:
	vector<Light> m_lights;
	vector<map<string, GLuint>> m_texutres;
public:
	glm::mat4 m_view = glm::mat4(1);
	glm::mat4 m_project = glm::mat4(1);
public:
	CSceneGL();
	virtual ~CSceneGL();
	virtual void render();
};

class CPlaneGL:public COject3DGL
{

};

class CCubeGL:public COject3DGL
{

};

class CSphereGL:public COject3DGL
{

};
#endif
#endif