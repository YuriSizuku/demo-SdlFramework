#include<memory>
#include"sdl_framework.hpp"
#include "gl_object3d.hpp"
using std::shared_ptr;

class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene(): CSceneGL("default")
    {
        shared_ptr<CPlaneGL> plane = shared_ptr<CPlaneGL>(new CPlaneGL(glm::mat4(1)));
        plane->setpShader(m_pShaders["default"]); 
        m_pObjects.pushObject(plane); 
    }
    virtual ~CSimpleScene()  
    {
    
    }
};

int main(int argc, char* argv[])
{
    CAppSDL app;
    CStageManegerSDL stage_manager(app);
    app.prepareWindow("gl test", 1280, 720);
    app.prepareStageManager(&stage_manager);
    app.prepareGL();
    glViewport(0, 0, 1280, 720);
    shared_ptr<CSceneGL> scene = shared_ptr<CSceneGL>(new CSimpleScene());
    shared_ptr<CStageSDL> stage = shared_ptr<CStageSDL>(new CStageSDL(app)); 
    stage->pushScene(scene);
    stage_manager.pushStage(stage);
    app.setFps(144);
    app.setBackground(0xff, 0xc0, 0xcb);
    app.enableGl(true);
    app.run();
    return 0;
}