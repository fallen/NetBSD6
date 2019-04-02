#!/bin/bash
NBMAKE=obj/tooldir.$(uname -s)-$(uname -r)-$(uname -m)/bin/nbmake-milkymist

./build.sh -m milkymist -u -U tools
./build.sh -m milkymist kernel=GENERIC
HAVE_GCC=45 USE_COMPILERCRTSTUFF=yes MKPIC=no MKATF=no MKGCC=no MKGCCCMDS=no MKBINUTILS=no MKGDB=no MKCOMPLEX=no MKDOC=no MKDYNAMICROOT=no MKHTML=no MKINET6=no MKINFO=no MKIPFILTER=no MKISCSI=no MKLDAP=no MKKMOD=no MKKERBEROS=no MKLIBSTDCXX=no MKLVM=no MKMAN=no MKLINT=no MKPOSTFIX=no MKPROFILE=no MKSHARE=no ./build.sh -m milkymist -u -U $* build
for lib in $(ls lib)
do
	cd $lib
	$NBMAKE
	$NBMAKE install
	cd -
done

cd rescue
$NBMAKE
cd -
