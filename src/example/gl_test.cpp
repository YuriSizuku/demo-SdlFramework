#include<memory>
#include"sdl_framework.hpp"
#include "gl_object3d.hpp"
using std::shared_ptr;

const string SHADER_NAME = "default";
const string SHADER_DIR = "./assets";

class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene(): CSceneGL(SHADER_NAME, SHADER_DIR)
    {
        setProject(glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f), SHADER_NAME);
        setView(glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, -3.f)), SHADER_NAME);
        glm::mat4 model;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        // a plane
        model = glm::scale(glm::mat4(1), glm::vec3(2.f, 2.f, 2.f));
        model = glm::rotate(model, glm::radians(-60.f), glm::vec3(1.f, 0, 0));
        auto plane = shared_ptr<CPlaneGL>(new CPlaneGL());
        plane->setpShader(m_shaders[SHADER_NAME]);
        plane->setModel(model);
        //m_objects.pushObject(plane); 

        // a cube
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.3f, 0.f));
        model = glm::rotate(model, glm::radians(45.f), glm::vec3(1.f, 1.f, 0.f));
        auto cube = shared_ptr<CCubeGL>(new CCubeGL());
        cube->setpShader(m_shaders[SHADER_NAME]);
        cube->setModel(model);
        m_objects.pushObject(cube);
    }
    virtual ~CSimpleScene()  
    {
      
    }
};

int main(int argc, char* argv[])
{
    CAppSDL app;
    app.prepareWindow("gl test", 1280, 720);
    app.prepareGL();
    glViewport(0, 0, 1280, 720);
    
    auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
    auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
    auto scene = shared_ptr<CSceneGL>(new CSimpleScene());
    stage->pushScene(scene);
    stage_manager->pushStage(stage);
    app.prepareStageManager(stage_manager);
    app.setFps(144);
    app.setBackground(0xff, 0xc0, 0xcb);
    app.enableGl(true);
    app.run();
    return 0;
}