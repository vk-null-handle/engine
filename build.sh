#!/bin/sh

BUILD_TYPE="Debug"

case "$1" in
  gen) cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE ;;
  build) ninja -C build -v ;;
  rebuild) rm -rf build && cmake -B build -G Ninja && ninja -C build -v ;;
  run) ninja -C build -v && ./build/testbed ;;
  clean) rm -rf build ;;
esac
