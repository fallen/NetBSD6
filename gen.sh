#!/bin/bash
HAVE_GCC=45 USE_COMPILERCRTSTUFF=no MKPIC=no MKATF=no MKGCC=no MKBINUTILS=no MKGDB=no ./build.sh -m milkymist -u -U $* build
