cmake_minimum_required(VERSION 3.5)
project(SdlFramework)

function(config_externlib TARGET_NAME)
    # set extern lib path
    set(EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/externlib)
    set(SDL2_DIR ${EXTERN_LIB_DIR}/SDL-2.0.15-14858)
    set(GLM_DIR  ${EXTERN_LIB_DIR}/glm-0.9.9)
    set(GLEW_DIR ${EXTERN_LIB_DIR}/glew-2.2.0)

    # extern include
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
    target_include_directories(${TARGET_NAME} PRIVATE
        ${SDL2_DIR}/include
        ${GLM_DIR}/glm
        ${GLEW_DIR}/include
    )

    # extern lib
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        if (${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
            message("adding win64 libs...")
            target_link_libraries(${TARGET_NAME} 
            ${SDL2_DIR}/VisualC/x64/Release/SDL2.lib
            ${SDL2_DIR}/VisualC/x64/Release/SDL2main.lib
            opengl32.lib
            )
            find_library(GLEW_LIBRARY glew32s.lib ${GLEW_DIR}/lib/Release/x64)
            if(GLEW_LIBRARY)
                message(${GLEW_LIBRARY}" found!")
                target_link_libraries(${TARGET_NAME} ${GLEW_LIBRARY})
            endif() 

            # install librarys
            install(FILES 
                ${SDL2_DIR}/VisualC/x64/Release/SDL2.dll 
                DESTINATION ${CMAKE_BINARY_DIR}/Release)
            install(FILES 
                ${SDL2_DIR}/VisualC/x64/Release/SDL2.dll 
                DESTINATION ${CMAKE_BINARY_DIR}/Debug)
        
        else() # x86
            message("adding win32 libs...")
            target_link_libraries(${TARGET_NAME} 
            ${SDL2_DIR}/VisualC/Win32/Release/SDL2.lib
            ${SDL2_DIR}/VisualC/Win32/Release/SDL2main.lib
            opengl32.lib
            )
            find_library(GLEW_LIBRARY glew32s.lib ${GLEW_DIR}/lib/Release/Win32)
            if(GLEW_LIBRARY)
                message(${GLEW_LIBRARY}" found!")
                target_link_libraries(${TARGET_NAME} ${GLEW_LIBRARY})
            endif() 

            # install librarys
            install(FILES 
                ${SDL2_DIR}/VisualC/Win32/Release/SDL2.dll 
                DESTINATION ${CMAKE_BINARY_DIR}/Release)
            install(FILES 
                ${SDL2_DIR}/VisualC/Win32/Release/SDL2.dll 
                DESTINATION ${CMAKE_BINARY_DIR}/Debug)
        endif()
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
    src/sdl_object2d.cpp)
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