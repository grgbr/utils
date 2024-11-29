#!/bin/sh -e

session='etux-session'
events='etux:*'

# Context fields added to each recorded trace events. As of 2.13.4, available
# userspace domain fields are:
#    procname
#    vpid
#    pthread_id
#    vtid
#    ip
#    cgroup_ns
#    ipc_ns
#    mnt_ns
#    net_ns
#    pid_ns
#    time_ns
#    user_ns
#    uts_ns
#    vuid
#    veuid
#    vsuid
#    vgid
#    vegid
#    vsgid
fields='procname vpid vtid vuid veuid vsuid vgid vegid vsgid'

# Create an empty session
if ! lttng list $session; then
	# Instantiate session
	lttng create $session
	# Setup context fields to record with tracepoint events
	lttng add-context --userspace \
	                  --session=$session \
	                  $(echo "$fields" | \
	                    sed 's/\([^[:blank:]]\+\)/--type=\1/g')
	# Setup tracepoint events and start recording
	lttng enable-event --userspace --session $session $events
fi
lttng clear $session

lttng start $session

# Run command to trace
$1

# Stop recording session
lttng regenerate metadata --session=etux-session
lttng stop $session

# Show session informations and collected traces
lttng list $session
lttng view $session
