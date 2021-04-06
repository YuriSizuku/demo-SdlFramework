#include<memory>
#include"sdl_framework.hpp"
#include "gl_object3d.hpp"
using std::shared_ptr;

class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene()
    {

    }
    virtual ~CSimpleScene()
    {

    }
};

int main(int argc, char* argv[])
{
    CAppSDL app;
    CStageManegerSDL stage_manager(app);
    shared_ptr<CSceneGL> scene = shared_ptr<CSceneGL>(new CSimpleScene());
    shared_ptr<CStageSDL> stage = shared_ptr<CStageSDL>(new CStageSDL(app));
    app.prepareWindow("gl test", 1280, 720);
    app.prepareStageManager(&stage_manager);
    app.prepareGL();
    stage->pushScene(scene);
    stage_manager.pushStage(stage);
    app.setFps(144);
    app.setBackground(0xff, 0xc0, 0xcb);
    app.enableGl();
    app.run();
    return 0;
}