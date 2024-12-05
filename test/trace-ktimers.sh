#!/bin/sh -xe

events='
timer:timer_cancel
timer:timer_expire_exit
timer:timer_expire_entry
timer:timer_start
timer:timer_init
'

orig_tstamp_clock=$(sed 's/.*\[\(.*\)\].*/\1/' \
                    /sys/kernel/tracing/trace_clock)

tracing_off()
{
	trap - EXIT

	echo 'Turning tracing off...' >&2

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

# Turn tracing off and clear trace buffer
tracing_off
echo > /sys/kernel/tracing/trace

trap 'tracing_off' EXIT INT QUIT TERM HUP

echo 'Turning tracing on...' >&2
# Enable timer related event traces
echo ${events} > /sys/kernel/tracing/set_event
# Use global system (cross CPU) trace event timestamping clock
echo global > /sys/kernel/tracing/trace_clock
# Start tracing
echo 1 > /sys/kernel/tracing/tracing_on

# Keep displaying trace buffer content forever...
echo 'Collecting traces (hit ^C to stop) ...' >&2
cat /sys/kernel/tracing/trace_pipe > $1
