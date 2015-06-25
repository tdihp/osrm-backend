/*

Copyright (c) 2014, Project OSRM contributors
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

#include "../../data_structures/raster_source.hpp"
#include "../../typedefs.h"
#include "../../util/osrm_exception.hpp"

#include <boost/filesystem.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <random>
#include <vector>
#include <unordered_map>
#include <iostream>

BOOST_AUTO_TEST_SUITE(raster_source)

int normalize(float coord)
{
    return coord * COORDINATE_PRECISION;
}

BOOST_AUTO_TEST_CASE(raster_test)
{
    int source_id = loadRasterSource("../unit_tests/fixtures/raster_data.asc", 0, 0.09, 0, 0.09);
    BOOST_CHECK_EQUAL(source_id, 0);

    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(0.02), normalize(0.02)).datum, 0);
    // BOOST_CHECK_EQUAL(getRasterDataFromSource(0, 0.0, 0.00), 0);
    // TODO: 'unknown location:0: fatal error in "raster_test": signal: SIGSEGV, si_code: 0 (memory access violation at address: 0x00000000)' - seems to be boost problem w 0
    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(0.06), normalize(0.06)).datum, 10);
    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(0.06), normalize(0.01)).datum, 4);
    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(0.05), normalize(0.02)).datum, 4);
    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(0.05), normalize(0.02)).has_data, true);
    BOOST_CHECK_EQUAL(getRasterInterpolateFromSource(0, normalize(0.02), normalize(0.02)).datum, 0);
    BOOST_CHECK_EQUAL(getRasterInterpolateFromSource(0, normalize(0.02), normalize(0.02)).has_data, true);
    BOOST_CHECK_EQUAL(getRasterInterpolateFromSource(0, normalize(0.045), normalize(0.045)).datum, 8);
    BOOST_CHECK_EQUAL(getRasterDataFromSource(0, normalize(-0.05), normalize(0.02)).has_data, false);
    BOOST_CHECK_EQUAL(getRasterInterpolateFromSource(0, normalize(0.145), normalize(0.045)).has_data, false);

    int source_already_loaded_id = loadRasterSource("../unit_tests/fixtures/raster_data.asc", 0, 0.09, 0, 0.09);

    BOOST_CHECK_EQUAL(source_already_loaded_id, 0);
    BOOST_CHECK_THROW(getRasterDataFromSource(1, normalize(0.02), normalize(0.02)), osrm::exception);

    BOOST_CHECK_THROW(loadRasterSource("../unit_tests/fixtures/nonexistent.asc", 0, 0.1, 0, 0.1), osrm::exception);
}

BOOST_AUTO_TEST_SUITE_END()
