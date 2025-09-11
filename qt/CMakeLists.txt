# cmake_minimum_required(VERSION 3.19)
# project(Qt LANGUAGES CXX)

# find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

# qt_standard_project_setup()

# qt_add_executable(Qt
#     WIN32 MACOSX_BUNDLE
#     main.cpp
#     mainwindow.cpp
#     mainwindow.h
#     mainwindow.ui
# )

# target_link_libraries(Qt
#     PRIVATE
#         Qt::Core
#         Qt::Widgets
# )

# include(GNUInstallDirs)

# install(TARGETS Qt
#     BUNDLE  DESTINATION .
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# qt_generate_deploy_app_script(
#     TARGET Qt
#     OUTPUT_SCRIPT deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${deploy_script})
cmake_minimum_required(VERSION 3.20)
project(SudokuSat CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)  # 让 Qt 的 moc 自动运行
find_package(Qt6 REQUIRED COMPONENTS Widgets)


add_library(qt_gui STATIC
    qt/sudoku_gui.cpp
)
target_link_libraries(qt_gui PRIVATE Qt6::Widgets)


add_library(core_lib STATIC
    main.cpp
)
target_compile_definitions(core_lib PUBLIC USE_QT_GUI)

add_executable(sudoku main.cpp)
target_link_libraries(sudoku PRIVATE core_lib qt_gui)