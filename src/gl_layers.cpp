#ifdef USE_OPENGL
#include <iostream>
#include "gl_layers.hpp"
using std::cerr;
using std::endl;

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

void CLayerGL::drawSceneObjects(shared_ptr<CShaderGL> shader)
{
	auto m_objects = m_scene.getObjects();
	bool drawed = false;
	if (beforeDrawLayer())
	{
		for (auto it = m_objects.get().begin(); it != m_objects.get().end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				(*it2)->draw(m_layerIndex, shader);
			}
		}
		drawed = true;
	}
	afterDrawLayer(drawed);
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
	drawSceneObjects(m_layerShader);
}
/*CLayerGL end*/

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
	drawSceneObjects(m_layerShader);
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
	m_layerShader->setUniformMat4fv(m_scene.VIEW_MATRIX_NAME, glm::value_ptr(view));
	m_layerShader->setUniformMat4fv(m_scene.PROJECTION_MATRIX_NAME, glm::value_ptr(project));
	glLineWidth(2.f);
	m_attitude->draw(0, m_layerShader);
	glLineWidth(1.f);
}

CLayerHudAttitude::~CLayerHudAttitude()
{

}
/*CLayerHudAttitude end*/
#endif