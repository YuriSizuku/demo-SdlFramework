#ifdef USE_OPENGL
#ifndef _GL_LAYERS_H
#define _GL_LAYERS_H
#include <memory>
#include <vector>
#include <string>
#include "gl_scene.hpp"
#include "gl_assets.hpp"
#include "gl_object3d.hpp"

using std::shared_ptr;
using std::vector;
using std::string;
class CSceneGL;
class CShaderGL;
class CTextureGL;
class CObject3DGL;
typedef struct Light Light;

// use multi layer for defered rendering, inFrameBuffer -> outFrameBuffer
class CLayerGL
{
protected:
	CSceneGL& m_scene;
	size_t m_layerIndex;
	shared_ptr<CShaderGL> m_layerShader;
	shared_ptr<CTextureGL> m_inFrameBuffer, m_outFrameBuffer; // in Frame is the from last

protected:
	virtual void drawSceneObjects(shared_ptr<CShaderGL> shader);
	virtual bool beforeDrawLayer(); // set unifroms and values here
	virtual bool afterDrawLayer(bool drawed);
public:
	CLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader = nullptr,
		shared_ptr<CTextureGL> outFrameBuffer = nullptr);
	void setInFramebuffer(shared_ptr<CTextureGL> inFrameBuffer);
	shared_ptr<CTextureGL> getOutFrameBuffer();
	virtual ~CLayerGL();
	virtual void draw();
};

class CLayerPhongGL: public CLayerGL
{

};

// generate the shadow map by every light, point light, direction light
// combine all light shadow positions to shadowMapTexture (in clip space)
class CShadowMapLayerGL : public CLayerGL
{
private:
	shared_ptr<CTextureGL> m_shadowMapTexture;

public:
	void genLightShadowMap(Light* light);
	CShadowMapLayerGL(CSceneGL& scene, shared_ptr<CTextureGL> shadowMapTexture);
	virtual ~CShadowMapLayerGL();
	virtual void draw();
};

// Dynamic Environment Mapping: viewing to each direction (without shadow)
// generate the enviroment map(cube map) by randering every object in 6 directions
class CEnviromentLayerGL : public CLayerGL
{
private:
	shared_ptr<CTextureCubeGL> m_enviromentMapTexture;

public:
	CEnviromentLayerGL(CSceneGL& scene, shared_ptr<CTextureCubeGL> m_enviromentMapTexture);
	virtual ~CEnviromentLayerGL();
	virtual void draw();
};

// blend all textures(such as shallow, reflect, sky box) for defered rendering  
class CBlendLayerGL :public CLayerGL
{
private:
	vector<shared_ptr<CTextureGL>> m_textures;

public:
	CBlendLayerGL(CSceneGL& scene, vector<shared_ptr<CTextureGL>>& textures);
	virtual ~CBlendLayerGL();
	virtual void draw();
};

// show the small window for debug information as HUD
class CLayerHudGL : public CLayerGL
{
protected:
	GLint m_orgViewport[4], m_hudViewPort[4];

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
	shared_ptr<CObject3DGL> m_attitude;

public:
	CLayerHudAttitude(CSceneGL& scene,
		shared_ptr<CShaderGL> attitudeShader, GLint hudViewPort[4] = NULL);
	virtual void drawHud();
	virtual ~CLayerHudAttitude();
};

// generate a cube in the light position for viewing light
class CLayerLightGL : public CLayerGL
{
public:
	CLayerLightGL(CSceneGL& scene, shared_ptr<CShaderGL> lightShader);
	virtual ~CLayerLightGL();
	virtual void draw();
};
#endif
#endif