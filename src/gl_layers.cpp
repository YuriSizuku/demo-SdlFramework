#ifdef USE_OPENGL
#include <iostream>
#include "gl_layers.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using std::cerr;
using std::cout;
using std::endl;
using std::to_string;

/*CLayerGL start*/
CLayerGL::CLayerGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader,
	shared_ptr<CTextureGL> outFrameBuffer) :
	m_scene(scene), m_layerShader(layerShader)
{
	m_layerIndex = scene.getLayers().size();
	glGetIntegerv(GL_VIEWPORT, m_scrViewport);
	setOutFrameTexture(outFrameBuffer);
	memset(m_frameViewport, 0, sizeof(m_frameViewport));
}

CLayerGL::~CLayerGL()
{
	if(m_outFBO!=-1) glDeleteFramebuffers(1, &m_outFBO);
	m_outFBO = -1;
}

GLint* CLayerGL::getScrViewport()
{
	return m_scrViewport;
}

GLint* CLayerGL::getFrameViewport()
{
	return m_frameViewport;
}

void CLayerGL::setScrViewport(int x, int y, int w, int h)
{
	m_scrViewport[0] = x; m_scrViewport[1] = y;
	m_scrViewport[2] = w; m_scrViewport[3] = h;
}

void CLayerGL::setFrameViewport(int x, int y, int w, int h)
{
	m_frameViewport[0] = x; m_frameViewport[1] = y;
	m_frameViewport[2] = w; m_frameViewport[3] = h;
}

GLenum CLayerGL::getFrameAttachment()
{
	return m_frameAttachment;
}

void CLayerGL::setInFrameTexture(shared_ptr<CTextureGL> inFrameTexture)
{
	m_inFrameTexture = inFrameTexture;
}

void CLayerGL::setOutFrameTexture(shared_ptr<CTextureGL> outFrameTexture, 
	GLenum attachment, GLint level, bool genDepthTexture)
{
	m_outFrameTexture = outFrameTexture;
	m_frameAttachment = attachment;
	if (!m_outFrameTexture)
	{
		glCheckError();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCheckError();
		glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
		return;
	}
	else
	{
		if (m_outFBO == -1)
		{
			glGenFramebuffers(1, &m_outFBO);
		}
		int viewWidth = outFrameTexture->getTexWidth();
		int viewHeight = outFrameTexture->getTexHeight();
		glBindFramebuffer(GL_FRAMEBUFFER, m_outFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, m_frameAttachment, GL_TEXTURE_2D, 
			m_outFrameTexture->getTexture(), level);

		// append depth buffer
		if (genDepthTexture)
		{
			auto outFrameDepthTexture = shared_ptr<CTexture2DGL>(new CTexture2DGL());
			outFrameDepthTexture->texImage2D(0, GL_DEPTH_COMPONENT,
				viewWidth, viewHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, outFrameDepthTexture->getTexture(), 0);
			m_outFrameDepthTexture = outFrameDepthTexture;
		}
		setFrameViewport(0, 0, viewWidth, viewHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

shared_ptr<CTextureGL> CLayerGL::getInFrameBuffer()
{
	return m_inFrameTexture;
}

shared_ptr<CTextureGL> CLayerGL::getOutFrameBuffer()
{
	return m_outFrameTexture;
}

void CLayerGL::drawSceneObject(CObject3DGL* object, CShaderGL* shader, 
	bool useTextures, PFNCOBJECT3DGLCB pfnObjectSetCallback,
	PFNCMESHGLCB pfnMeshSetCallback, void* data1, void* data2)
{
	if(object) object->draw(m_layerIndex, shader, useTextures, 
		pfnObjectSetCallback, pfnMeshSetCallback, &m_scene, data1, data2);
}

void CLayerGL::drawSceneObjects(CShaderGL* shader, bool useTextures, 
	PFNCOBJECT3DGLCB pfnObjectSetCallback,
	PFNCMESHGLCB pfnMeshSetCallback, void* data1, void* data2)
{
	auto m_objects = m_scene.getObjects();
	for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			drawSceneObject((*it2).get(), shader, useTextures, 
				pfnObjectSetCallback, pfnMeshSetCallback, data1, data2);
		}
	}
}

bool CLayerGL::beforeDrawLayer()
{
	return true;
}

bool CLayerGL::afterDrawLayer(bool drawed)
{
	return true;
}

void CLayerGL::draw()
{
	bool drawed = false;
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_frameViewport[0], m_frameViewport[1], m_frameViewport[2], m_frameViewport[3]);
	}
	if (m_outFBO != -1)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_outFBO);
		glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear output buffer before use
	}
	if (beforeDrawLayer())
	{
		drawSceneObjects(m_layerShader.get());
		drawed = true;
	}
	if (m_outFBO != -1)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
	}
	afterDrawLayer(drawed);
}
/*CLayerGL end*/

/*CLayerPhongGL start*/
CLayerPhongGL::CLayerPhongGL(CSceneGL& scene, shared_ptr<CShaderGL> layerShader,
	shared_ptr<CTextureGL> outFrameBuffer): CLayerGL(scene, layerShader, outFrameBuffer)
{
		
}

void CLayerPhongGL::setLightUniform(Light* light, CShaderGL* shader, GLsizei i)
{
	shader->setUniform4fv(STCIFIELDSTRING(LIGHTS_NAME, i, POSITION_NAME),
		glm::value_ptr(light->position));
	shader->setUniform3fv(STCIFIELDSTRING(LIGHTS_NAME, i, DIRECTION_NAME),
		glm::value_ptr(light->direction));
	shader->setUniform3fv(STCIFIELDSTRING(LIGHTS_NAME, i, AMBIENT_NAME),
		glm::value_ptr(light->ambient));
	shader->setUniform3fv(STCIFIELDSTRING(LIGHTS_NAME, i, DIFFUSE_NAME),
		glm::value_ptr(light->diffuse));
	shader->setUniform3fv(STCIFIELDSTRING(LIGHTS_NAME, i, SPECULAR_NAME),
		glm::value_ptr(light->specular));
	shader->setUniform3fv(STCIFIELDSTRING(LIGHTS_NAME, i, ATTENUATION_NAME),
		glm::value_ptr(light->attenuation));
	shader->setUniform1f(STCIFIELDSTRING(LIGHTS_NAME, i, CUTOFF_NAME),
		light->cutoff);
	shader->setUniform1f(STCIFIELDSTRING(LIGHTS_NAME, i, OUTERCUTOFF_NAME),
		light->outerCutoff);
}

bool CLayerPhongGL::beforeDrawLayer()
{
	m_usedProgram.clear();
	return true;
}

void CLayerPhongGL::drawSceneObject(CObject3DGL* object, CShaderGL* shader, bool useTextures, 
	PFNCOBJECT3DGLCB pfnObjectSetCallback,
	PFNCMESHGLCB pfnMeshSetCallback, void* data1, void* data2)
{
	vector<Light>& lights = m_scene.getLights();
	if (shader) // use layer shader
	{
		shader->setUniform3fv(string(CAMERAPOS_NAME),glm::value_ptr(m_scene.getCamera().position));
		shader->setUnifrom1i(string(LIGHT_NUM_NAME), lights.size());
		for (GLsizei i = 0; i < static_cast<GLsizei>(lights.size()); i++)
		{
			setLightUniform(&lights[i], shader, i);
		}
	}
	else
	{
		// assign global uniforms, such as Light, to avoid duplicate
		for (shared_ptr<CMeshGL>& mesh : object->getMeshs())
		{
			shader = mesh->getShaders()[m_layerIndex].get();
			if (m_usedProgram.find(shader->getProgram()) == m_usedProgram.end())
			{
				shader->setUnifrom1i(string(LIGHT_NUM_NAME), lights.size());
				shader->setUniform3fv(string(CAMERAPOS_NAME),
					glm::value_ptr(m_scene.getCamera().position));
				for (GLsizei i = 0; i < static_cast<GLsizei>(lights.size()); i++)
				{
					setLightUniform(&lights[i], shader, i);
				}
				m_usedProgram.insert(shader->getProgram());
			}
		}
	}

	// set individual unifrom for each mesh
	pfnMeshSetCallback = [](int shaderindex, CMeshGL* mesh, CSceneGL* scene, void* data1, void *data2)->void
	{
		if (!mesh)
		{
			cerr << "ERROR  CLayerPhongGL::drawSceneObject pfnMeshSetCallback mesh is NULL" << endl;
			return;
		}
		if (!mesh->getMaterial()) return;

		auto shader = mesh->getShaders()[shaderindex];
		MaterialPhong* material = static_cast<MaterialPhong*>(mesh->getMaterial().get());
		shader->setUniform3fv(STCFIELDSTRING(MATERIAL_NAME, AMBIENT_NAME),
			glm::value_ptr(material->ambient));
		shader->setUniform3fv(STCFIELDSTRING(MATERIAL_NAME, DIFFUSE_NAME),
			glm::value_ptr(material->diffuse));
		shader->setUniform3fv(STCFIELDSTRING(MATERIAL_NAME, SPECULAR_NAME),
			glm::value_ptr(material->specular));
		shader->setUniform1f(STCFIELDSTRING(MATERIAL_NAME, SHININESS_NAME),
			material->shininess);
		shader->setUniform1f(STCFIELDSTRING(MATERIAL_NAME, ALPHA_NAME),
			material->alpha);
	};
	object->draw(m_layerIndex, shader, true, NULL, pfnMeshSetCallback, &m_scene);
}
/*CLayerPhongGL end*/

/*CLayerShadowGL start*/
CLayerShadowGL::CLayerShadowGL(CSceneGL& scene, 
	shared_ptr<CShaderGL> depthMapShader,shared_ptr<CShaderGL> shadowShader,
	GLint width, GLint height, GLint level): CLayerGL(scene, shadowShader)
{
	m_depthMapShader = depthMapShader; 
	m_depthMapWidth = width, m_depthMapHeight = height;
	
	// prepare  depth texture
	m_depthMap2D = shared_ptr<CTexture2DGL>(new CTexture2DGL(GL_TEXTURE1));
	// https://gamedev.stackexchange.com/questions/151865/opengl-es-2-0-shadow-mapping-depth-only-fbo-not-working-due-to-gl-framebuffer
	m_depthMap2D->texImage2D(level, GL_DEPTH_COMPONENT, width, height, 
		0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL); // GL_FLOAT error 

	// prepare depth cube texture

	// prepare depth framebuffer;
	glGenFramebuffers(1, &m_depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap2D->getTexture(), 0);
	// glDrawBuffer(GL_NONE); // can not work on gles
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckError();
}

CLayerShadowGL::~CLayerShadowGL()
{
	glDeleteFramebuffers(1, &m_depthMapFBO);
}

void CLayerShadowGL::drawSceneObjects(CShaderGL* shader,
	CTextureGL* shadowMap, CTextureGL* renderTexture)
{
	PFNCMESHGLCB pfnMeshSetCallback = [](int shaderindex, CMeshGL* mesh, CSceneGL* scene, void* data1, void* data2)->void
	{
		if (!mesh)
		{
			cerr << "ERROR  CLayerShadowGL::drawSceneObject pfnMeshSetCallback mesh is NULL" << endl;
			return;
		}
		auto shadowMap = static_cast<CTextureGL*>(data1);
		auto renderTexture = static_cast<CTextureGL*>(data2);
		if (shadowMap)
		{
			shadowMap->active();
			shadowMap->bind();
		}
		if (renderTexture)
		{
			renderTexture->active();
			renderTexture->bind();
		}
	};
	CLayerGL::drawSceneObjects(shader, false, NULL, pfnMeshSetCallback, renderTexture, shadowMap);
}

void  CLayerShadowGL::setInFrameTexture(shared_ptr<CTextureGL> inFrameTexture)
{
	CLayerGL::setInFrameTexture(inFrameTexture);
	m_inFrameTexture->active(GL_TEXTURE0);
	m_layerShader->setUnifrom1i(string(RENDERTEX_NAME), m_inFrameTexture->getActiveIndex() - GL_TEXTURE0);
}

void CLayerShadowGL::setOrthoParams(float* orthoParams)
{
	memcpy(m_orthoParams, orthoParams, sizeof(m_orthoParams));
}

float* CLayerShadowGL::getOrthoParmas()
{
	return m_orthoParams;
}

bool CLayerShadowGL::enableCullFront(bool useCullFront)
{
	bool oldUseCullFront = m_useCullFront;
	m_useCullFront = useCullFront;
	return oldUseCullFront;
}

void CLayerShadowGL::setBias(GLfloat biasMin, GLfloat biasMax)
{
	m_biasMin = biasMin;
	m_biasMax = biasMax;
}

void CLayerShadowGL::draw()
{
	CMapList<std::shared_ptr<CObject3DGL>>& m_objects = m_scene.getObjects();
	bool drawed = false;
	
	// render on the view of each light
	for (Light& light : m_scene.getLights())
	{
		// transfor view to the light, and set uniforms
		glm::mat4 lightView;
		glm::mat4 lightProjection;
		if (fabs(light.position.w) < 0.001f) // direction light
		{
			lightView = glm::lookAt(glm::vec3(light.position), 
				glm::vec3(light.position) + glm::vec3(light.direction), 
				{ 0.f, 1.f, 0.f });
			lightProjection = glm::ortho(m_orthoParams[0], m_orthoParams[1], 
				m_orthoParams[2], m_orthoParams[3],
				m_orthoParams[4], m_orthoParams[5]);
			m_layerShader->setUniformMat4fv(string(LIGHTMATRIX_NAME), glm::value_ptr(lightProjection * lightView));
			m_layerShader->setUnifrom1i(string(SHADOWMAP2D_NAME), m_depthMap2D->getActiveIndex() - GL_TEXTURE0);
		}
		else
		{
			continue;
			if (light.cutoff > 0.f) // spot light
			{
				lightView = glm::lookAt(glm::vec3(light.position),
					glm::vec3(light.position) + glm::vec3(light.direction), { 0.f, 1.f, 0.f });
				lightProjection = m_scene.getProjection();
			}
			else // point light 
			{
				lightView = m_scene.getView();
				lightProjection = m_scene.getProjection();
			}
		}
		m_depthMapShader->setUniformMat4fv(string(VIEW_MATRIX_NAME), glm::value_ptr(lightView));
		m_depthMapShader->setUniformMat4fv(string(PROJECTION_MATRIX_NAME), glm::value_ptr(lightProjection));
		m_depthMapShader->setUniform4fv(STCFIELDSTRING(LIGHT_NAME, POSITION_NAME), glm::value_ptr(light.position));
		m_depthMapShader->setUniform1f(STCFIELDSTRING(LIGHT_NAME, CUTOFF_NAME), light.cutoff);
		m_layerShader->setUniform4fv(STCFIELDSTRING(LIGHT_NAME, POSITION_NAME), glm::value_ptr(light.position));
		m_layerShader->setUniform1f(STCFIELDSTRING(LIGHT_NAME, CUTOFF_NAME), light.cutoff);
		m_layerShader->setUniform1f(string(BIASMIN_NAME), m_biasMin);
		m_layerShader->setUniform1f(string(BIASMAX_NAME), m_biasMax);

		// render shadowmap
		glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
		glViewport(0, 0, m_depthMapWidth, m_depthMapHeight);
		glClear(GL_DEPTH_BUFFER_BIT); // this is very important, must clear before use!!!
		if(m_useCullFront) glCullFace(GL_FRONT);
		drawSceneObjects(m_depthMapShader.get(), NULL, NULL);
		if(m_useCullFront) glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 

		// blend the shadow 
		if (m_frameViewport[2] && m_frameViewport[3])
		{
			glViewport(m_frameViewport[0], m_frameViewport[1], m_frameViewport[2], m_frameViewport[3]);
		}
		else
		{
			glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
		}
		if (m_outFBO != -1)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_outFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear output buffer before use
		}
		glClear(GL_DEPTH_BUFFER_BIT);
		if (beforeDrawLayer())
		{						
			drawSceneObjects(m_layerShader.get(), 
				m_depthMap2D.get(), m_inFrameTexture.get());
			m_inFrameTexture->unbind();
			m_depthMap2D->unbind();
			drawed = true;
		}
		if (m_outFBO != -1)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		if (m_frameViewport[2] && m_frameViewport[3])
		{
			glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
		}
		afterDrawLayer(drawed);
	}
}
/*CLayerShadowGL end*/

/*CLayerViewTextureGL start*/
CLayerDrawTextureGL::CLayerDrawTextureGL(CSceneGL& scene, 
	shared_ptr<CShaderGL> textureShader, shared_ptr<CTextureGL> texture)
	: CLayerGL(scene, textureShader),m_viewTexture(texture)
{
	map<int, glm::vec2> texcoords;
	texcoords[0] = {1.f, 1.f};
	texcoords[1] = { 0.f, 1.f };
	texcoords[2] = { 0.f, 0.f };
	texcoords[3] = { 1.f, 0.f };
	m_viewMesh = unique_ptr<CMeshGL>(new CPlaneMeshGL(
		glm::scale(glm::mat4(1), { 2.f, 2.f, 2.f }),  
		nullptr, GL_STATIC_DRAW, texcoords));
}

void CLayerDrawTextureGL::setViewTexture(shared_ptr<CTextureGL> texture)
{
	m_viewTexture = texture;
}

shared_ptr<CTextureGL> CLayerDrawTextureGL::getViewTexture()
{
	return m_viewTexture;
}

void CLayerDrawTextureGL::draw()
{
	bool drawed = false;
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_frameViewport[0], m_frameViewport[1], m_frameViewport[2], m_frameViewport[3]);
	}
	if (m_outFBO != -1) glBindFramebuffer(GL_FRAMEBUFFER, m_outFBO);
	
	glm::mat4 identity = glm::mat4(1);
	m_layerShader->setUniformMat4fv(string(VIEW_MATRIX_NAME), glm::value_ptr(identity));
	m_layerShader->setUniformMat4fv(string(PROJECTION_MATRIX_NAME), glm::value_ptr(identity));
	m_viewTexture->active(GL_TEXTURE0);
	m_viewTexture->bind();
	if (beforeDrawLayer())
	{
		m_viewMesh->draw(identity, 0, m_layerShader.get());
		drawed = true;
	}
	m_viewTexture->unbind();
	if (m_outFBO != -1) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
	}
	afterDrawLayer(drawed);
}
/*CLayerViewTextureGL end*/

/*CLayerHudGL start*/
CLayerHudGL::CLayerHudGL(CSceneGL& scene,
	shared_ptr<CShaderGL> hudShader, GLint hudViewPort[4]) :CLayerGL(scene, hudShader)
{
	if (hudViewPort)
	{
		memcpy(m_frameViewport, hudViewPort, sizeof(GLint) * 4);
	}
	else
	{
		GLint unit = m_scrViewport[3] / 80;
		m_frameViewport[2] = 6 * unit;
		m_frameViewport[3] = 6 * unit;
		m_frameViewport[0] = m_scrViewport[2] - m_frameViewport[2] - 2 * unit;
		m_frameViewport[1] = m_scrViewport[3] - m_frameViewport[2] - 2 * unit;
	}
}

CLayerHudGL::~CLayerHudGL()
{

}

void CLayerHudGL::drawHud()
{
	CLayerGL::draw();
}

void CLayerHudGL::draw()
{
	glViewport(m_frameViewport[0], m_frameViewport[1], m_frameViewport[2], m_frameViewport[3]);
	drawHud();
	glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
}
/*CLayerHudGL end*/

/*CLayerHudAttitude start*/
CLayerHudAttitude::CLayerHudAttitude(CSceneGL& scene,
	shared_ptr<CShaderGL> attitudeShader,
	GLint hudViewPort[4]) :CLayerHudGL(scene, attitudeShader, hudViewPort)
{
	GLfloat vbo_buf[] = { // positon, texcoord(color rrrggg  bbbaaa)
		0.f, 0.f, 0.f, 999000.f, 000999.f, 1.f, 0.f, 0.f, 999000.f, 000999.f,
		0.f, 0.f, 0.f, 000999.f, 000999.f, 0.f, 1.f, 0.f, 000999.f, 000999.f,
		0.f, 0.f, 0.f, 000000.f, 999999.f, 0.f, 0.f, 1.f, 000000.f, 999999.f
	};
	
	auto mesh = shared_ptr<CMeshGL>(new CMeshGL());
	mesh->setDrawMode(GL_LINES);
	mesh->fillVBO(sizeof(vbo_buf), vbo_buf);
	mesh->fillVAO(vector<GLint>({ 3, 2 }));
	m_attitude = unique_ptr<CObject3DGL>(new CObject3DGL(glm::mat4(1), mesh));
}

void CLayerHudAttitude::drawHud()
{
	auto view = glm::mat4(glm::mat3(m_scene.getView()));
	view[3] = glm::vec4(0.f, 0.f, -3.f, 1.f); //fix the camera to(0,0,3)
	auto project = glm::perspective(glm::radians(30.f), 1.2f, 0.1f, 100.f);
	if (!m_layerShader)
	{
		cerr << "CLayerHudAttitude::drawHud layer shader is NULL" << endl;
		return;
	}
	m_layerShader->setUniformMat4fv(string(VIEW_MATRIX_NAME), glm::value_ptr(view));
	m_layerShader->setUniformMat4fv(string(PROJECTION_MATRIX_NAME), glm::value_ptr(project));
	glLineWidth(2.f);
	m_attitude->draw(0, m_layerShader.get());
	glLineWidth(1.f);
}
/*CLayerHudAttitude end*/

/*CLayerLightGL start*/
CLayerLightGL::CLayerLightGL(CSceneGL& scene, shared_ptr<CShaderGL> lightShader, float scale)
	:CLayerGL(scene), m_scale(glm::scale(glm::mat4(1), {scale, scale, scale}))
{
	vector<Light>& lights = m_scene.getLights();
	m_layerShader = lightShader;
	m_lightCubes = unique_ptr<CObject3DGL>(new CObject3DGL());

	for (size_t i = 0; i < lights.size(); i++)
	{
		if (fabs(lights[i].position.w) <= 0.001f)
		{
			auto mesh = shared_ptr<CMeshGL>(new CCubeMeshGL(m_scale, nullptr, GL_STATIC_DRAW));
			m_lightCubes->pushMesh(mesh);
			//m_lightCubes->pushMesh(nullptr);
		}
		else
		{
			auto mesh = shared_ptr<CMeshGL>(new CCubeMeshGL(m_scale, nullptr, GL_STATIC_DRAW));
			m_lightCubes->pushMesh(mesh);
		}
	}
}

void CLayerLightGL::draw()
{
	vector<Light>& lights = m_scene.getLights();
	vector<shared_ptr<CMeshGL>>& meshes = m_lightCubes->getMeshs();
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (!meshes[i]) continue;
		glm::vec3 blendColor = glm::clamp(lights[i].ambient + 
			lights[i].diffuse + lights[i].specular, glm::vec3(0), glm::vec3(1));
		glm::vec4 color = glm::vec4(blendColor, 1.f);
		m_lightCubes->setModel(glm::translate(glm::mat4(1), glm::vec3(lights[i].position)));
		m_layerShader->setUniform4fv(string("color"), glm::value_ptr(color));
		meshes[i]->draw(m_lightCubes->getModel(), 0, m_layerShader.get());
	}
}
/*CLayerLightGL end*/
#endif