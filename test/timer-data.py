#!/usr/bin/env python3
# -*- coding: utf-8 -*-
################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2024 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

import sys
import os
import argparse as argp
import re
from enum import Enum
import struct
import math
import io
import logging

INT32_MAX = (1<<31) - 1

class etuxTimerPerfKind(Enum):
    ARM_TSPEC   = 0
    ARM_MSEC    = 1
    ARM_SEC     = 2
    CANCEL      = 3
    ISSUE_TSPEC = 4
    ISSUE_MSEC  = 5
    RUN         = 6
    EXPIRE      = 7


def etux_jiffies_to_tspec(jiffies: int, hz: int) -> tuple[int, int]:
    sec = jiffies / hz
    nsec = (jiffies % hz) * (1000000000 / hz)
    assert nsec >= 0
    assert nsec < 1000000000

    return sec, nsec


def etux_tspec_is_valid(tspec: tuple[int, int]) -> bool:
    if len(tspec) != 2:
        return False
    if tspec[0] < 0:
        return False
    if tspec[1] < 0 or tspec[1] >= 1000000000:
        return False
    return True


def etux_task_is_valid(task: str) -> bool:
    length = len(task)
    if length <= 0 or length > 0xff:
        return False

    return True


class etuxTimerPerfEvent:
    def __init__(self,
                 stamp: tuple[int, int],
                 task:  str,
                 kind:  etuxTimerPerfKind):
        assert etux_tspec_is_valid(stamp) is True
        assert etux_task_is_valid(task) is True
        assert kind in etuxTimerPerfKind

        self._stamp_sec = stamp[0]
        self._stamp_nsec = stamp[1]
        self._task = task
        self._kind = kind

    def show(self):
        print("<{}.{:09d}> {}: ({})".format(self._stamp_sec, self._stamp_nsec,
                                            self._kind.name.lower(),
                                            self._task))


class etuxTimerPerfArmTspecEvent(etuxTimerPerfEvent):
    def __init__(self,
                 stamp: tuple[int, int],
                 task:  str,
                 timer: int,
                 tspec: tuple[int, int]):
        assert timer >= 0
        assert etux_tspec_is_valid(tspec) is True

        super().__init__(stamp, task, etuxTimerPerfKind.ARM_TSPEC)
        self._timer = timer
        self._sec = tspec[0]
        self._nsec = tspec[1]

    def show(self):
        print("<{}.{:09d}> {}: @0x{:x} [{}.{:09d}] ({})".format(
            self._stamp_sec, self._stamp_nsec,
            self._kind.name.lower(),
            self._timer,
            self._sec, self._nsec,
            self._task))


class etuxTimerPerfArmMsecEvent(etuxTimerPerfEvent):
    def __init__(self,
                 stamp: tuple[int, int],
                 task:  str,
                 timer: int,
                 msec:  int):
        assert timer >= 0
        assert msec >= 0

        super().__init__(stamp, task, etuxTimerPerfKind.ARM_MSEC)
        self._timer = timer
        self._sec = int(msec / 1000)
        self._nsec = (msec % 1000) * 1000000
        assert self._nsec >= 0
        assert self._nsec < 1000000000

    def show(self):
        print("<{}.{:09d}> {}: @0x{:x} [{}.{:09d}] ({})".format(
            self._stamp_sec, self._stamp_nsec,
            self._kind.name.lower(),
            self._timer,
            self._sec, self._nsec,
            self._task))


class etuxTimerPerfCancelEvent(etuxTimerPerfEvent):
    def __init__(self, stamp: tuple[int, int], task: str, timer: int):
        assert timer >= 0

        super().__init__(stamp, task, etuxTimerPerfKind.CANCEL)
        self._timer = timer

    def show(self):
        print("<{}.{:09d}> {}: @0x{:x} ({})".format(
            self._stamp_sec,
            self._stamp_nsec,
            self._kind.name.lower(),
            self._timer,
            self._task))


class etuxTimerPerfRunEvent(etuxTimerPerfEvent):
    def __init__(self, stamp: tuple[int, int], task: str):
        super().__init__(stamp, task, etuxTimerPerfKind.RUN)

    def show(self):
        print("<{}.{:09d}> {}: ({})".format(
            self._stamp_sec,
            self._stamp_nsec,
            self._kind.name.lower(),
            self._task))


class etuxTimerPerfEventUnpacker:
    def __init__(self, event_file: io.BufferedReader):
        self._probe_data(event_file.read())

    def _probe_data(self, data: bytes) -> None:
        fmt = "c"
        elen = struct.calcsize(fmt)
        if elen > len(data):
            raise Exception('Cannot probe performance event endianness: '
                            'missing input data')
        endian = struct.unpack_from(fmt, data, 0)[0]
        endian = endian.decode(encoding = 'ascii')
        if endian == 'n':
            self._endian = '='
        elif endian == 'l':
            self._endian = '<'
        elif endian == 'b':
            self._endian = '>'
        else:
            raise Exception('Cannot probe performance event endianness: '
                            'invalid endianness code')

        fmt = "{}I".format(self._endian)
        clen = struct.calcsize(fmt)
        self._off = elen + clen
        if self._off > len(data):
            raise Exception('Cannot probe performance event count: '
                            'missing input data')

        self._data = data
        self._count = struct.unpack_from(fmt, data, elen)[0]

    def __iter__(self):
        if self._off != struct.calcsize("cI"):
            self._probe_data(self._data)

        return self

    def __next__(self):
        if self._off >= len(self._data):
            raise StopIteration

        return self._unpack_one()

    @property
    def count(self) -> int:
        return self._count

    def _unpack_tspec(self) -> tuple[int, int]:
        fmt = "{}qq".format(self._endian)
        tlen = struct.calcsize(fmt)
        if (self._off + tlen) > len(self._data):
            raise Exception('Cannot unpack performance event timespec: '
                            'missing input data')
        tspec = struct.unpack_from(fmt, self._data, self._off)
        if etux_tspec_is_valid(tspec) is False:
            raise Exception('Cannot unpack performance event timespec: '
                            'invalid range')
        self._off += tlen

        return tspec

    def _unpack_msec(self) -> int:
        fmt = "{}q".format(self._endian)
        tlen = struct.calcsize(fmt)
        if (self._off + tlen) > len(self._data):
            raise Exception('Cannot unpack performance event milliseconds: '
                            'missing input data')
        msec = struct.unpack_from(fmt, self._data, self._off)[0]
        if msec < 0:
            raise Exception('Cannot unpack performance event milliseconds: '
                            'invalid range')
        self._off += tlen

        return msec

    def _unpack_task(self) -> str:
        fmt = "{}H".format(self._endian)
        clen = struct.calcsize(fmt)
        if (self._off + clen) > len(self._data):
            raise Exception('Cannot unpack performance event task: '
                            'missing input data')
        cnt = struct.unpack_from(fmt, self._data, self._off)[0]

        fmt = "{}s".format(cnt)
        slen = struct.calcsize(fmt)
        if (self._off + clen + slen) > len(self._data):
            raise Exception('Cannot unpack performance event task: '
                            'missing input data')
        task = struct.unpack_from(fmt, self._data, self._off + clen)[0]
        task = task.decode(encoding = 'ascii')
        if etux_task_is_valid(task) is False:
            raise Exception('Cannot unpack performance event task: '
                            'string too long')

        self._off += clen + slen

        return task

    def _unpack_kind(self) -> etuxTimerPerfKind:
        fmt = "{}B".format(self._endian)
        olen = struct.calcsize(fmt)
        if (self._off + olen) > len(self._data):
            raise Exception('Cannot unpack performance event operation type: '
                            'missing input data')
        oper = struct.unpack_from(fmt, self._data, self._off)[0]

        self._off += olen

        return etuxTimerPerfKind(oper)

    def _unpack_addr(self) -> int:
        fmt = "{}q".format(self._endian)
        alen = struct.calcsize(fmt)
        if (self._off + alen) > len(self._data):
            raise Exception('Cannot unpack performance event address: '
                            'missing input data')
        addr = struct.unpack_from(fmt, self._data, self._off)[0]
        if addr < 0:
            raise Exception('Cannot unpack performance event address: '
                            'invalid range')

        self._off += alen

        return addr

    def _unpack_arm_tspec(self,
                          stamp: tuple[int, int],
                          task:  str) -> etuxTimerPerfArmTspecEvent:
        addr = self._unpack_addr()
        tspec = self._unpack_tspec()

        return etuxTimerPerfArmTspecEvent(stamp, task, addr, tspec)

    def _unpack_arm_msec(self,
                         stamp: tuple[int, int],
                         task:  str) -> etuxTimerPerfArmMsecEvent:
        addr = self._unpack_addr()
        msec = self._unpack_msec()

        return etuxTimerPerfArmMsecEvent(stamp, task, addr, msec)

    def _unpack_cancel(self,
                       stamp: tuple[int, int],
                       task:  str) -> etuxTimerPerfCancelEvent:
        addr = self._unpack_addr()

        return etuxTimerPerfCancelEvent(stamp, task, addr)

    def _unpack_run(self,
                    stamp: tuple[int, int],
                    task:  str) -> etuxTimerPerfRunEvent:
        return etuxTimerPerfRunEvent(stamp, task)

    def _unpack_one(self) -> etuxTimerPerfEvent:
        unpackers = {
            etuxTimerPerfKind.ARM_TSPEC: self._unpack_arm_tspec,
            etuxTimerPerfKind.ARM_MSEC:  self._unpack_arm_msec,
            etuxTimerPerfKind.CANCEL:    self._unpack_cancel,
            etuxTimerPerfKind.RUN:       self._unpack_run,
        }

        stamp = self._unpack_tspec()
        task = self._unpack_task()
        kind = self._unpack_kind()
        if kind not in unpackers:
            excmsg = "Cannot unpack performance event: " \
                     "unsupported event kind '{}'"
            raise Exception(excmsg.format(kind))

        evt = unpackers[kind](stamp, task)

        return evt


class etuxTimerPerfKernelPacker:
    _glob_pat = re.compile(r'^\s*(.+)\[[0-9]+\]\s+.....?\s+([0-9.]+):\s+([^:]+):\s+timer=([0-9a-fA-F]+)(.*)$')
    _arm_pat = re.compile(r'.*\s+\[timeout=([0-9]+)\]\s+.*$')

    def __init__(self, endian: str):
        assert endian in [ 'native', 'little', 'big' ]

        if endian == 'native':
            self._endian = '='
        elif endian == 'little':
            self._endian = '<'
        else:
            self._endian = '>'

        # Retriece system tick frequency (kernel HZ value)
        self._clk_tck = os.sysconf("SC_CLK_TCK")
        self._count = 0

    @staticmethod
    def _pack_kind(oper: etuxTimerPerfKind) -> bytes:
        assert oper in etuxTimerPerfKind

        return struct.pack("B", oper.value)

    @property
    def count(self) -> int:
        return self._count

    def _pack_tspec(self, stamp: tuple[int, int]) -> bytes:
        assert etux_tspec_is_valid(stamp) is True

        return struct.pack("{}qq".format(self._endian), stamp[0], stamp[1])

    def _pack_msec(self, msec: int) -> bytes:
        assert msec >= 0
        assert msec <= INT32_MAX

        return struct.pack("{}q".format(self._endian), msec)

    def _pack_task(self, task: str) -> bytes:
        assert etux_task_is_valid(task) is True

        tlen = len(task)

        return struct.pack("{}H{}s".format(self._endian, tlen),
                           tlen,
                           bytes(task, encoding='ascii'))

    def _pack_addr(self, timer: int) -> bytes:
        assert timer > 0

        return struct.pack("{}q".format(self._endian), timer)

    def _pack_arm_msec(self, tokens: re.Match) -> bytes or None:
        assert len(tokens.groups()) == 5

        self._last = etuxTimerPerfKind.ARM_MSEC

        timer = int(tokens[4], 16)
        tokens = self._arm_pat.match(tokens[5])
        if tokens is None or len(tokens.groups()) != 1:
            logging.warning("Warning: ignoring trace: "
                            "invalid 'timer_start' timer operation")
            return None
        tmout = int(int(tokens[1]) * 1000 / self._clk_tck)
        if tmout < 0:
            logging.warning("Warning: ignoring trace: "
                            "invalid 'timer_start' timer milliseconds expiry")
            return None
        if tmout > INT32_MAX:
            tmout = INT32_MAX

        return self._pack_kind(etuxTimerPerfKind.ARM_MSEC) + \
               self._pack_addr(timer) + \
               self._pack_msec(tmout)

    def _pack_cancel(self, tokens: re.Match) -> bytes:
        assert len(tokens.groups()) == 5

        self._last = etuxTimerPerfKind.CANCEL

        timer = int(tokens[4], 16)

        return self._pack_kind(etuxTimerPerfKind.CANCEL) + \
               self._pack_addr(timer)

    def _pack_run(self, tokens: re.Match) -> bytes or None:
        assert len(tokens.groups()) == 5

        # Merge subsequent kernel timer_expire_entry / timer_expire_exit traces
        if self._last == etuxTimerPerfKind.RUN:
            return None
        self._last = etuxTimerPerfKind.RUN

        return self._pack_kind(etuxTimerPerfKind.RUN)

    def _skip_expire_exit(self, tokens: re.Match) -> None:
        assert len(tokens.groups()) == 5

        # Merge subsequent kernel timer_expire_entry / timer_expire_exit traces
        self._last = etuxTimerPerfKind.RUN

        return None

    def _pack_one(self, string: str) -> bytes or None:
        packers = {
            'timer_start':        self._pack_arm_msec,
            'timer_cancel':       self._pack_cancel,
            'timer_expire_entry': self._pack_run,
            'timer_expire_exit':  self._skip_expire_exit
        }

        tokens = self._glob_pat.match(string)
        if not tokens or len(tokens.groups()) != 5:
            logging.warning("Warning: ignoring trace: "
                            "unmatched regular expression")
            self._last = None
            return None

        oper = tokens[3]
        if oper not in packers:
            logging.debug("Debug: ignoring trace: "
                          "unsupported '{}' timer operation".format(oper))
            self._last = None
            return None

        stamp = tokens[2].split('.')
        if len(stamp) != 2:
            logging.warning("Warning: ignoring trace: invalid timestamp format")
            self._last = None
            return None
        if len(stamp[1]) != 6:
            logging.warning("Warning: ignoring trace: "
                            "invalid timestamp microseconds format")
            self._last = None
            return None
        stamp = int(stamp[0]), int(stamp[1]) * 1000
        if etux_tspec_is_valid(stamp) is False:
            logging.warning("Warning: ignoring trace: invalid timestamp range")
            self._last = None
            return None

        task = tokens[1].strip()

        data = packers[oper](tokens)
        if data is None:
            return None

        return self._pack_tspec(stamp) + self._pack_task(task) + data

    def _pack_head(self, count: int) -> bytes:
        assert count > 0
        assert self._endian in [ '=', '<', '>' ]

        if self._endian == '=':
            endian = 'n'
        elif self._endian == '<':
            endian = 'l'
        elif self._endian == '>':
            endian = 'b'

        head = struct.pack("c", bytes(endian, encoding='ascii'))
        head += struct.pack("{}I".format(self._endian), count)

        return head

    def pack(self, kern_file: io.TextIOWrapper) -> bytes or None:
        self._count = 0
        pack = bytes()

        for line in kern_file:
            data = self._pack_one(line)
            if data is not None:
                self._count += 1
                pack += data

        if self._count == 0:
            return None

        return self._pack_head(self._count) + pack


def etux_timer_perf_pack_kernel_traces(kern_file_path: str,
                                       perf_file_path: str,
                                       endian:         str) -> None:
    kern = None
    perf = None

    try:
        if kern_file_path == '-':
            kern = os.fdopen(sys.stdin.fileno(), 'rt', closefd=False)
        else:
            kern = open(kern_file_path, 'rt')
        if perf_file_path == '-':
            perf = os.fdopen(sys.stdout.fileno(), 'wb', closefd=False)
        else:
            perf = open(perf_file_path, 'wb')
        packer = etuxTimerPerfKernelPacker(endian)
        data = packer.pack(kern)
        if data is None:
            raise Exception("No valid kernel timer traces found")
        perf.write(data)

    except Exception as e:
        msg = "Cannot convert kernel timer traces to performance events: {}"
        raise Exception(msg.format(str(e)))

    finally:
        if kern is not None:
            kern.close()
        if perf is not None:
            perf.close()

    logging.info("Processed {} kernel timer traces".format(packer.count))


def etux_timer_perf_show_events(perf_file_path: str) -> None:
    perf = None

    try:
        if perf_file_path == '-':
            perf = os.fdopen(sys.stdin.fileno(), 'rb', closefd=False)
        else:
            perf = open(perf_file_path, 'rb')
        unpacker = etuxTimerPerfEventUnpacker(perf)
        for evt in unpacker:
            evt.show()

    except Exception as e:
        msg = "Cannot show timer performance events: {}"
        raise Exception(msg.format(str(e)))

    finally:
        if perf is not None:
            perf.close()

    msg = "Processed {} timer performance events"
    logging.info(msg.format(unpacker.count))


def main():
    common_parser = argp.ArgumentParser(add_help = False)
    common_parser.add_argument(
        '-l', '--log',
        type = str,
        choices = [ 'error', 'warning', 'info', 'debug' ],
        default = 'info',
        metavar = 'LOGLEVEL',
        help = "Level of verbosity (either 'error', 'warning', 'info' or "
               "'debug', defaults to 'info')")
    common_parser.add_argument(
        'input',
        type = str,
        nargs = '?',
        default = '-',
        metavar = 'INPUT_PATH',
        help = "Pathname to file where kernel timer traces are stored "
               "(defaults to '-', i.e., stdin)")
    main_parser = argp.ArgumentParser(description = 'Kernel timer trace tool')
    cmd_parser = main_parser.add_subparsers(dest = 'cmd',
                                            metavar = 'COMMAND',
                                            required = True)

    conv_parser = cmd_parser.add_parser(
        'convert',
        parents = [ common_parser ],
        help = 'Convert kernel timer traces to binary performance event format')
    conv_parser.add_argument(
        '-e', '--endian',
        type = str,
        choices = [ 'native', 'little', 'big' ],
        default = 'native',
        metavar = 'ENDIAN',
        help = "Performance event format endianness (either 'native', 'little' "
               "or 'big', defaults to 'native')")
    conv_parser.add_argument(
        '-o', '--output',
        type = str,
        default = '-',
        metavar = 'OUTPUT_PATH',
        help = "Pathname to file where to store timer commands (defaults to "
               "'-', i.e., stdout)")
    show_parser = cmd_parser.add_parser(
        'show',
        parents = [ common_parser ],
        help = 'Display content of a binary performance event formatted file')

    ifile = None
    ofile = None
    ret = 1
    args = main_parser.parse_args()

    try:
        loglvl = getattr(logging, args.log.upper())
        assert isinstance(loglvl, int)
        logging.basicConfig(format="%(message)s.", level=loglvl)

        if args.cmd == 'convert':
            etux_timer_perf_pack_kernel_traces(args.input,
                                               args.output,
                                               args.endian)
        elif args.cmd == 'show':
            etux_timer_perf_show_events(args.input)
    except KeyboardInterrupt:
        print("{}: Interrupted!".format(os.path.basename(sys.argv[0])),
              file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        #raise e
        print("{}: {}.".format(os.path.basename(sys.argv[0]), e),
              file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
