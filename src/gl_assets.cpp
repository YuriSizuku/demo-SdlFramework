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

CShaderGL::CShaderGL(string& vertPath, string& fragPath, string geometryPath) :CShaderGL()
{
	if (vertPath != "") addShaderFile(vertPath, GL_VERTEX_SHADER);
	if (geometryPath != "") addShaderFile(geometryPath, GL_GEOMETRY_SHADER);
	if (fragPath != "") addShaderFile(fragPath, GL_FRAGMENT_SHADER);
	linkProgram();
}

CShaderGL::~CShaderGL()
{
	for (auto shader : m_shadersID)
	{
		glDeleteShader(shader);
	}
	glDeleteProgram(m_programID);
}

void CShaderGL::addShaderFile(string& path, GLenum shaderType)
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
	string str = source.str();
	addShaderSource(str, shaderType);
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

GLint CShaderGL::getUniformLocation(string& uniformName)
{
	auto location = glGetUniformLocation(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformLocation uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::getUniformBlockIndex(string& uniformName)
{
	auto location = glGetUniformBlockIndex(m_programID, uniformName.c_str());
	if (location == -1)
	{
		cerr << "ERROR CShaderGL::getUniformBlockIndex uniform \"" << uniformName << "\" not found!" << endl;
	}
	return location;
}

GLint CShaderGL::setUnifrom1i(string& uniformName, GLint i0)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniform1i(location, i0);
	return location;
}

GLint CShaderGL::setUniform1f(string& uniformName, GLfloat v0)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniform1f(location, v0);
	return location;
}

GLint CShaderGL::setUniform3fv(string& uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniform3fv(location, 1, data);
	return location;
}

GLint CShaderGL::setUniform3fv(string& uniformName, GLsizei i, const GLfloat* data)
{
	return setUniform3fv(uniformName, i, 1, data);
}

GLint CShaderGL::setUniform3fv(string& uniformName, GLsizei i, GLsizei count, const GLfloat* data)
{
	string str = uniformName + "[" + to_string(i) + "]";
	auto location = getUniformLocation(str);
	glUseProgram(m_programID);
	glUniform3fv(location, count, data);
	return location;
}

GLint CShaderGL::setUniform4fv(string& uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniform4fv(location, 1, data);
	return location;
}

GLint CShaderGL::setUniform4fv(string& uniformName, GLsizei i, const GLfloat* data)
{
	return setUniform4fv(uniformName, i, 1, data);
}

GLint CShaderGL::setUniform4fv(string& uniformName, GLsizei i, GLsizei count, const GLfloat* data)
{
	string str = uniformName + "[" + to_string(i) + "]";
	auto location = getUniformLocation(str);
	glUseProgram(m_programID);
	glUniform4fv(location, count, data);
	return location;
}

GLint CShaderGL::setUniformMat4fv(string& uniformName, const GLfloat* data)
{
	auto location = getUniformLocation(uniformName);
	glUseProgram(m_programID);
	glUniformMatrix4fv(location, 1, false, data);
	return location;
}

GLuint CShaderGL::setUniformBlock(string& uniformName,
	GLintptr offset, GLsizei size, const void* data)
	// should glDeleteBuffer after draw
{
	glUseProgram(m_programID);
	GLuint block = -1;
	glGenBuffers(1, &block);
	glBindBuffer(GL_UNIFORM_BUFFER, block);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	auto block_index = getUniformBlockIndex(uniformName);
	glBindBufferBase(GL_UNIFORM_BUFFER, block_index, block);
	void* pBuf = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	memcpy(pBuf, data, size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return block;
}

void CShaderGL::use()
{
	glUseProgram(m_programID);
}

void CShaderGL::unuse()
{
	glUseProgram(0);
}
/*CShaderGL end*/

/*CTextureGL start*/
CTextureGL::CTextureGL(GLenum target, GLenum aciveIndex)
	:m_target(target), m_aciveIndex(aciveIndex)
{
	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER must be decleared
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(m_target, 0);
}

CTextureGL::~CTextureGL()
{
	if (m_texture == -1) glDeleteTextures(1, &m_texture);
}

GLuint CTextureGL::getTexture()
{
	return m_texture;
}

GLenum CTextureGL::getTarget()
{
	return m_target;
}

GLenum CTextureGL::getActiveIndex()
{
	return m_aciveIndex;
}

GLint CTextureGL::getTexLevelParameter(GLint level, GLenum  pname)
{
	int param;
	glBindTexture(m_target, m_texture);
	glGetTexLevelParameteriv(m_target, level, pname, &param);
	glBindTexture(m_target, 0);
	return param;
}

GLint CTextureGL::getTexWidth(GLint level)
{
	return getTexLevelParameter(level, GL_TEXTURE_WIDTH);
}

GLint CTextureGL::getTexHeight(GLint level)
{
	return getTexLevelParameter(level, GL_TEXTURE_HEIGHT);
}

GLint CTextureGL::getInternalFormat(GLint level)
{
	return getTexLevelParameter(level, GL_TEXTURE_INTERNAL_FORMAT);
}

void CTextureGL::getTexImage(GLint level, GLenum format, GLenum type, void* pixels)
{
	glBindTexture(m_target, m_texture);
	glGetTexImage(m_target, level, format, type, pixels);
	glBindTexture(m_target, 0);
}

void CTextureGL::texParameteri(GLenum pname, GLint param)
{
	glBindTexture(m_target, m_texture);
	glTexParameteri(m_target, pname, param);
	glBindTexture(m_target, 0);
}

GLenum CTextureGL::active(GLenum aciveIndex) // < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS -1
{
	auto oldActiveIndex = m_aciveIndex;
	m_aciveIndex = aciveIndex;
	glActiveTexture(m_aciveIndex);
	return oldActiveIndex;
}

GLenum CTextureGL::active() // return the previous activeIndex
{
	return active(m_aciveIndex);
}

void CTextureGL::bind()
{
	glBindTexture(m_target, m_texture);
}

void CTextureGL::unbind()
{
	glBindTexture(m_target, 0);
}
/*CTextureGL end*/

/*CTexture2DGL start*/ 
CTexture2DGL::CTexture2DGL(GLenum aciveIndex):CTextureGL(GL_TEXTURE_2D, aciveIndex)
{

}

CTexture2DGL::CTexture2DGL(GLsizei width, GLsizei height,
	GLenum aciveIndex, GLenum internalFormat) : CTexture2DGL(aciveIndex)
{
	m_width = width; m_height = height;
	m_internalFormat = internalFormat;
}

void CTexture2DGL::texImage2D(GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid* data)
{
	m_width = width; m_height = height; m_internalFormat = internalFormat;
	glBindTexture(m_target, m_texture);
	glTexImage2D(m_target, level, internalFormat, 
		width, height, border, format, type, data);
	if (level > 0) glGenerateMipmap(m_target);
	glBindTexture(m_target, 0);
}

void CTexture2DGL::texImage2D(const GLvoid* data, GLint level, GLenum format, GLenum type)
{
	if (m_width == 0 || m_height == 0)
	{
		cerr << "ERROR CTexture2DGL::texImage2D need to define texutre before" << endl;
		return;
	}
	texImage2D(level, m_internalFormat, m_width, m_height, 0, format, type, data);
}

void CTexture2DGL::texSubImage2D(GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type, const GLvoid* data)
{
	glBindTexture(m_target, m_texture);
	glTexSubImage2D(m_target, level, xoffset, yoffset,
		width, height, format, type, data);
	glBindTexture(m_target, 0);
}
/*CTexture2DGL end*/
#endif