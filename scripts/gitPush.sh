#!/bin/bash
if [[ "$PWD" =~ scripts ]]; then
	cd ../
fi

TIME=$(date "+%d %b %H:%M")
MSG="Updated on $TIME"
if [ "$1" != "" ]; then
	MSG=$1
fi
echo "Commit message: $MSG"
git commit -am "$MSG"
git push -u origin master
echo "Done"
