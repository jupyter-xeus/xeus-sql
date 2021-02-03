# xeus-sql
[![Build Status](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_apis/build/status/jupyter-xeus.xeus-sql?branchName=master)](https://dev.azure.com/jupyter-xeus/jupyter-xeus/_build/latest?definitionId=9&branchName=master)
[![Appveyor](https://ci.appveyor.com/api/projects/status/3u3fu514g7xbx8oy?svg=true)](https://ci.appveyor.com/project/jupyter-xeus/xeus-sql)
[![Documentation Status](https://readthedocs.org/projects/xeus-sql/badge/?version=latest)](https://xeus-sql.readthedocs.io/en/latest/?badge=latest)
[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/jupyter-xeus/xeus-sql/stable?urlpath=lab/tree/examples/SQLite.ipynb)

`xeus-sql` is a Jupyter kernel for general SQL implementations based on the native implementation of the Jupyter protocol [xeus](https://github.com/jupyter-xeus/xeus) and [SOCI](https://github.com/SOCI/soci), a database access library for C++.

## Usage

Launch the Jupyter notebook with `jupyter notebook` or Jupyter lab with `jupyter lab` and launch a new SQL notebook by selecting the **xsql** kernel.

Note that `xeus-sql` support the following SQL implementations:

* DB2
* Firebird
* MySQL
* ODBC (generic backend)
* Oracle
* PostgreSQL
* SQLite3

## Installation

`xeus-sql` has been packaged for the conda package manager.

To ensure that the installation works, it is preferable to install `xeus-sql` in a fresh conda environment.

To ensure that the installation works, it is preferable to install `xeus` in a fresh conda environment. It is also needed to use
a [miniconda](https://conda.io/miniconda.html) installation because with the full [anaconda](https://www.anaconda.com/)
you may have a conflict.

The safest usage is to create an environment named `xeus-sql` with your miniconda installation

```
conda create -n xeus-sql
conda activate xeus-sql
```

### Installing from conda

To install xeus-sql with the conda package manager:

```
conda install xeus-sql jupyterlab -c conda-forge
```

With mamba:

```
mamba install xeus-sql jupyterlab -c conda-forge
```

Conda forge offers packaged versions for MySQL, PostgreSQL and SQLite and you can download them with: `soci-mysql`, `soci-postresql` or `soci-sqlite`.

`xeus-sql` includes `soci-core` only. Which consists on the SOCI package with no DB extension attached.

## Documentation

https://xeus-sql.readthedocs.io/en/latest/

## Dependencies

``xeus-sql`` depends on

- [xeus](https://github.com/jupyter-xeus/xeus)
- [Tabulate](https://github.com/p-ranav/tabulate)
- [xvega](https://github.com/Quantstack/xvega)
- [SQLite\*](https://github.com/sqlite/sqlite)
- [PostgreSQL\*](https://github.com/postgres)
- [MySQL\*](https://github.com/mysql/mysql-server)

| `xeus-sql`  |      `xeus`     | `PostgreSQL*`   |    `SQLite*`    |   `MySQL*`      |    `Tabulate`   | `nlohmann_json` |   `xvega`   | `soci-core` | `xproperty` | `xvega-bindings` |
|--------------|-----------------|-----------------|-----------------|-----------------|-----------------|----------------|-------------|------------|-----------|--------------|
|    master    | >=1.0.0, <2.0   | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    |       >=1.4|>=3.0.0         |   >= 0.0.10 | 4.0.1      | >=0.11.0    | >=0.0.3          |
|    <=0.0.8     | >=1.0.0, <2.0.0 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    |       >=1.4|>=3.0.0         |   >= 0.0.10 | 4.0.1      | 0.10.4      | >=0.0.3          |
|    <=0.0.6   | >=0.24.0, <0.25 | >=3.30.1, <4    | >=3.0.0, <4     | >=1.3.0,<2.0    |       >=1.4|>=3.0.0         |   >= 0.0.5  | 4.0.1      | 0.10.4      | >=0.0.3          |

\* These are optional dependencies that come by default with `xeus-sql` but may or may not be added to your project in case you're building manually. For more information check the CONTRIBUTING file.

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to know how to contribute and set up a development environment.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
