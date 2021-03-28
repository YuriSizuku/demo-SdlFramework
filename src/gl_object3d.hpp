#ifdef USE_OPENGL
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <glm.hpp>
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
	glm::mat4 m_model;
public:
	COject3DGL();
	virtual ~COject3DGL();
};

// A scene contains multi objects,  as well as textures, lights to render a frame
class CSceneGL
{
	glm::mat4 m_view, m_project;
protected:
	vector<Light> m_lights;
	vector<COject3DGL*> m_pObjects3DGL;
	vector<map<string, GLuint>> m_texutres;
public:
	CSceneGL();
	virtual ~CSceneGL();
	virtual void draw();
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