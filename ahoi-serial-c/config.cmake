
# Ascon config
set(ALG_NAME "asconaead128" CACHE STRING "")
set(IMPL_NAME "opt64" CACHE STRING "")
set(ALG_LIST ${ALG_NAME} CACHE STRING "")
set(IMPL_LIST ${IMPL_NAME} CACHE STRING "")
set(TEST_LIST "" CACHE STRING "")

# App config
option(ENABLE_TESTS "Enable testing" ON)