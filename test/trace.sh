#!/bin/sh -e

session='etux-session'
events='etux_timer:*'

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

log_err()
{
	local msg="$1"

	echo "$0: $msg" >&2
}

log_info()
{
	local msg="$1"

	echo "$msg" >&2
}

# Create an empty session
if ! lttng --quiet list $session; then
	log_info "creating '$session' session..."

	# Instantiate session
	if ! lttng --quiet create $session; then
		log_err "failed to create '$session' session."
		exit 1
	fi
	# Setup context fields to record with tracepoint events
	if ! lttng --quiet \
	           add-context --userspace \
	                       --session=$session \
	                       $(echo "$fields" | \
	                         sed 's/\([^[:blank:]]\+\)/--type=\1/g'); then
		log_err "failed to setup '$session' session context fields."
		exit 1
	fi
	# Setup tracepoint events and start recording
	if ! lttng --quiet \
	           enable-event --userspace --session $session $events; then
		log_err "failed to enable '$session' session trace events."
		exit 1
	fi
fi
if ! lttng --quiet clear $session; then
	log_err "failed to clear '$session' session previous traces."
	exit 1
fi

log_info "starting '$session' session..."
if ! lttng --quiet start $session; then
	log_err "failed to start '$session' session."
	exit 1
fi

# Run command to trace
log_info "running '$1' command..."
$1

# Stop recording session
log_info "stopping '$session' session..."
if ! lttng --quiet regenerate metadata --session=etux-session; then
	log_err "failed to regenerate '$session' session metadata."
	exit 1
fi
if ! lttng --quiet stop $session; then
	log_err "failed to stop '$session' session."
	exit 1
fi

# Show session informations and collected traces
log_info ""
if ! lttng list $session; then
	log_err "failed to show '$session' session informations."
	exit 1
fi
if ! lttng --quiet view $session; then
	log_err "failed to show '$session' session traces."
	exit 1
fi
