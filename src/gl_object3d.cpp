#ifdef USE_OPENGL
#include <fstream>
#include<sstream>
#include "gl_object3d.hpp"

#define LOGBUF_SIZE 256
using std::ifstream;
using std::stringstream;
using std::ios;

/*CShaderGL start*/
CShaderGL::CShaderGL()
{
	m_program = glCreateProgram();
	if (!m_program)
	{
		cout << "ERROR CShaderGL::CShaderGL() create program failed" << endl;
		return;
	}
}

CShaderGL::CShaderGL(string vertPath, string fragPath, string geometryPath)
{
	m_program = glCreateProgram();
	if (!m_program)
	{
		cout << "ERROR CShaderGL::CShaderGL() create program failed" << endl;
		return;
	}
	if (vertPath != "") addShaderFile(vertPath, GL_VERTEX_SHADER);
	if (fragPath != "") addShaderFile(vertPath, GL_FRAGMENT_SHADER);
	if (geometryPath != "") addShaderFile(geometryPath, GL_GEOMETRY_SHADER);
	linkProgram();
}

void CShaderGL::addShaderFile(string path, GLenum shaderType)
{
	ifstream fin(path);
	stringstream source;
	if (fin.fail())
	{
		cout << "ERROR CShaderGL::addShaderFile open "<<path<<" failed" << endl;
		return;
	}
	char ch;
	while (source && fin.get(ch)) source.put(ch);
	addShaderSource(source.str(), shaderType);
}

void CShaderGL::addShaderSource(string& source, GLenum shaderType)
{
	auto shader = glCreateShader(shaderType);
	m_shaders.push_back(shader);
	const char* csource = source.c_str();
	GLint status;
	glShaderSource(shader, 1, &csource, NULL);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetShaderInfoLog(shader, LOGBUF_SIZE, NULL, logbuf);
		cout << logbuf << endl;
	}
	glAttachShader(m_program, shader);
}

void CShaderGL::linkProgram()
{
	glLinkProgram(m_program);
	GLint status;
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetProgramInfoLog(m_program, LOGBUF_SIZE, NULL, logbuf);
		cout << logbuf << endl;
	}
}

void CShaderGL::use()
{
	glUseProgram(m_program);
}

CShaderGL::~CShaderGL()
{
	for (auto shader : m_shaders)
	{
		glDeleteShader(shader);
	}
	glDeleteProgram(m_program);
}
/*CShaderGL end*/

/*CSceneGL start*/
CSceneGL::CSceneGL()
{

}

CSceneGL::~CSceneGL()
{

}

void CSceneGL::render()
{
	for (auto it = m_pObjects.get().begin(); it != m_pObjects.get().end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			(*it2)->draw();
		}
	}
}

/*CSceneGL end*/
#endif