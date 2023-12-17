set(EDITOR_LIBS_SOURCES)
set(EDITOR_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

set(EDITOR_LIBS_SOURCES)
set(EDITOR_LIBS_INCLUDE)

list(APPEND EDITOR_LIBS_SOURCES
	"pch.cpp"
	"Camera.cpp"
	"Editor.cpp"
	"EditorContext.cpp"
	"Simulation.cpp"
	"GUI/GuiManager.cpp"
	"GUI/GuiRenderer.cpp"
	"GUI/ThumbnailManager.cpp"
	"GUI/Components/Common.cpp"
	"GUI/Components/ContentWindowComponents.cpp"
	"GUI/DetailDrawer/DetailDrawer.cpp"
	"GUI/PostProcessing/GuiPostProcessing.cpp"
	"GUI/Windows/ContentWindow.cpp"
	"GUI/Windows/DetailsWindow.cpp"
	"GUI/Windows/GameWindow.cpp"
	"GUI/Windows/ProfilerWindow.cpp"
	"GUI/Windows/ResourceMonitorWindow.cpp"
	"GUI/Windows/SceneGraphWindow.cpp"
	"GUI/Windows/SceneWindow.cpp"
	"GUI/Windows/SettingsWindow.cpp"
	"GUI/Windows/Window.cpp"
)
list(APPEND EDITOR_LIBS_INCLUDE
	"GUI/Utils/Buffers.hpp"
	"pch.hpp"
	"Camera.hpp"
	"Editor.hpp"
	"EditorContext.hpp"
	"Simulation.hpp"
	"GUI/GuiManager.hpp"
	"GUI/GuiRenderer.hpp"
	"GUI/ThumbnailManager.hpp"
	"GUI/Components/Common.hpp"
	"GUI/Components/ContentWindowComponents.hpp"
	"GUI/DetailDrawer/DetailDrawer.hpp"
	"GUI/PostProcessing/GuiPostProcessing.hpp"
	"GUI/Windows/ContentWindow.hpp"
	"GUI/Windows/DetailsWindow.hpp"
	"GUI/Windows/GameWindow.hpp"
	"GUI/Windows/ProfilerWindow.hpp"
	"GUI/Windows/ResourceMonitorWindow.hpp"
	"GUI/Windows/SceneGraphWindow.hpp"
	"GUI/Windows/SceneWindow.hpp"
	"GUI/Windows/SettingsWindow.hpp"
	"GUI/Windows/Window.hpp"
)

add_library("Editor")

target_sources("Editor"
	PRIVATE
	${EDITOR_LIBS_SOURCES}
	${EDITOR_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(BOOST 1.75 REQUIRED)

set_target_properties("Editor" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Editor" PRIVATE pch.hpp )
endif()


include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

target_include_directories("Editor"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Editor" PUBLIC rttr_core Utils Graphics Scene Physics Animation Renderer PRIVATE ${Boost_LIBRARIES} IMGUI IMGUI_FLAME_GRAPH IMGUIFILEDIALOG)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	#add_boost_test(SOURCE "Tests/EditorTests.cpp" INCLUDE "." LINK Editor PREFIX Editor)
endif(BUILD_TESTS)