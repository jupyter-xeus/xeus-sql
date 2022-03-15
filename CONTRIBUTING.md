# Contributing to xeus-sql

xeus and xeus-sql are subprojects of Project Jupyter and subject to the [Jupyter governance](https://github.com/jupyter/governance) and [Code of conduct](https://github.com/jupyter/governance/blob/master/conduct/code_of_conduct.md).

## General Guidelines

For general documentation about contributing to Jupyter projects, see the [Project Jupyter Contributor Documentation](https://jupyter.readthedocs.io/en/latest/contributor/content-contributor.html).

## Community

The xeus team organizes public video meetings. The schedule for future meetings and minutes of past meetings can be found on our [team compass](https://jupyter-xeus.github.io/).

## Setting up a development environment

### Installing from source

To install the xeus-sql dependencies:

```bash
mamba install nlohmann_json xtl cppzmq xeus cpp-tabulate=1.3 xvega xvega-bindings xproperty jupyterlab soci-core compilers cmake -c conda-forge
```

#### Known issues

If you're facing issues with your `cmake` installation, installing it again with `mamba install cmake -c conda-forge` works for me.

Then you can compile the sources

```bash
mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=$CONDA_PREFIX ..
# You can add options like -DXSQL_WITH_SQLITE3 to make the db type as required
# and xeus-soci will handle the runtime error for that db. Example:
# cmake -D CMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DXSQL_WITH_SQLITE3 ..
# Other options include: XSQL_WITH_POSTGRE_SQL and XSQL_WITH_MYSQL
make
make install
```

### Build docs

```
mamba install sphinx breathe doxygen sphinx_rtd_theme
```
