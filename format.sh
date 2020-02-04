#!/bin/bash
clang-format -style='{ IndentWidth: 4, ColumnLimit: 150 }' -i src/*.{c,h}
