# xeus-soci
[![Azure Pipelines](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_apis/build/status/jupyter-xeus.xeus-soci?branchName=master)](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_build/latest?definitionId=5&branchName=master)
[![Documentation Status](https://readthedocs.org/projects/xeus-soci/badge/?version=latest)](https://xeus-soci.readthedocs.io/en/latest/?badge=latest)
[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/jupyter-xeus/xeus-soci/stable?filepath=examples/Simple%20operations.ipynb)

`xeus-soci` is a Jupyter kernel for general SQL implementations based on the native implementation of the Jupyter protocol [xeus](https://github.com/jupyter-xeus/xeus) and [SOCI](https://github.com/SOCI/soci), a database access library for C++.

## Usage

Launch the Jupyter notebook with `jupyter notebook` or Jupyter lab with `jupyter lab` and launch a new SQL notebook by selecting the **xsoci** kernel.

Note that `xeus-soci` support the following SQL implementations:

* DB2 _(Needs help with documentation)_
* Firebird _(Needs help with documentation)_
* MySQL
* ODBC (generic backend) _(Needs help with documentation)_
* Oracle _(Needs help with documentation)_
* PostgreSQL
* SQLite3

## Installation

`xeus-soci` has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install `xeus-soci` in a fresh conda environment.

To ensure that the installation works, it is preferable to install `xeus` in a fresh conda environment. It is also needed to use
a [miniconda](https://conda.io/miniconda.html) installation because with the full [anaconda](https://www.anaconda.com/)
you may have a conflict.

The safest usage is to create an environment named `xeus-soci` with your miniconda installation

```
conda create -n xeus-soci
conda activate xeus-soci
```

### Installing from conda

To install xeus-soci with the conda package manager:

```
conda install xeus-soci jupyterlab -c conda-forge
```

With mamba:

```
mamba install xeus-soci jupyterlab -c conda-forge
```

Conda forge offers packaged versions for MySQL, PostgreSQL and SQLite and you can download them with: `soci-mysql`, `soci-postresql` or `soci-sqlite`.

`xeus-soci` includes `soci-core` only. Which consists on the SOCI package with no DB extension attached.

## Documentation

https://xeus-soci.readthedocs.io/en/latest/

## Dependencies

``xeus-soci`` depends on

- [xeus](https://github.com/jupyter-xeus/xeus)
- [Tabulate](https://github.com/p-ranav/tabulate)
- [xvega](https://github.com/Quantstack/xvega)
- [SQLite\*](https://github.com/sqlite/sqlite)
- [PostgreSQL\*](https://github.com/postgres)
- [MySQL\*](https://github.com/mysql/mysql-server)

| `xeus-soci`  |      `xeus`     | `PostgreSQL*`   |    `SQLite*`    |   `MySQL*`      |    `Tabulate`   | `nlohmann_json` |   `xvega`   |
|--------------|-----------------|-----------------|-----------------|-----------------|-----------------|----------------|-------------|
|    master    | >=0.24.0, <0.25 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    |                 |>=3.0.0         |   >= 0.0.5  |

\* These are optional dependencies that come by default with `xeus-soci` but may or may not be added to your project in case you're building manually. For more information check the CONTRIBUTING file.

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to know how to contribute and set up a development environment.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
