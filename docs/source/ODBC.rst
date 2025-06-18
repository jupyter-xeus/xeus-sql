.. Copyright (c) 2025, Chiara Marmo

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

ODBC
====

Linux
-----

Installation
************

For now `soci-odbc` is not available from ``conda-forge``.
The package is available under different names depending on your Linux distribution.

For example:

.. tabs::

   .. group-tab:: Fedora 64bit

      .. code-block:: bash

         sudo dnf install libaio.x86_64 soci.x86_64 soci-odbc.x86_64

   .. group-tab:: Ubuntu 64bit

      .. code-block:: bash

         sudo apt-get install libaio-dev libsoci-core4.0 libsoci-odbc4.0

Then from conda

.. code::

    conda install xeus-sql jupyterlab -c conda-forge

Usage
*****

To use the ODBC driver from inside a notebook you need to connect to the compatible database with the
```LOAD``` magic and the related driver.

For example:

.. tabs::

   .. group-tab:: PostGreSQL (to be tested)

      Once the `PostGreSQL driver <https://odbc.postgresql.org/>`_ installed

      .. code-block:: bash

         %LOAD odbc DRIVER=<path to the pgsql driver>;DBQ=<hostname>:<port>/<database>;UID=<user>;PWD=<password>

   .. group-tab:: Oracle

      Once the `Oracle driver <https://github.com/mkleehammer/pyodbc/wiki/Connecting-to-Oracle-from-RHEL-or-Centos#install-the-oracle-instant-client-odbc-driver-for-linux>`_ installed

      .. code-block:: bash

         %LOAD odbc DRIVER=<path to the oracle driver>;DBQ=<hostname>:<port>/<SID>;UID=<user>;PWD=<password>

      Oracle does not distribute debian based packages but the driver can be installed on any Linux distribution via the
      `available zip archives <https://www.oracle.com/europe/database/technologies/instant-client/linux-x86-64-downloads.html>`_.

.. _documentation: http://soci.sourceforge.net/doc/release/4.0/backends/odbc/
