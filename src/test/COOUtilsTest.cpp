/**
 * @file COOUtilsTest.cpp
 *
 * @license
 * Copyright (c) 2009-2013
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * @endlicense
 *
 * @brief Contains tests for the COOUtils interface to be tested on different devices
 * @author: Thomas Brandes
 * @date 19.07.2013
 * @since 1.1.0
 **/

// boost
#include <boost/test/unit_test.hpp>

// others
#include <lama/ContextAccess.hpp>
#include <lama/LAMAArray.hpp>
#include <lama/LAMAInterface.hpp>
#include <lama/HostReadAccess.hpp>
#include <lama/HostWriteAccess.hpp>
#include <lama/ReadAccess.hpp>
#include <lama/WriteAccess.hpp>

#include <test/TestMacros.hpp>

using namespace boost;
using namespace lama;

/* ------------------------------------------------------------------------------------------------------------------ */

// Dummy type, needed to use the lama interface
typedef bool NoType;

/* ------------------------------------------------------------------------------------------------------------------ */

namespace lama
{
namespace COOUtilsTest
{

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename NoType>
void offsets2iaTest( ContextPtr loc )
{
    {  
        // Test without diagonl property 
        const IndexType offsets_values[] = { 0, 2, 2, 3, 5 };
        const IndexType ia_values[] = { 0, 0, 2, 3, 3 };

        const IndexType numOffsets = sizeof( offsets_values ) / sizeof( IndexType );
        const IndexType numRows = numOffsets - 1;
        const IndexType numValues = sizeof( ia_values ) / sizeof( IndexType );

        // verify that offsets and ia fit 

        BOOST_REQUIRE_EQUAL( numValues, offsets_values[numRows] );

        LAMAArray<IndexType> offsets( numOffsets, offsets_values );
        LAMAArray<IndexType> ia;

        try
        {
            LAMA_INTERFACE_FN( offsets2ia, loc, COOUtils, Counting );

            ReadAccess<IndexType> rOffsets( offsets, loc );
            WriteOnlyAccess<IndexType> wIA( ia, loc, numValues );

            const IndexType numDiagonals = 0;

            LAMA_CONTEXT_ACCESS( loc );

            offsets2ia( wIA.get(), numValues, rOffsets.get(), numRows, numDiagonals );
        }
        catch( Exception )
        {
            std::cout <<  "WARN: COOUtils::offsets2ia not available on " << *loc << ", not tested" << std::endl;
            return;
        }
    
        {
            HostReadAccess<IndexType> rIA( ia );
    
            for ( int i = 0; i < numValues; ++i )
            {
                BOOST_CHECK_EQUAL( rIA[i], ia_values[i] );
            }
        }
    }

    {  
        // Test with diagonal property 

        const IndexType offsets_values[] = { 0, 2, 5, 7, 9 };

        // Result with diagonals = 0: ia_values[] = { 0, 0, 1, 1, 1, 2, 2, 3, 3 };
        // But with 3 diagonals we get this:
        const IndexType ia_values[] = { 0, 1, 2, 0, 1, 1, 2, 3, 3 };

        const IndexType numOffsets = sizeof( offsets_values ) / sizeof( IndexType );
        const IndexType numRows = numOffsets - 1;
        const IndexType numValues = sizeof( ia_values ) / sizeof( IndexType );

        // verify that offsets and ia fit 

        BOOST_REQUIRE_EQUAL( numValues, offsets_values[numRows] );

        LAMAArray<IndexType> offsets( numOffsets, offsets_values );
        LAMAArray<IndexType> ia;

        try
        {
            LAMA_INTERFACE_FN( offsets2ia, loc, COOUtils, Counting );

            ReadAccess<IndexType> rOffsets( offsets, loc );
            WriteOnlyAccess<IndexType> wIA( ia, loc, numValues );

            const IndexType numDiagonals = 3;

            LAMA_CONTEXT_ACCESS( loc );

            offsets2ia( wIA.get(), numValues, rOffsets.get(), numRows, numDiagonals );
        }
        catch( Exception )
        {
            std::cout <<  "WARN: COOUtils::offsets2ia not available on " << *loc << ", not tested" << std::endl;
            return;
        }
    
        {
            HostReadAccess<IndexType> rIA( ia );
    
            for ( int i = 0; i < numValues; ++i )
            {
                // LAMA_LOG_TRACE( logger,  "rIA[" << i << "] = " << rIA[i] << ", expects " << ia_values[i] )
                BOOST_CHECK_EQUAL( rIA[i], ia_values[i] );
            }
        }
    }

}  // offsets2iaTest

template<typename NoType>
void setCSRDataTest( ContextPtr loc )
{
    // setCSRData is for conversion of CSR storage to COO storage
    // is usually just a copy but has some reordering if diagonal property is required
    // here we test only for csrJA

    const IndexType offsets_values[] = { 0, 2, 5, 7, 9 };

    const IndexType csrja_values[] = { 0, 5, 1, 4, 5, 2, 0, 4, 3 };
    const IndexType cooja_values[] = { 0, 1, 2, 5, 4, 5, 0, 4, 3 };

    const IndexType numOffsets = sizeof( offsets_values ) / sizeof( IndexType );
    const IndexType numRows = numOffsets - 1;
    const IndexType numDiagonals = 3;
    const IndexType numValues = sizeof( csrja_values ) / sizeof( IndexType );

    // verify that offsets and ia fit 

    BOOST_REQUIRE_EQUAL( numValues, offsets_values[numRows] );
    BOOST_REQUIRE( numDiagonals <= numRows );

    LAMAArray<IndexType> offsets( numOffsets, offsets_values );
    LAMAArray<IndexType> csrJA( numValues, csrja_values );
    LAMAArray<IndexType> cooJA;

    try
    {
        LAMA_INTERFACE_FN_TT( setCSRData, loc, COOUtils, Conversions, IndexType, IndexType );

        ReadAccess<IndexType> rOffsets( offsets, loc );
        ReadAccess<IndexType> rCSRJA( csrJA, loc );
        WriteOnlyAccess<IndexType> wCOOJA( cooJA, loc, numValues );

        LAMA_CONTEXT_ACCESS( loc );

        setCSRData( wCOOJA.get(), rCSRJA.get(), numValues, rOffsets.get(), numRows, numDiagonals );
    }
    catch( Exception )
    {
        std::cout <<  "WARN: COOUtils::setCSRData not available on " << *loc << ", not tested" << std::endl;
        return;
    }

    {
        HostReadAccess<IndexType> rCOOJA( cooJA );

        for ( int i = 0; i < numValues; ++i )
        {
            BOOST_CHECK_EQUAL( rCOOJA[i], cooja_values[i] );
        }
    }

}  // setCSRData

} //namespace COOUtilsTest

} //namespace lama

/* ------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE( COOUtilsTest );

LAMA_LOG_DEF_LOGGER( logger, "Test.COOUtilsTest" );

LAMA_AUTO_TEST_CASE_TDUMMY( offsets2iaTest, COOUtilsTest );
LAMA_AUTO_TEST_CASE_TDUMMY( setCSRDataTest, COOUtilsTest );

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END();