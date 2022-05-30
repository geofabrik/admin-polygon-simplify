Simplification of OSM Admin Boundaries
======================================

This utility can simplify the members of administrative boundary relations,
and ensure that all these way members carry an admin_level tag equivalent to 
the smalles admin_level tag of all relations containing the way.

Dependencies
------------

* Osmium Libary and its dependencies except GEOS
* GDAL library
* Cmake
* C++ compiler with C++11 support

Debian:

```sh
apt install libosmium-dev libgdal-dev cmake build-essentials
```

Building
--------

    mkdir build
    cd build
    cmake ..
    make

Unit Tests
----------

Build the project as described above, afterwards run `make test`.


Authors
-------

This software was written for Geofabrik by Michael Reichert.

License
-------

This software is made available under the GNU General Public License, v3 or later.
