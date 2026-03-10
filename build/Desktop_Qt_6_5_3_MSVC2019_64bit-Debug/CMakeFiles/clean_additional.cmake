# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ClassroomAttentionDemo_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ClassroomAttentionDemo_autogen.dir\\ParseCache.txt"
  "ClassroomAttentionDemo_autogen"
  )
endif()
