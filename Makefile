default: gendy~.pd_linux

gendy~.pd_linux:
	/home/srussell/local/src/flext/build.sh pd gcc

install: gendy~.pd_linux
	cp pd-linux/release-single/gendy~.pd_linux /usr/local/lib/pd/extra
