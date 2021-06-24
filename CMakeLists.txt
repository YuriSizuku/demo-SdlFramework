cmake_minimum_required(VERSION 3.19.5)
project(SdlFramework)
option(WIN64 "link the WIN64 lib" OFF)
option(MINGW_DIR "configure this to MINGW path to use" OFF) 

function(config_externlib TARGET_NAME)
    # extern lib path
    set(EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/externlib)
    set(SDL2_DIR ${EXTERN_LIB_DIR}/SDL2-2.0.14)
    set(GLM_DIR  ${EXTERN_LIB_DIR}/glm-0.9.9)
    set(GLEW_DIR ${EXTERN_LIB_DIR}/glew-2.1.0)
    set(STB_DIR ${EXTERN_LIB_DIR}/stb)
    # common extern include path
    target_include_directories(${TARGET_NAME} PRIVATE ${STB_DIR})
    
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        message("Linux enviroment for "  ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
        target_compile_definitions(${TARGET_NAME} PRIVATE _LINUX)
        target_link_libraries(${TARGET_NAME} 
            SDL2
            GLEW
            GL
            GLESv2
            ${CMAKE_DL_LIBS})
    
    elseif (MINGW) # if using mingw, automaticlly have this
        message("MINGW enviroment for " ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE 
            USE_OPENGL
            _LINUX)
        target_link_options(${TARGET_NAME} PRIVATE 
            -static-libgcc   # static link mingw env
            -static-libstdc++
            -mwindows) 
        target_link_libraries(${TARGET_NAME}  # static links
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive 
            -Wl,-Bstatic 
            mingw32 
            SDL2main 
            SDL2
            glew32)
        target_link_libraries(${TARGET_NAME}   # dynamic links
            -Wl,-Bdynamic
            dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 imm32 winmm gdi32 user32
            opengl32)
 
    elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
        message("Windows enviroment for " ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
        target_include_directories(${TARGET_NAME} PRIVATE
            ${SDL2_DIR}/include
            ${GLM_DIR}/
            ${GLEW_DIR}/include)

        if(CMAKE_C_COMPILER MATCHES "clang") #can not use directory -DCMAKE_EXE_LINKER_FLAGS, as some example compile test fail
            message("using clang for compiler!")
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem:windows") 
        endif()
        if(CMAKE_GENERATOR_PLATFORM MATCHES "x64")
            set(WIN64 ON) 
        endif()
        if (WIN64)
            message("adding win64 libs to " ${TARGET_NAME})
            target_link_directories(${TARGET_NAME} PRIVATE
                ${SDL2_DIR}/lib/x64
                ${GLEW_DIR}/lib/Release/x64)
            find_file(SDL2_DLL_PATH 
                SDL2.dll 
                ${SDL2_DIR}/lib/x64/)
        
        else() # x86
            message("adding win32 libs to "  ${TARGET_NAME})
            target_link_directories(${TARGET_NAME} PRIVATE
                ${SDL2_DIR}/lib/x86
                ${GLEW_DIR}/lib/Release/win32)
            find_file(SDL2_DLL_PATH 
                SDL2.dll 
                ${SDL2_DIR}/lib/x86)
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
config_externlib(${DEMO_NAME})

# circle collision
set(DEMO_NAME circle_collision)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})

# danmaku
set(DEMO_NAME circle_danmaku)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})

# gl pbong test
set(DEMO_NAME gl_phong_demo)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})
copy_assets(${DEMO_NAME})

# gl shadow test
set(DEMO_NAME gl_shadow_demo)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})
copy_assets(${DEMO_NAME})