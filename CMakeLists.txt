cmake_minimum_required(VERSION 3.5)
project(SdlFramework)

message("This is a simple sdl framework demo")
set(DEMO_NAME demo)
add_executable(${DEMO_NAME}
    src/demo.cpp
    src/sdl_util.h
    src/sdl_util.c
    src/sdl_framework.hpp
    src/sdl_framework.cpp
    src/gl_object.hpp
    src/gl_object.cpp
)

# set extern lib path
set(EXTERN_LIB_DIR ${PROJECT_SOURCE_DIR}/externlib)
set(SDL2_DIR ${EXTERN_LIB_DIR}/SDL-2.0.15-14858)
set(GLM_DIR  ${EXTERN_LIB_DIR}/glm-0.9.9)
set(GLEW_DIR ${EXTERN_LIB_DIR}/glew-2.2.0)

# extern include
target_include_directories(${DEMO_NAME} PRIVATE
    ${SDL2_DIR}/include
    ${GLM_DIR}/glm
    ${GLEW_DIR}/include
)

# extern lib
if (WIN32)
    target_link_libraries(${DEMO_NAME} 
    ${SDL2_DIR}/VisualC/Win32/Release/SDL2.lib
    ${SDL2_DIR}/VisualC/Win32/Release/SDL2main.lib
    )
    
    find_library(GLEW_LIBRARY glew32s.lib ${GLEW_DIR}/lib/Release/Win32)
    if(GLEW_LIBRARY)
        message(${GLEW_LIBRARY}" found!")
        target_link_libraries(${DEMO_NAME} ${GLEW_LIBRARY})
    endif() 

    # install librarys
    install(FILES 
        ${SDL2_DIR}/VisualC/Win32/Release/SDL2.dll 
        DESTINATION ${CMAKE_BINARY_DIR}/Release)
    install(FILES 
        ${SDL2_DIR}/VisualC/Win32/Release/SDL2.dll 
        DESTINATION ${CMAKE_BINARY_DIR}/Debug)
endif()