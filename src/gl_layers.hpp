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

protected:
	virtual void drawSceneObject(CObject3DGL* object, CShaderGL* shader);
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
private:
	set<GLuint> m_usedProgram;

protected:
	// give the uniform information to shaders seperately
	void setLightUniform(Light* light, CShaderGL* shader, int i);
	virtual bool beforeDrawLayer();
	virtual void drawSceneObject(CObject3DGL* object, CShaderGL* shader);
public:
	CLayerPhongGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader = nullptr,
		shared_ptr<CTextureGL> outFrameBuffer = nullptr);
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
	virtual void draw();
};

// blend all textures(such as shallow, reflect, sky box) for defered rendering  
class CBlendLayerGL :public CLayerGL
{
private:
	vector<shared_ptr<CTextureGL>> m_textures;

public:
	CBlendLayerGL(CSceneGL& scene, vector<shared_ptr<CTextureGL>>& textures);
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