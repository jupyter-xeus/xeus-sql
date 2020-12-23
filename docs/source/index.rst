.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Introduction
============

``xeus-sql`` is a Jupyter kernel for general SQL implementations based on the native implementation of the Jupyter protocol xeus_ and SOCI_, a database access library for C++.

Licensing
=========

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the LICENSE file for details.


.. toctree::
   :caption: Getting started
   :maxdepth: 4

   installation

.. toctree::
   :caption: Connecting to different databases
   :maxdepth: 4

    DB2
    Firebird
    MySQL
    ODBC
    Oracle
    PostgreSQL
    SQLite3

.. toctree::
   :caption: API
   :maxdepth: 4

   sql_magic
   xvega_magic

.. _xeus: https://github.com/jupyter-xeus/xeus
.. _SOCI: https://github.com/SOCI/soci
