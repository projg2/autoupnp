#!/bin/sh
# AutoUPnP
# automatically forward ports on open and remove forwardings on close
# (C) 2009 Michał Górny, 3-clause BSD license

# Dependencies: netstat, diff, awk, miniupnpc (http://miniupnp.free.fr/)

# --config--

# awk pattern to filter out unwanted listeners
# e.g. ones bound to local iface
CONF_FILTER='$4 == "0.0.0.0"'

# NOTE: below '1' means true and all other values mean false

# try to discover UPnP IGD on start to speedup further calls
# change if it fails to get correct URI
CONF_DISCOVER=1

# add already open ports on start
CONF_START_ADD=1

# remove redirected ports on exit
CONF_CLEANUP_RM=1

# --/config--

MY_PN=autoupnp
MY_PV=0.2

get_tempfile() {
	if ! mktemp 2>/dev/null; then
		local tmpfn
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
		($1 == "tcp" || $1 == "udp") && ('"${CONF_FILTER}"') { print $5 " " $1 }
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
				if (commands[i])
					system("upnpc '"${UPNPC_OPTS}"' " commands[i])
			}
		}
	'
}

check_changes() {
	diff "${TMPF1}" "${TMPF2}" | parse_changes
	cp -f "${TMPF2}" "${TMPF1}" # avoid removing temporary file
}

cleanup() {
	if [ ${CONF_CLEANUP_RM} -eq 1 ]; then
		printf '' > "${TMPF2}"
		check_changes
	fi
	rm -f "${TMPF1}" "${TMPF2}"
	exit 0
}

main() {
	TMPF1="$(get_tempfile 1)"
	TMPF2="$(get_tempfile 2)"

	if [ ! -w "${TMPF1}" -o ! -w "${TMPF2}" ]; then
		echo 'Unable to create temporary files.' >&2
		exit 1
	fi

	[ ${CONF_DISCOVER} -eq 1 ] && UPNPC_OPTS="$(find_upnp)"

	netstat -nl4 2>/dev/null | parse_output > "${TMPF2}"
	[ ${CONF_START_ADD} -eq 1 ] || cp "${TMPF2}" "${TMPF1}"
	while true; do
		check_changes
		sleep 3
		netstat -nl4 2>/dev/null | parse_output > "${TMPF2}"
	done
}

trap cleanup HUP INT QUIT TERM
main
