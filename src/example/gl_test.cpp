#include<memory>
#include"sdl_framework.hpp"
#include "gl_object3d.hpp"
using std::shared_ptr;

const char SHADER_DEFAULT[] = "default";
const char SHADER_DIR[] = "./assets";

class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene(): CSceneGL(SHADER_DEFAULT, SHADER_DIR)
    {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
         //prepare shaders and layers 
        addShader("debug_normal");
        addShader("debug_attitude");
        setCamera();
        auto layer = shared_ptr<CLayerGL>(new CLayerGL(*this));
        auto layer_normal = shared_ptr<CLayerGL>(new CLayerGL(*this, getShaders()["debug_normal"]));
        auto layer_attitude = shared_ptr<CLayerGL>(new CLayerHudAttitude(*this, getShaders()["debug_attitude"]));
        this->pushLayer(layer); 
        this->pushLayer(layer_normal);
        this->pushLayer(layer_attitude);

        // a plane
        glm::mat4 model;
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, -1.f, 0.f));
        model = glm::rotate(model, glm::radians(-60.f), glm::vec3(1.f, 0, 0)) ;
        model = glm::scale(model, glm::vec3(2.f, 2.f, 2.f));
        auto plane = shared_ptr<CPlaneGL>(new CPlaneGL());
        plane->setShader(m_shaders[SHADER_DEFAULT]); 
        plane->setModel(model);   
        m_objects.pushObject(plane);  

        // a cube 
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f)); // then pitch
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f)); // yaw first
        model = glm::scale(model, glm::vec3(1.0f, 0.5f, 0.5f));
        auto cube = shared_ptr<CCubeGL>(new CCubeGL());
        cube->setShader(m_shaders[SHADER_DEFAULT]); 
        cube->setModel(model);
        m_objects.pushObject(cube);

        GLint viewPort[4];
        glGetIntegerv(GL_VIEWPORT, viewPort);
    }

    void handleEvent(void* event)  
    {
        bool ret = Explore3DEventSDL(static_cast<SDL_Event*>(event), m_camera,     
            0.1f, glm::radians(1.f), glm::radians(1.f));
        if (ret)
        {
            SDL_Log("pos(%f %f %f), angle(p=%f y=%f r=%f)",  
                m_camera.pos.x, m_camera.pos.y, m_camera.pos.z, 
                glm::degrees(m_camera.angle.p), glm::degrees(m_camera.angle.y), glm::degrees(m_camera.angle.r)); 
        }
        if (static_cast<SDL_Event*>(event)->key.keysym.scancode == SDL_SCANCODE_R)
        {
            Camera camera;
            m_camera = camera;
        }
    }

    void render()
    {
       setCamera();
       CSceneGL::render();
    }
};

void start()
{
    string title = "gl test";
    CAppSDL app;
    app.prepareGL();
    app.enableGL(true);
    app.prepareWindow(title, 1280, 720);

    auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
    auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
    auto scene = shared_ptr<CSceneGL>(new CSimpleScene());
    stage->pushScene(scene);
    stage_manager->pushStage(stage);
    app.prepareStageManager(stage_manager);
    app.setBackground(0xff, 0xc0, 0xcb);
    //app.setBackground(0xff, 0xff, 0xff);
    //app.setFps(288);
    app.run();
    SDL_Log("%s exit", title.c_str());
}

int main(int argc, char* argv[])
{
    start();
#if(defined(_WIN32) || defined(_DEBUG))
    _CrtDumpMemoryLeaks(); // const string will not destruct before main end
#endif
    return 0;
}