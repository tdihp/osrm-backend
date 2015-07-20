## Branch osrm-c

This branch is an effort to build [osrm-c](https://github.com/tdihp/osrm-c) on Windows, as DLL.

### requirements

1. http://build.project-osrm.org/libs_osrm_Release_test.7z
2. [Visual Studio 2013](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx) minimum install
3. [CTP_Nov2013](https://www.microsoft.com/en-us/download/details.aspx?id=41151)

### build

1. extract libs_osrm_Release_test.7z
2. clone this branch into extracted folder
3. copy build_osrm.bat in this repo to extracted folder, overwrite the existing one.
4. run build_osrm.bat

## About

The Open Source Routing Machine is a high performance routing engine written in C++11 designed to run on OpenStreetMap data.

## Current build status

| build config |  branch | status |
|:-------------|:--------|:------------|
| Linux        | master  | [![Build Status](https://travis-ci.org/Project-OSRM/osrm-backend.png?branch=master)](https://travis-ci.org/Project-OSRM/osrm-backend) |
| Linux        | develop | [![Build Status](https://travis-ci.org/Project-OSRM/osrm-backend.png?branch=develop)](https://travis-ci.org/Project-OSRM/osrm-backend) |
| Windows      | master/develop | [![Build status](https://ci.appveyor.com/api/projects/status/4iuo3s9gxprmcjjh)](https://ci.appveyor.com/project/DennisOSRM/osrm-backend) |
| LUAbind fork | master  | [![Build Status](https://travis-ci.org/DennisOSRM/luabind.png?branch=master)](https://travis-ci.org/DennisOSRM/luabind) |

## Building

For instructions on how to [build](https://github.com/Project-OSRM/osrm-backend/wiki/Building-OSRM) and [run OSRM](https://github.com/Project-OSRM/osrm-backend/wiki/Running-OSRM), please consult [the Wiki](https://github.com/Project-OSRM/osrm-backend/wiki).

To quickly try OSRM use our [free and daily updated online service](http://map.project-osrm.org)

## Documentation

See the Wiki's [server API documentation](https://github.com/Project-OSRM/osrm-backend/wiki/Server-api) as well as the [library API documentation](https://github.com/Project-OSRM/osrm-backend/wiki/Library-api)

## References in publications

When using the code in a (scientific) publication, please cite

```
@inproceedings{luxen-vetter-2011,
 author = {Luxen, Dennis and Vetter, Christian},
 title = {Real-time routing with OpenStreetMap data},
 booktitle = {Proceedings of the 19th ACM SIGSPATIAL International Conference on Advances in Geographic Information Systems},
 series = {GIS '11},
 year = {2011},
 isbn = {978-1-4503-1031-4},
 location = {Chicago, Illinois},
 pages = {513--516},
 numpages = {4},
 url = {http://doi.acm.org/10.1145/2093973.2094062},
 doi = {10.1145/2093973.2094062},
 acmid = {2094062},
 publisher = {ACM},
 address = {New York, NY, USA},
}
```

