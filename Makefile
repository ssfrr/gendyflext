default: gendy~.pd_linux

debug:
	/home/srussell/local/src/flext/build.sh pd gcc build-debug-single

gendy~.pd_linux:
	/home/srussell/local/src/flext/build.sh pd gcc

install: gendy~.pd_linux
	cp pd-linux/release-single/gendy~.pd_linux /usr/local/lib/pd/extra

install-debug: debug
	cp pd-linux/debug-single/gendy~.pd_linux /usr/local/lib/pd/extra

clean:
	rm -rf pd-linux src/*.opp
