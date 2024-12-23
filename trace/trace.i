/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#undef LTTNG_UST_TRACEPOINT_PROVIDER
#define LTTNG_UST_TRACEPOINT_PROVIDER etux

#undef LTTNG_UST_TRACEPOINT_INCLUDE
#define LTTNG_UST_TRACEPOINT_INCLUDE "trace.i"

#if !defined(_ETUX_TRACE_I) || \
    defined(LTTNG_UST_TRACEPOINT_HEADER_MULTI_READ)
#define _ETUX_TRACE_I

#include <lttng/tracepoint.h>

LTTNG_UST_TRACEPOINT_EVENT_CLASS(
	etux,
	void_trccls,
	LTTNG_UST_TP_ARGS(void),
)

#define ETUX_VOID_TRACEPOINT_EVENT(_provider, _event) \
	LTTNG_UST_TRACEPOINT_EVENT_INSTANCE( \
		etux, \
		void_trccls, \
		_provider, \
		_event, \
		LTTNG_UST_TP_ARGS(void) \
	)

#define ETUX_TRACE_ENUM_VALUE(_val) \
	lttng_ust_field_enum_value(STROLL_STRING(_val), _val)

#endif /* _ETUX_TRACE_I */

#include <lttng/tracepoint-event.h>

/* ex: set filetype=c : */
