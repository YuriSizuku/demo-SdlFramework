#include<memory>
#include"sdl_framework.hpp"
#include "gl_object3d.hpp"
using std::shared_ptr;

const string SHADER_DEFAULT = "default";
const string SHADER_NORMAL = "normal_line";
const string SHADER_DIR = "./assets";


class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene(): CSceneGL(SHADER_DEFAULT, SHADER_DIR)
    {
        addShader(SHADER_NORMAL, SHADER_DIR);
        setProject(glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f), SHADER_DEFAULT);
        setView(glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, -3.f)), SHADER_DEFAULT); // stand at (0,0,3f)
        setProject(SHADER_NORMAL);
        setView(SHADER_NORMAL);
        glm::mat4 model;
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
        
        // a plane
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, -1.f, 0.f));
        model = glm::scale(model, glm::vec3(2.f, 2.f, 2.f));
        model = glm::rotate(model, glm::radians(-60.f), glm::vec3(1.f, 0, 0));
        auto plane = shared_ptr<CPlaneGL>(new CPlaneGL());
        plane->setShader(m_shaders[SHADER_DEFAULT]); 
        plane->setModel(model);   
        m_objects.pushObject(plane);  

        // a cube 
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f)); 
        model = glm::scale(model, glm::vec3(1.0f, 0.5f, 0.5f));
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f)); 
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f));
        auto cube = shared_ptr<CCubeGL>(new CCubeGL());
        cube->setShader(m_shaders[SHADER_DEFAULT]); 
        cube->setModel(model);
        m_objects.pushObject(cube);      

        // cube normal geomotry demo
        auto cube_gemo = shared_ptr<CCubeGL>(new CCubeGL());
        cube_gemo->setShader(m_shaders[SHADER_NORMAL]); 
        cube_gemo->setModel(model); 
        m_objects.pushObject(cube_gemo);   
    }
    virtual ~CSimpleScene()
    {
      
    }
};

int main(int argc, char* argv[])
{
    CAppSDL app;
    app.prepareGL();
    app.prepareWindow("gl test", 1280, 720);
    
    auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
    auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
    auto scene = shared_ptr<CSceneGL>(new CSimpleScene());
    auto layer = shared_ptr<CLayerGL>(new CLayerGL(*scene));
    scene->pushLayer(layer);
    stage->pushScene(scene);
    stage_manager->pushStage(stage);
    app.prepareStageManager(stage_manager);
    app.setBackground(0xff, 0xc0, 0xcb);
    app.setBackground(0xff, 0xff, 0xff);
    app.run();
    return 0;
}