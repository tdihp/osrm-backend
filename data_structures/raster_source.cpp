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

#include "raster_source.hpp"

#include "../util/simple_logger.hpp"
#include "../util/timing_util.hpp"
#include "../util/osrm_exception.hpp"

#include <osrm/coordinate.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <unordered_map>
#include <sstream>
#include <cmath>
#include <iostream>

// LoadedSources stores sources in memory; LoadedSourcePaths maps them to indices
std::vector<RasterSource> LoadedSources;
std::unordered_map<std::string, int> LoadedSourcePaths;

RasterDatum::RasterDatum(bool has_data) : has_data(has_data) {}

RasterDatum::RasterDatum(short datum) : has_data(true), datum(datum) {}

RasterDatum::~RasterDatum() {}

RasterSource::RasterSource(std::vector<std::vector<short>> _raster_data,
                           double _xmin,
                           double _xmax,
                           double _ymin,
                           double _ymax)
    : xstep(calcSize(_xmin, _xmax, _raster_data[0].size())),
      ystep(calcSize(_ymin, _ymax, _raster_data.size())), raster_data(_raster_data), xmin(_xmin),
      xmax(_xmax), ymin(_ymin), ymax(_ymax)
{
    BOOST_ASSERT(xstep != 0);
    BOOST_ASSERT(ystep != 0);
}

RasterSource::~RasterSource(){};

float RasterSource::calcSize(double min, double max, unsigned count) const
{
    BOOST_ASSERT(count > 0);
    return (max - min) / count;
}

// Query raster source for nearest data point
RasterDatum RasterSource::getRasterData(const float lon, const float lat)
{
    if (lon < xmin || lon > xmax || lat < ymin || lat > ymax)
    {
        return RasterDatum(false);
    }

    unsigned xthP = (lon - xmin) / xstep;
    int xth = ((xthP - floor(xthP)) > (xstep / 2) ? floor(xthP) : ceil(xthP));

    unsigned ythP = (ymax - lat) / ystep;
    int yth = ((ythP - floor(ythP)) > (ystep / 2) ? floor(ythP) : ceil(ythP));

    return RasterDatum(raster_data[yth][xth]);
}

// Query raster source using bilinear interpolation
RasterDatum RasterSource::getRasterInterpolate(const float lon, const float lat)
{
    if (lon < xmin || lon > xmax || lat < ymin || lat > ymax)
    {
        return RasterDatum(false);
    }

    unsigned xthP = (lon - xmin) / xstep;
    unsigned ythP = (ymax - lat) / ystep;
    int top = floor(ythP);
    int bottom = ceil(ythP);
    int left = floor(xthP);
    int right = ceil(xthP);

    float x = (lon - left * xstep + xmin) / xstep;
    float y = (ymax - top * ystep - lat) / ystep;
    float x1 = 1.0 - x;
    float y1 = 1.0 - y;

    return RasterDatum(static_cast<short>(
        (raster_data[top][left] * (x1 * y1) + raster_data[top][right] * (x * y1) +
         raster_data[bottom][left] * (x1 * y) + raster_data[bottom][right] * (x * y))));
}

// Load raster source into memory
int loadRasterSource(const std::string &source_path,
                     const double xmin,
                     const double xmax,
                     const double ymin,
                     const double ymax)
{
    auto itr = LoadedSourcePaths.find(source_path);
    if (itr != LoadedSourcePaths.end())
    {
        std::cout << "[source loader] Already loaded source '" << source_path << "' at source_id "
                  << itr->second << std::endl;
        return itr->second;
    }

    int source_id = LoadedSources.size();

    std::cout << "[source loader] Loading from " << source_path << "  ... " << std::flush;
    TIMER_START(loading_source);

    std::vector<std::vector<short>> rasterData;

    if (!boost::filesystem::exists(source_path.c_str()))
    {
        throw osrm::exception("error reading: no such path");
    }
    boost::filesystem::ifstream reader(source_path.c_str());

    std::stringstream ss;
    std::string line;
    std::vector<short> lineData;
    while (std::getline(reader, line))
    {
        ss.clear();
        ss.str("");
        ss << line;
        short datum;

        while (ss >> datum)
        {
            lineData.emplace_back(datum);
        }
        rasterData.emplace_back(lineData);
        lineData.clear();
    }

    RasterSource source(rasterData, xmin, xmax, ymin, ymax);
    LoadedSourcePaths.emplace(source_path, source_id);
    LoadedSources.emplace_back(source);

    TIMER_STOP(loading_source);
    std::cout << "ok, after " << TIMER_SEC(loading_source) << "s" << std::endl;

    return source_id;
}

// External function for looking up nearest data point from a specified source
RasterDatum getRasterDataFromSource(unsigned int source_id, int lon, int lat)
{
    if (LoadedSources.size() < source_id + 1)
    {
        throw osrm::exception("error reading: no such loaded source");
    }

    RasterSource found = LoadedSources[source_id];
    return found.getRasterData(float(lon) / COORDINATE_PRECISION,
                               float(lat) / COORDINATE_PRECISION);
}

// External function for looking up interpolated data from a specified source
RasterDatum getRasterInterpolateFromSource(unsigned int source_id, int lon, int lat)
{
    if (LoadedSources.size() < source_id + 1)
    {
        throw osrm::exception("error reading: no such loaded source");
    }

    RasterSource found = LoadedSources[source_id];
    return found.getRasterInterpolate(float(lon) / COORDINATE_PRECISION,
                                      float(lat) / COORDINATE_PRECISION);
}
