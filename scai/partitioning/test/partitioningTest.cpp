/**
 * @file partitioning/test/partitioningTest.cpp
 *
 * @license
 * Copyright (c) 2009-2017
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * This file is part of the SCAI framework LAMA.
 *
 * LAMA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 *
 * Other Usage
 * Alternatively, this file may be used in accordance with the terms and
 * conditions contained in a signed written agreement between you and
 * Fraunhofer SCAI. Please contact our distributor via info[at]scapos.com.
 * @endlicense
 *
 * @brief Driver for tests of module partitioning
 * @author Thomas Brandes
 * @date 23.08.2017
 */

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE partitioningTest

// indicate that default main of Boost is not used here

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

#include <scai/common/Settings.hpp>
#include <scai/common/OpenMP.hpp>
#include <scai/partitioning.hpp>

#include <iostream>

bool init_function()
{
    int nThreads;

    if ( scai::common::Settings::getEnvironment( nThreads, "SCAI_NUM_THREADS" ) )
    {
        omp_set_num_threads( nThreads );
    }

    try
    {
        scai::dmemo::CommunicatorPtr comm = scai::dmemo::Communicator::getCommunicatorPtr();
        return true;
    }
    catch ( scai::common::Exception& ex )
    {
        std::cerr << "Could not get the default communicator: " << ex.what() << std::endl;
        return false;
    }
}

int main( int argc, char* argv[] )
{
    scai::common::Settings::parseArgs( argc, const_cast<const char**>( argv ) );
    return boost::unit_test::unit_test_main( &init_function, argc, argv );
}