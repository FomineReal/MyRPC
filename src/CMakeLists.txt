aux_source_directory(. SRC_LIST)

add_library(myrpc ${SRC_LIST})


target_link_libraries(myrpc muduo_net muduo_base pthread zookeeper_mt)#越底层的库，越放到后面