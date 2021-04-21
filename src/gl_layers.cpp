#ifdef USE_OPENGL
#include <iostream>
#include "gl_layers.hpp"
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
	setOutFrameBuffer(outFrameBuffer);
	memset(m_frameViewport, 0, sizeof(m_frameViewport));
}

CLayerGL::~CLayerGL()
{
	if(m_frameBuffer!=-1) glDeleteFramebuffers(1, &m_frameBuffer);
	m_frameBuffer = -1;
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

void CLayerGL::setInFrameBuffer(shared_ptr<CTextureGL> inFrameBuffer)
{
	m_inFrameBuffer = inFrameBuffer;
}

void CLayerGL::setOutFrameBuffer(shared_ptr<CTextureGL> outFrameBuffer, 
	GLenum attachment, GLint level)
{
	m_outFrameBuffer = outFrameBuffer;
	m_frameAttachment = attachment;
	if (!m_outFrameBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
		return;
	}
	else
	{
		if (m_frameBuffer == -1)
		{
			glGenFramebuffers(1, &m_frameBuffer);
		}
		auto texture = m_outFrameBuffer->getTexture();
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, m_frameAttachment, GL_TEXTURE_2D, texture, level);
		setFrameViewport(0, 0, outFrameBuffer->getTexWidth(), outFrameBuffer->getTexHeight());
	}
}

shared_ptr<CTextureGL> CLayerGL::getInFrameBuffer()
{
	return m_inFrameBuffer;
}

shared_ptr<CTextureGL> CLayerGL::getOutFrameBuffer()
{
	return m_outFrameBuffer;
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
	if (beforeDrawLayer())
	{
		drawSceneObjects(m_layerShader.get());
		drawed = true;
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
		shader->setUniform3fv(string(VIEWPOS_NAME),glm::value_ptr(m_scene.getCamera().position));
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
				shader->setUniform3fv(string(VIEWPOS_NAME),
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
	shared_ptr<CShaderGL> shadowMapShader,shared_ptr<CShaderGL> shadowShader,
	GLint width, GLint height, GLint level): CLayerGL(scene, shadowShader)
{
	m_shadowMapShader = shadowMapShader;
	m_shadowMapTexture = unique_ptr<CTexture2DGL>(new CTexture2DGL(GL_TEXTURE0 + 1));
	// https://gamedev.stackexchange.com/questions/151865/opengl-es-2-0-shadow-mapping-depth-only-fbo-not-working-due-to-gl-framebuffer
	m_shadowMapTexture->texImage2D(level, GL_DEPTH_COMPONENT,
		width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL); // GL_FLOAT error
	glCheckError();
}

void CLayerShadowGL::drawSceneObjects(CShaderGL* shader, CTexture2DGL* texture)
{
	PFNCMESHGLCB pfnMeshSetCallback = [](int shaderindex, CMeshGL* mesh, CSceneGL* scene, void* data1, void* data2)->void
	{
		if (!mesh)
		{
			cerr << "ERROR  CLayerShadowGL::drawSceneObject pfnMeshSetCallback mesh is NULL" << endl;
			return;
		}
		auto texture = static_cast<CTexture2DGL*>(data1);
		if (texture)
		{
			texture->active();
			texture->bind();
		}
	};
	CLayerGL::drawSceneObjects(shader, false, NULL, pfnMeshSetCallback, texture, NULL);
}

void CLayerShadowGL::draw()
{
	CMapList<std::shared_ptr<CObject3DGL>>& m_objects = m_scene.getObjects();
	bool drawed = false;
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_frameViewport[0], m_frameViewport[1], m_frameViewport[2], m_frameViewport[3]);
	}
	if (beforeDrawLayer())
	{
		for (Light& light : m_scene.getLights())
		{
			glm::vec3 pos = glm::vec3(light.position);
			glm::vec3 center = (fabs(light.position.w) < 0.001f || light.cutoff > 0.f) ?
				pos + glm::vec3(light.direction) : m_scene.getCamera().position;
			glm::mat4 view;
			view = glm::lookAt(pos, center, { 0, 1, 0 });
			m_shadowMapShader->setUniformMat4fv(string(VIEW_MATRIX_NAME), glm::value_ptr(view));
			drawSceneObjects(m_shadowMapShader.get(), NULL);
		}
		drawSceneObjects(m_layerShader.get(),static_cast<CTexture2DGL*>(m_shadowMapTexture.get()));
		//auto texture = m_scene.getTextures()["misuzu"].get();
		//drawSceneObjects(m_layerShader.get(), static_cast<CTexture2DGL*>(texture));
		drawed = true;
	}
	if (m_frameViewport[2] && m_frameViewport[3])
	{
		glViewport(m_scrViewport[0], m_scrViewport[1], m_scrViewport[2], m_scrViewport[3]);
	}
	afterDrawLayer(drawed);
}
/*CLayerShadowGL end*/

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
			m_lightCubes->pushMesh(nullptr);
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