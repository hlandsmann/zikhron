set(ICONS_DIRECTORY ${CMAKE_BINARY_DIR}/resources/icons)
file(MAKE_DIRECTORY ${ICONS_DIRECTORY})

set(SVG_TO_PNG_TARGETS "" CACHE INTERNAL "List of svg_to_png targets for chaining")

function(svg_to_png filename pixel)
  get_filename_component(svg_base_name ${filename} NAME_WLE)
  set(target ${svg_base_name}_${pixel})
  set(png_file ${ICONS_DIRECTORY}/${svg_base_name}_${pixel}px.png)

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
  # Chain this target to the previous one, if any
  list(LENGTH SVG_TO_PNG_TARGETS num_targets)
  if(num_targets GREATER 0)
    list(GET SVG_TO_PNG_TARGETS -1 previous_target)
    add_dependencies(${target} ${previous_target})
  endif()

  # Append this target to the list
  list(APPEND SVG_TO_PNG_TARGETS ${target})
  set(SVG_TO_PNG_TARGETS "${SVG_TO_PNG_TARGETS}" CACHE INTERNAL "List of svg_to_png targets for chaining")
endfunction()
