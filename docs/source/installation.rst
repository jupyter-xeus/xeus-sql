.. Copyright (c) 2020, Mariana Meireles

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Installation
============

With Conda or Mamba
-------------------

``xeus-soci`` has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install ``xeus-soci`` in a fresh conda/mamba environment.
It is also needed to use a miniconda_ installation because with the full anaconda_ you may have a conflict with
the ``zeromq`` library which is already installed in the anaconda distribution.


The safest usage is to create an environment named ``xeus-soci`` with your miniconda installation

.. code::

    conda create -n xeus-soci
    conda activate xeus-soci # Or `source activate xeus-soci` for conda < 4.6

.. code::

    mamba create -n xeus-soci
    mamba activate xeus-soci

Then you can install in this freshly created environment ``xeus-soci`` and its dependencies

.. code::

    conda install xeus-soci notebook -c conda-forge

.. code::

    mamba install xeus-soci notebook -c conda-forge

or, if you prefer to use JupyterLab_

.. code::

    conda install xeus-soci jupyterlab -c conda-forge

.. code::

    mamba install xeus-soci jupyterlab -c conda-forge

Conda forge offers packaged versions for MySQL, PostgreSQL and SQLite and you can download them with: `soci-mysql`, `soci-postresql` or `soci-sqlite`.

``xeus-soci`` includes ``soci-core`` only. Which consists on the SOCI package with no DB extension attached.

From Source
-----------

You can install ```xeus-soci``` from source with cmake. This requires that you have all the dependencies installed in the same prefix.


.. code::

    conda install cmake nlohmann_json xtl cppzmq xeus mysql sqlite postgresql cpp-tabulate=1.3 xvega xvega-bindings xproperty jupyterlab -c conda-forge

.. code::

    mamba install cmake nlohmann_json xtl cppzmq xeus mysql sqlite postgresql cpp-tabulate=1.3 xvega xvega-bindings xproperty jupyterlab -c conda-forge

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
