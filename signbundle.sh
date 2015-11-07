codesign -f -s "iPhone Developer" mame
mkdir -p mame.app
cp -R ./res/Applications/iMAME4all.app/ mame.app/
touch mame.app/PkgInfo
cp mame mame.app/
