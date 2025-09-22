# aux_source_directory(./ SRC)
# add_definitions(-lprotobuf -pthread)

file(GLOB SRC_LIST ../*.cc *.cc)
add_executable(customer ${SRC_LIST})
target_link_libraries(customer mprpc protobuf pthread)