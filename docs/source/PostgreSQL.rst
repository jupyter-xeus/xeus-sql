.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

PostgreSQL
==========

Linux
-----

If you're running ``xeus-sql`` with a conda-forge PostgreSQL build, you will have to create a symbolic link to ``/tmp/``:

.. code::

    sudo ln -s /var/run/postgresql/.s.PGSQL.5432 /tmp/.s.PGSQL.5432

Create an user:

.. code::

    sudo -u postgres createuser --superuser $USER_NAME

Start the service:

.. code::

    sudo service postgresql start sudo -u $USER_NAME psql

Create a database using the example available on this repository:

.. code::

    pg_restore --dbname=newdvdrental -U $USER_NAME pathto/xeus-sql/examples/dvdrental.tar

You can now run the example contained in ``examples/PostgreSQL.ipynb``.

More information about PostgreSQL can be found in the SOCI documentation_.

.. _documentation: http://soci.sourceforge.net/doc/release/4.0/backends/postgresql/