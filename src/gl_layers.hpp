#ifdef USE_OPENGL
#ifndef _GL_LAYERS_H
#define _GL_LAYERS_H
#include <memory>
#include <vector>
#include <set>
#include <string>
#include "gl_scene.hpp"
#include "gl_assets.hpp"
#include "gl_object3d.hpp"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::set;
using std::string;
class CMeshGL;
class CSceneGL;
class CShaderGL;
class CTextureGL;
class CObject3DGL;
typedef struct Light Light;
typedef struct MaterialPhong MaterialPhong;
typedef void (*PFNCOBJECT3DGLCB)(int shaderIndex, CObject3DGL* object, CSceneGL* scene, void* data1, void* data2);
typedef void (*PFNCMESHGLCB)(int shaderIndex, CMeshGL* mesh, CSceneGL* scene, void* data1, void* data2);
#define STCFIELDSTRING(STC, FIELD) string(STC##"."##FIELD)
#define STCIFIELDSTRING(STC, INDEX, FIELD) string(STC) +"["+ to_string(INDEX) +"]."+ string(FIELD)

#define VIEWPOS_NAME "viewPos"
#define LIGHTS_NAME "lights"
#define LIGHT_NUM_NAME "lightsNum"
#define POSITION_NAME "position"
#define COLOR_NAME "color"
#define ATTENUATION_NAME "attenuation"
#define DIRECTION_NAME "direction"
#define CUTOFF_NAME "cutoff"
#define OUTERCUTOFF_NAME "outerCutoff"

#define MATERIAL_NAME "material"
#define AMBIENT_NAME "ambient"
#define DIFFUSE_NAME "diffuse"
#define SPECULAR_NAME "specular"
#define SHININESS_NAME "shininess"
#define ALPHA_NAME "alpha"

// use multi layer for defered rendering, inFrameBuffer -> outFrameBuffer
class CLayerGL
{
protected:
	CSceneGL& m_scene;
	size_t m_layerIndex;
	shared_ptr<CShaderGL> m_layerShader;
	shared_ptr<CTextureGL> m_inFrameBuffer, m_outFrameBuffer; // in Frame is the from last
	GLint m_scrViewport[4];
	GLint m_frameViewport[4];
	GLuint m_frameBuffer = -1;
	GLenum m_frameAttachment = 0;

protected:
	virtual void drawSceneObject(CObject3DGL* object, CShaderGL* shader, bool useTextures = true, 
		PFNCOBJECT3DGLCB pfnObjectSetCallback = NULL,
		PFNCMESHGLCB pfnMeshSetCallback = NULL, 
		void* data1=NULL, void *data2=NULL);
	virtual void drawSceneObjects(CShaderGL* shader, bool useTextures=true,
		PFNCOBJECT3DGLCB pfnObjectSetCallback = NULL,
		PFNCMESHGLCB pfnMeshSetCallback = NULL,
		void* data1 = NULL, void* data2 = NULL);
	virtual bool beforeDrawLayer(); // set unifroms and values here
	virtual bool afterDrawLayer(bool drawed);
public:
	CLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader = nullptr,
		shared_ptr<CTextureGL> outFrameBuffer = nullptr);
	GLint* getScrViewport();
	GLint* getFrameViewport();
	void setScrViewport(int x, int y, int w, int h);
	void setFrameViewport(int x, int y, int w, int h);
	GLenum getFrameAttachment();
	void setInFrameBuffer(shared_ptr<CTextureGL> inFrameBuffer);
	void setOutFrameBuffer(shared_ptr<CTextureGL> outFrameBuffer, 
		GLenum attachment= GL_COLOR_ATTACHMENT0, GLint level=0);
	shared_ptr<CTextureGL> getInFrameBuffer();
	shared_ptr<CTextureGL> getOutFrameBuffer();
	virtual ~CLayerGL();
	virtual void draw();
};

class CLayerPhongGL: public CLayerGL
{
private:
	set<GLuint> m_usedProgram;

protected:
	// give the uniform information to shaders seperately
	void setLightUniform(Light* light, CShaderGL* shader, int i);
	virtual bool beforeDrawLayer();
	virtual void drawSceneObject(CObject3DGL* object, CShaderGL* shader, bool useTextures = true,
		PFNCOBJECT3DGLCB pfnObjectSetCallback = NULL,
		PFNCMESHGLCB pfnMeshSetCallback = NULL,
		void* data1 = NULL, void* data2 = NULL);
public:
	CLayerPhongGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader = nullptr,
		shared_ptr<CTextureGL> outFrameBuffer = nullptr);
};

// generate the shadow map by every light, point light, direction light
// and then draw the shadow on the buffer
class CLayerShadowGL : public CLayerGL
{
private:
	unique_ptr<CTexture2DGL> m_shadowMapTexture;
	shared_ptr<CShaderGL> m_shadowMapShader;

protected: 
	void CLayerShadowGL::drawSceneObjects(CShaderGL* shader, CTexture2DGL* texture);
public:
	CLayerShadowGL(CSceneGL& scene,  shared_ptr<CShaderGL> shadowMapShader, 
					  shared_ptr<CShaderGL> shadowShader, 
	                  GLint width=1024, GLint height=1024, GLint level=0);
	virtual void draw();
};

// Dynamic Environment Mapping: viewing to each direction (without shadow)
// generate the enviroment map(cube map) by randering every object in 6 directions
class CLayerEnviromentGL : public CLayerGL
{
private:
	shared_ptr<CTextureCubeGL> m_enviromentMapTexture;

public:
	CLayerEnviromentGL(CSceneGL& scene, shared_ptr<CTextureCubeGL> m_enviromentMapTexture);
	virtual void draw();
};

// show the small window for debug information as HUD
class CLayerHudGL : public CLayerGL
{

public:
	CLayerHudGL(CSceneGL& scene,
		shared_ptr<CShaderGL> hudShader, GLint hudViewPort[4] = NULL);
	virtual ~CLayerHudGL();
	virtual void drawHud();
	virtual void draw();
};

//  render a small window for viewing the attitude(posture) of the camera
class CLayerHudAttitude : public CLayerHudGL
{
private:
	unique_ptr<CObject3DGL> m_attitude;

public:
	CLayerHudAttitude(CSceneGL& scene,
		shared_ptr<CShaderGL> attitudeShader, GLint hudViewPort[4] = NULL);
	void drawHud();
};

// generate a cube in the light position for viewing light
class CLayerLightGL : public CLayerGL
{
private:
	unique_ptr<CObject3DGL> m_lightCubes;
	glm::mat4 m_scale;

public:
	CLayerLightGL(CSceneGL& scene, shared_ptr<CShaderGL> lightShader, float scale=0.1f);
	void draw();
};
#endif
#endif