apt-get update
apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libasound2-dev g++ libgl1-mesa-dev libglu1-mesa-dev libudev-dev libdrm-dev libglew1.5-dev libopenal-dev libsndfile-dev libfreeimage-dev libcairo2-dev libjack-dev python-lxml python-argparse binutils
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
	LIBSPATH=linux64
else
	LIBSPATH=linux
fi

WHO=`sudo who am i`;ID=`echo ${WHO%% *}`
GROUP_ID=`id --group -n ${ID}`
cd ../../../libs/openFrameworksCompiled/project/$LIBSPATH
make Debug
if [ $? != 0 ]; then
	echo "there has been a problem compiling Debug OF library"
	echo "please report this problem in the forums"
	exit
fi
chown -R $ID:$GROUP_ID obj ../../lib/${LIBSPATH}/*
make Release
if [ $? != 0 ]; then
        echo "there has been a problem compiling Release OF library"
        echo "please report this problem in the forums"
fi
chown -R $ID:$GROUP_ID obj ../../lib/${LIBSPATH}/*
# libpoco-dev 
