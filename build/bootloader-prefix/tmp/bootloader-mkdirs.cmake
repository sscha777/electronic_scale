# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "I:/Espressif/frameworks/esp-idf-v5.5.1/components/bootloader/subproject")
  file(MAKE_DIRECTORY "I:/Espressif/frameworks/esp-idf-v5.5.1/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader"
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix"
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/tmp"
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/src"
  "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Dropbox/work/electricScale/Prog/ElecScale_v2/cursor/Esp32smartSclev2_v0.0c/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
