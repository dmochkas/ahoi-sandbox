
# Ascon
set(ALG_NAME "asconaead128")
set(IMPL_NAME "opt64")
set(ALG_LIST ${ALG_NAME})
set(IMPL_LIST ${IMPL_NAME})
set(TEST_LIST "")
set(ASCON_LIB_NAME crypto_aead_${ALG_NAME}_${IMPL_LIST})

# Project
set(PROJECT_NAME "ascon-demo")
set(EXECUTABLE_NAME ${PROJECT_NAME})