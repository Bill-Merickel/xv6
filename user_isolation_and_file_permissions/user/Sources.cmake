# Files in the "User-space library"  These are common routines (like system
#    call definitions) that user-space programs will use
set(ulib_SOURCES
  # Contains all of the system calls
  asm/usys.S

  # Common library functions
  src/ulib.c
  src/umalloc.c
  src/printf.c
  src/crypto/aes256.c
  src/crypto/sha256.c
  src/login/login_lib.c
  )

set(login_SOURCES
  # Init -- the first program the kernel launchs
  src/login/login_init.c
  # Login system (launched by init)
  src/login/login.c
  )

set(default_SOURCES
  # Init -- the first program the kernel launchs
  src/init.c
  )

# User-space programs, 
set(user_SOURCES
  # The shell (launched by init)
  src/sh.c

  # Common utility programs
  src/ls.c

  # Test programs
  src/crypto/test/test_sha256.c
  src/crypto/test/test_aes256.c
  )

