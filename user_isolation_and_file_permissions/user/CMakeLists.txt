project(user ASM)

include(Sources.cmake)

include_directories(include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -fno-builtin -nostdinc -nostartfiles -nodefaultlibs -fno-pic -m32")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "ld -m elf_i386 -N -e main -Ttext 0 <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# We need to do several things --
# Need ulib sources (easy enough)
# Need to compile the programs
#  -- binary should add a _...
# Need to create disassembly
# Need to create symbol files

set_property(GLOBAL PROPERTY user_programs)
set_property(GLOBAL PROPERTY user_targets)

set_property(GLOBAL PROPERTY default_programs)
set_property(GLOBAL PROPERTY default_targets)

set_property(GLOBAL PROPERTY login_programs)
set_property(GLOBAL PROPERTY login_targets)

add_library(ulib OBJECT
  ${ulib_SOURCES})

function(add_user_program)
  set(options)
  set(oneValueArgs PREFIX SRC TARGETS PROGRAMS)
  set(multiValueArgs)
  cmake_parse_arguments(ADD_USER_PROG "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  # Strip out src/ and .c
  get_filename_component(prog_base ${ADD_USER_PROG_SRC} NAME_WE)
  # Add _
  set(output_exe "_${ADD_USER_PROG_PREFIX}${prog_base}")
  message("Have output_exe ${output_exe}")

  # First, build the program
  add_executable(${output_exe} ${ADD_USER_PROG_SRC} $<TARGET_OBJECTS:ulib>)
  set(output_name "_${prog_base}")
  set_target_properties(${output_exe} PROPERTIES OUTPUT_NAME "${output_name}")

  get_property(l_programs GLOBAL PROPERTY ${ADD_USER_PROG_PROGRAMS})
  list(APPEND l_programs "${output_name}")
  set_property(GLOBAL PROPERTY ${ADD_USER_PROG_PROGRAMS} ${l_programs})
  
  get_property(l_targets GLOBAL PROPERTY ${ADD_USER_PROG_TARGETS})
  list(APPEND l_targets "${output_exe}")
  set_property(GLOBAL PROPERTY ${ADD_USER_PROG_TARGETS} ${l_targets})
  
  # Second, disassemble the program
  add_custom_command(TARGET ${output_exe}
    POST_BUILD
    COMMAND objdump -S ${output_name} > ${output_name}.asm
    COMMENT "Disassembling ${output_name} > ${CMAKE_CURRENT_BINARY_DIR}/${output_name}.asm"
    BYPRODUCTS ${output_name}.asm)
endfunction(add_user_program)

foreach(user_src ${user_SOURCES})
  add_user_program(SRC ${user_src} TARGETS user_targets PROGRAMS user_programs)
endforeach(user_src)

get_property(l_user_programs GLOBAL PROPERTY user_programs)
message("USER_PROGRAMS includes: ${l_user_programs}")
get_property(l_user_targets GLOBAL PROPERTY user_targets)
message("USER_TARGETS includes: ${l_user_targets}")

foreach(login_src ${login_SOURCES})
  add_user_program(SRC ${login_src} TARGETS login_targets PROGRAMS login_programs
                   PREFIX login_)
endforeach(login_src)

get_property(l_login_programs GLOBAL PROPERTY login_programs)
message("LOGIN_PROGRAMS includes: ${l_login_programs}")
get_property(l_login_targets GLOBAL PROPERTY login_targets)
message("LOGIN_TARGETS includes: ${l_login_targets}")

foreach(default_src ${default_SOURCES})
  add_user_program(SRC ${default_src} TARGETS default_targets
                   PROGRAMS default_programs PREFIX default_)
endforeach(default_src)

get_property(l_default_programs GLOBAL PROPERTY default_programs)
message("DEFAULT_PROGRAMS includes: ${l_default_programs}")
get_property(l_default_targets GLOBAL PROPERTY default_targets)
message("DEFAULT_TARGETS includes: ${l_default_targets}")

configure_file(${CMAKE_SOURCE_DIR}/README ${CMAKE_CURRENT_BINARY_DIR}/README COPYONLY)

add_custom_command(
  OUTPUT fs.img
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../tools/mkfs fs.img ${l_default_programs} README ${l_user_programs} 
  DEPENDS mkfs ${CMAKE_SOURCE_DIR}/README ${l_user_targets} ${l_default_targets}
)

add_custom_command(
  OUTPUT login_fs.img
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../tools/mkfs login_fs.img ${l_login_programs} README ${l_user_programs} 
  DEPENDS mkfs ${CMAKE_SOURCE_DIR}/README ${l_user_targets} ${l_login_targets}
)

add_custom_target(
  makeuserfs ALL
  DEPENDS fs.img login_fs.img)

