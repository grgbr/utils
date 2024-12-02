#!/usr/bin/env python3
# -*- coding: utf-8 -*-
################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Stroll.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

import sys
import bt2
import collections
import logging
import statistics as stats

class etuxOpDelay:
    def __init__(self):
        self._enter = -1
        self._elapsed = list()
    
    def begin(self, message):
        if self._enter >= 0:
            warnmsg = "ignoring previous '{}' trace: " \
                      "duplicate enter / missed exit event"
            logging.warning(warnmsg.format(message.event.name))

        self._enter = message.default_clock_snapshot.ns_from_origin
        
    def end(self, message):
        if self._enter < 0:
            warnmsg = "skipping unexpected '{}' trace: enter event missed"
            logging.warning(warnmsg.format(message.event.name))
            return

        self._elapsed.append(message.default_clock_snapshot.ns_from_origin -
                             self._enter)
        self._enter = -1

    def stats(self):
        nr = len(self._elapsed)
        if nr == 0:
            return None
        
        mean = stats.mean(self._elapsed)
        
        return (nr,
                min(self._elapsed),
                max(self._elapsed),
                stats.stdev(self._elapsed, mean) if nr > 1 else None,
                stats.median(self._elapsed),
                mean)

    def show(self):
        print(self._elapsed)


class etuxTimerOpDelay:
    def __init__(self):
        self._timers = dict()
        self._elapsed = list()
    
    def _validate(self, message):
        if 'timer_addr' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer address event field"
            raise Exception(errmsg.format(message.event.name))
     
    def begin(self, message):
        self._validate(message)
        
        timer_addr = int(message.event['timer_addr'])
        if timer_addr in self._timers:
            warnmsg = "ignoring previous '{}' trace: " \
                      "duplicate enter / missed exit event"
            logging.warning(warnmsg.format(message.event.name))

        self._timers[timer_addr] = message.default_clock_snapshot.ns_from_origin
        
    def end(self, message):
        self._validate(message)
        
        timer_addr = int(message.event['timer_addr'])
        if timer_addr not in self._timers:
            warnmsg = "skipping unexpected '{}' trace: enter event missed"
            logging.warning(warnmsg.format(message.event.name))
            return

        self._elapsed.append(message.default_clock_snapshot.ns_from_origin -
                             self._timers[timer_addr])
        self._timers.pop(timer_addr)

    def stats(self):
        nr = len(self._elapsed)
        if nr == 0:
            return None
        
        mean = stats.mean(self._elapsed)
        
        return (nr,
                min(self._elapsed),
                max(self._elapsed),
                stats.stdev(self._elapsed, mean) if nr > 1 else None,
                stats.median(self._elapsed),
                mean)

    def show(self):
        print(self._elapsed)


class etuxTimerTraceParser:
    def __init__(self):
        self._arm_tspec_parser = etuxTimerOpDelay()
        self._arm_msec_parser = etuxTimerOpDelay()
        self._arm_sec_parser = etuxTimerOpDelay()
        self._cancel_parser = etuxTimerOpDelay()
        self._issue_tspec_parser = etuxOpDelay()
        self._issue_msec_parser = etuxOpDelay()
        self._run_parser = etuxOpDelay()
        self._expire_parser = etuxTimerOpDelay()
   
    def parse(self, message):
        name = message.event.name
        if name == 'etux_timer:arm_tspec_enter_trcevt':
            self._arm_tspec_parser.begin(message)
        elif name == 'etux_timer:arm_tspec_exit_trcevt':
            self._arm_tspec_parser.end(message)
        elif name == 'etux_timer:arm_msec_enter_trcevt':
            self._arm_msec_parser.begin(message)
        elif name == 'etux_timer:arm_msec_exit_trcevt':
            self._arm_msec_parser.end(message)
        elif name == 'etux_timer:arm_sec_enter_trcevt':
            self._arm_sec_parser.begin(message)
        elif name == 'etux_timer:arm_sec_exit_trcevt':
            self._arm_sec_parser.end(message)
        elif name == 'etux_timer:cancel_enter_trcevt':
            self._cancel_parser.begin(message)
        elif name == 'etux_timer:cancel_exit_trcevt':
            self._cancel_parser.end(message)
        elif name == 'etux_timer:issue_tspec_enter_trcevt':
            self._issue_tspec_parser.begin(message)
        elif name == 'etux_timer:issue_tspec_exit_trcevt':
            self._issue_tspec_parser.end(message)
        elif name == 'etux_timer:issue_msec_enter_trcevt':
            self._issue_msec_parser.begin(message)
        elif name == 'etux_timer:issue_msec_exit_trcevt':
            self._issue_msec_parser.end(message)
        elif name == 'etux_timer:run_enter_trcevt':
            self._run_parser.begin(message)
        elif name == 'etux_timer:run_exit_trcevt':
            self._run_parser.end(message)
        elif name == 'etux_timer:expire_enter_trcevt':
            self._expire_parser.begin(message)
        elif name == 'etux_timer:expire_exit_trcevt':
            self._expire_parser.end(message)
        else:
            warnmsg = "ignoring unexpected '{}' trace: not supported."
            logging.warning(warnmsg.format(name))

    def show(self):
        print("arm tspec", end = ": ")
        print(self._arm_tspec_parser.stats())
        print("arm msec", end = ": ")
        print(self._arm_msec_parser.stats())
        print("arm sec", end = ": ")
        print(self._arm_sec_parser.stats())
        print("cancel", end = ": ")
        print(self._cancel_parser.stats())
        print("issue tspec", end = ": ")
        print(self._issue_tspec_parser.stats())
        print("issue msec", end = ": ")
        print(self._issue_msec_parser.stats())
        print("run", end = ": ")
        print(self._run_parser.stats())
        print("expire", end = ": ")
        print(self._expire_parser.stats())


class etuxTraceScanner:
    def __init__(self, path):
        self._iter = bt2.TraceCollectionMessageIterator(path)

    def parse(self, parser):
        for msg in self._iter:
            if type(msg) is not bt2._EventMessageConst:
                    continue

            parser.parse(msg)


scanner = etuxTraceScanner(sys.argv[1])
parser = etuxTimerTraceParser()
scanner.parse(parser)
parser.show()

## Get the message iterator's first stream beginning message.
#for msg in iter:
#    # `bt2._StreamBeginningMessageConst` is the Python type of a stream
#    # beginning message.
#    if type(msg) is bt2._StreamBeginningMessageConst:
#        break
#
## A stream beginning message holds a stream.
#stream = msg.stream
#
## Get the stream's class.
#stream_class = stream.cls
#
## The stream class object offers a mapping interface (like a read-only
## `dict`), where keys are event class IDs and values are
## `bt2._EventClassConst` objects.
#for event_class in stream_class.values():
#    print('{}:'.format(event_class.name))
#
#    # The `payload_field_class` property of an event class returns a
#    # `bt2._StructureFieldClassConst` object. This object offers a
#    # mapping interface, where keys are member names and values are
#    # `bt2._StructureFieldClassMemberConst` objects.
#    for member in event_class.payload_field_class.values():
#        fmt = '  {}: `{}.{}`'
#        print(fmt.format(member.name, bt2.__name__,
#                         member.field_class.__class__.__name__))
