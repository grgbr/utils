.. SPDX-License-Identifier: GPL-3.0-only
   
   This file is part of Utils.
   Copyright (C) 2017-2023 Grégor Boirie <gregor.boirie@free.fr>

.. include:: _cdefs.rst

Overview
========

What follows here provides a thorough description of how to use Utils_'s
library.

Basically, Utils_ library is a basic C framework that provides definitions
usefull to carry out common Linux system tasks in C.
The library is implemented to run on GNU Linux / glibc platforms only (although
porting to alternate C library such as `musl libc <https://www.musl-libc.org/>`_
should not be much of a hassle).

Utils_ library API is organized around the following functional areas which
you can refer to for further details :

* `Filesystem tree`_,
* `Timers`_.

Utils_ sources are distributed under the :ref:`GNU Lesser General Public
License <lgpl>` whereas documentation manuals are distributed under the
:ref:`GNU General Public License <gpl>`.

.. index:: build configuration, configuration macros

Build configuration
===================

At :ref:`Build configuration time <workflow-configure-phase>`, multiple build
options are available to customize final Utils_ build. From client code, you
may eventually refer to the corresponding C macros listed below:

* :c:macro:`CONFIG_ETUX_FSTREE`
* :c:macro:`CONFIG_ETUX_PTEST`
* :c:macro:`CONFIG_ETUX_TIMER_SUBSEC_BITS`
* :c:macro:`CONFIG_ETUX_TIMER_LIST`
* :c:macro:`CONFIG_ETUX_TIMER_HEAP`
* :c:macro:`CONFIG_ETUX_TIMER_HWHEEL`
* :c:macro:`CONFIG_ETUX_TRACE`
* :c:macro:`CONFIG_UTILS_ASSERT_API`
* :c:macro:`CONFIG_UTILS_ASSERT_INTERN`
* :c:macro:`CONFIG_UTILS_ATOMIC`
* :c:macro:`CONFIG_UTILS_DIR`
* :c:macro:`CONFIG_UTILS_FD`
* :c:macro:`CONFIG_UTILS_FILE`
* :c:macro:`CONFIG_UTILS_MQUEUE`
* :c:macro:`CONFIG_UTILS_NET`
* :c:macro:`CONFIG_UTILS_PATH`
* :c:macro:`CONFIG_UTILS_PIPE`
* :c:macro:`CONFIG_UTILS_POLL`
* :c:macro:`CONFIG_UTILS_POLL_UNSK`
* :c:macro:`CONFIG_UTILS_PWD`
* :c:macro:`CONFIG_UTILS_SIGNAL`
* :c:macro:`CONFIG_UTILS_SIGNAL_FD`
* :c:macro:`CONFIG_UTILS_STR`
* :c:macro:`CONFIG_UTILS_TIME`
* :c:macro:`CONFIG_UTILS_THREAD`
* :c:macro:`CONFIG_UTILS_UNSK`
* :c:macro:`CONFIG_UTILS_UTEST`

.. index:: filesystem

Filesystem tree
===============

When compiled with the :c:macro:`CONFIG_ETUX_FSTREE` build configuration
option enabled, the Utils_ library provides support for filesystem operations
upon directory tree hierarchies. These are:

.. hlist::

   * Iterate over directory entries:

      * :c:func:`etux_fstree_walk`
      * :c:func:`etux_fstree_sort_walk`

   * Scan filesystem hierarchies:

      * :c:func:`etux_fstree_scan`
      * :c:func:`etux_fstree_sort_scan`

   * Filesystem entry properties:

      * :c:struct:`etux_fstree_entry`
      * :c:func:`etux_fstree_entry_isdot`
      * :c:func:`etux_fstree_entry_name`
      * :c:func:`etux_fstree_entry_path`
      * :c:func:`etux_fstree_entry_sized_path`
      * :c:func:`etux_fstree_entry_sized_slink`
      * :c:func:`etux_fstree_entry_slink`
      * :c:func:`etux_fstree_entry_stat`
      * :c:func:`etux_fstree_entry_type`

   * Iteration / scanning state:

      * :c:struct:`etux_fstree_iter`
      * :c:func:`etux_fstree_iter_depth`
      * :c:func:`etux_fstree_iter_dir`
      * :c:func:`etux_fstree_iter_dirfd`
      * :c:func:`etux_fstree_iter_path`

.. index:: timer, time

Timers
======

.. todo:: Complete me !!

.. index:: API reference, reference

Reference
=========

Configuration macros
--------------------

CONFIG_ETUX_TIMER_HEAP
**********************

.. doxygendefine:: CONFIG_ETUX_TIMER_HEAP

CONFIG_ETUX_TIMER_HWHEEL
************************

.. doxygendefine:: CONFIG_ETUX_TIMER_HWHEEL

CONFIG_ETUX_TIMER_LIST
**********************

.. doxygendefine:: CONFIG_ETUX_TIMER_LIST

CONFIG_ETUX_TIMER_SUBSEC_BITS
*****************************

.. doxygendefine:: CONFIG_ETUX_TIMER_SUBSEC_BITS

CONFIG_ETUX_PTEST
*****************

.. doxygendefine:: CONFIG_ETUX_PTEST

CONFIG_ETUX_TRACE
*****************

.. doxygendefine:: CONFIG_ETUX_TRACE

CONFIG_UTILS_ASSERT_API
***********************

.. doxygendefine:: CONFIG_UTILS_ASSERT_API

CONFIG_UTILS_ASSERT_INTERN
**************************

.. doxygendefine:: CONFIG_UTILS_ASSERT_INTERN

CONFIG_UTILS_ATOMIC
*******************

.. doxygendefine:: CONFIG_UTILS_ATOMIC

CONFIG_UTILS_DIR
****************

.. doxygendefine:: CONFIG_UTILS_DIR

CONFIG_UTILS_FD
***************

.. doxygendefine:: CONFIG_UTILS_FD

CONFIG_UTILS_FILE
*****************

.. doxygendefine:: CONFIG_UTILS_FILE

CONFIG_ETUX_FSTREE
******************

.. doxygendefine:: CONFIG_ETUX_FSTREE

CONFIG_UTILS_MQUEUE
*******************

.. doxygendefine:: CONFIG_UTILS_MQUEUE

CONFIG_UTILS_NET
****************

.. doxygendefine:: CONFIG_UTILS_NET

CONFIG_UTILS_PATH
*****************

.. doxygendefine:: CONFIG_UTILS_PATH

CONFIG_UTILS_PIPE
*****************

.. doxygendefine:: CONFIG_UTILS_PIPE

CONFIG_UTILS_POLL
*****************

.. doxygendefine:: CONFIG_UTILS_POLL

CONFIG_UTILS_POLL_UNSK
**********************

.. doxygendefine:: CONFIG_UTILS_POLL_UNSK

CONFIG_UTILS_PWD
****************

.. doxygendefine:: CONFIG_UTILS_PWD

CONFIG_UTILS_SIGNAL
*******************

.. doxygendefine:: CONFIG_UTILS_SIGNAL

CONFIG_UTILS_SIGNAL_FD
**********************

.. doxygendefine:: CONFIG_UTILS_SIGNAL_FD

CONFIG_UTILS_STR
****************

.. doxygendefine:: CONFIG_UTILS_STR

CONFIG_UTILS_TIME
*****************

.. doxygendefine:: CONFIG_UTILS_TIME

CONFIG_UTILS_THREAD
*******************

.. doxygendefine:: CONFIG_UTILS_THREAD

CONFIG_UTILS_UNSK
*****************

.. doxygendefine:: CONFIG_UTILS_UNSK

CONFIG_UTILS_UTEST
******************

.. doxygendefine:: CONFIG_UTILS_UTEST

Macros
------

.. _etux_fstree_opts-group:

Filesystem tree traversal options
*********************************

.. doxygengroup:: etux_fstree_opts-group
   :content-only:

.. _etux_fstree_cmds-group:

Filesystem tree traversal commands
**********************************

.. doxygengroup:: etux_fstree_cmds-group
   :content-only:

Typedefs
--------

etux_fstree_cmp_fn
******************

.. doxygentypedef:: etux_fstree_cmp_fn

etux_fstree_filter_fn
*********************

.. doxygentypedef:: etux_fstree_filter_fn

etux_fstree_handle_fn
*********************

.. doxygentypedef:: etux_fstree_handle_fn

Enumerations
------------

etux_fstree_event
*****************

.. doxygenenum:: etux_fstree_event

Structures
----------

etux_fstree_entry
*****************

.. doxygenstruct:: etux_fstree_entry

etux_fstree_iter
****************

.. doxygenstruct:: etux_fstree_iter

Functions
---------

etux_fstree_entry_isdot()
*************************

.. doxygenfunction:: etux_fstree_entry_isdot

etux_fstree_entry_name()
************************

.. doxygenfunction:: etux_fstree_entry_name

etux_fstree_entry_path()
************************

.. doxygenfunction:: etux_fstree_entry_path

etux_fstree_entry_sized_path()
******************************

.. doxygenfunction:: etux_fstree_entry_sized_path

etux_fstree_entry_sized_slink()
*******************************

.. doxygenfunction:: etux_fstree_entry_sized_slink

etux_fstree_entry_slink()
*************************

.. doxygenfunction:: etux_fstree_entry_slink

etux_fstree_entry_stat()
************************

.. doxygenfunction:: etux_fstree_entry_stat

etux_fstree_entry_type()
************************

.. doxygenfunction:: etux_fstree_entry_type

etux_fstree_iter_depth()
************************

.. doxygenfunction:: etux_fstree_iter_depth

etux_fstree_iter_dir
********************

.. doxygenfunction:: etux_fstree_iter_dir

etux_fstree_iter_dirfd
**********************

.. doxygenfunction:: etux_fstree_iter_dirfd

etux_fstree_iter_path()
***********************

.. doxygenfunction:: etux_fstree_iter_path

etux_fstree_scan()
******************

.. doxygenfunction:: etux_fstree_scan

etux_fstree_sort_scan()
***********************

.. doxygenfunction:: etux_fstree_sort_scan

etux_fstree_sort_walk()
***********************

.. doxygenfunction:: etux_fstree_sort_walk

etux_fstree_walk()
******************

.. doxygenfunction:: etux_fstree_walk
