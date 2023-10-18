# General settings:
cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0074 NEW)

project(wva DESCRIPTION "wx vtk atnimation")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(wxWidgets COMPONENTS core base propgrid REQUIRED CONFIG)

if(UNIX)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
endif()

find_package(Gettext)

find_package(
             VTK REQUIRED COMPONENTS
             CommonColor
             CommonCore
             CommonDataModel
             FiltersSources
             InteractionStyle
             RenderingContextOpenGL2
             RenderingCore
             RenderingFreeType
             RenderingGL2PSOpenGL2
             RenderingOpenGL2
            )

add_executable(wva WIN32
    src/App.cpp
)
target_include_directories(wva PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(wva
    ${wxWidgets_LIBRARIES}
    ${VTK_LIBRARIES}
    ${GTK3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    OpenGL::GLU
    Threads::Threads
)

