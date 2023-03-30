#!/bin/bash

find . \( -name '*.cc' -o -name '*.hh' \) -exec clang-format -i {} \+
