idf_component_register(SRCS "uart_repl.c" "forth_prims.s" "forth.c" 
                    INCLUDE_DIRS "")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
