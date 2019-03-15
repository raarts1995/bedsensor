TIME=$(date "+%d %b %H:%M")
MSG="Updated on $TIME"
if [ "$1" != "" ]; then
	MSG=$1
fi
echo "Commit message: $MSG"
git add *
git commit -m "$MSG"
git push -u origin master
echo "Done"
