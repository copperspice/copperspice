#!/bin/sh

# This script generates the qrc_harfbuzz.cpp file used in CsCore/locale/qunicodetools.cpp

$CS_HOME/bin/rcc -o ../../src/3rdparty/harfbuzz/fonts/resource_harfbuzz.cpp  ../../src/3rdparty/harfbuzz/fonts/harfbuzz.qrc

