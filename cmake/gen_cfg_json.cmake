function(gen_cfg_json)
    set(dictionary ${CMAKE_SOURCE_DIR}/dictionaries/)
    set(database_directory $ENV{HOME}/zikhron)
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/script/zikhron_cfg_json.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.json
        @ONLY
    )
endfunction(gen_cfg_json)
