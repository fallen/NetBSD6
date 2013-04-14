NetBSD
======

LatticeMico32 and Milkymist port for NetBSD

Clone it:

$ git clone git://github.com/fallen/NetBSD.git

Generate lm32--netbsd cross compilation toolchain:

$ ./build.sh -m milkymist -U tools

Compile NetBSD kernel for Milkymist SoC:

$ ./build.sh -m milkymist -U kernel=GENERIC

Note: The kernel does not compile yet. This is a work in progress. Feel free to help :)
