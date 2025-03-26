.. SPDX-License-Identifier: GPL-3.0-only
   
   This file is part of Utils.
   Copyright (C) 2017-2025 Grégor Boirie <gregor.boirie@free.fr>

.. include:: _cdefs.rst

.. _junit:                https://en.wikipedia.org/wiki/JUnit
.. _cute:                 https://github.com/grgbr/cute/
.. _breathe:              https://github.com/breathe-doc/breathe
.. _ebuild:               https://github.com/grgbr/ebuild/
.. _stroll:               https://github.com/grgbr/stroll/
.. _gnu_make:             https://www.gnu.org/software/make/
.. |eBuild|               replace:: `eBuild <ebuild_>`_
.. |eBuild User Guide|    replace:: :external+ebuild:doc:`eBuild User Guide <user>`
.. |eBuild Prerequisites| replace:: :external+ebuild:ref:`eBuild Prerequisites <sect-user-prerequisites>`
.. |cute-run|             replace:: :external+cute:doc:`cute-run(1) <man/cute-run>`
.. |cute-report|          replace:: :external+cute:doc:`cute-report(1) <man/cute-report>`
.. |Configure|            replace:: :external+ebuild:ref:`sect-user-configure`
.. |Build|                replace:: :external+ebuild:ref:`sect-user-build`
.. |Install|              replace:: :external+ebuild:ref:`sect-user-install`
.. |Staged install|       replace:: :external+ebuild:ref:`sect-user-staged-install`
.. |BUILDDIR|             replace:: :external+ebuild:ref:`var-builddir`
.. |PREFIX|               replace:: :external+ebuild:ref:`var-prefix`
.. |BINDIR|               replace:: :external+ebuild:ref:`var-bindir`
.. |CROSS_COMPILE|        replace:: :external+ebuild:ref:`var-cross_compile`
.. |DESTDIR|              replace:: :external+ebuild:ref:`var-destdir`
.. |GNU Make|             replace:: `GNU Make <gnu_make_>`_

Overview
========

This guide mainly focuses upon the construction process required to install
Utils_.

Utils_'s build logic is based upon |eBuild|. In addition to the build process
description explained below, you may refer to the |eBuild User Guide|
for further detailed informations.

Prerequisites
=============

In addition to the standard |eBuild Prerequisites|, the Stroll_ package is
required to build Utils_.

Optionally, you will need CUTe_ at build time and at runtime when unit
testsuite_ is enabled (see :c:macro:`CONFIG_UTILS_UTEST`).

Optionally, you will need multiple packages installed to build the
documentation_. In addition to packages listed into |eBuild Prerequisites|,
Utils_'s documentation_ generation process requires breathe_.

Getting help
============

From Utils_ source tree root, enter:

.. code-block:: console

   $ make help

Also note that a more detailed help message is available:

.. code-block:: console

   $ make help-full

Refer to :external+ebuild:ref:`eBuild help target <target-help>` and
:external+ebuild:ref:`eBuild help-full target <target-help-full>` for further
informations.

The :external+ebuild:ref:`eBuild Troubleshooting <sect-user-troubleshooting>`
section also contains valuable informations.

Build Workflow
==============

As mentioned earlier, Utils_'s build logic is based on |eBuild|, a |GNU make|
based build system. To build and install Utils_, the typical workflow is:

#. Prepare and collect workflow requirements,
#. |Configure| the construction logic,
#. |Build| programs, libraries, etc.,
#. |Install| components, copying files previously built to
   system-wide directories

Alternatively, you may replace the last step mentioned above with a |Staged
Install|. You will find below a **quick starting guide** showing how to build
Utils_.

You are also provided with the ability to :

* generate documentation_,
* build, install, and / or run Utils_'s testsuite_.

Preparation phase
-----------------

The overall :external+ebuild:ref:`eBuild Workflow <sect-user-workflow>` is
customizable thanks to multiple :command:`make` variable settings. You should
adjust values according to your specific needs.

Most of the time, setting |BUILDDIR|, |PREFIX|, |CROSS_COMPILE| is enough.
You should also set the :envvar:`PATH` environment variable according to the
set of tools required by the build process.

Optionally, you may set ``EXTRA_CFLAGS`` and ``EXTRA_LDFLAGS`` variables to
give the compiler and linker additional flags respectively.

Refer to :external+ebuild:ref:`eBuild Tools <sect-user-tools>` and
:external+ebuild:ref:`eBuild Variables <sect-user-variables>` for further
informations.

.. _workflow-configure-phase:
   
Configure phase
---------------

To begin with, |Configure| the build process interactively by running the
:external+ebuild:ref:`eBuild menuconfig target <target-menuconfig>`:

.. code-block:: console

   $ make menuconfig BUILDDIR=$HOME/build/utils

Build phase
-----------

Now, proceed to the |Build| phase and compile / link programs, libraries, etc.
by running the :external+ebuild:ref:`eBuild build target <target-build>`:

.. code-block:: console

   $ make build BUILDDIR=$HOME/build/utils PREFIX=/usr
 
Install phase
-------------

Finally, |Install| programs, libraries, etc.: by running the
:external+ebuild:ref:`eBuild install target <target-install>`:

.. code-block:: console
   
   $ make install BUILDDIR=$HOME/build/utils PREFIX=/usr
 
Alternative staged install phase
--------------------------------

Alternatively, perform a |Staged install| by specifying the |DESTDIR| variable
instead:
   
.. code-block:: console

   $ make install BUILDDIR=$HOME/build/utils PREFIX=/usr DESTDIR=$HOME/staging

Documentation
-------------

You may generate Utils_ documentation by running the
:external+ebuild:ref:`eBuild doc target <target-doc>` like so:

.. code-block:: console

   $ make doc BUILDDIR=$HOME/build/utils PREFIX=/usr

You may further install generated documentation by running the
:external+ebuild:ref:`eBuild install-doc target <target-install-doc>` target:

.. code-block:: console

   $ make install-doc BUILDDIR=$HOME/build/utils PREFIX=/usr DESTDIR=$HOME/staging

.. _testsuite:

Testing
-------

When the :c:macro:`CONFIG_UTILS_UTEST` build configuration setting is enabled,
you may build Utils_ testsuite by running the
:external+ebuild:ref:`eBuild build-check target <target-build-check>` target
like so:

.. code-block:: console

   $ make build-check BUILDDIR=$HOME/build/utils PREFIX=/usr

You may further install generated testsuite by running the
:external+ebuild:ref:`eBuild install-check target <target-install-check>`
target:

.. code-block:: console

   $ make install-check BUILDDIR=$HOME/build/utils PREFIX=/usr DESTDIR=$HOME/staging

Use the :command:`utils-utest` program installed under the |BINDIR| directory
to run the testsuite.

When *not cross-compiling*, running the ``check`` target builds and runs
the testsuite all at once with no required installation:

.. code-block:: console

   $ make check BUILDDIR=$HOME/build/utils CHECK_FORCE=n CHECK_VERBOSE=--terse

The ``CHECK_FORCE`` :command:`make` variable given above may be used to skip
testing operations when a previous testsuite run has already completed. It may
be specified as :

* one of ``y`` or ``1`` values to enforce a testsuite run (the default);
* or any other value to disable testsuite run enforcement.

The ``CHECK_VERBOSE`` :command:`make` variable given above is passed as-is to
the :command:`utils-utest` command line arguments and may be used to enable
testsuite verbose output. See CUTe_'s |cute-run| manual for more informations.

The ``check`` target requests :command:`utils-utest` to store results into the
:file:`utils-utest.xml` JUnit_ file located under the ``$(BUILDDIR)/test``
directory so that you may perform further analysis thanks to the CUTe_'s
|cute-report| reporting tool:

.. code-block:: console

   $ cute-report sumup $HOME/build/utils/test/utils-utest.xml

Further informations
--------------------

Finally, you may find lots of usefull informations into the
:external+ebuild:ref:`Reference <sect-user-reference>` section of the |eBuild
User Guide|.
