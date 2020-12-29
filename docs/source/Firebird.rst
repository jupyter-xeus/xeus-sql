.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Firebird
========

Linux
-----

Installation
************

Install Firebird using your preferred method. You will likely need the developer's version of Firebird for it to be picked up by SOCI.

If you're using a system's package manager your Firebird installation won't be picked up by the SOCI package available on ``conda-forge`` meaning you will either have to install SOCI from source or download it from SOCI page_.

Make sure you're installing SOCI after you have Firebird in your system.

You can still get ``xeus-sql`` from conda with:

.. code::

    conda install xeus-sql jupyterlab -c conda-forge

From source
+++++++++++

To install SOCI from source use the following flags when building:

.. code::

    cmake -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -G "Unix Makefiles" -DWITH_BOOST=OFF -DSOCI_CXX11=ON -DCMAKE_BUILD_TYPE=Release -DSOCI_LIBDIR=lib ..

    make install

Just make sure to do that in the same environment as the rest of your installation is going.

Usage
*****

Either way you decide to conduce your installation process you have to make sure that the directory where your database is is accessible by Jupyter Lab. To do that you can add the user of the machine to the ``firebird`` group:

.. code::

    sudo chown firebird:firebird $DATABASE_NAME
    sudo usermod -a -G firebird $USER_NAME

That's all that there's to it. You can now run the example contained in ``examples/Firebird.ipynb``.

More information about Firebird can be found in the SOCI documentation_.

.. _documentation: http://soci.sourceforge.net/doc/release/4.0/backends/firebird/
