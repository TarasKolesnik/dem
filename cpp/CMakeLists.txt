cmake_minimum_required(VERSION 3.16)
project(can_drive_protocol_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_program(PROTOC_EXECUTABLE protoc REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../proto/can_drive.proto)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN_DIR})

set(PB_CC ${GEN_DIR}/can_drive.pb.cc)
set(PB_H  ${GEN_DIR}/can_drive.pb.h)

add_custom_command(
  OUTPUT ${PB_CC} ${PB_H}
  COMMAND ${PROTOC_EXECUTABLE}
          -I ${CMAKE_CURRENT_SOURCE_DIR}/../proto
          --cpp_out=${GEN_DIR}
          ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
  COMMENT "Generating C++ protobuf from can_drive.proto"
)

add_executable(can_drive_example
  src/example_client.cpp
  ${PB_CC}
)

target_include_directories(can_drive_example PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${GEN_DIR}
  ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(can_drive_example PRIVATE protobuf::libprotobuf)
