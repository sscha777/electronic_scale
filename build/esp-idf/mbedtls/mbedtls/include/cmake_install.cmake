# Install script for directory: I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/main")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "I:/Espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119/xtensa-esp-elf/bin/xtensa-esp32-elf-objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/block_cipher.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/build_info.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/compat-2.x.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_legacy_crypto.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_legacy_from_psa.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_psa_from_legacy.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_psa_superset_legacy.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_ssl.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_adjust_x509.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/config_psa.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/constant_time.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/lms.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/mbedtls_config.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/pkcs7.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/private_access.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/psa_util.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/sha3.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/build_info.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_adjust_auto_enabled.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_adjust_config_dependencies.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_adjust_config_key_pair_types.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_adjust_config_synonyms.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_builtin_composites.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_builtin_key_derivation.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_builtin_primitives.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_compat.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_config.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_driver_common.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_key_derivation.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_extra.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_legacy.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_platform.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_se_driver.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_sizes.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_struct.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_types.h"
    "I:/Espressif/frameworks/esp-idf-v5.5.1/components/mbedtls/mbedtls/include/psa/crypto_values.h"
    )
endif()

