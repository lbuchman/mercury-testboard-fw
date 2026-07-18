#!/bin/sh

STYLE=java
OPTIONS_STYLE="--indent-preprocessor --verbose --indent-cases --indent-switches --indent-classes --break-elseifs --add-brackets --unpad-paren --convert-tabs --break-closing-brackets --break-blocks --indent-col1-comments --indent-switches --indent-classes --pad-oper"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../mylib/*.h"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../mylib/*.hpp"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../mylib/*.cpp"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../arduinoLib/*.cpp"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../arduinoLib/*.h"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../arduinoLib/*.hpp"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../src/*.h"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../src/*.hpp"
astyle -n --recursive --style=${STYLE} ${OPTIONS_STYLE} "../src/*.cpp"

