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

// for light
#define CAMERAPOS_NAME "cameraPosition"
#define LIGHT_NAME "light"
#define LIGHTS_NAME "lights"
#define LIGHT_NUM_NAME "lightsNum"
#define POSITION_NAME "position"
#define COLOR_NAME "color"
#define ATTENUATION_NAME "attenuation"
#define DIRECTION_NAME "direction"
#define CUTOFF_NAME "cutoff"
#define OUTERCUTOFF_NAME "outerCutoff"

// for material 
#define MATERIAL_NAME "material"
#define AMBIENT_NAME "ambient"
#define DIFFUSE_NAME "diffuse"
#define SPECULAR_NAME "specular"
#define SHININESS_NAME "shininess"
#define ALPHA_NAME "alpha"

// for shadow
#define LIGHTMATRIX_NAME "lightMatrix"
#define LIGHTMATRIXS_NAME "lightMatrixs"
#define RENDERTEX_NAME "renderTexture"
#define SHADOWMAP2D_NAME "shadowMap2D"
#define SHADOWMAPCUBE_NAME "shadowMapCube"
#define BIASMIN_NAME "biasMin"
#define BIASMAX_NAME "biasMax"

// use multi layer for defered rendering, inFrameTexture -> outFrameTexture
class CLayerGL
{
protected:
	CSceneGL& m_scene;
	size_t m_layerIndex;
	shared_ptr<CShaderGL> m_layerShader;
	// in Frame is from last layer
	shared_ptr<CTextureGL> m_inFrameTexture, m_outFrameTexture, m_outFrameDepthTexture; 
	GLint m_scrViewport[4];
	GLint m_frameViewport[4];
	GLuint m_outFBO = -1;
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
	void setInFrameTexture(shared_ptr<CTextureGL> inFrameTexture);
	void setOutFrameTexture(shared_ptr<CTextureGL> outFrameTexture, 
		GLenum attachment= GL_COLOR_ATTACHMENT0, GLint level=0, 
		bool genDepthTexture=true);
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
	shared_ptr<CTexture2DGL> m_depthMap2D; // for directional light, spot light
	shared_ptr<CTextureCubeGL> m_depthMapCube; // for point light
	int m_depthMapWidth, m_depthMapHeight;
	float m_orthoParams[6] = {-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f};
	shared_ptr<CShaderGL> m_depthMapShader;
	GLuint m_depthMapFBO = -1;
	bool m_useCullFront = true;
	GLfloat m_biasMin = 0.f, m_biasMax = 0.f; // bias for Shadow Acne problem

protected: 
	void CLayerShadowGL::drawSceneObjects(CShaderGL* shader, 
		CTextureGL* shadowMap, CTextureGL* renderTexture);
public:
	CLayerShadowGL(CSceneGL& scene,  shared_ptr<CShaderGL> depthMapShader, 
					  shared_ptr<CShaderGL> shadowShader, 
	                  GLint width=1024, GLint height=1024, GLint level=0);
	virtual void setInFrameTexture(shared_ptr<CTextureGL> inFrameTexture);
	void setOrthoParams(float *orthoParams);
	float* getOrthoParmas();
	bool enableCullFront(bool useCullFront);
	void setBias(GLfloat biasMin, GLfloat biasMax);
	virtual ~CLayerShadowGL();
	virtual void draw();
};

// Dynamic Environment Mapping: viewing to each direction (without shadow)
// generate the enviroment map(cube map) by randering every object in 6 directions
class CLayerEnviromentGL : public CLayerGL
{
private:
	shared_ptr<CTextureCubeGL> m_enviromentMap = nullptr;
	vector<shared_ptr<CLayerGL>> m_renderLayers;
	int m_enviromentMapWidth, m_enviromentMapHeight;

public:
	CLayerEnviromentGL(CSceneGL& scene, shared_ptr<CShaderGL> enviromentMapShader,
		GLint width = 1024, GLint height = 1024, GLint level = 0);
	void setEnvriomentMap(shared_ptr<CTextureCubeGL> m_enviromentMap);
	shared_ptr<CTextureCubeGL> getEnvriomentMap();
	void pushRenderLayer(shared_ptr<CLayerGL> layer);
	vector<shared_ptr<CLayerGL>>& getRenderLayers();
	virtual void draw();
};

// use this layer to view the texture on the screen
class CLayerDrawTextureGL : public CLayerGL
{
private:
	unique_ptr<CMeshGL> m_viewMesh;
	shared_ptr<CTextureGL> m_viewTexture;
	
public:
	CLayerDrawTextureGL(CSceneGL& scene, shared_ptr<CShaderGL> textureShader, 
		shared_ptr<CTextureGL> texture = nullptr);
	void setViewTexture(shared_ptr<CTextureGL> texture);
	shared_ptr<CTextureGL> getViewTexture();
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