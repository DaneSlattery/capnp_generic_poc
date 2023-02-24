cmake_minimum_required(VERSION 3.16.3)
project(ipc_demo)
set(CMAKE_CXX_STANDARD 17)

# CAPNP specific
file(GLOB_RECURSE CAPNP_SCHEMAS src/*.capnp)
if(NOT CapnProto_FOUND)
    find_package(CapnProto CONFIG REQUIRED)
endif()
set(CAPNPC_SRC_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "" FORCE)
set(CAPNP_EXECUTABLE /usr/local/bin/capnp)
set(CAPNPC_CXX_EXECUTABLE /usr/local/bin/capnpc-c++)
capnp_generate_cpp(autogenSources autogenHeaders ${CAPNP_SCHEMAS})

set(PROJECTS server client)
foreach (X ${PROJECTS})
    set(EXE_NAME ${X})
    file(GLOB_RECURSE ${EXE_NAME}_SRCS src/${EXE_NAME}.cpp)

    add_executable(${EXE_NAME} ${${EXE_NAME}_SRCS} ${autogenSources})
    target_link_libraries(${EXE_NAME} PRIVATE CapnProto::capnp-rpc)
    target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/../schema)

endforeach ()
