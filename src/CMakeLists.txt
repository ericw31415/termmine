add_executable(termmine Game.cxx main.cxx play.cxx)
set_property(TARGET termmine PROPERTY CXX_STANDARD 20)
target_link_libraries(termmine ncursesw)
