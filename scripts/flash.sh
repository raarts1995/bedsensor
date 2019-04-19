#!/bin/bash
if [[ "$PWD" =~ scripts ]]; then
	cd ../
fi
make -j3 flash monitor
