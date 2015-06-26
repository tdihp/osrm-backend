/*

Copyright (c) 2015, Project OSRM contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef RASTER_SOURCE_HPP
#define RASTER_SOURCE_HPP

#include <vector>

struct RasterDatum
{
    bool has_data;
    short datum;

    RasterDatum();

    RasterDatum(bool has_data);

    RasterDatum(short datum);

    ~RasterDatum();
};

class RasterSource
{
  private:
    const float xstep;
    const float ystep;

    float calcSize(double min, double max, unsigned count) const;

  public:
    std::vector<std::vector<short>> raster_data;

    const double xmin;
    const double xmax;
    const double ymin;
    const double ymax;

    RasterDatum getRasterData(const float lon, const float lat);

    RasterDatum getRasterInterpolate(const float lon, const float lat);

    RasterSource(std::vector<std::vector<short>> _raster_data,
                 double _xmin,
                 double _xmax,
                 double _ymin,
                 double _ymax);

    ~RasterSource();
};

int loadRasterSource(const std::string &source_path,
                     const double xmin,
                     const double xmax,
                     const double ymin,
                     const double ymax);

RasterDatum getRasterDataFromSource(int source_id, int lon, int lat);

RasterDatum getRasterInterpolateFromSource(int source_id, int lon, int lat);

#endif /* RASTER_SOURCE_HPP */
