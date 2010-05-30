#!/bin/sh
# AutoUPnP - automatically UPnP-forward ports when they are opened
# and remove the forwardings as soon as they're closed
# (C) 2009-2010 Michał Górny <gentoo@mgorny.alt.pl>
# Released under the terms of the 3-clause BSD license

# RDEPEND: netstat, diff, awk, miniupnpc (http://miniupnp.free.fr/)

# How to configure:
# 1) If you're keeping your private copy of this script, you might just
# adjust the configuration defaults below.
# 2) On a system-wide install, you'd probably want to set them in
# '.autoupnp.rc' file in your home directory. You might just copy
# the 'Default configuration' part below into that file and modify it
# as needed.
# 3) You might also change the config options using the environmental
# variables equal to configuration keys below. Please notice that if
# you're using 'autoupnp.rc', you need to set them on a similar
# (default-out) basis as below or else envvars will be overwritten by
# your 'autoupnp.rc'.

[ -f ~/.autoupnp.rc ] && . ~/.autoupnp.rc

# -- Default configuration --

# awk pattern to filter out unwanted listeners, e.g. ones bound to
# a local interface
: ${AUPNP_FILTER='$4 == "0.0.0.0"'}

# NOTE: below are booleans. 1/y/t/yes/true/on means true, everything
# else will be interpreted as false.

# Try to discover UPnP IGD on start to speedup further calls.
# It doesn't work with some gateways, thus you might want to disable it.
: ${AUPNP_DISCOVER=1}

# Forward all ports which are already open on start.
: ${AUPNP_START_ADD=1}

# Remove all redirected ports on exit.
: ${AUPNP_CLEANUP_RM=1}

# Redirect the output of upnpc calls. By default it is silenced out
# (upnpc is very verbose) but you might want to set some logfile there
# or '&1' to get the output on console running AutoUPnP.
: ${AUPNP_UPNPC_REDIRECT=/dev/null}

# Interval between open port checks (netstat calls), in seconds.
: ${AUPNP_INTERVAL=3}

# -- /configuration --

MY_PN=autoupnp
MY_PV=0.3

bool() {
	case "$1" in
		1|[yY]|[tT]|[yY][eE][sS]|[tT][rR][uU][eE]|[oO][nN])
			return 0;;
		*)
			return 1;;
	esac
}

get_tempfile() {
	if ! mktemp 2>/dev/null; then
		local tmpfn 2>/dev/null
		tmpfn="/tmp/tmp.${MY_PN}.$$.$1"
		touch "${tmpfn}"
		echo "${tmpfn}"
	fi
}

find_upnp() {
	upnpc -r | awk '
		$1 == "desc:" { url[i++] = $2 }
		/^Found valid IGD/ { valid = $5 }
		END {
			if (valid) {
				for (i in url) {
					for (j = length(url[i]); j > 0; j--) {
						if (substr(url[i], 1, j) == substr(valid, 1, j)) {
							if (j > bestmatchlen) {
								bestmatch = url[i]
								bestmatchlen = j
								break
							}
						}
					}
				}

				if (bestmatch)
					print "-u " bestmatch
			}
		}
	'
}

parse_output() {
	awk '
		BEGIN { FS = "[ \t:]+" }
		($1 == "tcp" || $1 == "udp") && ('"${AUPNP_FILTER}"') { print $5 " " $1 }
	'
}

parse_changes() {
	awk '
		$1 == ">" { to_open[i++] = $2 " " $3 }
		$1 == "<" { to_close[j++] = $2 " " $3 }
		END {
			i = 0
			for (j in to_close) {
				if (!(to_close[j] in to_open))
					commands[i++] = "-d " to_close[j]
			}

			for (j in to_open) {
				if (!(to_open[j] in to_close))
					commands[i] = commands[i] " " to_open[j]
			}
			if (commands[i])
				commands[i] = "-r" commands[i]

			for (i in commands) {
				if (commands[i]) {
					print("$ upnpc '"${AUPNP_UPNPC_OPTS}"' " commands[i])
					system("upnpc '"${AUPNP_UPNPC_OPTS}"' " commands[i] ">'${AUPNP_UPNPC_REDIRECT}'")
				}
			}
		}
	'
}

check_changes() {
	diff "${AUPNP_TMPF1}" "${AUPNP_TMPF2}" | parse_changes
	cp -f "${AUPNP_TMPF2}" "${AUPNP_TMPF1}" # avoid removing temporary file
}

cleanup() {
	if bool ${AUPNP_CLEANUP_RM}; then
		printf '' > "${AUPNP_TMPF2}"
		check_changes
	fi
	rm -f "${AUPNP_TMPF1}" "${AUPNP_TMPF2}"
	exit 0
}

main() {
	AUPNP_TMPF1="$(get_tempfile 1)"
	AUPNP_TMPF2="$(get_tempfile 2)"

	if [ ! -w "${AUPNP_TMPF1}" -o ! -w "${AUPNP_TMPF2}" ]; then
		echo 'Unable to create temporary files.' >&2
		exit 1
	fi

	bool ${AUPNP_DISCOVER} && AUPNP_UPNPC_OPTS="$(find_upnp)"

	netstat -nl4 2>/dev/null | parse_output > "${AUPNP_TMPF2}"
	bool ${AUPNP_START_ADD} || cp "${AUPNP_TMPF2}" "${AUPNP_TMPF1}"
	while true; do
		check_changes
		sleep ${AUPNP_INTERVAL}
		netstat -nl4 2>/dev/null | parse_output > "${AUPNP_TMPF2}"
	done
}

trap cleanup HUP INT QUIT TERM
main
