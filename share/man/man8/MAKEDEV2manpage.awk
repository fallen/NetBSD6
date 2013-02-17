#       $NetBSD: MAKEDEV2manpage.awk,v 1.13 2010/03/23 19:19:03 jakllsch Exp $
#
# Copyright (c) 2002
#	Dieter Baron <dillo@NetBSD.org>.  All rights reserved.
# Copyright (c) 1999
#       Hubert Feyrer <hubertf@NetBSD.org>.  All rights reserved.
# [converted from Hubert's Perl version]
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Hubert Feyrer for
#      the NetBSD Project.
# 4. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#
###########################################################################
#
# Convert src/etc/MAKEDEV.tmpl and
# src/share/man/man8/MAKEDEV.8.template to
# src/share/man/man8/MAKEDEV.8, replacing
#  - @@@SPECIAL@@@ with all targets in the first section (all, std, ...)
#  - @@@DEVICES@@@ with the remaining targets
#  - @@@ARCH@@@ with the architecture name
#

# XXX: uses non-standard AWK function toupper()

BEGIN {
	MAKEDEV = "../../../etc/MAKEDEV.tmpl";
    print ".\\\" *** ------------------------------------------------------------------";
    print ".\\\" *** This file was generated automatically";
    print ".\\\" *** from src/etc/MAKEDEV.tmpl and";
    print ".\\\" *** src/share/man/man8/MAKEDEV.8.template";
    print ".\\\" ***";
    print ".\\\" *** DO NOT EDIT - any changes will be lost!!!";
    print ".\\\" *** ------------------------------------------------------------------";
    print ".\\\"";
}

function read1line() {
	if (r1kept)
		r1l = r1last;
	else
		getline r1l < MAKEDEV;
	
	while (r1l ~ /^#[ \t]*$/)
		getline r1l < MAKEDEV;

	if (r1l ~ /^#[ \t]/) {
		if (r1l ~ /^# /) {
            		# Not a device/other target
            		r1kept = 0;
		}
        	else {
            		# Continuation line (?)
			getline r1ll < MAKEDEV;
			while (r1ll ~ /^#\t[ \t]/) {
				sub(/^#\t[ \t]/, " ", r1ll);
				r1l = r1l r1ll;
				getline r1ll < MAKEDEV;
			}
			r1last = r1ll;
			r1kept = 1;
        	}
    	}
	else
		r1kept = 0;

	return 1;
}

/^@@@SPECIAL@@@$/ {
        print ".\\\" " $0;
    	print ".Bl -tag -width 01234567 -compact";

	while (getline l < MAKEDEV > 0 && l !~ /^#.*Device.*Valid.*argument/)
		;
	while (read1line() && r1l ~ /^#\t/) {
		sub(/#[ \t]*/, "", r1l);
		target=r1l;
		sub(/[ \t].*/, "", target);
		line=r1l;
		sub(/[^ \t]*[ \t]/, "", line);
		# replace "foo" with ``foo''
		gsub(/\"[^\"]*\"/, "``&''", line)
		gsub(/\"/, "", line)
		gsub(/[ \t]+/, " ", line);
          	print ".It Ar " target;
		print toupper(substr(line, 1, 1)) substr(line, 2);

	}
	r1last = r1l;
	r1kept = 1;
	print ".El";
	next;
}
/^@@@DEVICES@@@$/ {
        print ".\\\" " $0;
    	print ".Bl -tag -width 01";

	read1line();
    	do {
		sub(/^#[ \t]+/, "", r1l);
		if (r1l ~ /[^ \t]:$/)
			sub(/:$/, " :", r1l);
		print ".It " r1l;	# print section heading

        	print ". Bl -tag -width 0123456789 -compact";
        	while(read1line() && r1l ~ /^#\t/) {
			gsub(/#[ \t]+/, "", r1l);
			target=r1l;
			sub(/[ \t].*/, "", target);
			line=r1l;
			sub(/[^ \t]*[ \t]+/, "", line);
			sub(/\*/, "#", target);
			# replace "foo" with ``foo''
			gsub(/\"[^\"]*\"/, "``&''", line)
			gsub(/\"/, "", line)
			# fix collateral damage of previous
			sub(/5 1\/4''/, "5 1/4\"", line)
			sub(/3 1\/2``/, "3 1/2\"", line)
			# gc whitespace
			sub(/\(XXX[^)]*\)/, "", line);
			sub(/[ \t]*$/, "", line);

              		# add manpage, if available
			if (target == "fd#")
				page = "fdc"
			else if (target == "pms#")
				page = "opms"
			else if (target == "ed#")
				page = "edc"
			else if (target == "ttye#")
				page = "ite"
			else if (target == "ttyh#")
				page = "sab"
			else if (target == "ttyU#")
				page = "ucom"
			else if (target == "fd")
				page = "-----" # force no .Xr
			else if (target == "sysmon")
				page = "envsys"
			else if (target == "ttyZ#")
				page = "zstty"
			else if (target == "ttyCZ?")
				page = "cz"
			else if (target == "ttyCY?")
				page = "cy"
			else if (target == "ttyB?")
				page = "scc"
			else if (target == "random")
				page = "rnd"
			else if (target == "scsibus#")
				page = "scsi"
			else {
				page=target;
				sub(/[^a-zA-Z]+/, "", page);
			}

			str = "ls ../man4/" page ".4 ../man4/man4.*/" page ".4 2>/dev/null"
			while(str | getline) {
				if (system("test -f " $0) != 0)
					continue

				# get the manpage including opt. arch name
				sub(/^\.\.\/man4\//, "")
				sub(/^man4\./, "")
				sub(/\.4$/, "")

				sub(/[ \t]*$/, "", line);
				if (line ~ /see/) {
                      		    # already a manpage there, e.g. scsictl(8)
				    line = line " ,";
				}
				else
				    line = line ", see";
				# Add .Xr \&foo 4 - ampersand to work around
				# manpages that are *roff commands at the same
				# time
                  		line = line "\n.Xr \\&" $0 " 4";
			}
			close(str)

			gsub(/[ \t]+$/, "", line);
			gsub(/[ \t]+/, " ", line);

              		print ". It Ar " target;
			line2=toupper(substr(line, 1, 1)) substr(line, 2);
			sub(/Wscons/, "wscons", line2);
			sub(/Pccons/, "pccons", line2);
			print line2;
        	}
        	print MANPAGE ". El";
    	} while (r1l ~ /^# /);

    	print ".El";
	next;
}
/@@@ARCH@@@/ {
	gsub(/@@@ARCH@@@/, ARCH);
}
# date is substituted in the shell script
#/@@@DATE@@@/ {
#	# date
#}
/\$NetBSD/ {
	sub(/\$NetBSD.*\$/, "$""NetBSD$");
}
{ print }
