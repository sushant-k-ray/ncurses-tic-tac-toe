cmake_minimum_required(VERSION 3.16)

project(tictactoe LANGUAGES C)

add_executable(tictactoe main.c)
target_link_libraries(tictactoe ncurses)

include(GNUInstallDirs)
install(TARGETS tictactoe
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
