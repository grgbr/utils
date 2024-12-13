#!/usr/bin/env python3
# -*- coding: utf-8 -*-
################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Stroll.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

import os
import sys
import pprint

arg0 = os.path.basename(sys.argv[0])

try:
    from typing import Literal, Optional
    from argparse import ArgumentParser
    from bt2 import TraceCollectionMessageIterator as bt2TraceCollectionMessageIterator
    from bt2.field_class import _StructureFieldClassMemberConst as bt2StructureFieldClassMemberConst
    from bt2.event_class import _EventClassConst as bt2EventClassConst
    from bt2.message import _EventMessageConst as bt2EventMessageConst
    from bt2.message import _StreamBeginningMessageConst as bt2StreamBeginningMessageConst
    import collections
    import logging
    import statistics as stats
    from rich.table import Table, Column
    from rich.padding import Padding, PaddingDimensions
    from rich.console import Console, RenderableType
    from rich.text import Text
    from rich.style import Style
    from rich.box import Box
    from rich.progress import Progress
except Exception as e:
    print("{}: Python module import failed: {}.".format(arg0, e),
          file=sys.stderr)
    exit(1)


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


#class etuxTimerTraceContext:
#    def __init__(self) -> None:
#        self._props = {
#            'cpu_id':   set(),
#            'procname': set(),
#            'vpid':     set(),
#            'vtid':     set(),
#            'vuid':     set(),
#            'veuid':    set(),
#            'vsuid':    set(),
#            'vgid':     set(),
#            'vegid':    set(),
#            'vsgid':    set(),
#            'vuid':     set()
#        }
#    
#    def update(self, message: bt2EventMessageConst) -> None:
#        if 'cpu_id' in message.event.packet.context_field:
#        for v in message.event.common_context_field.keys():
#        #    print(v)
#        #pprint.pprint(message.event.packet.context_field.keys())
        

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

    def _validate(self, message: bt2EventMessageConst) -> None:
        if 'timer_addr' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer address event field"
            raise Exception(errmsg.format(message.event.name))

    def begin(self, message: bt2EventMessageConst) -> None:
        self._validate(message)

        timer_addr = int(message.event['timer_addr'])
        if timer_addr in self._timers:
            warnmsg = "ignoring previous '{}' trace: " \
                      "duplicate enter / missed exit event"
            logging.warning(warnmsg.format(message.event.name))

        self._timers[timer_addr] = message.default_clock_snapshot.ns_from_origin

    def end(self, message: bt2EventMessageConst) -> None:
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
    def __init__(self) -> None:
        super().__init__()
        self._enter = -1

    def begin(self, message: bt2EventMessageConst) -> None:
        if self._enter >= 0:
            warnmsg = "ignoring previous '{}' trace: " \
                      "duplicate enter / missed exit event"
            logging.warning(warnmsg.format(message.event.name))

        self._enter = message.default_clock_snapshot.ns_from_origin

    def end(self, message: bt2EventMessageConst) -> None:
        if self._enter < 0:
            warnmsg = "skipping unexpected '{}' trace: enter event missed"
            logging.warning(warnmsg.format(message.event.name))
            return

        self._elapsed.append(message.default_clock_snapshot.ns_from_origin -
                             self._enter)
        self._enter = -1


class etuxTimerTraceExpireElapse(etuxTimerTraceKeyedElapse):
    def __init__(self) -> None:
        super().__init__()
        self._latency = list()

    def begin(self, message: bt2EventMessageConst) -> None:
        super().begin(message)
        if 'now_sec' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer event 'now_sec' field"
            raise Exception(errmsg.format(message.event.name))
        if 'now_nsec' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer event 'now_nsec' field"
            raise Exception(errmsg.format(message.event.name))
        if 'timer_sec' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer event 'timer_sec' field"
            raise Exception(errmsg.format(message.event.name))
        if 'timer_nsec' not in message.event.payload_field:
            errmsg = "invalid '{}' trace: missing timer event 'timer_nsec' field"
            raise Exception(errmsg.format(message.event.name))

        lat_sec = int(message.event['now_sec']) - \
                  int(message.event['timer_sec'])
        lat_nsec = int(message.event['now_nsec']) - \
                   int(message.event['timer_nsec'])
        if lat_nsec < 0:
            lat_sec -= 1
            lat_nsec += 1000000000

        self._latency.append((lat_sec * 1000000000) + lat_nsec)

    def latency_stats(self) -> etuxTimerTraceStat:
        return etuxTimerTraceStat(
            list(map(lambda nsec: nsec / 1000.0, self._latency)))


class etuxTimerTraceRunElapse(etuxTimerTraceElapse):
    def __init__(self, expire: etuxTimerTraceExpireElapse):
        super().__init__()
        self._expire = expire
        self._expire_mark = -1

    def begin(self, message: bt2EventMessageConst) -> None:
        super().begin(message)
        self._expire_mark = self._expire._mark

    def end(self, message: bt2EventMessageConst) -> None:
        if self._enter < 0:
            warnmsg = "skipping unexpected '{}' trace: enter event missed"
            logging.warning(warnmsg.format(message.event.name))
            return

        nsec = message.default_clock_snapshot.ns_from_origin - self._enter
        self._elapsed.append(nsec -
                             sum(self._expire._samples(self._expire_mark)))
        self._enter = -1


class etuxTimerTraceParser:
    def __init__(self) -> None:
        self._arm_tspec_parser = etuxTimerTraceKeyedElapse()
        self._arm_msec_parser = etuxTimerTraceKeyedElapse()
        self._arm_sec_parser = etuxTimerTraceKeyedElapse()
        self._cancel_parser = etuxTimerTraceKeyedElapse()
        self._issue_tspec_parser = etuxTimerTraceElapse()
        self._issue_msec_parser = etuxTimerTraceElapse()
        self._expire_parser = etuxTimerTraceExpireElapse()
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
        self._expire_latency_stats = None
        self._total_stats = None

    def parse(self, message: bt2EventMessageConst) -> None:
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
    def expire_latency_stats(self) -> etuxTimerTraceStat:
        if self._expire_latency_stats is None:
            self._expire_latency_stats = self._expire_parser.latency_stats()
        return self._expire_latency_stats

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


class etuxTimerTraceAttrDesc:
    def __init__(self, member: bt2StructureFieldClassMemberConst) -> None:
        self._member = member

    def __lt__(self, obj):
        return self.name < obj.name

    def __gt__(self, obj):
        return self.name > obj.name

    def __le__(self, obj):
        return self.name <= obj.name

    def __ge__(self, obj):
        return self.name >= obj.name

    def __eq__(self, obj):
        return self.name == obj.name

    def __hash__(self):
        return hash(self.name)

    @property
    def name(self) -> str:
        return self._member.name

    @property
    def kind(self) -> str:
        return self._member.field_class.__class__._NAME.lower()


class etuxTimerTraceDesc:
    def __init__(self, event_class: bt2EventClassConst) -> None:
        self._event_class = event_class
        self._attrs = set()

        stream_class = event_class.stream_class
        
        for attr in stream_class.packet_context_field_class.values():
            self._attrs.add(etuxTimerTraceAttrDesc(attr))
        for attr in stream_class.event_common_context_field_class.values():
            self._attrs.add(etuxTimerTraceAttrDesc(attr))
        for attr in stream_class.user_attributes.values():
            self._attrs.add(etuxTimerTraceAttrDesc(attr))

        # The `payload_field_class` property of an event class returns a
        # `bt2._StructureFieldClassConst` object. This object offers a
        # mapping interface, where keys are attribute names and values are
        # `bt2._StructureFieldClassMemberConst` objects.
        for attr in event_class.payload_field_class.values():
            self._attrs.add(etuxTimerTraceAttrDesc(attr))

    def __lt__(self, obj):
        return self.name < obj.name

    def __gt__(self, obj):
        return self.name > obj.name

    def __le__(self, obj):
        return self.name <= obj.name

    def __ge__(self, obj):
        return self.name >= obj.name

    def __eq__(self, obj):
        return self.name == obj.name

    def __hash__(self):
        return hash(self.name)

    @property
    def name(self) -> str:
        return self._event_class.name

    @property
    def attrs(self) -> set[etuxTimerTraceAttrDesc]:
        return self._attrs


class etuxTimerTraceScanner:
    def __init__(self, path: str) -> None:
        self._path = path
        self._count = -1

    def _iter(self) -> bt2TraceCollectionMessageIterator:
        try:
            iter = bt2TraceCollectionMessageIterator(self._path)
        except Exception as e:
            raise Exception("trace iteration failed: {}".format(e))
        return iter

    @property
    def count(self) -> int:
        if self._count < 0:
            self._count = 0
            iter = self._iter()
            try:
                for msg in iter:
                    self._count += 1
            except Exception as e:
                raise Exception("'{}': counting traces failed: "
                                "{}".format(self._path, e))
        return self._count

    def parse(self, parser: etuxTimerTraceParser, observer = None) -> None:
        iter = self._iter()
        try:
            for msg in iter:
                if observer is not None:
                    observer.update()
                if type(msg) is not bt2EventMessageConst:
                        continue

                parser.parse(msg)
        except Exception as e:
            raise Exception("'{}': parsing failed: {}".format(self._path, e))

    def desc(self) -> set[etuxTimerTraceDesc]:
        evts = set()

        # Get the message iterator's first stream beginning message.
        iter = self._iter()
        try:
            for msg in iter:
                # `bt2._StreamBeginningMessageConst` is the Python type of a
                # stream beginning message.
                if type(msg) is bt2StreamBeginningMessageConst:
                    break

            # Retrieve class of beginning message stream which offers
            # a mapping interface (like a read-only `dict`), where keys are
            # event class IDs and values are `bt2._EventClassConst` objects.
            for ecls in msg.stream.cls.values():
                evts.add(etuxTimerTraceDesc(ecls))
        except Exception as e:
            raise Exception("'{}': trace discovery failed: "
                            "{}".format(self._path, e))

        return evts


class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args,
                                                                 **kwargs)
        return cls._instances[cls]


class etuxTimerTraceConsole(Console, metaclass = Singleton):
    def __init__(self) -> None:
        super().__init__(legacy_windows = True, no_color = True)


class eTuxTimerTraceProgress(Progress):
    def __init__(self) -> None:
        super().__init__(auto_refresh = False,
                         transient = True,
                         console = etuxTimerTraceConsole())
        self._step = 1

    def start(self, total: int) -> None:
        self._tid = self.add_task("Loading traces", total = total)
        self._step = int(total / 100)
        super().start()

    def update(self) -> None:
        super().advance(self._tid)
        task = self.tasks[self._tid]
        if task.completed % self._step == 0:
            self.refresh()


class etuxTimerTraceTableColumn(Column):
    def __init__(self,
                 header: str = '',
                 justify: Literal['default',
                                  'left',
                                  'center',
                                  'right',
                                  'full'] = 'left') -> None:
        super().__init__(header = header,
                         footer = '',
                         justify = justify,
                         no_wrap = True)


class etuxTimerTraceTable(Table):
    _box = Box(
        """\
    
    
    
    
 -- 
    
    
    
""",
            ascii = True
    )

    def __init__(self, *columns: Column) -> None:
        self._cons = etuxTimerTraceConsole()
        super().__init__(*columns,
                         box = self._box,
                         show_header = True,
                         show_edge = False,
                         pad_edge = False)

    def show(self) -> None:
        self._cons.print(self)


class etuxTimerTraceReport(etuxTimerTraceTable):
    def __init__(self, parser: etuxTimerTraceParser) -> None:
        super().__init__(
            etuxTimerTraceTableColumn(header = 'OPERATION\n'),
            etuxTimerTraceTableColumn(header = 'MIN\nusec',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = 'MAX\nusec',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = 'STDDEV\nusec',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = 'MEDIAN\nusec',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = 'MEAN\nusec',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = 'DURATION( RATIO)\nmsec(     %)',
                                      justify = 'right'),
            etuxTimerTraceTableColumn(header = '#NR( RATIO)\n    (     %)',
                                      justify = 'right'))
        all_stats = parser.total_stats

        self._begin_section("Arm statistics")
        self._add_row("arm tspec", parser.arm_tspec_stats, all_stats)
        self._add_row("arm msec", parser.arm_msec_stats, all_stats)
        self._add_row("arm sec", parser.arm_sec_stats, all_stats)
        self._end_section_with_total(parser.arm_stats, all_stats)

        self._begin_section("Issue statistics")
        self._add_row("issue (tspec)", parser.issue_tspec_stats, all_stats)
        self._add_row("issue (msec)", parser.issue_msec_stats, all_stats)
        self._end_section_with_total(parser.issue_stats, all_stats)

        self._begin_section("Expiry statistics")
        self._add_row("expire", parser.expire_stats, all_stats)
        self.add_row("latency",
                     "{:.3f}".format(parser.expire_latency_stats.min),
                     "{:.3f}".format(parser.expire_latency_stats.max),
                     "{:.3f}".format(parser.expire_latency_stats.stdev),
                     "{:.3f}".format(parser.expire_latency_stats.median),
                     "{:.3f}".format(parser.expire_latency_stats.mean),
                     "",
                     "")
        self._end_section()

        self._begin_section("Overall statistics")
        self._add_row("arm", parser.arm_stats, all_stats)
        self._add_row("cancel", parser.cancel_stats, all_stats)
        self._add_row("issue", parser.issue_stats, all_stats)
        self._add_row("run", parser.run_stats, all_stats)
        self._end_final_section(all_stats)

    def _add_row(self,
                 oper_name:  str,
                 oper_stats: etuxTimerTraceStat,
                 all_stats:  etuxTimerTraceStat,
                 style: Optional[Style] = None) -> None:
        self.add_row(oper_name,
                     "{:.3f}".format(oper_stats.min),
                     "{:.3f}".format(oper_stats.max),
                     "{:.3f}".format(oper_stats.stdev),
                     "{:.3f}".format(oper_stats.median),
                     "{:.3f}".format(oper_stats.mean),
                     "{:.3f}({:6.2f})".format(oper_stats.sum / 1000.0,
                                              oper_stats.sum * 100.0 /
                                              all_stats.sum),
                     "{}({:6.2f})".format(oper_stats.count,
                                          oper_stats.count * 100.0 /
                                          all_stats.count),
                     style = style)

    def _begin_section(self, sect_name: str) -> None:
        self.add_row(sect_name, style = Style(italic = True), end_section = True)

    def _end_section(self) -> None:
        self.add_row()

    def _end_section_with_total(self,
                                oper_stats: etuxTimerTraceStat,
                                all_stats:  etuxTimerTraceStat) -> None:
        self._add_row("Total",
                      oper_stats,
                      all_stats,
                      style = Style(bold = True))
        self.add_row()

    def _end_final_section(self, all_stats: etuxTimerTraceStat) -> None:
        self.add_row("Total",
                     "",
                     "",
                     "",
                     "",
                     "",
                     Padding("{:.3f}".format(all_stats.sum / 1000.0),
                             (0, 8, 0, 0)),
                     Padding(str(all_stats.count), (0, 8, 0, 0)),
                     style = Style(bold = True))


class etuxTimerTraceDescReport(etuxTimerTraceTable):
    def __init__(self, scanner: etuxTimerTraceScanner) -> None:
        super().__init__(etuxTimerTraceTableColumn(header = 'NAME'),
                         etuxTimerTraceTableColumn(header = 'TYPE'))
        descs = list(sorted(scanner.desc()))
        self._fill_desc(descs[0], (0, 0))
        for d in descs[1:]:
            self._fill_desc(d, (1, 0, 0, 0))

    def _fill_desc(self,
                   desc:    etuxTimerTraceDesc,
                   padding: PaddingDimensions) -> None:
            self.add_row(Padding(desc.name, padding),
                         style = Style(bold = True),
                         end_section = True)
            if len(desc.attrs) > 0:
                for attr in desc.attrs:
                    self.add_row(attr.name,
                                 Text(attr.kind, style = Style(italic = True)))
            else:
                    self.add_row('none',
                                 Text('NA', style = Style(italic = True)))


def etux_timer_trace_check_path(path: str) -> str:
    if not os.path.isdir(path):
        raise NotADirectoryError("'{}': not a directory".format(path))
    if not os.access(path, os.R_OK | os.X_OK):
        raise PermissionError("'{}': access permission denied".format(path))

    return path


def main():
    common_parser = ArgumentParser(add_help = False)
    common_parser.add_argument('path',
                               type = etux_timer_trace_check_path,
                               metavar = 'DIR_PATH',
                               help = 'Pathname to directory where LTTng '
                                      'traces are stored')
    common_parser.add_argument(
        '-l', '--log',
        type = str,
        choices = [ 'error', 'warning', 'info', 'debug' ],
        default = 'info',
        metavar = 'LOGLEVEL',
        help = "Level of verbosity (either 'error', 'warning', 'info' or "
               "'debug', defaults to 'info')")
    main_parser = ArgumentParser(description = 'eTux timer performance '
                                               'reporting tool')
    cmd_parser = main_parser.add_subparsers(dest = 'cmd',
                                            metavar = 'COMMAND')
    cmd_parser.add_parser('stat',
                          parents = [common_parser],
                          help = 'Show timer statistics computed from trace '
                                 'events found into trace directory')
    cmd_parser.add_parser('list',
                          parents = [common_parser],
                          help = 'List trace event types found into trace '
                                 'directory')

    progbar = None

    try:
        args = main_parser.parse_args()
        loglvl = getattr(logging, args.log.upper())
        assert isinstance(loglvl, int)
        logging.basicConfig(format="%(message)s.", level=loglvl)

        scanner = etuxTimerTraceScanner(args.path)

        if args.cmd == 'stat':
            trc_parser = etuxTimerTraceParser()
            progbar = eTuxTimerTraceProgress()
            progbar.start(scanner.count)
            scanner.parse(trc_parser, progbar)
            progbar.stop()
            report = etuxTimerTraceReport(trc_parser)
        elif args.cmd == 'list':
            report = etuxTimerTraceDescReport(scanner)
        report.show()
    except KeyboardInterrupt:
        if progbar is not None:
            progbar.stop()
        print("{}: Interrupted!".format(arg0, file=sys.stderr))
        sys.exit(1)
    except Exception as e:
        if progbar is not None:
            progbar.stop()
        #raise e
        print("{}: {}.".format(arg0, e), file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
