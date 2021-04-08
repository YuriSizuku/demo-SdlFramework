cmake_minimum_required(VERSION 3.5)
project(SdlFramework)

function(config_externlib TARGET_NAME)
    # set extern lib path
    set(EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/externlib)
    set(SDL2_DIR ${EXTERN_LIB_DIR}/SDL2-2.0.14)
    set(GLM_DIR  ${EXTERN_LIB_DIR}/glm-0.9.9)
    set(GLEW_DIR ${EXTERN_LIB_DIR}/glew-2.1.0)

    # extern include
    target_include_directories(${TARGET_NAME} PRIVATE
        ${SDL2_DIR}/include
        ${GLM_DIR}/
        ${GLEW_DIR}/include
    )

    # extern lib
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
        if (${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
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
        install(FILES 
            ${SDL2_DLL_PATH}
            DESTINATION ${CMAKE_BINARY_DIR}/Release)
        install(FILES 
            ${SDL2_DLL_PATH}
            DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # sudo aptitude install libgl1-mesa-dev
    # sudo aptitude install libglew-dev glew-utils
    # sudo aptitude install libsdl2-dev
    # sudo apt install libglm-dev
        message("adding linux libs to "  ${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PRIVATE _LINUX)
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
        target_link_libraries(${TARGET_NAME} 
            SDL2
            GLEW
            GL
            ${CMAKE_DL_LIBS})
    endif()
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
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})

# danmaku
set(DEMO_NAME circle_danmaku)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})

# gl test
set(DEMO_NAME gl_test)
add_executable(${DEMO_NAME}
    src/example/${DEMO_NAME}.cpp
    ${SDL_FRAMEWORK_CODE}
    ${GL_FRAMEWORK_CODE})
target_include_directories(${DEMO_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_externlib(${DEMO_NAME})
set(SHADER_PATHS 
    #src/shader/default.vert
    src/shader/default.frag)
add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E 
       copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/shader
   COMMAND ${CMAKE_COMMAND} -E
       copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/Release/shader
   COMMAND ${CMAKE_COMMAND} -E 
       copy_directory  ${CMAKE_SOURCE_DIR}/src/shader ${CMAKE_BINARY_DIR}/Debug/shader)