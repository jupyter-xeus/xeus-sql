.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Installation
============

With Conda or Mamba
-------------------

``xeus-sql`` has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install ``xeus-sql`` in a fresh conda/mamba environment.
It is also needed to use a miniconda_ installation because with the full anaconda_ you may have a conflict with
the ``zeromq`` library which is already installed in the anaconda distribution.


The safest usage is to create an environment named ``xeus-sql`` with your miniconda installation

.. code::

    conda create -n xeus-sql
    conda activate xeus-sql # Or `source activate xeus-sql` for conda < 4.6

.. code::

    mamba create -n xeus-sql
    mamba activate xeus-sql

Then you can install in this freshly created environment ``xeus-sql`` and its dependencies

.. code::

    conda install xeus-sql notebook -c conda-forge

.. code::

    mamba install xeus-sql notebook -c conda-forge

or, if you prefer to use JupyterLab_

.. code::

    conda install xeus-sql jupyterlab -c conda-forge

.. code::

    mamba install xeus-sql jupyterlab -c conda-forge

Conda forge offers packaged versions for MySQL, PostgreSQL and SQLite and you can download them with: `soci-mysql`, `soci-postresql` or `soci-sqlite`.

``xeus-sql`` includes ``soci-core`` only. Which consists on the SOCI package with no DB extension attached.

From Source
-----------

You can install ``xeus-sql`` from source with ``cmake``. This requires that you have all the dependencies installed in the same prefix.


.. code::

    conda install cmake nlohmann_json xtl cppzmq xeus mysql sqlite postgresql cpp-tabulate>=1.5 xvega xvega-bindings xproperty jupyterlab compilers -c conda-forge

.. code::

    mamba install cmake nlohmann_json xtl cppzmq xeus mysql sqlite postgresql cpp-tabulate>=1.5 xvega xvega-bindings xproperty jupyterlab compilers -c conda-forge

.. code::

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    make install

On Windows platforms, from the source directory:

.. code::

    mkdir build
    cd build
    cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    nmake
    nmake install

.. _miniconda: https://conda.io/miniconda.html
.. _anaconda: https://www.anaconda.com
.. _JupyterLab: https://jupyterlab.readthedocs.io
