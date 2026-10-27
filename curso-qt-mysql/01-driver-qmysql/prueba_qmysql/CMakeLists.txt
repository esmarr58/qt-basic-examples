cmake_minimum_required(VERSION 3.19)
project(prueba_qmysql LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Sql)

qt_standard_project_setup()

qt_add_executable(prueba_qmysql
    main.cpp
)

target_link_libraries(prueba_qmysql
    PRIVATE
        Qt::Core
        Qt::Sql
)

install(TARGETS prueba_qmysql
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
