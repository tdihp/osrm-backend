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

#include "../util/simple_logger.hpp"
#include "../util/timing_util.hpp"
#include "../util/osrm_exception.hpp"

#include "../typedefs.h"

#include <osrm/coordinate.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <unordered_map>
#include <vector>
#include <sstream>
#include <cmath>
#include <iostream>

class RasterSource
{
  private:
    const float xstep;
    const float ystep;

    float calcSize(double min, double max, unsigned count)
    {
        SimpleLogger().Write() << "count: " << count;
        return (max - min) / count;
    };

  public:
    std::vector<std::vector<short>> raster_data;

    const double xmin;
    const double xmax;
    const double ymin;
    const double ymax;

    signed short getRasterData(const float lon, const float lat)
    {
        if (lon < xmin || lon > xmax || lat < ymin || lat > ymax)
        {
            return -1;
            // throw osrm::exception("Requested data out of range");
        }

        unsigned xthP = (lon - xmin) / xstep;
        int xth = ((xthP - floor (xthP)) > (xstep / 2) ? floor (xthP) : ceil (xthP));

        unsigned ythP = (ymax - lat) / ystep;
        int yth = ((ythP - floor (ythP)) > (ystep / 2) ? floor (ythP) : ceil (ythP));

        return raster_data[yth][xth];
    };

    signed short getRasterInterpolate(const float lon, const float lat)
    {
        if (lon < xmin || lon > xmax || lat < ymin || lat > ymax)
        {
            return -1;
            // throw osrm::exception("Requested data out of range");
        }

        unsigned xthP = (lon - xmin) / xstep;
        unsigned ythP = (ymax - lat) / ystep;
        int top    = floor (ythP);
        int bottom = ceil  (ythP);
        int left   = floor (xthP);
        int right  = ceil  (xthP);

        float x = (lon - left * xstep + xmin) / xstep;
        float y = (ymax - top * ystep - lat) / ystep;
        float x1 = 1.0 - x;
        float y1 = 1.0 - y;

        return raster_data[top][left]     * (x1 * y1) +
               raster_data[top][right]    * (x  * y1) +
               raster_data[bottom][left]  * (x1 *  y) +
               raster_data[bottom][right] * (x  *  y);
    };

    RasterSource(std::vector<std::vector<short>> _raster_data,
                 double _xmin,
                 double _xmax,
                 double _ymin,
                 double _ymax)
        : xstep(calcSize(_xmin, _xmax, _raster_data[0].size())),
          ystep(calcSize(_ymin, _ymax, _raster_data.size())),
          raster_data(_raster_data),
          xmin(_xmin),
          xmax(_xmax),
          ymin(_ymin),
          ymax(_ymax) {};

    ~RasterSource() {};
};

std::unordered_map<std::string, RasterSource> LoadedSources;
std::unordered_map<std::string, std::string> LoadedSourcePaths;

void loadRasterSource(const std::string &source_path, const std::string &source_id, const double xmin, const double xmax, const double ymin, const double ymax)
{
    auto itr = LoadedSourcePaths.find(source_path);
    if (itr != LoadedSourcePaths.end())
    {
        std::cout << "[source loader] Already loaded source '" << source_path << "' with source_id " << itr->second << std::endl;
        return;
    }

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

        while (ss >> datum) {
            lineData.emplace_back(datum);
        }
        rasterData.emplace_back(lineData);
        lineData.clear();
    }

    RasterSource source(rasterData, xmin, xmax, ymin, ymax);
    LoadedSourcePaths.emplace(source_path, source_id);
    LoadedSources.emplace(source_id, source);

    TIMER_STOP(loading_source);
    std::cout << "ok, after " << TIMER_SEC(loading_source) << "s" << std::endl;
};

signed short getRasterDataFromSource(const std::string &source_id, const int lat, const int lon)
{
    auto itr = LoadedSources.find(source_id);
    if (itr != LoadedSources.end())
    {
        RasterSource found = itr->second;
        return found.getRasterData(float(lat) / COORDINATE_PRECISION, float(lon) / COORDINATE_PRECISION);
    }
    else
    {
        throw osrm::exception("error reading: no such loaded source");
    }
};

signed short getRasterInterpolateFromSource(const std::string &source_id, const int lat, const int lon)
{
    auto itr = LoadedSources.find(source_id);
    if (itr != LoadedSources.end())
    {
        RasterSource found = itr->second;
        return found.getRasterInterpolate(float(lat) / COORDINATE_PRECISION, float(lon) / COORDINATE_PRECISION);
    }
    else
    {
        throw osrm::exception("error reading: no such loaded source");
    }
};

#endif /* RASTER_SOURCE_HPP */
