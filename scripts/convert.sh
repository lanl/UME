#!/bin/bash

CMD=src/txt2bin

if [ ! -x ${CMD} ]; then
    echo "${CMD} doesn't exist"
    exit 1
fi

for file in *.umetxt; do
    ${CMD} "$file" "${file%.umetxt}.ume"
done
