/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of Utils.
 * Copyright (C) 2017-2024 Grégor Boirie <gregor.boirie@free.fr>
 ******************************************************************************/

#ifndef _ETUX_TRACE_H
#define _ETUX_TRACE_H

#define ETUX_TRACE_VOID_CLASS(_provider) \
	LTTNG_UST_TRACEPOINT_EVENT_CLASS( \
		_provider, \
		void_trccls, \
		LTTNG_UST_TP_ARGS(void), \
	)

#define ETUX_TRACE_VOID_EVENT(_provider, _event) \
	LTTNG_UST_TRACEPOINT_EVENT_INSTANCE( \
		_provider, \
		void_trccls, \
		_provider, \
		_event, \
		LTTNG_UST_TP_ARGS(void) \
	)

#define ETUX_TRACE_ENUM_VALUE(_val) \
	lttng_ust_field_enum_value(STROLL_STRING(_val), _val)

#endif /* _ETUX_TRACE_H */

/* ex: set filetype=c : */
