idf_component_register(SRCS "forth_prims.c" "forth.c" 
                    INCLUDE_DIRS "")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
