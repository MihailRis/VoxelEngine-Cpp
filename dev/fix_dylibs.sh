#!/bin/bash

PROJECT_NAME=$1
CONFIG=$2
OUTPUT_DIR=$3
LIBS_DIR="$OUTPUT_DIR/libs"

mkdir -p "$LIBS_DIR"

TMP_BINARY="$OUTPUT_DIR/tmp_$PROJECT_NAME"
BINARY="$OUTPUT_DIR/$PROJECT_NAME"

cp "$BINARY" "$TMP_BINARY"

otool -L "$TMP_BINARY" | grep -o '/.*dylib' | while read -r dylib; do
    if [[ "$dylib" == /usr/lib/* || "$dylib" == /System/Library/* ]]; then
        continue
    fi

    cp "$dylib" "$LIBS_DIR"
    install_name_tool -change "$dylib" "@executable_path/libs/$(basename "$dylib")" "$TMP_BINARY"
done

mv "$TMP_BINARY" "$BINARY"
