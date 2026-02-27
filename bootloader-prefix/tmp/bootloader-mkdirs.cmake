# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/tmp"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/src/bootloader-stamp"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/src"
  "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/work/Dropbox/work/electricScale/Prog/ElecScale/Esp32smartSclev4.05b/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
