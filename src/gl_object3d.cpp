#ifdef USE_OPENGL
#include "gl_object3d.hpp"
/*CSceneGL start*/
CSceneGL::CSceneGL()
{

}

CSceneGL::~CSceneGL()
{

}

void CSceneGL::render()
{
	for (auto it = m_pObjects.get().begin(); it != m_pObjects.get().end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			(*it2)->draw();
		}
	}
}

/*CSceneGL end*/
#endif USE_OPENGL