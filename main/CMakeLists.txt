idf_component_register(SRCS "forth.c" "forth_prims.c" "forth_dict.s"
                    INCLUDE_DIRS "")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
