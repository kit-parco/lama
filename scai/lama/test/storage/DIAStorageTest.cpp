/**
 * @file DIAStorageTest.cpp
 *
 * @license
 * Copyright (c) 2009-2015
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
 * @brief Contains the implementation of the class DIAStorageTest
 * @author Thomas Brandes
 * @date 02.03.2012
 * @since 1.0.0
 */

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include <scai/lama/storage/DIAStorage.hpp>
#include <scai/common/test/TestMacros.hpp>
#include <scai/utilskernel/LArray.hpp>

using namespace scai;
using namespace lama;
using namespace utilskernel;
using namespace hmemo;

/* ------------------------------------------------------------------------- */

BOOST_AUTO_TEST_SUITE( DIAStorageTest )

SCAI_LOG_DEF_LOGGER( logger, "Test.DIAStorageTest" )

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( constructorTest, ValueType, scai_arithmetic_test_types )
{
    // Test the full DIAStorge constructor and the individual getter routines of DIA storage

    const IndexType numRows = 3;
    const IndexType numColumns = 3;
    const IndexType offsets[] =
    { 0, 1 };
    const ValueType values[] =
    { 0.5f, 0.5f, 0.3f, 0.2f, 0.1f, 0.0f };
    const IndexType numValues = sizeof( values ) / sizeof( ValueType );
    const IndexType numDiagonals = sizeof( offsets ) / sizeof( IndexType );
    LArray<IndexType> diaOffsets( 2, offsets );
    LArray<ValueType> diaValues( numValues, values );

    DIAStorage<ValueType> diaStorage( numRows, numColumns, numDiagonals, diaOffsets, diaValues );

    BOOST_REQUIRE_EQUAL( numRows, diaStorage.getNumRows() );
    BOOST_REQUIRE_EQUAL( numColumns, diaStorage.getNumColumns() );
    BOOST_REQUIRE_EQUAL( numDiagonals, diaStorage.getNumDiagonals() );
    {
        ReadAccess<IndexType> diaOffsets( diaStorage.getOffsets() );
        ReadAccess<ValueType> diaValues( diaStorage.getValues() );

        // DIA keeps values in same order

        for ( IndexType i = 0; i < numDiagonals; ++i )
        {
            BOOST_CHECK_EQUAL( offsets[i], diaOffsets[i] );
        }

        for ( IndexType i = 0; i < numValues; ++i )
        {
            BOOST_CHECK_EQUAL( values[i], diaValues[i] );
        }
    }
    // copy constructor (TODO Bea: on all available locations, after host runs without mistake
    // DIAStorage<ValueType> diaStorageCopy( diaStorage, loc );
//    TODO ThoBra: weiter unten die "Nachbildung" des Copy-Constructors laeuft fehlerfrei,
//    hier knallt es beim letzten BOOST_CHECK_EQUAL
//    DIAStorage<ValueType> diaStorageCopy( diaStorage );
//    SCAI_LOG_INFO( logger, "copy constructor" );
//
//    BOOST_REQUIRE_EQUAL( numRows, diaStorageCopy.getNumRows() );
//    BOOST_REQUIRE_EQUAL( numColumns, diaStorageCopy.getNumColumns() );
//    BOOST_REQUIRE_EQUAL( numDiagonals, diaStorageCopy.getNumDiagonals() );
//
//    {
//        ReadAccess<ValueType> diaValues( diaStorageCopy.getValues() );
//        BOOST_CHECK_EQUAL( diaValues.size(), numValues );
//
//        BOOST_CHECK_EQUAL( values[0], diaValues[0] );
//        BOOST_CHECK_EQUAL( values[1], diaValues[1] );
//        BOOST_CHECK_EQUAL( values[2], diaValues[2] );
//        BOOST_CHECK_EQUAL( values[3], diaValues[3] );
//        BOOST_CHECK_EQUAL( values[4], diaValues[4] );
//        BOOST_CHECK_EQUAL( values[5], diaValues[5] );
//    }
    LArray<IndexType> csrIa;
    LArray<IndexType> csrJa;
    LArray<ValueType> csrValues;
    // const IndexType csrIaResult[] = { 0, 2, 4, 5 };
    // const IndexType csrJaResult[] = { 0, 1, 1, 2, 2 };
    // const ValueType csrValuesResult[] = { 0.5, 0.2, 0.5, 0.1, 0.3 };
    diaStorage.buildCSRData( csrIa, csrJa, csrValues );
    IndexType numRowsDia = diaStorage.getNumRows();
    IndexType numColumnsDia = diaStorage.getNumColumns();
    IndexType numValuesCSR = csrJa.size();
    diaStorage.setCSRData( numRowsDia, numColumnsDia, numValuesCSR, csrIa, csrJa, csrValues );
    {
        ReadAccess<ValueType> diaValues( diaStorage.getValues() );
        SCAI_LOG_INFO( logger, "diaValues.size() = " << diaValues.size() );
        BOOST_CHECK_EQUAL( diaValues.size(), numValues );

        for ( IndexType i = 0; i < numValues; ++i )
        {
            BOOST_CHECK_EQUAL( values[0], diaValues[0] );
            BOOST_CHECK_EQUAL( values[1], diaValues[1] );
            BOOST_CHECK_EQUAL( values[2], diaValues[2] );
            BOOST_CHECK_EQUAL( values[3], diaValues[3] );
            BOOST_CHECK_EQUAL( values[4], diaValues[4] );
            BOOST_CHECK_EQUAL( values[5], diaValues[5] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_CASE_TEMPLATE( typenameTest, ValueType, scai_arithmetic_test_types )
{
    SCAI_LOG_INFO( logger, "typeNameTest for DIAStorage<" << common::TypeTraits<ValueType>::id() << ">" )

    // context does not matter here, so runs for every context

    std::string s = DIAStorage<ValueType>::typeName();

    BOOST_CHECK( s.length() > 0 );
}

/* ------------------------------------------------------------------------- */

BOOST_AUTO_TEST_SUITE_END();