set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/implot)
set(IMGUIFILEDIALOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog)
set(IMGUI_FLAME_GRAPH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFlameGraph)

ADD_LIBRARY(IMGUIFILEDIALOG STATIC)

target_sources(IMGUIFILEDIALOG
                PRIVATE
                ${IMGUIFILEDIALOG_DIR}/ImGuiFileDialog.cpp)

target_include_directories(IMGUIFILEDIALOG
                PUBLIC
                ${IMGUIFILEDIALOG_DIR}/ImGuiFileDialog.h)

add_library(IMGUI STATIC)

target_sources(IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp
                    ${IMGUIZMO_DIR}/GraphEditor.cpp
                    ${IMGUIZMO_DIR}/ImCurveEdit.cpp
                    ${IMGUIZMO_DIR}/ImGradient.cpp
                    ${IMGUIZMO_DIR}/ImGuizmo.cpp
                    ${IMGUIZMO_DIR}/ImSequencer.cpp
                    ${IMPLOT_DIR}/implot.cpp
                    ${IMPLOT_DIR}/implot_items.cpp
                    ${IMPLOT_DIR}/implot_demo.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
                )

target_include_directories(IMGUI
                            PUBLIC
                                ${IMGUI_DIR}
                                ${IMGUI_DIR}/backends
                                ${IMGUIZMO_DIR}
                                ${IMPLOT_DIR}
                            )

target_link_libraries(IMGUIFILEDIALOG PUBLIC IMGUI)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFlameGraph/)
target_link_libraries(IMGUI_FLAME_GRAPH PUBLIC IMGUI)

