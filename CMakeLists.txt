cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "tp2-asteroids")
project(${PROJECT_NAME})

# Try to find packages
find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(OpenGL REQUIRED)

# Try to find GLEW, but don't fail if not found - we'll handle it manually
find_package(GLEW QUIET)

# If GLEW not found, try to find it manually or use vcpkg
if(NOT GLEW_FOUND)
    # Try vcpkg path
    if(EXISTS "C:/vcpkg/installed/x64-windows/include/GL/glew.h")
        set(GLEW_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows/include")
        set(GLEW_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/glew32.lib")
        message(STATUS "Found GLEW in vcpkg")
    elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/include/GL/glew.h")
        set(GLEW_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows-static/include")
        set(GLEW_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/glew32.lib")
        message(STATUS "Found GLEW in vcpkg (static)")
    else()
        # Try common installation paths
        if(EXISTS "C:/Program Files (x86)/GLEW/include/GL/glew.h")
            set(GLEW_INCLUDE_DIRS "C:/Program Files (x86)/GLEW/include")
            set(GLEW_LIBRARIES "C:/Program Files (x86)/GLEW/lib/Release/x64/glew32.lib")
        elseif(EXISTS "C:/Program Files/GLEW/include/GL/glew.h")
            set(GLEW_INCLUDE_DIRS "C:/Program Files/GLEW/include")
            set(GLEW_LIBRARIES "C:/Program Files/GLEW/lib/Release/x64/glew32.lib")
        else()
            message(FATAL_ERROR "Could not find GLEW. Please install GLEW or set GLEW_INCLUDE_DIRS and GLEW_LIBRARIES manually.")
        endif()
    endif()
endif()

add_executable(${PROJECT_NAME}
        Source/Renderer/Shader.cpp
        Source/Renderer/Shader.h
        Source/Main.cpp
        Source/Math.cpp
        Source/Random.cpp
        Source/Actors/Actor.cpp
        Source/Actors/Actor.h
        Source/Components/Component.cpp
        Source/Components/Component.h
        Source/Game.cpp
        Source/Game.h
        Source/Components/DrawComponent.cpp
        Source/Components/DrawComponent.h
        Source/Components/RigidBodyComponent.cpp
        Source/Components/RigidBodyComponent.h
        Source/Actors/Player.cpp
        Source/Actors/Player.h
        Source/Actors/Enemy.cpp
        Source/Actors/Enemy.h
        Source/Actors/Projectile.cpp
        Source/Actors/Projectile.h
        Source/Components/CircleColliderComponent.cpp
        Source/Components/CircleColliderComponent.h
        Source/Renderer/Renderer.cpp
        Source/Renderer/Renderer.h
        Source/Renderer/VertexArray.h
        Source/Renderer/VertexArray.cpp
        Source/Renderer/TextRenderer.cpp
        Source/Renderer/TextRenderer.h
        Source/Components/ParticleSystemComponent.cpp
        Source/Components/ParticleSystemComponent.h
        Source/Menus/Menu.cpp
        Source/Menus/Menu.h
        Source/Menus/MainMenu.cpp
        Source/Menus/MainMenu.h
        Source/Menus/PauseMenu.cpp
        Source/Menus/PauseMenu.h
        Source/Menus/UpgradeMenu.cpp
        Source/Menus/UpgradeMenu.h
)

# Link libraries
if(GLEW_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            GLEW::GLEW
            SDL2::SDL2main
            SDL2::SDL2
            opengl32
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${GLEW_LIBRARIES}
            SDL2::SDL2main
            SDL2::SDL2
            opengl32
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${GLEW_INCLUDE_DIRS}
    )
endif()

target_include_directories(tp2-asteroids PRIVATE
        "C:/vcpkg/installed/x64-windows/include"
        "C:/Program Files/SDL2/include"
)