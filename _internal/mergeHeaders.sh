#!/bin/bash

OUTPUT="tests/test.h"

cat _internal/_macros.h > "$OUTPUT"
cat _internal/_staticLib.h >> "$OUTPUT"
cat _internal/_objectFile.h >> "$OUTPUT"
cat _internal/_mock.h >> "$OUTPUT"
cat _internal/_framework.h >> "$OUTPUT"
cat _internal/_platforms.h >> "$OUTPUT"
cat _internal/_tail.h >> "$OUTPUT"