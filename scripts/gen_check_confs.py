#!/usr/bin/env python3
# -*- coding: utf-8 -*-
################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Utils.
# Copyright (C) 2025 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

import sys
import os
import argparse
import itertools

conf_syms = frozenset({
    frozenset({
        frozenset({ 'CONFIG_UTILS_ASSERT_API=y' }),
        frozenset({ 'CONFIG_UTILS_ASSERT_API=y',
                    'CONFIG_UTILS_ASSERT_INTERN=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_TIME=n' }),
        frozenset({ 'CONFIG_UTILS_TIME=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_LIST=n' }),
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_LIST=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_HEAP=n' }),
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_HEAP=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_HWHEEL=n' }),
        frozenset({ 'CONFIG_UTILS_TIMER=y', 'CONFIG_UTILS_TIMER_HWHEEL=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_FSTREE=n' }),
        frozenset({ 'CONFIG_UTILS_FSTREE=y' })
    }),
    frozenset({
        frozenset({ 'CONFIG_UTILS_UTEST=y' }),
        frozenset({ 'CONFIG_UTILS_UTEST=y' })
    }),
})


def sort_uniq(collection):
    return sorted(tuple(frozenset(collection)))


def make_conf(syms):
    def flatten(collection):
        for item in collection:
            if isinstance(item, (list, tuple, set, frozenset)):
                yield from flatten(item)
            else:
                yield str(item)

    return tuple(sorted(frozenset(flatten(syms))))


def conf_db():
    lst = []
    for cfg in itertools.product(*conf_syms):
        lst.append(make_conf(cfg))
    return sort_uniq(lst)


def conf_str(db, index):
    return '\n'.join(db[index])


def gen_conf(db, index, path):
    with open(path, 'wt') as f:
        print(conf_str(db, index), file=f)


def count_conf():
    return len(conf_db())


def show_one_conf(index):
    db = conf_db()
    if index < 0 or index >= len(db):
        raise Exception("cannot show configuration '{}': "
                        "invalid specified index".format(index))
    print(conf_str(db, index))


def gen_one_conf(index, path):
    db = conf_db()
    if index < 0 or index >= len(db):
        raise Exception("cannot generate configuration '{}': "
                        "invalid specified index".format(index))
    gen_conf(conf_db(), index, path)


def gen_all_confs(out_dpath):
    try:
        os.mkdir(out_dpath)
    except FileExistsError:
        pass
    for root, dirs, files in os.walk(out_dpath, topdown=False):
        for name in files:
            os.unlink(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))
    db = conf_db()
    for idx in range(len(db)):
        fpath = os.path.join(out_dpath, 'conf{}.in'.format(idx))
        gen_conf(db, idx, fpath)


def main():
    global arg0

    arg0 = os.path.basename(sys.argv[0])

    parser = argparse.ArgumentParser(description = 'Stroll testing build '
                                                   'configuration generator')
    subparser = parser.add_subparsers(dest = 'cmd')
    subparser.add_parser('count', help = 'Display count of configurations')
    show_parser = subparser.add_parser('show',
                                       help = 'Show content of a configuration')
    show_parser.add_argument('index',
                             nargs = 1,
                             type = int,
                             default = None,
                             metavar = 'CONF_INDEX',
                             help = 'Configuration index')
    genone_parser = subparser.add_parser('genone',
                                         help = 'Generate one configuration')
    genone_parser.add_argument('index',
                               nargs = 1,
                               type = int,
                               default = None,
                               metavar = 'CONF_INDEX',
                               help = 'Configuration index')
    genone_parser.add_argument('out_fpath',
                               nargs = 1,
                               type = str,
                               default = None,
                               metavar = 'CONF_FILEPATH',
                               help = 'Pathname to output configuration file')
    genall_parser = subparser.add_parser('genall',
                                         help = 'Generate all configurations')
    genall_parser.add_argument('out_dpath',
                               nargs = 1,
                               type = str,
                               default = None,
                               metavar = 'CONF_DIRPATH',
                               help = 'Pathname to configurations '
                                      'output directory')

    args = parser.parse_args()

    try:
        cmd = args.cmd
        if cmd == 'count':
            print(count_conf())
        elif cmd == 'show':
            show_one_conf(args.index[0])
        elif cmd == 'genone':
            gen_one_conf(args.index[0], args.out_fpath[0])
        else:
            gen_all_confs(args.out_dpath[0])
    except Exception as e:
        print("{}: {}.".format(arg0, e), file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
