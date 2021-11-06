function(gen_cfg_json)
    set(dictionary ${CMAKE_SOURCE_DIR}/dictionaries/cedict_ts.u8)
    set(card_db $ENV{HOME}/zikhron)
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/script/zikhron_cfg.json.in
        ${CMAKE_CURRENT_BINARY_DIR}/zikhron_cfg.json
        @ONLY
    )
endfunction(gen_cfg_json)
