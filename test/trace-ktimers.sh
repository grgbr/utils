################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Stroll.
# Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

#!/bin/sh -ex

events='
timer:timer_cancel
timer:timer_expire_exit
timer:timer_expire_entry
timer:timer_start
timer:timer_init
'

orig_tstamp_clock=$(sed 's/.*\[\(.*\)\].*/\1/' \
                    /sys/kernel/tracing/trace_clock)

ECHOE="/bin/echo -e"

error()
{
	$ECHOE "$arg0: $*" >&2
}

log()
{
	$ECHOE "$*" >&2
}

tracing_off()
{
	trap - EXIT

	log 'Turning tracing off...'

	# Stop tracing
	echo 0 > /sys/kernel/tracing/tracing_on

	# Clear all event traces
	echo > /sys/kernel/tracing/set_event

	# Clear all event trace filters
	find /sys/kernel/tracing/events \
	     -maxdepth 2 \
	     -name "filter" \
	     -exec sh -c "echo 0 > {}" \;

	echo $orig_tstamp_clock > /sys/kernel/tracing/trace_clock

	sync
}

parse_uint()
{
	local arg="$1"
	local msg="$2"
	local val

	val=$(echo "$arg" | sed -n "s/[0-9]*//gp")
	if [ -n "$val" ]; then
		error "$msg '$arg': integer expected.\n"
		return 1
	fi
	if [ $((arg)) -lt 1 ]; then
		error "$msg '$arg': integer >= 1.\n"
		return 1
	fi

	echo $arg
}

usage()
{
	cat >&2 <<_EOF
Usage: $arg0 [OPTIONS] [FILE]
Collect kernel timer trace events (see <linux>/Documentation/trace/events.rst).

Where OPTIONS:
    -c | --count COUNT    stop collecting after COUNT traces (defaults to unlimited)
    -s | --second SECONDS stop collecting after SECONDS seconds (defaults to unlimited)
    -h | --help           this help message
With:
    FILE    -- a pathname to file where to store collected traces (defaults to stdout)
    COUNT   -- maximum count of kernel timer traces
    SECONDS -- maximum number of seconds after which to stop collection
_EOF
}

arg0="$(basename $0)"

cmdln=$(getopt --options c:s:lh \
               --longoptions count:,second:,help \
               --name "$arg0" \
               -- "$@")
if [ $? -gt 0 ]; then
	usage
	exit 1
fi

cnt=0
secs=0
out='/dev/stdout'
eval set -- "$cmdln"
while true; do
	case "$1" in
	-c|--count)
		if ! cnt=$(parse_uint "$2" "invalid count of traces"); then
			usage
			exit 1
		fi
		shift 2;;
	-s|--second)
		if ! secs=$(parse_uint "$2" "invalid number of seconds"); then
			usage
			exit 1
		fi
		shift 2;;
	-h|--help)
		usage
		exit 0;;
	--)
		shift;
		break;;
	*)
		break;;
	esac
done

if [ $# -gt 1 ]; then
	error 'invalid number of arguments.\n'
	usage
	exit 1
elif [ $# -eq 1 ]; then
	out="$1"
fi

# Turn tracing off and clear trace buffer
tracing_off
echo > /sys/kernel/tracing/trace

trap 'tracing_off' EXIT INT QUIT TERM HUP

log 'Turning tracing on...'
# Enable timer related event traces
echo ${events} > /sys/kernel/tracing/set_event
# Use global system (cross CPU) trace event timestamping clock
echo global > /sys/kernel/tracing/trace_clock
# Start tracing
echo 1 > /sys/kernel/tracing/tracing_on

if [ $cnt -gt 0 ] || [ $secs -gt 0 ]; then
	: > $out
	if [ $secs -gt 0 ]; then
		end=$(($SECONDS + $secs))
	else
		end=0
	fi
	cat /sys/kernel/tracing/trace_pipe | \
	while [ 1 ]; do
		if [ $end -gt 0 ]; then
			tmout=$(($end - $SECONDS))
			if [ $tmout -le 0 ]; then
				break
			fi
			if ! read -t $tmout line; then
				break
			fi
		else
			if ! read line; then
				break
			fi
		fi
		echo "$line" >> $out
		if [ $cnt -gt 0 ]; then
			cnt=$((cnt - 1))
			if [ $cnt -le 0 ]; then
				break
			fi
		fi
	done
else
	# Keep displaying trace buffer content forever...
	log 'Collecting traces (hit ^C to stop) ...'
	cat /sys/kernel/tracing/trace_pipe > $out
fi
