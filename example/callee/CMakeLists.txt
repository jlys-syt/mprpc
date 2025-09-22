# aux_source_directory(./ SRC)
# add_definitions(-lprotobuf -pthread)

file(GLOB SRC_LIST ../*.cc ./*.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc protobuf pthread)