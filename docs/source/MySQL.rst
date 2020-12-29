.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

MySQL
=====

Linux
-----

Installation
************

You can get MySQL from ``conda-forge``

.. code::

    conda install xeus-sql soci-mysql jupyterlab -c conda-forge

Usage
*****

To use MySQL from inside a notebook you need to first edit your MySQL configuration file that can usually be found at ``/etc/mysql/my.cnf``:

.. code::

    [server]
    socket=/tmp/mysql.sock

    [client]
    socket=/tmp/mysql.sock


To run the example notebook you'll need to load the ``dbname`` database or create one for yourself:

.. code::

    CREATE DATABASE dbname;
    CREATE TABLE example ( id smallint unsigned not null auto_increment, name varchar(20) not null, constraint pk_example primary key (id) );

You will also need to create a new user and grant access to it to the newly create database. You can achieve this with the following lines in your MySQL console:

.. code::

    CREATE USER 'user1'@'localhost' IDENTIFIED BY 'Password123#@!';
    GRANT ALL PRIVILEGES ON dbname.* TO ‘user1’@’localhost’;

Note that it might be necessary to install MySQL from different sources other than conda if you intend to manipulate users and creation of databases.

More information about Firebird can be found in the SOCI documentation_.

.. _documentation: http://soci.sourceforge.net/doc/release/4.0/backends/mysql/
