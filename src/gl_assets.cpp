#ifdef USE_OPENGL
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include "gl_assets.hpp"

#define LOGBUF_SIZE 5024
using std::ifstream;
using std::stringstream;
using std::to_string;
using std::ios;
using std::cerr;
using std::endl;

/*CShaderGL start*/
CShaderGL::CShaderGL()
{
	m_programID = glCreateProgram();
	if (!m_programID)
	{
		cerr << "ERROR CShaderGL::CShaderGL() create program failed" << endl;
		return;
	}
	glCheckError();
}

CShaderGL::CShaderGL(string vertPath, string fragPath, string geometryPath) :CShaderGL()
{
	if (vertPath != "") addShaderFile(vertPath, GL_VERTEX_SHADER);
	if (geometryPath != "") addShaderFile(geometryPath, GL_GEOMETRY_SHADER);
	if (fragPath != "") addShaderFile(fragPath, GL_FRAGMENT_SHADER);
	linkProgram();
}

void CShaderGL::addShaderFile(string path, GLenum shaderType)
{
	ifstream fin(path);
	stringstream source;
	if (fin.fail())
	{
		cerr << "ERROR CShaderGL::addShaderFile open " << path << " failed" << endl;
		return;
	}
	char ch;
	while (source && fin.get(ch)) source.put(ch);
	addShaderSource(source.str(), shaderType);
	fin.close();
}

void CShaderGL::addShaderSource(string& source, GLenum shaderType)
{
	auto shader = glCreateShader(shaderType);
	m_shadersID.push_back(shader);
	const char* csource = source.c_str();
	GLint status;
	glShaderSource(shader, 1, &csource, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetShaderInfoLog(shader, LOGBUF_SIZE, NULL, logbuf);
		cerr << logbuf << endl;
	}
	glAttachShader(m_programID, shader);
}

void CShaderGL::linkProgram()
{
	glLinkProgram(m_programID);
	GLint status;
	glGetProgramiv(m_programID, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char logbuf[LOGBUF_SIZE];
		glGetProgramInfoLog(m_programID, LOGBUF_SIZE, NULL, logbuf);
		cerr << logbuf << endl;
	}
}

GLuint CShaderGL::getProgram()
{
	return m_programID;
}

GLint CShaderGL::getUniformLocation(string uniformName)
{
	auto location = glGetUniformLocation(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformLocation uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::getUniformBlockIndex(string uniformName)
{
	auto location = glGetUniformBlockIndex(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformBlockIndex uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::setUniform4fv(string uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUniform4fv(location, 1, data);
	return location;
}

GLint CShaderGL::setUniform4fv(string uniformName, GLsizei i, const GLfloat* data)
{
	return setUniform4fv(uniformName, i, 1, data);
}

GLint CShaderGL::setUniform4fv(string uniformName, GLsizei i, GLsizei count, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName + "[" + to_string(i) + "]");
	glUseProgram(m_programID);
	glUniform4fv(location, count, data);
	return location;
}

GLint CShaderGL::setUniformMat4fv(string uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniformMatrix4fv(location, 1, false, data);
	return location;
}

GLint CShaderGL::setUniformBlock(string uniformName,
	GLintptr offset, GLsizei size, const void* data)
{
	glUseProgram(m_programID);
	GLuint block = -1;
	glGenBuffers(1, &block);
	glBindBuffer(GL_UNIFORM_BUFFER, block);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	auto block_index = getUniformBlockIndex(uniformName);
	glBindBufferBase(GL_UNIFORM_BUFFER, block_index, block);
	void* pBuf = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT);
	memcpy(pBuf, data, size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &block);
	return block_index;
}

void CShaderGL::use()
{
	glUseProgram(m_programID);
}

CShaderGL::~CShaderGL()
{
	for (auto shader : m_shadersID)
	{
		glDeleteShader(shader);
	}
	glDeleteProgram(m_programID);
}
/*CShaderGL end*/
#endif