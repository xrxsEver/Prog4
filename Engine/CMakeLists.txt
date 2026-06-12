add_library(minigin_engine STATIC
  Core/Component.cpp
  Core/GameObject.cpp
  Core/Minigin.cpp
  Input/Gamepad.cpp
  Input/InputManager.cpp
  Rendering/Font.cpp
  Rendering/RenderComponent.cpp
  Rendering/Renderer.cpp
  Rendering/ResourceManager.cpp
  Rendering/TextComponent.cpp
  Rendering/Texture2D.cpp
  Scene/Scene.cpp
  Scene/SceneManager.cpp
  Scene/SnoBeeManager.cpp
  UI/CacheBenchmarkWindow.cpp
  UI/ImGuiManager.cpp
  Events/Subject.cpp
  Audio/ServiceLocator.cpp
  Audio/SDLSoundSystem.cpp
  Audio/LoggingSoundSystem.cpp
  Audio/AudioLogger.cpp
)

target_compile_features(minigin_engine PUBLIC cxx_std_20)

target_include_directories(minigin_engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Core
  ${CMAKE_CURRENT_SOURCE_DIR}/Input
  ${CMAKE_CURRENT_SOURCE_DIR}/Rendering
  ${CMAKE_CURRENT_SOURCE_DIR}/Scene
  ${CMAKE_CURRENT_SOURCE_DIR}/UI
  ${CMAKE_CURRENT_SOURCE_DIR}/Events
  ${CMAKE_CURRENT_SOURCE_DIR}/Audio
  ${CMAKE_SOURCE_DIR}/Game/Systems
)

# Add VLD if available
if(VLD_INCLUDE_DIR)
  target_include_directories(minigin_engine PUBLIC ${VLD_INCLUDE_DIR})
  target_link_libraries(minigin_engine PUBLIC ${VLD_LIBRARIES})
endif()

# Link against external dependencies
target_link_libraries(minigin_engine PUBLIC
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
  glm::glm
  imgui
)

if (WIN32)
  target_link_libraries(minigin_engine PRIVATE xinput)
endif()

if(MSVC)
  target_compile_options(minigin_engine PRIVATE /W4 /WX)
else()
  target_compile_options(minigin_engine PRIVATE -Wall -Wextra -Werror)
endif()
