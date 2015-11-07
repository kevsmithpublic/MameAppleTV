TARGET  = mmuhack
#INCLUDE = -I/gp2xsdk/Tools/arm-gp2x-linux/sys-include
#INCLUDE = -I/build/devgp2x/arm-unknown-linux-gnu/arm-unknown-linux-gnu/sys-include
INCLUDE = -I/usr/local/arm-dev/arm-linux/sys-include
CFLAGS  = -O2 -DMODULE -D__KERNEL__ ${INCLUDE}
#CC	= /build/devgp2x/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-gcc
#CXX	= /build/devgp2x/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-g++
CC	= /usr/local/arm-dev/bin/arm-linux-gcc
CXX	= /usr/local/arm-dev/bin/arm-linux-g++

all: ${TARGET}.o hackbench

${TARGET}.o: ${TARGET}.c

hackbench: hackbench.cpp
	${CXX} -o hackbench hackbench.cpp

clean:
	rm -rf ${TARGET}.o hackbench
