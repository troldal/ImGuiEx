add_executable(ImGuiExDemo WIN32
        main.cpp
)

target_link_libraries(ImGuiExDemo PRIVATE
        ImGuiEx
        imgui-win32
        imgui-dx11
)

