#!/bin/bash

rm -r build/include/aergia
cmake .
make
make install
cd examples
cmake .
make