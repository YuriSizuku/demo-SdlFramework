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
	m_scene(scene), m_layerShader(layerShader), m_outFrameBuffer(outFrameBuffer)
{
	m_layerIndex = scene.getLayers().size();
}

CLayerGL::~CLayerGL()
{

}

void CLayerGL::setInFramebuffer(shared_ptr<CTextureGL> inFrameBuffer)
{
	m_inFrameBuffer = inFrameBuffer;
}

shared_ptr<CTextureGL> CLayerGL::getOutFrameBuffer()
{
	return m_outFrameBuffer;
}

void CLayerGL::drawSceneObject(CObject3DGL* object, CShaderGL* shader)
{
	if(object) object->draw(m_layerIndex, shader);
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
	auto m_objects = m_scene.getObjects();
	bool drawed = false;
	if (beforeDrawLayer())
	{
		for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				drawSceneObject((*it2).get(), m_layerShader.get());
			}
		}
		drawed = true;
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

void CLayerPhongGL::drawSceneObject(CObject3DGL* object, CShaderGL* shader)
{
	vector<Light>& lights = m_scene.getLights();
	if (shader) // use layer shader
	{
		shader->setUniform3fv(string(VIEWPOS_NAME),glm::value_ptr(m_scene.getCamera().pos));
		shader->setUnifrom1i(string(LIGHT_NUM_NAME), lights.size());
		for (GLsizei i = 0; i < static_cast<GLsizei>(lights.size()); i++)
		{
			setLightUniform(&lights[i], shader, i);
		}
	}
	else
	{
		// assign global uniforms, such as Light, to avoid duplicate
		for (auto mesh : object->getMeshs())
		{
			shader = mesh->getShaders()[m_layerIndex].get();
			if (m_usedProgram.find(shader->getProgram()) == m_usedProgram.end())
			{
				shader->setUnifrom1i(string(LIGHT_NUM_NAME), lights.size());
				shader->setUniform3fv(string(VIEWPOS_NAME),
					glm::value_ptr(m_scene.getCamera().pos));
				for (GLsizei i = 0; i < static_cast<GLsizei>(lights.size()); i++)
				{
					setLightUniform(&lights[i], shader, i);
				}
				m_usedProgram.insert(shader->getProgram());
			}
		}
	}

	// set individual unifrom for each mesh
	PFNCMESHGLCB pfnMeshSetCallback = [](int shaderindex, CMeshGL* mesh, CSceneGL* scene)->void
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
	object->draw(m_layerIndex, shader, NULL, pfnMeshSetCallback);
}
/*CLayerPhongGL end*/

/*CLayerHudGL start*/
CLayerHudGL::CLayerHudGL(CSceneGL& scene,
	shared_ptr<CShaderGL> hudShader, GLint hudViewPort[4]) :CLayerGL(scene, hudShader)
{
	glGetIntegerv(GL_VIEWPORT, m_orgViewport);
	if (hudViewPort)
	{
		memcpy(m_hudViewPort, hudViewPort, sizeof(GLint) * 4);
	}
	else
	{
		GLint unit = m_orgViewport[3] / 80;
		m_hudViewPort[2] = 6 * unit;
		m_hudViewPort[3] = 6 * unit;
		m_hudViewPort[0] = m_orgViewport[2] - m_hudViewPort[2] - 2 * unit;
		m_hudViewPort[1] = m_orgViewport[3] - m_hudViewPort[2] - 2 * unit;
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
	glViewport(m_hudViewPort[0], m_hudViewPort[1], m_hudViewPort[2], m_hudViewPort[3]);
	drawHud();
	glViewport(m_orgViewport[0], m_orgViewport[1], m_orgViewport[2], m_orgViewport[3]);
}
/*CLayerHudGL end*/

/*CLayerHudAttitude start*/
CLayerHudAttitude::CLayerHudAttitude(CSceneGL& scene,
	shared_ptr<CShaderGL> attitudeShader,
	GLint hudViewPort[4]) :CLayerHudGL(scene, attitudeShader, hudViewPort)
{
	if (!m_layerShader)
	{
		cerr << "CLayerHudAttitude::drawHud layer shader is NULL" << endl;
		return;
	}
	GLfloat vbo_buf[] = { // positon, texcoord(color rrrggg  bbbaaa)
		0.f, 0.f, 0.f, 999000.f, 000999.f, 1.f, 0.f, 0.f, 999000.f, 000999.f,
		0.f, 0.f, 0.f, 000999.f, 000999.f, 0.f, 1.f, 0.f, 000999.f, 000999.f,
		0.f, 0.f, 0.f, 000000.f, 999999.f, 0.f, 0.f, 1.f, 000000.f, 999999.f
	};
	
	auto mesh = shared_ptr<CMeshGL>(new CMeshGL());
	mesh->setDrawMode(GL_LINES);
	mesh->fillVBO(sizeof(vbo_buf), vbo_buf);
	mesh->fillVAO(vector<GLint>({ 3, 2 }));
	m_attitude = shared_ptr<CObject3DGL>(new CObject3DGL(glm::mat4(1), mesh));
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
#endif