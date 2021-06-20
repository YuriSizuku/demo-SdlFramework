#ifdef USE_OPENGL
#ifndef _GL_ASSETS_H
#define _GL_ASSETS_H
#include <vector>
#include <string>
#include <GL/glew.h>
using std::vector;
using std::string;

GLenum _glCheckError(const char* file, int line);
#ifdef _DEBUG
#define glCheckError() _glCheckError(__FILE__, __LINE__)
#else
#define glCheckError()
#endif

class CModelLoader
{

};

// a class of shader manager, for loading shaders, compile, link and use program for rendering
class CShaderGL
{
protected:
	GLuint m_programID = -1;
	vector<GLuint> m_shadersID;

public:
	CShaderGL();
	CShaderGL(string& vertPath, string& fragPath, string geometryPath = string(""));
	void addShaderFile(string& path, GLenum shaderType);
	void addShaderSource(string& source, GLenum shaderType);
	void linkProgram();
	GLuint getProgram();

	// get, set uniform, return location
	GLint getUniformLocation(string& uniformName);
	GLint getUniformBlockIndex(string& uniformName);
	GLint setUnifrom1i(string& uniformName, GLint i0);
	GLint setUniform1f(string& uniformName, GLfloat v0);
	GLint setUniform3fv(string& uniformName, const GLfloat* data);
	GLint setUniform3fv(string& uniformName, GLsizei i, const GLfloat* data);
	GLint setUniform3fv(string& uniformName, GLsizei i, GLsizei count, const GLfloat* data);
	GLint setUniform4fv(string& uniformName, const GLfloat* data);
	GLint setUniform4fv(string& uniformName, GLsizei i, const GLfloat* data);
	GLint setUniform4fv(string& uniformName, GLsizei i, GLsizei count, const GLfloat* data);
	GLint setUniformMat4fv(string& uniformName, const GLfloat* data);
	// return gpu block buffer, should glDeleteBuffer after draw
	GLuint setUniformBlock(string& uniformName, 
		GLintptr offset, GLsizei size, const void* data);
	void use();
	void unuse();
	virtual ~CShaderGL();
};

// a class to load images from file
class CImageGL
{

};

// a class of texture manager, for filling texture of reading from texture
class CTextureGL
{
protected:
	GLuint m_texture = -1;
	GLenum m_target = GL_TEXTURE_2D;
	GLenum m_internalFormat = GL_RGBA;
	GLsizei m_width = 0, m_height = 0;
	GLenum m_aciveIndex = GL_TEXTURE0;

public:
	CTextureGL(GLenum target, GLenum aciveIndex= GL_TEXTURE0);
	GLuint getTexture();
	GLenum getTarget();
	GLenum getActiveIndex();
	GLint getTexLevelParameter(GLint level, GLenum  pname);
	GLint getTexWidth(GLint level=0);
	GLint getTexHeight(GLint level = 0);
	GLint getInternalFormat(GLint level=0);
	void getTexImage(GLint level, GLenum format, GLenum type, void* pixels);
	void texParameteri(GLenum pname, GLint param);
	GLenum active(GLenum aciveIndex); // < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS -1
	GLenum active(); // return the previous activeIndex
	void bind();
	void unbind();
	virtual ~CTextureGL();
};

class CTexture2DGL : public CTextureGL
{
public:
	CTexture2DGL(GLenum aciveIndex=GL_TEXTURE0);
	CTexture2DGL(GLsizei width, GLsizei height, 
		GLenum aciveIndex = GL_TEXTURE0, GLenum internalFormat = GL_RGBA);
	void texImage2D(GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid* data);
	void texImage2D(const GLvoid* data, GLint level=0, 
		GLenum format=GL_RGBA, GLenum type=GL_UNSIGNED_BYTE);
	void texSubImage2D(GLint level, GLint xoffset, GLint yoffset,
		GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid* data);
};

class CTexture3DGL :public CTextureGL
{
protected: 
	GLsizei m_depth;

public:
	CTexture3DGL(GLenum target, GLenum aciveIndex = GL_TEXTURE0);
	CTexture3DGL(GLenum target, GLsizei width, GLsizei height, GLsizei depth,
		GLenum aciveIndex = GL_TEXTURE0, GLenum internalFormat = GL_RGBA);
	void texImage3D(GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLsizei depth,
		GLint border, GLenum format, GLenum type, const GLvoid* data);
	void texSubImage3D(GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
		GLsizei width, GLsizei height, GLsizei depth,
		GLenum format, GLenum type, const GLvoid* data);
};

class CTextureCubeGL : public CTextureGL
{
protected:
	GLint m_faceTextures[6] = { -1, -1, -1, -1, -1, -1 }; // genbuffer if -1 when use

public:
	CTextureCubeGL(GLenum aciveIndex = GL_TEXTURE0);
	CTextureCubeGL(GLsizei width, GLsizei height,
		GLenum aciveIndex = GL_TEXTURE0, GLenum internalFormat = GL_RGBA);
	void setFaceTextures(GLuint faceTextures[6]);
	GLuint* getFaceTextures();
	void texImage2DI(GLenum i, GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid* data);
	virtual ~CTextureCubeGL();
};

#endif
#endif