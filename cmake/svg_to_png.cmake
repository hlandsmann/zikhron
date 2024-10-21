set(ICONS_DIRECTORY ${CMAKE_BINARY_DIR}/resources/icons)
file(MAKE_DIRECTORY ${ICONS_DIRECTORY})

function(svg_to_png filename pixel)
  get_filename_component(svg_base_name ${filename} NAME_WLE)
  set(target ${svg_base_name}_${pixel})
  set(png_file ${ICONS_DIRECTORY}/${svg_base_name}_${pixel}px.png)
  # for some reason ninja needs to be executed with -j1, inkscape won't work correctly ohterwise (dbus error)
  add_custom_command(
      OUTPUT
          ${png_file}
      COMMAND
          magick +antialias -background none
          ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
          -resize ${pixel}x${pixel} ${png_file}
  )
  add_custom_target(${target} ALL
        DEPENDS
            "${png_file}"
  )
endfunction()
