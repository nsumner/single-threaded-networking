
add_executable(chatclient
  chatclient.cpp
  ChatWindow.cpp
)

set_target_properties(chatclient
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      CXX_STANDARD 14
                      PREFIX ""
)

find_package(Boost 1.58 COMPONENTS system REQUIRED)
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CURSES_INCLUDE_DIR}
)

target_link_libraries(chatclient
  networking
  ${Boost_LIBRARIES}
  ${CURSES_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS chatclient
  RUNTIME DESTINATION bin
)

