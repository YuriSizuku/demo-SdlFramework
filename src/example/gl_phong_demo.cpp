#include <iostream>
#include <memory>
#include "sdl_framework.hpp"
#include "gl_scene.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
using std::shared_ptr;
using std::cout;
using std::endl;

#if defined(_MSC_VER ) && !defined(_DEBUG)
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

const char SHADER_DEFAULT[] = "phong";
const char SHADER_DIR[] = "./assets";

class CSimpleScene : public CSceneGL
{
public:
    CSimpleScene() : CSceneGL(SHADER_DEFAULT, SHADER_DIR)
    {
        initShaders();
        initLights();
        initLayers();
        initObjects();
        setCamera();
    }

    void initShaders()
    {
#ifdef _PSV
        addShader("debug_light");
#else
        addShader("debug_normal");
        addShader("debug_attitude");
        addShader("debug_light");
#endif
    }

    void initLights()
    {
        // directional light
        Light light;
        light.position = glm::vec4(0);
        light.direction = { 5.f, -3.f, 1.f };
        light.diffuse = { 1.f, 1.f, 1.f };
        light.ambient = { 1.f, 1.f, 1.f };
        light.specular = { 1.f, 1.f, 1.f };
        light.attenuation = { 1.f ,0.f, 0.f };
        light.cutoff = glm::radians(0.f); 
        light.outerCutoff = glm::radians(0.f);
        
        // point light
        Light light2 = light;
        light2.position = { -0.1f, 0.2f, 1.f, 1.f };
        light2.ambient *= 0.f;

        // spot light
        Light light3 = light;
        light3.position = { 1.f, 0.f, 0.f, 1.f };
        light3.direction = { -1.f, 0.f, 0.f };
        light3.cutoff = glm::radians(20.f);
        light3.outerCutoff = glm::radians(40.f);
        light2.ambient *= 0.f;

        pushLight(light);
        pushLight(light2);
        pushLight(light3);
    }

    void initLayers()
    {
        auto layer = shared_ptr<CLayerGL>(new CLayerPhongGL(*this));
        this->pushLayer(layer);
#ifdef _PSV
#else
        auto layer_normal = shared_ptr<CLayerGL>(new CLayerGL(*this, getShaders()["debug_normal"]));
        // this->pushLayer(layer_normal);
        auto layer_attitude = shared_ptr<CLayerGL>(new CLayerHudAttitude(*this, getShaders()["debug_attitude"]));
        this->pushLayer(layer_attitude);
#endif
        auto layer_light = shared_ptr<CLayerGL>(new CLayerLightGL(*this, getShaders()["debug_light"]));
        this->pushLayer(layer_light); 
    }

    void initObjects()
    {
        // load images
        int w, h, c;
        //stbi_set_flip_vertically_on_load(1);
        auto imgdata = stbi_load("./assets/misuzu.png", &w, &h, &c, 0);
        auto tex = shared_ptr<CTexture2DGL>(new CTexture2DGL(w, h));
        tex->texImage2D(imgdata);
        m_textures["misuzu"] = tex;
        delete[] imgdata;
        
        // prepare material
        auto _material = new MaterialPhong;
        _material->ambient = { 0.2f, 0.2f, 0.2f };
        _material->diffuse = { 0.8f, 0.8f, 0.8f };
        _material->specular = { 0.8f, 0.8f, 0.8f };
        _material->shininess = 25.f;
        _material->alpha = 1.f;
        auto material = shared_ptr<void>(_material);
        auto _material2 = new MaterialPhong;
        memcpy(_material2, _material, sizeof(*_material));
        _material2->ambient = glm::vec3(0.3f);
        _material2->diffuse = glm::vec3(0.6f);
        _material2->specular = glm::vec3(0.2f);
        auto material2 = shared_ptr<void>(_material2);

        // plane
        glm::mat4 model;
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, -1.f, 0.f));
        model = glm::rotate(model, glm::radians(-60.f), glm::vec3(1.f, 0, 0));
        model = glm::scale(model, glm::vec3(2.f, 2.f, 2.f));
        auto planeMesh = shared_ptr<CPlaneMeshGL>(new CPlaneMeshGL());
        planeMesh->setMaterial(material2);
        planeMesh->setShader(m_shaders[SHADER_DEFAULT]);
        planeMesh->addTexture("misuzu", tex);
        auto planeObject = shared_ptr<CObject3DGL>(new CObject3DGL(model, planeMesh));
        m_objects.pushObject(planeObject);

        // cube 
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f)); // then pitch
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f)); // yaw first
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));
        auto cubeMesh = shared_ptr<CCubeMeshGL>(new CCubeMeshGL());
        cubeMesh->setMaterial(material);
        cubeMesh->setShader(m_shaders[SHADER_DEFAULT]);
        cubeMesh->addTexture("misuzu", tex);
        auto cubeObject = shared_ptr<CObject3DGL>(new CObject3DGL(model, cubeMesh));
        m_objects.pushObject(cubeObject);

        // sphere
        model = glm::translate(glm::mat4(1), glm::vec3(-1.f, 1.f, 0.f));
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));
        auto sphereMesh = shared_ptr<CSphereMeshGL>(new CSphereMeshGL());
        sphereMesh->setMaterial(material2);
        sphereMesh->setShader(m_shaders[SHADER_DEFAULT]);
        sphereMesh->addTexture("misuzu", tex);
        auto sphereObject = shared_ptr<CObject3DGL>(new CObject3DGL(model, sphereMesh));
        m_objects.pushObject(sphereObject);
    }

    void handleEvent(void* event)  
    {
        bool ret = Explore3DEventSDL(static_cast<SDL_Event*>(event), m_camera,     
            0.1f, glm::radians(1.f), glm::radians(1.f));
        if (ret)
        {
            SDL_Log("position(%f %f %f), angle(p=%f y=%f r=%f)",  
                m_camera.position.x, m_camera.position.y, m_camera.position.z, 
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
    string title = "gl phong";
    CAppSDL app;
#ifdef _PSV
    app.prepareGL(0, 2, 0, SDL_GL_CONTEXT_PROFILE_ES);
    app.enableGL(true);
    app.prepareWindow(title, 960, 544);
#else
    //app.prepareGL(0, 3, 2, SDL_GL_CONTEXT_PROFILE_ES); // intergrated graphic card might not be compatible with GLES, also raspberry pi have some problem in this mode
    app.prepareGL(0, 3, 3, SDL_GL_CONTEXT_PROFILE_CORE);  //linux, export MESA_GL_VERSION_OVERRIDE=3.3
    app.enableGL(true);
    app.prepareWindow(title, 1280, 720);
#endif 

    auto stage_manager = shared_ptr<CStageManegerSDL>(new CStageManegerSDL(app));
    auto stage = shared_ptr<CStageSDL>(new CStageSDL(app));
    auto scene = shared_ptr<CSceneGL>(new CSimpleScene());
    stage->pushScene(scene);
    stage_manager->pushStage(stage);
    app.prepareStageManager(stage_manager);
    app.setBackground(0xff, 0xc0, 0xcb);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    //glDisable(GL_CULL_FACE);
    //app.setBackground(0xff, 0xff, 0xff);
    //app.setFps(288);
    app.run();
    SDL_Log("%s exit", title.c_str());
}

int main(int argc, char* argv[])
{
    start();
#if defined(_WIN32) && defined(_DEBUG) && !defined(__GNUC__)
    _CrtDumpMemoryLeaks(); // const string will not destruct before main end
#endif
    return 0;
}