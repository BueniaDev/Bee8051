#!/bin/bash

if [[ ! -f "libbee8051.a" ]]; then
	echo "Run this script from the directory where you built the Bee8051 engine."
	exit 1
fi

mkdir -p dist

if [ -d "examples" ]; then
	for lib in $(ldd examples/sim8051.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp examples/sim8051.exe dist
fi

