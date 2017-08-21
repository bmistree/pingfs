#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Relative to root of project
LIB_DIR=$DIR/../lib
APP_DIR=$DIR/../app
TEST_DIR=$DIR/../test

ALL_FILES=($(find $LIB_DIR -name *.cc))
ALL_FILES+=($(find $LIB_DIR -name *.hpp))
ALL_FILES+=($(find $APP_DIR -name *.cc))
ALL_FILES+=($(find $APP_DIR -name *.hpp))
ALL_FILES+=($(find $TEST_DIR -name *.hpp))
ALL_FILES+=($(find $TEST_DIR -name *.cc))

python $DIR/../deps/cpplint/cpplint.py \
       --verbose 0 \
       --filter -legal/copyright,-build/c++11 \
       ${ALL_FILES[@]}
