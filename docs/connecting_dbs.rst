If you're running ``xeus-soci`` with a conda-forge ``postgres`` build:

Create a symbolic link to ``/tmp/``:

``
sudo ln -s /var/run/postgresql/.s.PGSQL.5432 /tmp/.s.PGSQL.5432
``

sudo -u postgres createuser --superuser mariana

sudo service postgresql start 
sudo -u mariana psql 

sudo ln -s /var/run/postgresql/.s.PGSQL.5432 /tmp/.s.PGSQL.5432
