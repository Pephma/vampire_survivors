cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "vampire-survivors")
project(${PROJECT_NAME})

# Try to find packages
find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(OpenGL REQUIRED)

# Get SDL2 include directory to check for SDL2_mixer and SDL2_image in same location
get_target_property(SDL2_INCLUDE_DIR SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
if(SDL2_INCLUDE_DIR)
    list(GET SDL2_INCLUDE_DIR 0 SDL2_INCLUDE_DIR)
    get_filename_component(SDL2_BASE_DIR "${SDL2_INCLUDE_DIR}" DIRECTORY)
    message(STATUS "SDL2 include directory: ${SDL2_INCLUDE_DIR}")
    message(STATUS "SDL2 base directory: ${SDL2_BASE_DIR}")
endif()

# Try to find SDL2_mixer, but handle manually if not found
find_package(SDL2_mixer QUIET)
if(NOT SDL2_mixer_FOUND)
    # Try same location as SDL2
    if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_mixer.h")
        # Point to SDL2 subdirectory so #include <SDL_mixer.h> works
        set(SDL2_mixer_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")
        # Try to find library in same location
        if(EXISTS "${SDL2_BASE_DIR}/lib/SDL2_mixer.lib")
            set(SDL2_mixer_LIBRARIES "${SDL2_BASE_DIR}/lib/SDL2_mixer.lib")
        elseif(EXISTS "${SDL2_BASE_DIR}/../lib/SDL2_mixer.lib")
            get_filename_component(SDL2_LIB_DIR "${SDL2_BASE_DIR}/../lib" ABSOLUTE)
            set(SDL2_mixer_LIBRARIES "${SDL2_LIB_DIR}/SDL2_mixer.lib")
        else()
            # Try to find in vcpkg lib directory
            if(EXISTS "C:/vcpkg/installed/x64-windows/lib/SDL2_mixer.lib")
                set(SDL2_mixer_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_mixer.lib")
            elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/lib/SDL2_mixer.lib")
                set(SDL2_mixer_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_mixer.lib")
            endif()
        endif()
        if(SDL2_mixer_LIBRARIES)
            message(STATUS "Found SDL2_mixer in same location as SDL2")
            set(SDL2_mixer_FOUND TRUE)
        endif()
    endif()
    
    # Try vcpkg paths if not found yet
    if(NOT SDL2_mixer_FOUND)
        if(EXISTS "C:/vcpkg/installed/x64-windows/include/SDL2/SDL_mixer.h")
            # Point to SDL2 subdirectory so #include <SDL_mixer.h> works
            set(SDL2_mixer_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows/include/SDL2")
            set(SDL2_mixer_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_mixer.lib")
            message(STATUS "Found SDL2_mixer in vcpkg")
            set(SDL2_mixer_FOUND TRUE)
        elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/include/SDL2/SDL_mixer.h")
            # Point to SDL2 subdirectory so #include <SDL_mixer.h> works
            set(SDL2_mixer_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows-static/include/SDL2")
            set(SDL2_mixer_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_mixer.lib")
            message(STATUS "Found SDL2_mixer in vcpkg (static)")
            set(SDL2_mixer_FOUND TRUE)
        else()
            # Try common installation paths
            if(EXISTS "C:/SDL2_mixer/include/SDL2/SDL_mixer.h")
                # Point to SDL2 subdirectory so #include <SDL_mixer.h> works
                set(SDL2_mixer_INCLUDE_DIRS "C:/SDL2_mixer/include/SDL2")
                set(SDL2_mixer_LIBRARIES "C:/SDL2_mixer/lib/x64/SDL2_mixer.lib")
                set(SDL2_mixer_FOUND TRUE)
            else()
                message(FATAL_ERROR "Could not find SDL2_mixer. Please install SDL2_mixer or set SDL2_mixer_INCLUDE_DIRS and SDL2_mixer_LIBRARIES manually.")
            endif()
        endif()
    endif()
endif()

# Try to find SDL2_ttf, but handle manually if not found
find_package(SDL2_ttf QUIET)
if(NOT SDL2_ttf_FOUND)
    # Try same location as SDL2
    if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_ttf.h")
        set(SDL2_ttf_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")
        if(EXISTS "${SDL2_BASE_DIR}/lib/SDL2_ttf.lib")
            set(SDL2_ttf_LIBRARIES "${SDL2_BASE_DIR}/lib/SDL2_ttf.lib")
        elseif(EXISTS "${SDL2_BASE_DIR}/../lib/SDL2_ttf.lib")
            get_filename_component(SDL2_LIB_DIR "${SDL2_BASE_DIR}/../lib" ABSOLUTE)
            set(SDL2_ttf_LIBRARIES "${SDL2_LIB_DIR}/SDL2_ttf.lib")
        else()
            if(EXISTS "C:/vcpkg/installed/x64-windows/lib/SDL2_ttf.lib")
                set(SDL2_ttf_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_ttf.lib")
            elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/lib/SDL2_ttf.lib")
                set(SDL2_ttf_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_ttf.lib")
            endif()
        endif()
        if(SDL2_ttf_LIBRARIES)
            message(STATUS "Found SDL2_ttf in same location as SDL2")
            set(SDL2_ttf_FOUND TRUE)
        endif()
    endif()
    
    # Try vcpkg paths if not found yet
    if(NOT SDL2_ttf_FOUND)
        if(EXISTS "C:/vcpkg/installed/x64-windows/include/SDL2/SDL_ttf.h")
            set(SDL2_ttf_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows/include/SDL2")
            set(SDL2_ttf_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_ttf.lib")
            message(STATUS "Found SDL2_ttf in vcpkg")
            set(SDL2_ttf_FOUND TRUE)
        elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/include/SDL2/SDL_ttf.h")
            set(SDL2_ttf_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows-static/include/SDL2")
            set(SDL2_ttf_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_ttf.lib")
            message(STATUS "Found SDL2_ttf in vcpkg (static)")
            set(SDL2_ttf_FOUND TRUE)
        else()
            if(EXISTS "C:/SDL2_ttf/include/SDL2/SDL_ttf.h")
                set(SDL2_ttf_INCLUDE_DIRS "C:/SDL2_ttf/include/SDL2")
                set(SDL2_ttf_LIBRARIES "C:/SDL2_ttf/lib/x64/SDL2_ttf.lib")
                set(SDL2_ttf_FOUND TRUE)
            else()
                message(WARNING "Could not find SDL2_ttf. Text rendering will use fallback method.")
            endif()
        endif()
    endif()
endif()

# Try to find SDL2_image, but handle manually if not found
find_package(SDL2_image QUIET)
if(NOT SDL2_image_FOUND)
    # Try same location as SDL2
    if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_image.h")
        # Point to SDL2 subdirectory so #include <SDL_image.h> works
        set(SDL2_image_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")
        # Try to find library in same location
        if(EXISTS "${SDL2_BASE_DIR}/lib/SDL2_image.lib")
            set(SDL2_image_LIBRARIES "${SDL2_BASE_DIR}/lib/SDL2_image.lib")
        elseif(EXISTS "${SDL2_BASE_DIR}/../lib/SDL2_image.lib")
            get_filename_component(SDL2_LIB_DIR "${SDL2_BASE_DIR}/../lib" ABSOLUTE)
            set(SDL2_image_LIBRARIES "${SDL2_LIB_DIR}/SDL2_image.lib")
        else()
            # Try to find in vcpkg lib directory
            if(EXISTS "C:/vcpkg/installed/x64-windows/lib/SDL2_image.lib")
                set(SDL2_image_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_image.lib")
            elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/lib/SDL2_image.lib")
                set(SDL2_image_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_image.lib")
            endif()
        endif()
        if(SDL2_image_LIBRARIES)
            message(STATUS "Found SDL2_image in same location as SDL2")
            set(SDL2_image_FOUND TRUE)
        endif()
    endif()
    
    # Try vcpkg paths if not found yet
    if(NOT SDL2_image_FOUND)
        if(EXISTS "C:/vcpkg/installed/x64-windows/include/SDL2/SDL_image.h")
            # Point to SDL2 subdirectory so #include <SDL_image.h> works
            set(SDL2_image_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows/include/SDL2")
            set(SDL2_image_LIBRARIES "C:/vcpkg/installed/x64-windows/lib/SDL2_image.lib")
            message(STATUS "Found SDL2_image in vcpkg")
            set(SDL2_image_FOUND TRUE)
        elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/include/SDL2/SDL_image.h")
            # Point to SDL2 subdirectory so #include <SDL_image.h> works
            set(SDL2_image_INCLUDE_DIRS "C:/vcpkg/installed/x64-windows-static/include/SDL2")
            set(SDL2_image_LIBRARIES "C:/vcpkg/installed/x64-windows-static/lib/SDL2_image.lib")
            message(STATUS "Found SDL2_image in vcpkg (static)")
            set(SDL2_image_FOUND TRUE)
        else()
            # Try common installation paths
            if(EXISTS "C:/SDL2_image/include/SDL2/SDL_image.h")
                # Point to SDL2 subdirectory so #include <SDL_image.h> works
                set(SDL2_image_INCLUDE_DIRS "C:/SDL2_image/include/SDL2")
                set(SDL2_image_LIBRARIES "C:/SDL2_image/lib/x64/SDL2_image.lib")
                set(SDL2_image_FOUND TRUE)
            else()
                message(FATAL_ERROR "Could not find SDL2_image. Please install SDL2_image or set SDL2_image_INCLUDE_DIRS and SDL2_image_LIBRARIES manually.")
            endif()
        endif()
    endif()
endif()

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
        Source/Renderer/Texture.h
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
        Source/Audio/AudioSystem.cpp
        Source/Audio/AudioSystem.h
        Source/Actors/Boss.cpp
        Source/Actors/Boss.h
        Source/Actors/DelayedExplosion.cpp
        Source/Actors/DelayedExplosion.h
        Source/Actors/ExperienceOrb.cpp
    Source/Actors/FloatingText.cpp
        Source/Actors/ExperienceOrb.h
        Source/Components/AnimatorComponent.cpp
        Source/Renderer/Texture.cpp
)

# Link libraries
if(GLEW_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            GLEW::GLEW
            SDL2::SDL2main
            SDL2::SDL2
            OpenGL::GL
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${GLEW_LIBRARIES}
            SDL2::SDL2main
            SDL2::SDL2
            OpenGL::GL
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${GLEW_INCLUDE_DIRS}
    )
endif()

# Link SDL2_mixer
if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_mixer::SDL2_mixer)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_mixer_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_mixer_INCLUDE_DIRS})
endif()

# Link SDL2_image
if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_image_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_image_INCLUDE_DIRS})
endif()

# Link SDL2_ttf
if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
elseif(SDL2_ttf_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_ttf_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_ttf_INCLUDE_DIRS})
endif()

# Copy required DLLs to build directory
if(WIN32)
    # Function to find and copy a DLL
    function(copy_dll_if_exists dll_name)
        # Try to find DLL in various locations
        set(DLL_FOUND FALSE)
        set(DLL_PATH "")
        
        # Try vcpkg bin directories
        if(EXISTS "C:/vcpkg/installed/x64-windows/bin/${dll_name}")
            set(DLL_PATH "C:/vcpkg/installed/x64-windows/bin/${dll_name}")
            set(DLL_FOUND TRUE)
        elseif(EXISTS "C:/vcpkg/installed/x64-windows-static/bin/${dll_name}")
            set(DLL_PATH "C:/vcpkg/installed/x64-windows-static/bin/${dll_name}")
            set(DLL_FOUND TRUE)
        else()
            # Try to find in same directory as library files
            if(SDL2_image_LIBRARIES)
                get_filename_component(LIB_DIR "${SDL2_image_LIBRARIES}" DIRECTORY)
                get_filename_component(BIN_DIR "${LIB_DIR}/../bin" ABSOLUTE)
                if(EXISTS "${BIN_DIR}/${dll_name}")
                    set(DLL_PATH "${BIN_DIR}/${dll_name}")
                    set(DLL_FOUND TRUE)
                endif()
            endif()
            
            # Try SDL2_mixer library directory
            if(NOT DLL_FOUND AND SDL2_mixer_LIBRARIES)
                get_filename_component(LIB_DIR "${SDL2_mixer_LIBRARIES}" DIRECTORY)
                get_filename_component(BIN_DIR "${LIB_DIR}/../bin" ABSOLUTE)
                if(EXISTS "${BIN_DIR}/${dll_name}")
                    set(DLL_PATH "${BIN_DIR}/${dll_name}")
                    set(DLL_FOUND TRUE)
                endif()
            endif()
            
            # Try SDL2 location - get DLL directory from target
            if(NOT DLL_FOUND AND TARGET SDL2::SDL2)
                # Try to get DLL location from SDL2 target
                get_target_property(SDL2_DLL_LOCATION SDL2::SDL2 IMPORTED_LOCATION)
                if(SDL2_DLL_LOCATION)
                    get_filename_component(SDL2_DLL_DIR "${SDL2_DLL_LOCATION}" DIRECTORY)
                    if(EXISTS "${SDL2_DLL_DIR}/${dll_name}")
                        set(DLL_PATH "${SDL2_DLL_DIR}/${dll_name}")
                        set(DLL_FOUND TRUE)
                    endif()
                endif()
                # Also try bin directory relative to library
                if(NOT DLL_FOUND)
                    get_target_property(SDL2_LIB_LOCATION SDL2::SDL2 IMPORTED_LOCATION_RELEASE)
                    if(NOT SDL2_LIB_LOCATION)
                        get_target_property(SDL2_LIB_LOCATION SDL2::SDL2 IMPORTED_LOCATION)
                    endif()
                    if(SDL2_LIB_LOCATION)
                        get_filename_component(SDL2_LIB_DIR "${SDL2_LIB_LOCATION}" DIRECTORY)
                        get_filename_component(BIN_DIR "${SDL2_LIB_DIR}/../bin" ABSOLUTE)
                        if(EXISTS "${BIN_DIR}/${dll_name}")
                            set(DLL_PATH "${BIN_DIR}/${dll_name}")
                            set(DLL_FOUND TRUE)
                        endif()
                    endif()
                endif()
            endif()
            
            # Try common SDL2 installation paths
            if(NOT DLL_FOUND)
                if(EXISTS "C:/SDL2/bin/${dll_name}")
                    set(DLL_PATH "C:/SDL2/bin/${dll_name}")
                    set(DLL_FOUND TRUE)
                elseif(EXISTS "C:/SDL2/lib/x64/${dll_name}")
                    set(DLL_PATH "C:/SDL2/lib/x64/${dll_name}")
                    set(DLL_FOUND TRUE)
                endif()
            endif()
            
            # Try SDL2_image specific paths
            if(NOT DLL_FOUND AND "${dll_name}" STREQUAL "SDL2_image.dll")
                if(EXISTS "C:/SDL2_image/bin/${dll_name}")
                    set(DLL_PATH "C:/SDL2_image/bin/${dll_name}")
                    set(DLL_FOUND TRUE)
                elseif(EXISTS "C:/SDL2_image/lib/x64/${dll_name}")
                    set(DLL_PATH "C:/SDL2_image/lib/x64/${dll_name}")
                    set(DLL_FOUND TRUE)
                endif()
            endif()
            
            # Try SDL2_mixer specific paths
            if(NOT DLL_FOUND AND "${dll_name}" STREQUAL "SDL2_mixer.dll")
                if(EXISTS "C:/SDL2_mixer/bin/${dll_name}")
                    set(DLL_PATH "C:/SDL2_mixer/bin/${dll_name}")
                    set(DLL_FOUND TRUE)
                elseif(EXISTS "C:/SDL2_mixer/lib/x64/${dll_name}")
                    set(DLL_PATH "C:/SDL2_mixer/lib/x64/${dll_name}")
                    set(DLL_FOUND TRUE)
                endif()
            endif()
        endif()
        
        # Copy DLL if found
        if(DLL_FOUND)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMENT "Copying ${dll_name} to output directory"
            )
            message(STATUS "Will copy ${dll_name} from ${DLL_PATH}")
        else()
            message(WARNING "Could not find ${dll_name}. The application may not run correctly.")
        endif()
    endfunction()
    
    # Copy SDL2 DLLs
    copy_dll_if_exists("SDL2.dll")
    copy_dll_if_exists("SDL2_image.dll")
    copy_dll_if_exists("SDL2_mixer.dll")
    copy_dll_if_exists("SDL2_ttf.dll")
    
    # Copy SDL2 dependency DLLs (required by SDL2_image and SDL2_mixer)
    set(SDL2_DEPENDENCY_DLLS
        "libpng16.dll"
        "zlib1.dll"
        "ogg.dll"
        "vorbis.dll"
        "vorbisfile.dll"
        "vorbisenc.dll"
        "brotlicommon.dll"
        "brotlidec.dll"
        "brotlienc.dll"
        "bz2.dll"
        "freetype.dll"
    )
    
    foreach(dep_dll ${SDL2_DEPENDENCY_DLLS})
        copy_dll_if_exists("${dep_dll}")
    endforeach()
    
    # Copy GLEW DLL if using dynamic GLEW
    if(GLEW_FOUND AND TARGET GLEW::GLEW)
        # GLEW might be static, so this might not be needed
        copy_dll_if_exists("glew32.dll")
    elseif(NOT GLEW_FOUND)
        # Check if GLEW DLL exists in the library directory
        if(GLEW_LIBRARIES)
            get_filename_component(GLEW_LIB_DIR "${GLEW_LIBRARIES}" DIRECTORY)
            get_filename_component(GLEW_BIN_DIR "${GLEW_LIB_DIR}/../bin" ABSOLUTE)
            if(EXISTS "${GLEW_BIN_DIR}/glew32.dll")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${GLEW_BIN_DIR}/glew32.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                    COMMENT "Copying glew32.dll to output directory"
                )
            endif()
        endif()
    endif()
endif()

# Copy Assets and Shaders directories to build directory so the executable can find them
file(COPY ${CMAKE_SOURCE_DIR}/Assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_BINARY_DIR})

# Set the working directory to the build directory so the executable can find assets
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)