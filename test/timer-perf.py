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

class etuxTimerPerfKind(Enum):
    ARM_TSPEC   = 0
    ARM_MSEC    = 1
    ARM_SEC     = 2
    CANCEL      = 3
    ISSUE_TSPEC = 4
    ISSUE_MSEC  = 5
    RUN         = 6
    EXPIRE      = 7


def etux_decode_tspec(tspec: float) -> tuple[int, int]:
    nsec, sec = math.modf(tspec)
    sec = int(sec)
    if sec < 0:
        return None
    nsec = int(nsec * 1000000000)
    if nsec < 0 or nsec >= 1000000000:
        return None

    return (sec, nsec)


def etux_decode_msec(tspec: float) -> int:
    sec, nsec = etux_decode_tspec(tspec)

    return (sec * 1000) + (nsec / 1000000)

def etux_task_is_valid(task: str) -> bool:
    length = len(task)
    if length <= 0 or length > 0xff:
        return False

    return True


class etuxTimerPerfEvent:
    def __init__(self, stamp, task, kind):
        assert etux_decode_tspec(stamp) is not None
        assert etux_task_is_valid(task) is True
        assert kind in etuxTimerPerfKind

        self._stamp = stamp
        self._task = task
        self._kind = kind

    def show(self):
        print("<{:.9f}> {}: ({})".format(self._stamp,
                                         self._kind.name.lower(),
                                         self._task))


class etuxTimerPerfArmTspecEvent(etuxTimerPerfEvent):
    def __init__(self, stamp, task, timer, tspec):
        assert timer >= 0
        assert etux_decode_tspec(stamp) is not None

        super().__init__(stamp, task, etuxTimerPerfKind.ARM_TSPEC)
        self._timer = timer
        self._tspec = tspec

    def show(self):
        print("<{:.9f}> {}: @0x{} [{:.9f}] ({})".format(self._stamp,
                                                        self._kind.name.lower(),
                                                        self._timer,
                                                        self._tspec,
                                                        self._task))


class etuxTimerPerfCancelEvent(etuxTimerPerfEvent):
    def __init__(self, stamp, task, timer):
        assert timer >= 0

        super().__init__(stamp, task, etuxTimerPerfKind.CANCEL)
        self._timer = timer

    def show(self):
        print("<{:.9f}> {}: @0x{} ({})".format(self._stamp,
                                               self._kind.name.lower(),
                                               self._timer,
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
        self._endian = endian.decode(encoding = 'ascii')
        if self._endian not in [ '=', '<', '>' ]:
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
    def count(self):
        return self._count

    def _unpack_tspec(self) -> float:
        fmt = "{}qq".format(self._endian)
        tlen = struct.calcsize(fmt)
        if (self._off + tlen) > len(self._data):
            raise Exception('Cannot unpack performance event timespec: '
                            'missing input data')
        tspec = struct.unpack_from(fmt, self._data, self._off)
        stamp = float(tspec[0]) + (float(tspec[1]) / 1000000000)
        if etux_decode_tspec(stamp) is None:
            raise Exception('Cannot unpack performance event timespec: '
                            'invalid range')
        self._off += tlen

        return stamp

    def _unpack_task(self) -> str:
        fmt = "{}B".format(self._endian)
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
                          stamp: float,
                          task:  str) -> etuxTimerPerfArmTspecEvent:
        addr = self._unpack_addr()
        tspec = self._unpack_tspec()

        return etuxTimerPerfArmTspecEvent(stamp, task, addr, tspec)

    def _unpack_cancel(self,
                       stamp: float,
                       task:  str) -> etuxTimerPerfCancelEvent:
        addr = self._unpack_addr()

        return etuxTimerPerfCancelEvent(stamp, task, addr)

    def _unpack_one(self) -> etuxTimerPerfEvent:
        unpackers = {
            etuxTimerPerfKind.ARM_TSPEC: self._unpack_arm_tspec,
            etuxTimerPerfKind.CANCEL:    self._unpack_cancel
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
    _pattern = re.compile(r'^\s*(.+)\[[0-9]+\]\s+....\s+([0-9.]+):\s+([^:]+):\s+timer=([0-9a-fA-F]+)(.*)$')

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
    def _pack_kind(oper):
        assert oper in etuxTimerPerfKind

        return struct.pack("B", oper.value)

    @property
    def count(self):
        return self._count

    def _pack_tspec(self, stamp):
        tspec = etux_decode_tspec(stamp)
        assert tspec is not None

        return struct.pack("{}qq".format(self._endian), tspec[0], tspec[1])

    def _pack_msec(self, stamp):
        msec = etux_decode_msec(stamp)
        assert msec >= 0

        return struct.pack("{}q".format(self._endian), msec)

    def _pack_task(self, task):
        length = len(task)
        assert length > 0 and length <= 0xff

        return struct.pack("{}B{}s".format(self._endian, length),
                           length,
                           bytes(task, encoding='ascii'))

    def _pack_addr(self, timer):
        assert timer > 0

        return struct.pack("{}q".format(self._endian), timer)

    def _pack_arm_tspec(self, tokens):
        assert len(tokens.groups()) == 5

        timer = int(tokens[4], 16)

        match = re.match('.*\s+\[timeout=([0-9]+)\]\s+.*$', tokens[5])
        if match is None or len(match.groups()) != 1:
            logging.warning("Warning: ignoring trace: "
                            "invalid 'timer_start' timer operation")
            return None
        tmout = float(match[1]) / self._clk_tck

        return self._pack_kind(etuxTimerPerfKind.ARM_TSPEC) + \
               self._pack_addr(timer) + \
               self._pack_tspec(tmout)

    def _pack_cancel(self, tokens):
        assert len(tokens.groups()) == 5

        timer = int(tokens[4], 16)

        return self._pack_kind(etuxTimerPerfKind.CANCEL) + \
               self._pack_addr(timer)

    def _pack_one(self, string):
        packers = {
            'timer_start':  self._pack_arm_tspec,
            'timer_cancel': self._pack_cancel
        }

        tokens = self._pattern.match(string)
        if not tokens or len(tokens.groups()) != 5:
            logging.warning("Warning: ignoring trace: "
                            "unmatched regular expression")
            return None

        oper = tokens[3]
        if oper not in packers:
            logging.debug("Debug: ignoring trace: "
                          "unsupported '{}' timer operation".format(oper))
            return None
        data = packers[oper](tokens)
        if data is None:
            return None

        stamp = float(tokens[2])
        task = tokens[1].strip()

        return self._pack_tspec(stamp) + self._pack_task(task) + data

    def _pack_head(self, count: int) -> bytes:
        assert count > 0

        head = struct.pack("c", bytes(self._endian, encoding='ascii'))
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
