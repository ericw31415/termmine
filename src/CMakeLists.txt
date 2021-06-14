add_executable(termmine Game.cxx main.cxx play.cxx Timer.cxx)
set_property(TARGET termmine PROPERTY CXX_STANDARD 20)
target_link_libraries(termmine ncursesw)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_options(termmine PUBLIC -DNCURSES_STATIC)
    target_include_directories(
        termmine
        PUBLIC ${CMAKE_SYSROOT}/include/ncursesw)
    target_link_options(
        termmine
        PUBLIC -static -static-libgcc -static-libstdc++)
endif()
