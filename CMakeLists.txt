cmake_minimum_required(VERSION 3.16.3)
project(SdlFramework)
option(WIN64 "link the WIN64 lib" OFF)
option(MINGW_DIR "configure this to MINGW path to use" OFF) 

function(config_linux TARGET_NAME)
    message("Linux enviroment for "  ${TARGET_NAME})
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        USE_OPENGL
        _LINUX)
    # static, pkg-config --libs sdl2 glew --static --cflags
    # target_link_libraries(${TARGET_NAME} #${CMAKE_DL_LIBS} -lGLEW -lSDL2 -Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt -lGLU -lm -lGL) 
    target_link_libraries(${TARGET_NAME}  # dynamic
        SDL2 
        GLEW
        GLU 
        GL)
endfunction()

function(config_windows TARGET_NAME)
    if (MINGW) # if using mingw, automaticlly have this
        message("MINGW enviroment for " ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE 
            USE_OPENGL
            _LINUX)
        target_link_options(${TARGET_NAME} PRIVATE 
            -static-libgcc   # static link mingw env
            -static-libstdc++) 
        if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
            message("generating debug configuration")
            target_compile_options(${TARGET_NAME} PRIVATE -g)
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,console") 
        else()
            target_link_options(${TARGET_NAME} PRIVATE -mwindows) # -mwindows will not have console
        endif()
        target_link_libraries(${TARGET_NAME}  # static links
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive 
            -Wl,-Bstatic 
            mingw32 
            SDL2main 
            SDL2
            glew32
            opengl32)
        target_link_libraries(${TARGET_NAME}   # dynamic links
            -Wl,-Bdynamic
            dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 
            imm32 winmm gdi32 user32)
        find_file(SDL2_DLL_PATH NAMES SDL2.dll)
        message("use ${SDL2_DLL_PATH}")

    else() # msvc, clang-cl
        message("Windows enviroment for " ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
        target_include_directories(${TARGET_NAME} PRIVATE
            ${SDL2_DIR}/include
            ${GLM_DIR}/
            ${GLEW_DIR}/include)

        if(CMAKE_C_COMPILER MATCHES "clang") #can not use directory -DCMAKE_EXE_LINKER_FLAGS, as some example compile test fail
            message("using clang to build!")
            if(CMAKE_BUILD_TYPE MATCHES "Debug")
                target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem:console") 
            else()
                target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem:windows")
            endif()
    endif()

    if(CMAKE_GENERATOR_PLATFORM MATCHES "x64")
        set(WIN64 ON) 
    endif()
    if (WIN64)
        message("adding win64 libs to " ${TARGET_NAME})
        target_link_directories(${TARGET_NAME} PRIVATE
            ${SDL2_DIR}/lib/x64
            ${GLEW_DIR}/lib/Release/x64)
        set(SDL2_DLL_PATH ${SDL2_DIR}/lib/x64/SDL2.dll)
        message(${SDL2_DLL_PATH}, ${SDL2_DIR})

    else() # x86
        message("adding win32 libs to "  ${TARGET_NAME})
        target_link_directories(${TARGET_NAME} PRIVATE
            ${SDL2_DIR}/lib/x86
            ${GLEW_DIR}/lib/Release/win32)
        set(SDL2_DLL_PATH ${SDL2_DIR}/lib/x86/SDL2.dll)
    endif()
    target_link_libraries(${TARGET_NAME} 
        SDL2.lib
        SDL2main.lib
        glew32s.lib
        opengl32.lib)
    endif()
    
    install(FILES 
        ${SDL2_DLL_PATH}
        DESTINATION ${CMAKE_BINARY_DIR}/Release)
    install(FILES 
        ${SDL2_DLL_PATH}
        DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    install(FILES 
        ${SDL2_DLL_PATH}
        DESTINATION ${CMAKE_BINARY_DIR}/) # for clang with ninja 
endfunction()

function(config_psv TARGET_NAME)
    message("PSV enviroment for "  ${TARGET_NAME} 
        ", titleid=" ${VITA_TITLEID} 
        ", version=" ${VITA_VERSION})
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
    set(VITA_APP_NAME ${TARGET_NAME})
    if(NOT DEFINED VITA_TITLEID) 
        set(VITA_TITLEID  "DEVAPPTMP")
    endif()
    if(NOT DEFINED VITA_VERSION)
        set(VITA_VERSION  "01.00")
    endif()
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        _LINUX
        _PSV
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        $ENV{VITASDK}/share/gcc-arm-vita-eabi/samples/common
    )

    set(SDL_LIBS
        stdc++
        SDL2_ttf
        freetype
        png
        z
        c
        SDL2 # must have these functions under SDL2
        SceGxm_stub
        SceDisplay_stub
        SceCtrl_stub
        SceAppMgr_stub
        SceAppUtil_stub
        SceAudio_stub
        SceAudioIn_stub
        SceSysmodule_stub
        SceIofilemgr_stub
        SceCommonDialog_stub
        SceTouch_stub
        SceHid_stub
        SceMotion_stub
        ScePower_stub
        SceProcessmgr_stub
        SceMotion_stub
        m
    )
    set(GL_LIBS
        vitaGL
        SceIme_stub 
        SceGxm_stub
        vitashark
        mathneon
        SceShaccCg_stub
        SceShaccCgExt
        SceKernelDmacMgr_stub
        taihen_stub
    )
    if(${TARGET_NAME} MATCHES gl_)
        target_compile_definitions(${TARGET_NAME} PRIVATE 
            USE_OPENGL
        )
    endif()
    target_link_libraries(${TARGET_NAME}
        ${SDL_LIBS}
        ${GL_LIBS}
    )
    vita_create_self(${TARGET_NAME}.self ${TARGET_NAME})
    vita_create_vpk(${TARGET_NAME}.vpk ${VITA_TITLEID} ${TARGET_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE src/shader/phong.frag assets/phong.frag
        FILE src/shader/phong.vert assets/phong.vert
        FILE src/shader/debug_light.frag assets/debug_light.frag
        FILE src/shader/debug_light.vert assets/debug_light.vert
        FILE assets/misuzu.png assets/misuzu.png
        FILE assets/psv_icon.png sce_sys/icon0.png
        FILE assets/psv_bg.png sce_sys/livearea/contents/bg.png
        FILE assets/psv_startup.png sce_sys/livearea/contents/startup.png
        FILE assets/psv_template.xml sce_sys/livearea/contents/template.xml
        FILE assets/default.ttf default.ttf
    )    
endfunction()

function(config_platform TARGET_NAME)
    # extern lib path
    set(EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/externlib)
    set(SDL2_DIR ${EXTERN_LIB_DIR}/SDL2-2.0.14)
    set(GLM_DIR  ${EXTERN_LIB_DIR}/glm-0.9.9)
    set(GLEW_DIR ${EXTERN_LIB_DIR}/glew-2.1.0)
    set(STB_DIR ${EXTERN_LIB_DIR}/stb)
    
    # common extern include path
    target_include_directories(${TARGET_NAME} PRIVATE ${STB_DIR})
    
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        config_linux(${TARGET_NAME})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
        config_windows(${TARGET_NAME})
    elseif(CMAKE_TOOLCHAIN_FILE MATCHES "vita")
        config_psv(${TARGET_NAME})
    endif()
endfunction()

function(copy_assets TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E 
           copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/assets
       COMMAND ${CMAKE_COMMAND} -E 
           copy_directory  ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
       COMMAND ${CMAKE_COMMAND} -E
           copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/Release/assets
       COMMAND ${CMAKE_COMMAND} -E 
           copy_directory  ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/Release/assets
       COMMAND ${CMAKE_COMMAND} -E 
           copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/Debug/assets
       COMMAND ${CMAKE_COMMAND} -E 
           copy_directory  ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/Debug/assets)
endfunction()

message("This is a simple sdl framework demo")
set(CMAKE_CXX_STANDARD 11)
set(SDL_FRAMEWORK_CODE
    src/sdl_util.h
    src/sdl_util.c
    src/sdl_framework.hpp
    src/sdl_framework.cpp
    src/sdl_object2d.hpp
    src/sdl_object2d.cpp
    src/data_types.hpp
    src/physics_object.hpp
    src/physics_object.cpp)
set(GL_FRAMEWORK_CODE
    src/gl_util.h
    src/gl_util.c
    src/gl_scene.hpp
    src/gl_scene.cpp
    src/gl_layers.hpp
    src/gl_layers.cpp
    src/gl_assets.hpp
    src/gl_assets.cpp
    src/gl_object3d.hpp
    src/gl_object3d.cpp)

# simple circle
set(DEMO_NAME circle_one)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_platform(${DEMO_NAME})

# circle collision
set(DEMO_NAME circle_collision)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_platform(${DEMO_NAME})

# danmaku
set(DEMO_NAME circle_danmaku)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
set(VITA_TITLEID "DEVAPP001")
set(VITA_VERSION "01.00")
config_platform(${DEMO_NAME})

# gl pbong test
set(DEMO_NAME gl_phong_demo)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
set(VITA_TITLEID "DEVAPP002")
set(VITA_VERSION "01.00")
config_platform(${DEMO_NAME})
copy_assets(${DEMO_NAME})

# gl shadow test
set(DEMO_NAME gl_shadow_demo)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
set(VITA_TITLEID "DEVAPP003")
set(VITA_VERSION "01.00")
config_platform(${DEMO_NAME})
copy_assets(${DEMO_NAME})