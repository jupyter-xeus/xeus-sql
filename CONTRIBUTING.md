# Contributing to xeus-soci

Xeus and xeus-sqlite are subprojects of Project Jupyter and subject to the [Jupyter governance](https://github.com/jupyter/governance) and [Code of conduct](https://github.com/jupyter/governance/blob/master/conduct/code_of_conduct.md).

## General Guidelines

For general documentation about contributing to Jupyter projects, see the [Project Jupyter Contributor Documentation](https://jupyter.readthedocs.io/en/latest/contributor/content-contributor.html).

## Community

The Xeus team organizes public video meetings. The schedule for future meetings and minutes of past meetings can be found on our [team compass](https://jupyter-xeus.github.io/).

## Setting up a development environment

### Installing from source

To install the xeus-soci dependencies:

```bash
conda install cmake nlohmann_json xtl cppzmq xeus sqlite mysql sqlite postgresql cpp-tabulate=1.3 xvega xvega-bindings xproperty jupyterlab -c conda-forge
```

Then you can compile the sources

```bash
mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=$CONDA_PREFIX ..
make
make install
```

### Build docs

```
mamba install sphinx breathe doxygen sphinx_rtd_theme
```

#### Tips on subprojects:

To build SOCI from source using a conda environment: `cmake -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -G "Unix Makefiles" -DWITH_BOOST=OFF -DSOCI_CXX11=ON -DCMAKE_BUILD_TYPE=Release ..`