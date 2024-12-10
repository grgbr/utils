#!/usr/bin/env python3
# -*- coding: utf-8 -*-
################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Stroll.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

from typing import Literal

import sys

import bt2
import collections
import logging
import statistics as stats
from rich.table import Table, Column
from rich.padding import PaddingDimensions
from rich.console import Console, RenderableType

class etuxTimerTraceStat:
    def __init__(self, data: list[float]) -> None:
        self._data = data
        self._nr = len(data)
        if self._nr != 0:
            self._min = None
            self._max = None
            if self._nr > 1:
                self._stdev = None
            else:
                self._stdev = 0.0
            self._median = None
            self._mean = None
            self._sum = None
        else:
            self._min = 0.0
            self._max = 0.0
            self._stdev = 0.0
            self._median = 0.0
            self._mean = 0.0
            self._sum = 0.0

    @property
    def data(self) -> list[float]:
        return self._data

    @property
    def count(self) -> int:
        return self._nr

    @property
    def min(self) -> float:
        if self._min is None:
            self._min = min(self._data)
        return self._min

    @property
    def max(self) -> float:
        if self._max is None:
            self._max = max(self._data)
        return self._max

    @property
    def max(self) -> float:
        if self._max is None:
            self._max = max(self._data)
        return self._max

    @property
    def mean(self) -> float:
        if self._mean is None:
            self._mean = stats.mean(self._data)
        return self._mean

    @property
    def stdev(self) -> float:
        if self._stdev is None:
            self._stdev = stats.stdev(self._data, self.mean)
        return self._stdev

    @property
    def median(self) -> float:
        if self._median is None:
            self._median = stats.median(self._data)
        return self._median

    @property
    def sum(self) -> float:
        if self._sum is None:
            self._sum = sum(self._data)
        return self._sum


class etuxTimerTraceElapseBase:
    def __init__(self) -> None:
        self._elapsed = list()

    @property
    def _mark(self) -> int:
        return len(self._elapsed)
    
    def _samples(self, mark: int) -> list[int]:
        assert mark >= 0
        return self._elapsed[mark:-1]
    
    def stats(self) -> etuxTimerTraceStat:
        return etuxTimerTraceStat(
            list(map(lambda nsec: nsec / 1000.0, self._elapsed)))


class etuxTimerTraceKeyedElapse(etuxTimerTraceElapseBase):
    def __init__(self) -> None:
        super().__init__()
        self._timers = dict()

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


class etuxTimerTraceElapse(etuxTimerTraceElapseBase):
    def __init__(self):
        super().__init__()
        self._enter = -1

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


class etuxTimerTraceRunElapse(etuxTimerTraceElapse):
    def __init__(self, expire: etuxTimerTraceKeyedElapse):
        super().__init__()
        self._expire = expire
        self._expire_mark = -1

    def begin(self, message):
        super().begin(message)
        self._expire_mark = self._expire._mark

    def end(self, message):
        if self._enter < 0:
            warnmsg = "skipping unexpected '{}' trace: enter event missed"
            logging.warning(warnmsg.format(message.event.name))
            return

        nsec = message.default_clock_snapshot.ns_from_origin - self._enter
        self._elapsed.append(nsec -
                             sum(self._expire._samples(self._expire_mark)))
        self._enter = -1


class etuxTimerTraceParser:
    def __init__(self):
        self._arm_tspec_parser = etuxTimerTraceKeyedElapse()
        self._arm_msec_parser = etuxTimerTraceKeyedElapse()
        self._arm_sec_parser = etuxTimerTraceKeyedElapse()
        self._cancel_parser = etuxTimerTraceKeyedElapse()
        self._issue_tspec_parser = etuxTimerTraceElapse()
        self._issue_msec_parser = etuxTimerTraceElapse()
        self._expire_parser = etuxTimerTraceKeyedElapse()
        self._run_parser = etuxTimerTraceRunElapse(self._expire_parser)

        self._arm_tspec_stats = None
        self._arm_msec_stats = None
        self._arm_sec_stats = None
        self._arm_stats = None
        self._cancel_stats = None
        self._issue_tspec_stats = None
        self._issue_msec_stats = None
        self._issue_stats = None
        self._run_stats = None
        self._expire_stats = None
        self._total_stats = None

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

    @property
    def arm_tspec_stats(self) -> etuxTimerTraceStat:
        if self._arm_tspec_stats is None:
            self._arm_tspec_stats = self._arm_tspec_parser.stats()
        return self._arm_tspec_stats

    @property
    def arm_msec_stats(self) -> etuxTimerTraceStat:
        if self._arm_msec_stats is None:
            self._arm_msec_stats = self._arm_msec_parser.stats()
        return self._arm_msec_stats

    @property
    def arm_sec_stats(self) -> etuxTimerTraceStat:
        if self._arm_sec_stats is None:
            self._arm_sec_stats = self._arm_sec_parser.stats()
        return self._arm_sec_stats

    @property
    def arm_stats(self) -> etuxTimerTraceStat:
        if self._arm_stats is None:
            self._arm_stats = etuxTimerTraceStat(self.arm_tspec_stats.data +
                                                 self.arm_msec_stats.data +
                                                 self.arm_sec_stats.data)
        return self._arm_stats

    @property
    def cancel_stats(self) -> etuxTimerTraceStat:
        if self._cancel_stats is None:
            self._cancel_stats = self._cancel_parser.stats()
        return self._cancel_stats

    @property
    def issue_tspec_stats(self) -> etuxTimerTraceStat:
        if self._issue_tspec_stats is None:
            self._issue_tspec_stats = self._issue_tspec_parser.stats()
        return self._issue_tspec_stats

    @property
    def issue_msec_stats(self) -> etuxTimerTraceStat:
        if self._issue_msec_stats is None:
            self._issue_msec_stats = self._issue_msec_parser.stats()
        return self._issue_msec_stats

    @property
    def issue_stats(self) -> etuxTimerTraceStat:
        if self._issue_stats is None:
            self._issue_stats = etuxTimerTraceStat(self.issue_tspec_stats.data +
                                                   self.issue_msec_stats.data)
        return self._issue_stats

    @property
    def expire_stats(self) -> etuxTimerTraceStat:
        if self._expire_stats is None:
            self._expire_stats = self._expire_parser.stats()
        return self._expire_stats

    @property
    def run_stats(self) -> etuxTimerTraceStat:
        if self._run_stats is None:
            self._run_stats = self._run_parser.stats()
        return self._run_stats

    @property
    def total_stats(self) -> etuxTimerTraceStat:
        if self._total_stats is None:
            self._total_stats = etuxTimerTraceStat(self.arm_tspec_stats.data +
                                                   self.arm_msec_stats.data +
                                                   self.arm_sec_stats.data +
                                                   self.cancel_stats.data +
                                                   self.issue_tspec_stats.data +
                                                   self.issue_msec_stats.data +
                                                   self.run_stats.data)
        return self._total_stats


    def show(self):
        def _show(op, sts, tot):
            print("{:12.12s} {:8.3f} {:8.3f} {:8.3f} {:8.3f} {:8.3f} {:8.3f}/{:8.3f} {:6d}/{:6d}".format(
                  op,
                  sts.min,
                  sts.max,
                  sts.stdev,
                  sts.median,
                  sts.mean,
                  sts.sum / 1000.0,
                  tot.sum / 1000.0,
                  sts.count,
                  tot.count))
            
        print("OPERATION         MIN      MAX   STDDEV     MEDIAN   MEAN        MSEC/TOTAL    #NR/#TOTAL")
        print("               (usec)   (usec)   (usec)     (usec) (usec)            (msec)")
        print("=========================================================================================")
        
        tot_sts = self.total_stats
        _show("arm tspec", self.arm_tspec_stats, tot_sts)
        _show("arm msec", self.arm_msec_stats, tot_sts)
        _show("arm sec", self.arm_sec_stats, tot_sts)
        print("-----------------------------------------------------------------------------------------")
        _show("arm", self.arm_stats, tot_sts)
        print("=========================================================================================")
        _show("issue tspec", self.issue_tspec_stats, tot_sts)
        _show("issue msec", self.issue_msec_stats, tot_sts)
        print("-----------------------------------------------------------------------------------------")
        _show("issue", self.issue_stats, tot_sts)
        print("=========================================================================================")
        _show("arm", self.arm_stats, tot_sts)
        _show("cancel", self.cancel_stats, tot_sts)
        _show("issue", self.issue_stats, tot_sts)
        _show("run", self.run_stats, tot_sts)
        print("=========================================================================================")
        _show("total", tot_sts, tot_sts)



class etuxTimerTraceTableColumn(Column):
    def __init__(self,
                 header: str = '',
                 footer: RenderableType = '',
                 justify: Literal['default',
                                  'left',
                                  'center',
                                  'right',
                                  'full'] = 'left',
                 wrap: bool = False) -> None:
        super().__init__(header = header,
                         footer = footer,
                         justify = justify,
                         no_wrap = not wrap)


class etuxTimerTraceTable(Table):
    def __init__(self,
                 *columns: Column,
                 title: str,
                 show_header: bool = False,
                 padding: PaddingDimensions = (0, 0, 0, 1)) -> None:
        ttl = etuxTimerTraceSectionTitle(title)
        super().__init__(*columns,
                         title = ttl,
                         title_justify = 'left',
                         box = None,
                         show_header = show_header,
                         show_edge = False,
                         pad_edge = False,
                         min_width = ttl.width,
                         padding = padding)


class etuxTimerTraceReport(Table):
    def __init__(self, parser: etuxTimerTraceParser) -> None:
        super().__init__(etuxTimerTraceTableColumn(header = 'MIN'),
                         etuxTimerTraceTableColumn(header = 'MAX'),
                         etuxTimerTraceTableColumn(header = 'STDDEV'),
                         etuxTimerTraceTableColumn(header = 'MEDIAN'),
                         etuxTimerTraceTableColumn(header = 'MEAN'),
                         etuxTimerTraceTableColumn(header = 'DURATION'),
                         etuxTimerTraceTableColumn(header = '#NR'),
                         title = 'Timer statistics',
                         show_header = True)

# Report
#
# Timer arm operations
# --------------------
#
# OPERATION       MIN     MAX   STDDEV   MEDIAN     MEAN  MSEC/TOTAL_MSEC  #NR/#TOTAL_NR
#              (nsec)  (nsec)   (nsec)   (nsec)   (nsec)           (msec)
# arm tspec       204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# arm msec        204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# arm sec         204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# --------------------------------------------------------------------------------------
# Total           204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
#
# Timer issue operations
# ----------------------
#
# OPERATION       MIN     MAX   STDDEV   MEDIAN     MEAN  MSEC/TOTAL_MSEC  #NR/#TOTAL_NR
#              (nsec)  (nsec)   (nsec)   (nsec)   (nsec)           (msec)
# issue tspec     204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# issue msec      204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# --------------------------------------------------------------------------------------
# Total           204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
#
#
# All operations
# --------------
#
# OPERATION       MIN     MAX   STDDEV   MEDIAN     MEAN  MSEC/TOTAL_MSEC  #NR/#TOTAL_NR
#              (nsec)  (nsec)   (nsec)   (nsec)   (nsec)           (msec)
# arm             204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# cancel          204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# issue           204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# run             204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%
# --------------------------------------------------------------------------------------
# Total           204   16180  189.260  592.000  593.628    22.119/28.768    37260/74671
#                                                                  76.00%            49%




class etuxTimerTraceScanner:
    def __init__(self, path: str) -> None:
        self._iter = bt2.TraceCollectionMessageIterator(path)

    def parse(self, parser: etuxTimerTraceParser) -> None:
        for msg in self._iter:
            if type(msg) is not bt2._EventMessageConst:
                    continue

            parser.parse(msg)


scanner = etuxTimerTraceScanner(sys.argv[1])
parser = etuxTimerTraceParser()
scanner.parse(parser)
parser.show()

Console().print(etuxTimerTraceReport(parser))

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
