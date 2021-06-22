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

const char SHADER_DEFAULT[] = "phong";
const char SHADER_DIR[] = "./assets";

class CSimpleScene : public CSceneGL
{
private:
    shared_ptr<CTexture2DGL> m_outTexture;
public:
    CSimpleScene() : CSceneGL(SHADER_DEFAULT, SHADER_DIR)
    {
        initShaders();
        initTextures();
        initLights();
        initLayers();
        initObjects();
        setCamera();
    }

    void initShaders()
    {
        addShader("debug_attitude");
        addShader("debug_light");
        addShader("depthmap");
        addShader("shadow"); 
        addShader("draw_texture");
    }

    void initTextures()
    {
        // load images 
        int w, h, c;
        //stbi_set_flip_vertically_on_load(1);
        auto imgdata = stbi_load("./assets/misuzu.png", &w, &h, &c, 0);
        auto tex = shared_ptr<CTexture2DGL>(new CTexture2DGL(w, h));
        tex->texImage2D(imgdata);
        m_textures["misuzu"] = tex;
        delete[] imgdata;

        // init output texture buffer
        m_outTexture = shared_ptr<CTexture2DGL>(new CTexture2DGL(1280, 720));
        m_outTexture->texImage2D(NULL);
        m_outTexture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        m_outTexture->unbind(); 
    }

    void initLights()
    {
        // directional light
        Light light;
        light.position = { 0, 1.f, -2.f, 0.f };
        light.direction = { 0.f, -10.f, 10.f };
        light.diffuse = { 1.f, 1.f, 1.f };
        light.ambient = { 1.f, 1.f, 1.f };
        light.specular = { 1.f, 1.f, 1.f };
        light.attenuation = { 1.f ,0.f, 0.f };
        light.cutoff = glm::radians(0.f);
        light.outerCutoff = glm::radians(0.f);

        Light light12 = light;
        light12.position = { 0, 1.f, 2.f, 0.f };
        light12.direction = {0.f, 10.f, -10.f};
        
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
        pushLight(light12);
        //pushLight(light2);
        //pushLight(light3);
    }

    void initLayers()
    {
        // phong rendering 
        auto layer_phong = shared_ptr<CLayerPhongGL>(new CLayerPhongGL(*this));
        layer_phong->setOutFrameTexture(m_outTexture, GL_COLOR_ATTACHMENT0);
        
        // shadow rendering
        auto layer_shadow = shared_ptr<CLayerShadowGL>(new CLayerShadowGL(*this, getShaders()["depthmap"], getShaders()["shadow"], 1024, 1024));
        float orthoParams[6] = { -2.f, 2.f, -2.f, 2.f, 0.1f, 5.f };
        layer_shadow->setOrthoParams(orthoParams);
        layer_shadow->enableCullFront(true);
        layer_shadow->setShadowBias(0.005f, 0.05f);
        layer_shadow->setShadowBrightness(0.5f);
        
        // other component 
        auto layer_attitude = shared_ptr<CLayerHudAttitude>(new CLayerHudAttitude(*this, getShaders()["debug_attitude"]));
        auto layer_light = shared_ptr<CLayerLightGL>(new CLayerLightGL(*this, getShaders()["debug_light"]));
        auto layer_texture = shared_ptr<CLayerDrawTextureGL>(new CLayerDrawTextureGL(*this, getShaders()["draw_texture"], m_outTexture));
           
        // assemble layers 
        this->pushLayer(layer_phong);
        this->pushLayer(layer_shadow);
        this->pushLayer(layer_attitude); 
        this->pushLayer(layer_light);
        //this->pushLayer(layer_texture); 
    }

    void initObjects()
    {
        auto tex = m_textures["misuzu"];   
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
        _material2->ambient = glm::vec3(0.2f);
        _material2->diffuse = glm::vec3(0.8f);
        _material2->specular = glm::vec3(0.9f);
        _material2->shininess = 25.f;
        auto material2 = shared_ptr<void>(_material2);

        // plane
        glm::mat4 model;
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, -1.f, 0.f));
        model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0, 0));
        model = glm::scale(model, glm::vec3(5.f, 5.f, 5.f));
        auto planeMesh = shared_ptr<CPlaneMeshGL>(new CPlaneMeshGL());
        planeMesh->setMaterial(material2);
        planeMesh->setShader(m_shaders[SHADER_DEFAULT]);
        planeMesh->addTexture("misuzu", tex);
        auto planeObject = shared_ptr<CObject3DGL>(new CObject3DGL(model, planeMesh));
        m_objects.pushObject(planeObject); 

        // cube
        model = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f));
        //model = glm::rotate(model, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f)); // then pitch 
        model = glm::rotate(model, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f)); // yaw first
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));
        auto cubeMesh = shared_ptr<CCubeMeshGL>(new CCubeMeshGL());
        cubeMesh->setMaterial(material);
        cubeMesh->setShader(m_shaders[SHADER_DEFAULT]);
        cubeMesh->addTexture("misuzu", tex);
        auto cubeObject = shared_ptr<CObject3DGL>(new CObject3DGL(model, cubeMesh)); 
        m_objects.pushObject(cubeObject);

        // sphere
        model = glm::translate(glm::mat4(1), glm::vec3(-1.f, 0.f, 1.f));
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));
        auto sphereMesh = shared_ptr<CSphereMeshGL>(new CSphereMeshGL(glm::mat4(1), nullptr, GL_STATIC_DRAW, glm::radians(10.f), glm::radians(5.f)));
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
    string title = "gl shadow"; 
    CAppSDL app;
    app.prepareGL(0, 3, 2, SDL_GL_CONTEXT_PROFILE_ES); // intergrated graphic card might not be compatible with GLES
    //app.prepareGL(0, 3, 3, SDL_GL_CONTEXT_PROFILE_CORE); // linux, export MESA_GL_VERSION_OVERRIDE=3.3
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
    app.run();
    SDL_Log("%s exit", title.c_str());
}

int main(int argc, char* argv[])
{
    start();
#if(defined(_WIN32) && defined(_DEBUG)) 
    _CrtDumpMemoryLeaks(); // const string will not destruct before main end
#endif
    return 0;
}