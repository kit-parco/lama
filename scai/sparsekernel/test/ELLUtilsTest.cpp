/**
 * @file ELLUtilsTest.cpp
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
 * @brief Contains tests for the class CUDAELLUtils and OpenMPELLUtils
 * @author: Jan Ecker
 * @date 15.10.2012
 * @since 1.0.0
 **/

// boost
#include <boost/test/unit_test.hpp>

// others
#include <scai/sparsekernel/ELLKernelTrait.hpp>
#include <scai/kregistry.hpp>
#include <scai/hmemo.hpp>

#include <scai/sparsekernel/test/TestMacros.hpp>

using namespace scai::sparsekernel;
using namespace scai::hmemo;
using namespace scai::kregistry;

using scai::common::Exception;

/* ------------------------------------------------------------------------------------------------------------------ */

// Dummy type, needed to use the lama interface
typedef bool NoType;

/* ------------------------------------------------------------------------------------------------------------------ */

namespace scai
{

namespace lama
{

namespace ELLUtilsTest
{

template<typename NoType>
void countNonEmptyRowsBySizesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::countNonEmptyRowsBySizes> countNonEmptyRowsBySizes;

    // count valid array
    {
        const IndexType values[] = { 3, 0, 1, 0, 0, 1, 0, 4 };
        const IndexType n = sizeof( values ) / sizeof( IndexType );
        HArray<IndexType> sizes( n );

        initArray( sizes, values, n );

        ReadAccess<IndexType> rSizes( sizes, loc );
        SCAI_CONTEXT_ACCESS( loc );
        IndexType count = countNonEmptyRowsBySizes[loc->getType()]( rSizes.get(), n );
        BOOST_CHECK_EQUAL( 4, count );
    }
    // count empty array
    {
        HArray<IndexType> sizes;
        ReadAccess<IndexType> rSizes( sizes, loc );
        SCAI_CONTEXT_ACCESS( loc );
        IndexType count = countNonEmptyRowsBySizes[loc->getType()]( rSizes.get(), sizes.size() );
        BOOST_CHECK_EQUAL( 0, count );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename NoType>
void setNonEmptyRowsBySizesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::setNonEmptyRowsBySizes> setNonEmptyRowsBySizes;

    const IndexType values[] = { 3, 0, 1, 0, 0, 1, 0, 4, 3, 0 };
    const IndexType valuesResult[] = { 0, 2, 5, 7, 8 };
    const IndexType n = 10;
    const IndexType numNonEmptyRows = 5;
    HArray<IndexType> sizes( n );
    HArray<IndexType> rowIndexes( numNonEmptyRows );

    initArray( sizes, values, n );
    initArray( rowIndexes, 0, numNonEmptyRows );

    {
        ReadAccess<IndexType> rSizes( sizes, loc );
        WriteAccess<IndexType> wRowIndexes( rowIndexes, loc );
        SCAI_CONTEXT_ACCESS( loc );
        setNonEmptyRowsBySizes[loc->getType()]( wRowIndexes.get(), numNonEmptyRows, rSizes.get(), n );
    }
    {
        ReadAccess<IndexType> rRowIndexes( rowIndexes );

        for ( IndexType i = 0; i < numNonEmptyRows; ++i )
        {
            BOOST_CHECK_EQUAL( valuesResult[i], rRowIndexes[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename NoType>
void hasDiagonalPropertyTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::hasDiagonalProperty> hasDiagonalProperty;

    // positive test
    {
        const IndexType ellJaValues[] = { 0, 1, 2, 3, 4, 5, 6, 7, 5, 3, 9, 10, 7, 8, 9, 10 };
        const IndexType n = sizeof( ellJaValues ) / sizeof( IndexType );
        const IndexType numDiagonals = 8;
        HArray<IndexType> ellJa( n );

        initArray( ellJa, ellJaValues, n );

        ReadAccess<IndexType> rEllJa( ellJa, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty = hasDiagonalProperty[loc->getType()]( numDiagonals, rEllJa.get() );
        BOOST_CHECK_EQUAL( true, diagonalProperty );
    }
    // negative test
    {
        const IndexType ellJaValues[] = { 0, 1, 2, 3, 7, 5, 6, 7, 5, 3, 9, 10, 7, 8, 9, 10 };
        const IndexType n = sizeof( ellJaValues ) / sizeof( IndexType );
        const IndexType numDiagonals = 8;
        HArray<IndexType> ellJa( n );

        initArray( ellJa, ellJaValues, n );

        ReadAccess<IndexType> rEllJa( ellJa, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty = hasDiagonalProperty[loc->getType()]( numDiagonals, rEllJa.get() );
        BOOST_CHECK_EQUAL( false, diagonalProperty );
    }
    // test empty array
    {
        const IndexType numDiagonals = 0;
        HArray<IndexType> ellJa;
        ReadAccess<IndexType> rEllJa( ellJa, loc );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty = hasDiagonalProperty[loc->getType()]( numDiagonals, rEllJa.get() );
        BOOST_CHECK_EQUAL( false, diagonalProperty );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename NoType>
void checkTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::check> check;
    // check with correct values
    {
        const IndexType valuesIa[] = { 4, 3, 5, 2 };
        const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
        const IndexType valuesJa[] = { 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, -1, 4, -1, 4, 0, 0, 0, 5, 0 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        const IndexType numRows = nIa;
        const IndexType numValuesPerRow = 5;
        const IndexType numColumns = 6;
        HArray<IndexType> ia( nIa );
        HArray<IndexType> ja( nJa );

        initArray( ia, valuesIa, nIa );
        initArray( ja, valuesJa, nJa );

        ReadAccess<IndexType> rIa( ia, loc );
        ReadAccess<IndexType> rJa( ja, loc );
        SCAI_CONTEXT_ACCESS( loc );
        BOOST_CHECK_NO_THROW( check[loc->getType()]( numRows, numValuesPerRow, numColumns, rIa.get(), rJa.get(), "checkTest" ) );
    }
    // check with invalid ia
    {
        const IndexType valuesIa[] = { 4, 3, 7, 2 };
        const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
        const IndexType valuesJa[] = { 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 0, 4, 0, 4, 0, 0, 0, 5, 0 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        const IndexType numRows = nIa;
        const IndexType numValuesPerRow = 5;
        const IndexType numColumns = 5;
        HArray<IndexType> ia( nIa );
        HArray<IndexType> ja( nJa );

        initArray( ia, valuesIa, nIa );
        initArray( ja, valuesJa, nJa );

        ReadAccess<IndexType> rIa( ia, loc );
        ReadAccess<IndexType> rJa( ja, loc );
        SCAI_CONTEXT_ACCESS( loc );
        BOOST_CHECK_THROW( check[loc->getType()]( numRows, numValuesPerRow, numColumns, rIa.get(), rJa.get(), "checkTest" ),
                           Exception );
    }
    // check with invalid ja
    {
        const IndexType valuesIa[] = { 4, 3, 5, 2 };
        const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
        const IndexType valuesJa[] = { 1, 1, 1, 1, 2, 2, 2, -1, 3, 3, 3, 0, 4, 0, 4, 0, 0, 0, 5, 0 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        const IndexType numRows = nIa;
        const IndexType numValuesPerRow = 5;
        const IndexType numColumns = 5;
        HArray<IndexType> ia( nIa );
        HArray<IndexType> ja( nJa );

        initArray( ia, valuesIa, nIa );
        initArray( ja, valuesJa, nJa );

        ReadAccess<IndexType> rIa( ia, loc );
        ReadAccess<IndexType> rJa( ja, loc );
        SCAI_CONTEXT_ACCESS( loc );
        BOOST_CHECK_THROW( check[loc->getType()]( numRows, numValuesPerRow, numColumns, rIa.get(), rJa.get(), "checkTest" ),
                           Exception );
    }
    // check with valid empty values
    {
        const IndexType numRows = 0;
        const IndexType numValuesPerRow = 0;
        const IndexType numColumns = 0;
        HArray<IndexType> ia;
        HArray<IndexType> ja;
        ReadAccess<IndexType> rIa( ia, loc );
        ReadAccess<IndexType> rJa( ja, loc );
        SCAI_CONTEXT_ACCESS( loc );
        BOOST_CHECK_NO_THROW( check[loc->getType()]( numRows, numValuesPerRow, numColumns, rIa.get(), rJa.get(), "checkTest" ) );
    }
    // check with invalid empty values
    {
        const IndexType numRows = 0;
        const IndexType numValuesPerRow = 1;
        const IndexType numColumns = 1;
        HArray<IndexType> ia;
        HArray<IndexType> ja;
        ReadAccess<IndexType> rIa( ia, loc );
        ReadAccess<IndexType> rJa( ja, loc );
        SCAI_CONTEXT_ACCESS( loc );
        BOOST_CHECK_THROW( check[loc->getType()]( numRows, numValuesPerRow, numColumns, rIa.get(), rJa.get(), "checkTest" ),
                           Exception );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType, typename OtherValueType>
void getRowTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::getRow<ValueType, OtherValueType> > getRow;

    // check with valid dense values
    {
        ValueType valuesValues[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
        const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
        IndexType valuesIa[] = { 5, 5, 5 };
        const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
        IndexType valuesJa[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        OtherValueType expectedValues[] = { 0, 1, 2, 3, 4 };
        const IndexType i = 1;
        const IndexType numRows = nIa;
        const IndexType numValuesPerRow = nJa / nIa;
        const IndexType numColumns = 5;
        HArray<ValueType> values( nValues );
        HArray<IndexType> ia( nIa );
        HArray<IndexType> ja( nJa );
        HArray<OtherValueType> row( numColumns );

        initArray( values, valuesValues, nValues );
        initArray( ia, valuesIa, nIa );
        initArray( ja, valuesJa, nJa );
        initArray( row, OtherValueType(0), numColumns );

        {
            ReadAccess<ValueType> rValues( values, loc );
            ReadAccess<IndexType> rIa( ia, loc );
            ReadAccess<IndexType> rJa( ja, loc );
            WriteOnlyAccess<OtherValueType> wRow( row, loc, numColumns );
            SCAI_CONTEXT_ACCESS( loc );
            getRow[loc->getType()]( wRow.get(), i, numRows, numColumns, numValuesPerRow, rIa.get(), rJa.get(), rValues.get() );
        }
        ReadAccess<OtherValueType> rRow( row );

        for ( IndexType i = 0; i < numColumns; i++ )
        {
            BOOST_CHECK_EQUAL( expectedValues[i], rRow[i] );
        }
    }
    // check with valid sparse values
    {
        ValueType valuesValues[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
        const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
        IndexType valuesIa[] = { 5, 5, 5 };
        const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
        IndexType valuesJa[] =
        { 0, 0, 0, 2, 2, 2, 4, 4, 4, 6, 6, 6, 10, 10, 10 };
        const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
        OtherValueType expectedValues[] =
        { 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 4 };
        const IndexType i = 1;
        const IndexType numRows = nIa;
        const IndexType numColumns = 11;
        const IndexType numValuesPerRow = nJa / nIa;
        HArray<ValueType> values( nValues );
        HArray<IndexType> ia( nIa );
        HArray<IndexType> ja( nJa );
        HArray<OtherValueType> row( numColumns );

        initArray( values, valuesValues, nValues );
        initArray( ia, valuesIa, nIa );
        initArray( ja, valuesJa, nJa );
        initArray( row, OtherValueType(0), numColumns );

        {
            ReadAccess<ValueType> rValues( values, loc );
            ReadAccess<IndexType> rIa( ia, loc );
            ReadAccess<IndexType> rJa( ja, loc );
            WriteOnlyAccess<OtherValueType> wRow( row, loc, numColumns );
            SCAI_CONTEXT_ACCESS( loc );
            getRow[loc->getType()]( wRow.get(), i, numRows, numColumns, numValuesPerRow, rIa.get(), rJa.get(), rValues.get() );
        }
        ReadAccess<OtherValueType> rRow( row );

        for ( IndexType i = 0; i < numColumns; i++ )
        {
            BOOST_CHECK_EQUAL( expectedValues[i], rRow[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void getValueTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::getValue<ValueType> > getValue;

    ValueType valuesValues[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType nValues = sizeof( valuesValues ) / sizeof( ValueType );
    IndexType valuesIa[] =
    { 5, 5, 5 };
    const IndexType nIa = sizeof( valuesIa ) / sizeof( IndexType );
    IndexType valuesJa[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType nJa = sizeof( valuesJa ) / sizeof( IndexType );
    ValueType expectedValues[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType numRows = nIa;
    const IndexType numValuesPerRow = nValues / numRows;
    BOOST_REQUIRE_EQUAL( numRows * numValuesPerRow, nValues );
    HArray<ValueType> values( nValues );
    HArray<IndexType> ia( nIa );
    HArray<IndexType> ja( nJa );

    initArray( values, valuesValues, nValues );
    initArray( ia, valuesIa, nIa );
    initArray( ja, valuesJa, nJa );

    ReadAccess<ValueType> rValues( values, loc );
    ReadAccess<IndexType> rIa( ia, loc );
    ReadAccess<IndexType> rJa( ja, loc );
    SCAI_CONTEXT_ACCESS( loc );

    for ( IndexType i = 0; i < numRows; i++ )
    {
        for ( IndexType j = 0; j < valuesIa[i]; j++ )
        {
            ValueType result = getValue[loc->getType()]( i, j, numRows, numValuesPerRow, rIa.get(), rJa.get(), rValues.get() );
            BOOST_CHECK_EQUAL( expectedValues[j * numRows + i], result );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType, typename OtherValueType>
void scaleValueTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::scaleValue<ValueType, OtherValueType> > scaleValue;

    ValueType mValues[] =
    { 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 4, 2, 0, 1, 3, 0, 0, 0, 0, 3 };
    const IndexType nValues = sizeof( mValues ) / sizeof( ValueType );
    const ValueType expectedValues[] =
    { 2, 4, 15, 8, 10, 4, 4, 10, 4, 4, 8, 4, 0, 2, 6, 0, 0, 0, 0, 6 };
    HArray<ValueType> ellValues( nValues );

    initArray( ellValues, mValues, nValues );
    {
        const IndexType numRows = 5;
        const IndexType numValuesPerRow = nValues / numRows;
        const IndexType ellIaValues[] =
        { 3, 3, 3, 3, 4 };
        const IndexType n = sizeof( ellIaValues ) / sizeof( IndexType );
        const OtherValueType values[] =
        { 2, 2, 5, 2, 2 };
        HArray<IndexType> ellIa( n );
        HArray<OtherValueType> scaleValues( n );

        initArray( ellIa, ellIaValues, n );
        initArray( scaleValues, values, n );

        ReadAccess<IndexType> rEllIa( ellIa, loc );
        WriteAccess<ValueType> wEllValues( ellValues, loc );
        ReadAccess<OtherValueType> rScaleValues( scaleValues, loc );
        SCAI_CONTEXT_ACCESS( loc );
        scaleValue[loc->getType()]( numRows, numValuesPerRow, rEllIa.get(), wEllValues.get(), rScaleValues.get() );
    }
    ReadAccess<ValueType> rEllValues( ellValues );

    for ( IndexType i = 0; i < nValues; i++ )
    {
        BOOST_CHECK_EQUAL( expectedValues[i], rEllValues[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType, typename OtherValueType>
void getCSRValuesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::getCSRValues<ValueType, OtherValueType> > getCSRValues;

    ValueType valuesELLValues[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
    IndexType valuesELLIa[] =
    { 5, 5, 5 };
    const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
    IndexType valuesELLJa[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
    IndexType valuesCSRIa[] =
    { 0, 5, 10, 15 };
    const IndexType nCSRIa = sizeof( valuesCSRIa ) / sizeof( IndexType );
    OtherValueType expectedCSRValues[] =
    { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 };
    IndexType expectedCSRJa[] =
    { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 };
    const IndexType numRows = nELLIa;
    const IndexType numValuesPerRow = nELLValues / numRows;
    // make sure that division did fit
    BOOST_REQUIRE_EQUAL( numValuesPerRow * numRows, nELLValues );
    const IndexType nCSRValues = 15;
    HArray<ValueType> ellValues( nELLValues );
    HArray<IndexType> ellIa( nELLIa );
    HArray<IndexType> ellJa( nELLJa );
    HArray<OtherValueType> csrValues( nCSRValues );
    HArray<IndexType> csrIa( nCSRIa );
    HArray<IndexType> csrJa( nCSRValues );

    initArray( ellValues, valuesELLValues, nELLValues );
    initArray( ellIa, valuesELLIa, nELLIa );
    initArray( ellJa, valuesELLJa, nELLJa );
    initArray( csrValues, OtherValueType(0), nCSRValues );
    initArray( csrIa, valuesCSRIa, nCSRIa );
    initArray( csrJa, 0, nCSRValues );

    {
        ReadAccess<ValueType> rELLValues( ellValues, loc );
        ReadAccess<IndexType> rELLIa( ellIa, loc );
        ReadAccess<IndexType> rELLJa( ellJa, loc );
        ReadAccess<IndexType> rCSRIa( csrIa, loc );
        WriteOnlyAccess<OtherValueType> wCSRValues( csrValues, loc, nCSRValues );
        WriteOnlyAccess<IndexType> wCSRJa( csrJa, loc, nCSRValues );
        SCAI_CONTEXT_ACCESS( loc );
        getCSRValues[loc->getType()] ( wCSRJa.get(), wCSRValues.get(), rCSRIa.get(), numRows, numValuesPerRow, rELLIa.get(),
                            rELLJa.get(), rELLValues.get() );
    }
    ReadAccess<IndexType> rCSRJa( csrJa );
    ReadAccess<OtherValueType> rCSRValues( csrValues );

    for ( IndexType i = 0; i < nCSRValues; i++ )
    {
        BOOST_CHECK_EQUAL( expectedCSRJa[i], rCSRJa[i] );
        BOOST_CHECK_EQUAL( expectedCSRValues[i], rCSRValues[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType, typename OtherValueType>
void setCSRValuesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::setCSRValues<OtherValueType, ValueType> > setCSRValues;

    ValueType valuesCSRValues[] =
    { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 };
    const IndexType nCSRValues = sizeof( valuesCSRValues ) / sizeof( ValueType );
    IndexType valuesCSRIa[] =
    { 0, 5, 10, 15 };
    const IndexType nCSRIa = sizeof( valuesCSRIa ) / sizeof( IndexType );
    IndexType valuesCSRJa[] =
    { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 };
    const IndexType nCSRJa = sizeof( valuesCSRJa ) / sizeof( IndexType );
    IndexType valuesELLIa[] =
    { 5, 5, 5 };
    const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
    OtherValueType expectedELLValues[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    IndexType expectedELLJa[] =
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };
    const IndexType numRows = nELLIa;
    const IndexType nELLValues = 15;
    const IndexType numValuesPerRow = 5;
    HArray<ValueType> csrValues( nCSRValues );
    HArray<IndexType> csrIa( nCSRIa );
    HArray<IndexType> csrJa( nCSRJa );
    HArray<IndexType> ellIa( nELLIa );
    // initialization of ellValues and ellJA, even if not mandatory
    HArray<OtherValueType> ellValues( nELLValues );
    HArray<IndexType> ellJa( nELLValues );

    initArray( csrValues, valuesCSRValues, nCSRValues );
    initArray( csrIa, valuesCSRIa, nCSRIa );
    initArray( csrJa, valuesCSRJa, nCSRJa );
    initArray( ellIa, valuesELLIa, nELLIa );
    initArray( ellValues, OtherValueType(0), nELLValues );
    initArray( ellJa, nELLValues, 0 );
    {
        ReadAccess<ValueType> rCSRValues( csrValues, loc );
        ReadAccess<IndexType> rCSRIa( csrIa, loc );
        ReadAccess<IndexType> rCSRJa( csrJa, loc );
        ReadAccess<IndexType> rELLIa( ellIa, loc );
        WriteOnlyAccess<OtherValueType> wELLValues( ellValues, loc, nELLValues );
        WriteOnlyAccess<IndexType> wELLJa( ellJa, loc, nELLValues );
        SCAI_CONTEXT_ACCESS( loc );
        setCSRValues[loc->getType()]( wELLJa.get(), wELLValues.get(), rELLIa.get(), numRows, numValuesPerRow, rCSRIa.get(),
                           rCSRJa.get(), rCSRValues.get() );
    }
    ReadAccess<IndexType> rELLJa( ellJa );
    ReadAccess<OtherValueType> rELLValues( ellValues );

    for ( IndexType i = 0; i < nELLValues; i++ )
    {
        BOOST_CHECK_EQUAL( expectedELLJa[i], rELLJa[i] );
        BOOST_CHECK_EQUAL( expectedELLValues[i], rELLValues[i] );
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void compressIATest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::compressIA<ValueType> > compressIA;

    // Check without epsilon
    {
        ValueType valuesELLValues[] =
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        IndexType expectedELLIa[] =
        { 2, 3, 4 };
        const IndexType numRows = nELLIa;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const ValueType eps = 0.0;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<IndexType> newEllIa( nELLIa );

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllIa, 0, nELLIa );
        {
            SCAI_CONTEXT_ACCESS( loc );

            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<IndexType> wNewELLIa( newEllIa, loc, nELLIa );
            compressIA[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps, wNewELLIa.get() );
        }
        ReadAccess<IndexType> rNewELLIa( newEllIa );

        for ( IndexType i = 0; i < nELLIa; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLIa[i], rNewELLIa[i] );
        }
    }
    // Check with epsilon
    {
        ValueType valuesELLValues[] =
        { 1, 1, 1, 1, 1, 1, 0.01, 0.01, -0.01, -0.001, 0.001, 0.02, 0.001, 1, 1 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        IndexType expectedELLIa[] =
        { 2, 3, 4 };
        const IndexType numRows = nELLIa;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const ValueType eps = 0.01;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<IndexType> newEllIa( nELLIa);

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllIa, 0, nELLIa );

        {
            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<IndexType> wNewELLIa( newEllIa, loc, nELLIa );
            SCAI_CONTEXT_ACCESS( loc );
            compressIA[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps, wNewELLIa.get() );
        }
        ReadAccess<IndexType> rNewELLIa( newEllIa );

        for ( IndexType i = 0; i < nELLIa; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLIa[i], rNewELLIa[i] );
        }
    }
    // Check if compress destroys diagonal property (it shouldn't!)
    {
        ValueType valuesELLValues[] =
        { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        IndexType expectedELLIa[] =
        { 5, 5, 5 };
        const IndexType numRows = nELLIa;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const ValueType eps = 0.0;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<IndexType> newEllIa( nELLIa );

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllIa, 0, nELLIa );
        {
            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<IndexType> wNewELLIa( newEllIa, loc, nELLIa );
            SCAI_CONTEXT_ACCESS( loc );
            compressIA[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps, wNewELLIa.get() );
        }
        ReadAccess<IndexType> rNewELLIa( newEllIa );

        for ( IndexType i = 0; i < nELLIa; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLIa[i], rNewELLIa[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void compressValuesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::compressValues<ValueType> > compressValues;

    // Check without epsilon
    {
        ValueType valuesELLValues[] =
        { 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 7, 0, 8, 9 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        ValueType expectedELLValues[] =
        { 1, 2, 3, 4, 5, 6, 0, 8, 7, 0, 0, 9 };
        IndexType expectedELLJa[] =
        { 0, 1, 2, 3, 3, 3, 0, 6, 5, 0, 0, 6 };
        const IndexType numRows = nELLIa;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const ValueType eps = 0.0;
        const IndexType numValues = 12;
        const IndexType newNumValuesPerRow = numValues / nELLIa;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<ValueType> newEllValues( numValues );
        HArray<IndexType> newEllJa( numValues );

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllValues, ValueType(0), numValues );
        initArray( newEllJa, 0, numValues );
        {
            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<ValueType> wNewELLValues( newEllValues, loc, numValues );
            WriteOnlyAccess<IndexType> wNewELLJa( newEllJa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            compressValues[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps,
                                 newNumValuesPerRow, wNewELLJa.get(), wNewELLValues.get() );
        }
        ReadAccess<ValueType> rNewELLValues( newEllValues );
        ReadAccess<IndexType> rNewELLJa( newEllJa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLValues[i], rNewELLValues[i] );
            BOOST_CHECK_EQUAL( expectedELLJa[i], rNewELLJa[i] );
        }
    }
    // Check with epsilon
    {
        ValueType valuesELLValues[] =
        { 0.02, 2, 3, 4, 5, 6, 0.01, -0.01, 0.002, -0.002, 0.01, 7, -0.01, 8, 9 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        ValueType expectedELLValues[] =
        { 0.02, 2, 3, 4, 5, 6, 0, 8, 7, 0, 0, 9 };
        IndexType expectedELLJa[] =
        { 0, 1, 2, 3, 3, 3, 0, 6, 5, 0, 0, 6 };
        const IndexType numRows = nELLIa;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const ValueType eps = 0.01;
        const IndexType numValues = 12;
        const IndexType newNumValuesPerRow = numValues / nELLIa;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<ValueType> newEllValues( numValues );
        HArray<IndexType> newEllJa( numValues );

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllValues, ValueType(0), numValues );
        initArray( newEllJa, 0, numValues );
        {
            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<ValueType> wNewELLValues( newEllValues, loc, numValues );
            WriteOnlyAccess<IndexType> wNewELLJa( newEllJa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            compressValues[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps,
                                 newNumValuesPerRow, wNewELLJa.get(), wNewELLValues.get() );
        }
        ReadAccess<ValueType> rNewELLValues( newEllValues );
        ReadAccess<IndexType> rNewELLJa( newEllJa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLValues[i], rNewELLValues[i] );
            BOOST_CHECK_EQUAL( expectedELLJa[i], rNewELLJa[i] );
        }
    }
    // Check if compress destroys diagonal property (it shouldn't!)
    {
        ValueType valuesELLValues[] =
        { 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0, 7, 0, 8, 9 };
        const IndexType nELLValues = sizeof( valuesELLValues ) / sizeof( ValueType );
        IndexType valuesELLIa[] =
        { 5, 5, 5 };
        const IndexType nELLIa = sizeof( valuesELLIa ) / sizeof( IndexType );
        IndexType valuesELLJa[] =
        { 0, 1, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6 };
        const IndexType nELLJa = sizeof( valuesELLJa ) / sizeof( IndexType );
        ValueType expectedELLValues[] =
        { 0, 0, 0, 4, 5, 6, 0, 8, 7, 0, 0, 9 };
        IndexType expectedELLJa[] =
        { 0, 1, 2, 3, 3, 3, 0, 6, 5, 0, 0, 6 };
        const IndexType numRows = nELLIa;
        const ValueType eps = 0.01;
        const IndexType numValues = 12;
        const IndexType numValuesPerRow = nELLJa / nELLIa;
        const IndexType newNumValuesPerRow = numValues / nELLIa;
        HArray<ValueType> ellValues( nELLValues );
        HArray<IndexType> ellIa( nELLIa );
        HArray<IndexType> ellJa( nELLJa );
        HArray<ValueType> newEllValues( numValues );
        HArray<IndexType> newEllJa( numValues );

        initArray( ellValues, valuesELLValues, nELLValues );
        initArray( ellIa, valuesELLIa, nELLIa );
        initArray( ellJa, valuesELLJa, nELLJa );
        initArray( newEllValues, ValueType(0), numValues );
        initArray( newEllJa, 0, numValues );
        {
            ReadAccess<ValueType> rELLValues( ellValues, loc );
            ReadAccess<IndexType> rELLIa( ellIa, loc );
            ReadAccess<IndexType> rELLJa( ellJa, loc );
            WriteOnlyAccess<ValueType> wNewELLValues( newEllValues, loc, numValues );
            WriteOnlyAccess<IndexType> wNewELLJa( newEllJa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            compressValues[loc->getType()]( rELLIa.get(), rELLJa.get(), rELLValues.get(), numRows, numValuesPerRow, eps,
                                 newNumValuesPerRow, wNewELLJa.get(), wNewELLValues.get() );
        }
        ReadAccess<ValueType> rNewELLValues( newEllValues );
        ReadAccess<IndexType> rNewELLJa( newEllJa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedELLValues[i], rNewELLValues[i] );
            BOOST_CHECK_EQUAL( expectedELLJa[i], rNewELLJa[i] );
        }
    }
}

template<typename NoType>
void matrixMultiplySizesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::matrixMultiplySizes> matrixMultiplySizes;

    // Check with symmetric matrix
    {
        IndexType valuesAIa[] =
        { 2, 3, 2, 3, 4 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        IndexType valuesBIa[] =
        { 2, 2, 2, 3, 3 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType expectedCIa[] =
        { 4, 4, 3, 4, 4 };
        IndexType numValues = 5; // all matrices have shape 5 x 5
        IndexType aNumValuesPerRow = aNumValues / numValues;
        IndexType bNumValuesPerRow = bNumValues / numValues;
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<IndexType> CIa( numValues );

        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CIa, 0, numValues );
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            WriteOnlyAccess<IndexType> wCIa( CIa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            matrixMultiplySizes[loc->getType()]( wCIa.get(), numValues, numValues, numValues, false, rAIa.get(), rAJa.get(),
                                      aNumValuesPerRow, rBIa.get(), rBJa.get(), bNumValuesPerRow );
        }
        ReadAccess<IndexType> rCIa( CIa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCIa[i], rCIa[i] );
        }
    }
    // Check with asymmetric matrix
    {
        //   A       B
        //   x x 0   x  x
        //   0 x x   x
        //   0 0 x   x
        IndexType valuesAIa[] =
        { 2, 2, 2 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 0, 1, 0, 2, 3, 3 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        IndexType valuesBIa[] =
        { 2, 1, 2, 3 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 1, 0, 0, 2, 0, 1, 1, 0, 0, 0, 2 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType expectedCIa[] =
        { 3, 3, 3 };
        IndexType cNumRows = sizeof( expectedCIa ) / sizeof( IndexType );
        BOOST_REQUIRE_EQUAL( aNumRows, cNumRows );
        // a and a * b have same number rows
        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<IndexType> CIa( cNumRows );

        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CIa, 0, cNumRows );
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            WriteOnlyAccess<IndexType> wCIa( CIa, loc, cNumRows );
            SCAI_CONTEXT_ACCESS( loc );
            IndexType numColumns = 5; // does not really matter
            matrixMultiplySizes[loc->getType()]( wCIa.get(), aNumRows, numColumns, bNumRows, false, rAIa.get(), rAJa.get(),
                                      aNumValuesPerRow, rBIa.get(), rBJa.get(), bNumValuesPerRow );
        }
        ReadAccess<IndexType> rCIa( CIa );

        for ( IndexType i = 0; i < cNumRows; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCIa[i], rCIa[i] );
        }
    }
}

template<typename ValueType>
void matrixMultiplyTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::matrixMultiply<ValueType> > matrixMultiply;

    // Check with symmetric matrix
    {
        ValueType valuesAValues[] =
        { 1, 5, 2, 4, 3, 3, 7, 3, 7, 9, 0, 8, 0, 9, 8, 0, 0, 0, 0, 7 };
        const IndexType nAValues = sizeof( valuesAValues ) / sizeof( ValueType );
        IndexType valuesAIa[] =
        { 2, 3, 2, 3, 4 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        ValueType valuesBValues[] =
        { 3, 4, 9, 8, 3, 8, 8, 7, 9, 7, 0, 0, 0, 5, 0 };
        const IndexType nBValues = sizeof( valuesBValues ) / sizeof( ValueType );
        IndexType valuesBIa[] =
        { 2, 2, 2, 3, 2 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType valuesCIa[] =
        { 4, 4, 3, 4, 4 };
        const IndexType cNumRows = sizeof( valuesCIa ) / sizeof( IndexType );
        ValueType expectedCValues[] =
        { 28, 71, 8, 84, 73, 27, 63, 27, 144, 153, 15, 99, 37, 77, 85, 8, 56, 0, 49, 112 };
        IndexType expectedCJa[] =
        { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 3, 2, 2, 3, 3, 0, 3, 3 };
        IndexType numValues = 20;
        ValueType alpha = 1;
        HArray<ValueType> AValues( nAValues );
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<ValueType> BValues( nBValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<ValueType> CValues( numValues );
        HArray<IndexType> CIa( cNumRows );
        HArray<IndexType> CJa( numValues );

        initArray( AValues, valuesAValues, nAValues );
        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BValues, valuesBValues, nBValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CValues, ValueType(0), numValues );
        initArray( CIa, valuesCIa, cNumRows );
        initArray( CJa, 0, numValues );

        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        IndexType cNumValuesPerRow = numValues / cNumRows;
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<ValueType> rAValues( AValues, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            ReadAccess<ValueType> rBValues( BValues, loc );
            ReadAccess<IndexType> rCIa( CIa, loc );
            WriteOnlyAccess<ValueType> wCValues( CValues, loc, numValues );
            WriteOnlyAccess<IndexType> wCJa( CJa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            IndexType numColumns = 5; // not really needed here but internally used
            bool diagonalProperty = false; // do not care about it here
            matrixMultiply[loc->getType()]( wCJa.get(), wCValues.get(), rCIa.get(), cNumValuesPerRow, aNumRows, numColumns, bNumRows,
                                 diagonalProperty, alpha, rAIa.get(), rAJa.get(), rAValues.get(), aNumValuesPerRow,
                                 rBIa.get(), rBJa.get(), rBValues.get(), bNumValuesPerRow );
        }
        ReadAccess<ValueType> rCValues( CValues );
        ReadAccess<IndexType> rCJa( CJa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCValues[i], rCValues[i] );
            BOOST_CHECK_EQUAL( expectedCJa[i], rCJa[i] );
        }
    }
    // Check with set alpha
    {
        ValueType valuesAValues[] =
        { 1, 5, 2, 4, 3, 3, 7, 3, 7, 9, 0, 8, 0, 9, 8, 0, 0, 0, 0, 7 };
        const IndexType nAValues = sizeof( valuesAValues ) / sizeof( ValueType );
        IndexType valuesAIa[] =
        { 2, 3, 2, 3, 4 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        ValueType valuesBValues[] =
        { 3, 4, 9, 8, 3, 8, 8, 7, 9, 7, 0, 0, 0, 5, 0 };
        const IndexType nBValues = sizeof( valuesBValues ) / sizeof( ValueType );
        IndexType valuesBIa[] =
        { 2, 2, 2, 3, 2 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType valuesCIa[] =
        { 4, 4, 3, 4, 4 };
        const IndexType cNumRows = sizeof( valuesCIa ) / sizeof( IndexType );
        ValueType expectedCValues[] =
        { 28, 71, 8, 84, 73, 27, 63, 27, 144, 153, 15, 99, 37, 77, 85, 8, 56, 0, 49, 112 };
        IndexType expectedCJa[] =
        { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 3, 2, 2, 3, 3, 0, 3, 3 };
        IndexType numValues = 20;
        ValueType alpha = 2.5;
        HArray<ValueType> AValues( nAValues );
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<ValueType> BValues( nBValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<ValueType> CValues( numValues );
        HArray<IndexType> CIa( cNumRows );
        HArray<IndexType> CJa( numValues );

        initArray( AValues, valuesAValues, nAValues );
        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BValues, valuesBValues, nBValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CValues, ValueType(0), numValues );
        initArray( CIa, valuesCIa, cNumRows );
        initArray( CJa, 0, numValues );


        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        IndexType cNumValuesPerRow = numValues / cNumRows;
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<ValueType> rAValues( AValues, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            ReadAccess<ValueType> rBValues( BValues, loc );
            ReadAccess<IndexType> rCIa( CIa, loc );
            WriteOnlyAccess<ValueType> wCValues( CValues, loc, numValues );
            WriteOnlyAccess<IndexType> wCJa( CJa, loc, numValues );
            SCAI_CONTEXT_ACCESS( loc );
            bool diagonalProperty = false; // do not care about it
            IndexType numColumns = 15; // does not matter here but internally used for optimizations
            matrixMultiply[loc->getType()]( wCJa.get(), wCValues.get(), rCIa.get(), cNumValuesPerRow, aNumRows, numColumns, bNumRows,
                                 diagonalProperty, alpha, rAIa.get(), rAJa.get(), rAValues.get(), aNumValuesPerRow,
                                 rBIa.get(), rBJa.get(), rBValues.get(), bNumValuesPerRow );
        }
        ReadAccess<ValueType> rCValues( CValues );
        ReadAccess<IndexType> rCJa( CJa );

        for ( IndexType i = 0; i < numValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCValues[i]*alpha, rCValues[i] );
            BOOST_CHECK_EQUAL( expectedCJa[i], rCJa[i] );
        }
    }
    // Check with asymmetric matrix
    {
        ValueType valuesAValues[] =
        { 2, 4, 4, 3, 1, 5 };
        const IndexType nAValues = sizeof( valuesAValues ) / sizeof( ValueType );
        IndexType valuesAIa[] =
        { 2, 2, 2 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 0, 1, 0, 2, 3, 3 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        BOOST_REQUIRE_EQUAL( aNumValues, nAValues );
        ValueType valuesBValues[] =
        { 4, 3, 7, 5, 9, 0, 6, 8, 0, 0, 0, 9 };
        const IndexType nBValues = sizeof( valuesBValues ) / sizeof( ValueType );
        IndexType valuesBIa[] =
        { 2, 1, 2, 3 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 1, 0, 0, 2, 0, 1, 1, 0, 0, 0, 2 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        BOOST_REQUIRE_EQUAL( bNumValues, nBValues );
        IndexType valuesCIa[] =
        { 3, 3, 3 };
        const IndexType cNumRows = sizeof( valuesCIa ) / sizeof( IndexType );
        ValueType expectedCValues[] =
        { 29, 5, 41, 18, 20, 40, 18, 9, 81 };
        IndexType expectedCJa[] =
        { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
        IndexType cNumValues = 9;
        ValueType alpha = 1;
        HArray<ValueType> AValues( nAValues );
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<ValueType> BValues( nBValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<IndexType> CIa( cNumRows );
        HArray<ValueType> CValues( cNumValues );
        HArray<IndexType> CJa( cNumValues );

        initArray( AValues, valuesAValues, nAValues );
        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BValues, valuesBValues, nBValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CIa, valuesCIa, cNumRows );

        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        IndexType cNumValuesPerRow = cNumValues / cNumRows;
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<ValueType> rAValues( AValues, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            ReadAccess<ValueType> rBValues( BValues, loc );
            ReadAccess<IndexType> rCIa( CIa, loc );
            WriteOnlyAccess<ValueType> wCValues( CValues, loc, cNumValues );
            WriteOnlyAccess<IndexType> wCJa( CJa, loc, cNumValues );
            SCAI_CONTEXT_ACCESS( loc );
            bool diagonalProperty = false; // do not care about it
            IndexType numColumns = 15; // does not matter here but internally used for optimizations
            matrixMultiply[loc->getType()]( wCJa.get(), wCValues.get(), rCIa.get(), cNumValuesPerRow, aNumRows, numColumns, bNumRows,
                                 diagonalProperty, alpha, rAIa.get(), rAJa.get(), rAValues.get(), aNumValuesPerRow,
                                 rBIa.get(), rBJa.get(), rBValues.get(), bNumValuesPerRow );
        }
        ReadAccess<ValueType> rCValues( CValues );
        ReadAccess<IndexType> rCJa( CJa );

        for ( IndexType i = 0; i < cNumValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCValues[i], rCValues[i] );
            BOOST_CHECK_EQUAL( expectedCJa[i], rCJa[i] );
        }
    }
}

template<typename NoType>
void matrixAddSizesTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::matrixAddSizes > matrixAddSizes;

    IndexType valuesAIa[] =
    { 2, 3, 2, 3, 4 };
    const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
    IndexType valuesAJa[] =
    { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
    const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
    IndexType valuesBIa[] =
    { 2, 2, 2, 3, 3 };
    const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
    IndexType valuesBJa[] =
    { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
    const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
    IndexType expectedCIa[] =
    { 4, 3, 3, 4, 4 };
    const IndexType expectedCNumRows = sizeof( expectedCIa ) / sizeof( IndexType );
    // for matrix add A and B must have same shape
    BOOST_REQUIRE_EQUAL( aNumRows, bNumRows );
    IndexType cNumRows = aNumRows; // C gets same shape as A and B
    BOOST_REQUIRE_EQUAL( cNumRows, expectedCNumRows );
    // values per row needed, verify that numValues is multiple of numRows
    IndexType aNumValuesPerRow = aNumValues / aNumRows;
    BOOST_REQUIRE_EQUAL( aNumRows * aNumValuesPerRow, aNumValues );
    IndexType bNumValuesPerRow = bNumValues / bNumRows;
    BOOST_REQUIRE_EQUAL( bNumRows * bNumValuesPerRow, bNumValues );
    HArray<IndexType> AIa( aNumRows );
    HArray<IndexType> AJa( aNumValues );
    HArray<IndexType> BIa( bNumRows );
    HArray<IndexType> BJa( bNumValues );
    HArray<IndexType> CIa( cNumRows );

    initArray( AIa, valuesAIa, aNumRows );
    initArray( AJa, valuesAJa, aNumValues );
    initArray( BIa, valuesBIa, bNumRows );
    initArray( BJa, valuesBJa, bNumValues );
    initArray( CIa, 0, cNumRows );
    {
        ReadAccess<IndexType> rAIa( AIa, loc );
        ReadAccess<IndexType> rAJa( AJa, loc );
        ReadAccess<IndexType> rBIa( BIa, loc );
        ReadAccess<IndexType> rBJa( BJa, loc );
        WriteOnlyAccess<IndexType> wCIa( CIa, loc, cNumRows );
        SCAI_CONTEXT_ACCESS( loc );
        bool diagonalProperty = false;
        IndexType numColumns = aNumRows; // square matrices here
        matrixAddSizes[loc->getType()]( wCIa.get(), aNumRows, numColumns, diagonalProperty, rAIa.get(), rAJa.get(), aNumValuesPerRow,
                             rBIa.get(), rBJa.get(), bNumValuesPerRow );
    }
    ReadAccess<IndexType> rCIa( CIa );

    for ( IndexType i = 0; i < cNumRows; i++ )
    {
        BOOST_CHECK_EQUAL( expectedCIa[i], rCIa[i] );
    }
}

template<typename ValueType>
void matrixAddTest( ContextPtr loc )
{
    KernelTraitContextFunction<ELLKernelTrait::matrixAdd<ValueType> > matrixAdd;
    // Check with neutral beta
    {
        ValueType valuesAValues[] =
        { 1, 5, 2, 4, 3, 3, 7, 3, 7, 9, 0, 8, 0, 9, 8, 0, 0, 0, 0, 7 };
        const IndexType nAValues = sizeof( valuesAValues ) / sizeof( ValueType );
        IndexType valuesAIa[] =
        { 2, 3, 2, 3, 4 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        ValueType valuesBValues[] =
        { 3, 4, 9, 8, 3, 8, 8, 7, 9, 7, 0, 0, 0, 5, 0 };
        const IndexType nBValues = sizeof( valuesBValues ) / sizeof( ValueType );
        IndexType valuesBIa[] =
        { 2, 2, 2, 3, 2 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType valuesCIa[] =
        { 4, 3, 3, 4, 4 };
        const IndexType cNumRows = sizeof( valuesCIa ) / sizeof( IndexType );
        ValueType expectedCValues[] =
        { 3, 9, 11, 12, 3, 1, 15, 3, 9, 12, 8, 8, 7, 12, 15, 3, 0, 0, 9, 7 };
        IndexType expectedCJa[] =
        { 0, 0, 1, 0, 0, 1, 3, 2, 1, 2, 2, 4, 3, 2, 3, 3, 0, 0, 3, 4 };
        IndexType cNumValues = 20;
        IndexType numColumns = 5; // for convenience
        ValueType alpha = 1;
        ValueType beta = 1;
        HArray<ValueType> AValues( nAValues );
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<ValueType> BValues( nBValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<ValueType> CValues( cNumValues );
        HArray<IndexType> CIa( cNumRows );
        HArray<IndexType> CJa( cNumValues );

        initArray( AValues, valuesAValues, nAValues );
        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BValues, valuesBValues, nBValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CValues, ValueType(0), cNumValues );
        initArray( CIa, valuesCIa, cNumRows );
        initArray( CJa, 0, cNumValues );

        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        IndexType cNumValuesPerRow = cNumValues / cNumRows;
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<ValueType> rAValues( AValues, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            ReadAccess<ValueType> rBValues( BValues, loc );
            ReadAccess<IndexType> rCIa( CIa, loc );
            WriteOnlyAccess<ValueType> wCValues( CValues, loc, cNumValues );
            WriteOnlyAccess<IndexType> wCJa( CJa, loc, cNumValues );
            SCAI_CONTEXT_ACCESS( loc );
            bool diagonalProperty = false; // does not matter here
            matrixAdd[loc->getType()]( wCJa.get(), wCValues.get(), rCIa.get(), cNumValuesPerRow, aNumRows, numColumns, diagonalProperty,
                            alpha, rAIa.get(), rAJa.get(), rAValues.get(), aNumValuesPerRow, beta, rBIa.get(), rBJa.get(),
                            rBValues.get(), bNumValuesPerRow );
        }
        ReadAccess<ValueType> rCValues( CValues );
        ReadAccess<IndexType> rCJa( CJa );

        for ( IndexType i = 0; i < cNumValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCValues[i], rCValues[i] );
            BOOST_CHECK_EQUAL( expectedCJa[i], rCJa[i] );
        }
    }
    // Check with set beta
    {
        ValueType valuesAValues[] =
        { 1, 5, 2, 4, 3, 3, 7, 3, 7, 9, 0, 8, 0, 9, 8, 0, 0, 0, 0, 7 };
        const IndexType nAValues = sizeof( valuesAValues ) / sizeof( ValueType );
        IndexType valuesAIa[] =
        { 2, 3, 2, 3, 4 };
        const IndexType aNumRows = sizeof( valuesAIa ) / sizeof( IndexType );
        IndexType valuesAJa[] =
        { 1, 0, 1, 0, 0, 3, 3, 2, 2, 2, 0, 4, 0, 3, 3, 0, 0, 0, 0, 4 };
        const IndexType aNumValues = sizeof( valuesAJa ) / sizeof( IndexType );
        ValueType valuesBValues[] =
        { 3, 4, 9, 8, 3, 8, 8, 7, 9, 7, 0, 0, 0, 5, 0 };
        const IndexType nBValues = sizeof( valuesBValues ) / sizeof( ValueType );
        IndexType valuesBIa[] =
        { 2, 2, 2, 3, 2 };
        const IndexType bNumRows = sizeof( valuesBIa ) / sizeof( IndexType );
        IndexType valuesBJa[] =
        { 0, 0, 1, 0, 2, 2, 3, 3, 1, 3, 0, 0, 0, 2, 0 };
        const IndexType bNumValues = sizeof( valuesBJa ) / sizeof( IndexType );
        IndexType valuesCIa[] =
        { 4, 3, 3, 4, 4 };
        const IndexType cNumRows = sizeof( valuesCIa ) / sizeof( IndexType );
        ValueType expectedCValues[] =
        { 6, 13, 20, 20, 3, 1, 23, 3, 18, 15, 16, 8, 14, 17, 22, 3, 0, 0, 9, 7 };
        IndexType expectedCJa[] =
        { 0, 0, 1, 0, 0, 1, 3, 2, 1, 2, 2, 4, 3, 2, 3, 3, 0, 0, 3, 4 };
        IndexType cNumValues = 20;
        IndexType numColumns = 5; // for convenience
        ValueType alpha = 1;
        ValueType beta = 2;
        HArray<ValueType> AValues( nAValues );
        HArray<IndexType> AIa( aNumRows );
        HArray<IndexType> AJa( aNumValues );
        HArray<ValueType> BValues( nBValues );
        HArray<IndexType> BIa( bNumRows );
        HArray<IndexType> BJa( bNumValues );
        HArray<ValueType> CValues( cNumValues );
        HArray<IndexType> CIa( cNumRows );
        HArray<IndexType> CJa( cNumValues );

        initArray( AValues, valuesAValues, nAValues );
        initArray( AIa, valuesAIa, aNumRows );
        initArray( AJa, valuesAJa, aNumValues );
        initArray( BValues, valuesBValues, nBValues );
        initArray( BIa, valuesBIa, bNumRows );
        initArray( BJa, valuesBJa, bNumValues );
        initArray( CValues, ValueType(0), cNumValues );
        initArray( CIa, valuesCIa, cNumRows );
        initArray( CJa, 0, cNumValues );

        IndexType aNumValuesPerRow = aNumValues / aNumRows;
        IndexType bNumValuesPerRow = bNumValues / bNumRows;
        IndexType cNumValuesPerRow = cNumValues / cNumRows;
        // This did no work but should
        // LArray<ValueType> CValues;
        // LArray<IndexType> CJa;
        {
            ReadAccess<IndexType> rAIa( AIa, loc );
            ReadAccess<IndexType> rAJa( AJa, loc );
            ReadAccess<ValueType> rAValues( AValues, loc );
            ReadAccess<IndexType> rBIa( BIa, loc );
            ReadAccess<IndexType> rBJa( BJa, loc );
            ReadAccess<ValueType> rBValues( BValues, loc );
            ReadAccess<IndexType> rCIa( CIa, loc );
            WriteOnlyAccess<ValueType> wCValues( CValues, loc, cNumValues );
            WriteOnlyAccess<IndexType> wCJa( CJa, loc, cNumValues );
            SCAI_CONTEXT_ACCESS( loc );
            bool diagonalProperty = false; // does not matter here
            matrixAdd[loc->getType()]( wCJa.get(), wCValues.get(), rCIa.get(), cNumValuesPerRow, aNumRows, numColumns, diagonalProperty,
                            alpha, rAIa.get(), rAJa.get(), rAValues.get(), aNumValuesPerRow, beta, rBIa.get(), rBJa.get(),
                            rBValues.get(), bNumValuesPerRow );
        }
        ReadAccess<ValueType> rCValues( CValues );
        ReadAccess<IndexType> rCJa( CJa );

        for ( IndexType i = 0; i < cNumValues; i++ )
        {
            BOOST_CHECK_EQUAL( expectedCValues[i], rCValues[i] );
            BOOST_CHECK_EQUAL( expectedCJa[i], rCJa[i] );
        }
    }
}

// TODO: add SPMV tests

} /* end namespace ELLUtilsTest */

} /* end namespace lama */

} /* end namespace scai */

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE( ELLUtilsTest )

SCAI_LOG_DEF_LOGGER( logger, "Test.ELLUtilsTest" )

LAMA_AUTO_TEST_CASE_CTDUMMY( countNonEmptyRowsBySizesTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTDUMMY( setNonEmptyRowsBySizesTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTDUMMY( hasDiagonalPropertyTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTDUMMY( checkTest, ELLUtilsTest )

LAMA_AUTO_TEST_CASE_CTDUMMY( matrixMultiplySizesTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTDUMMY( matrixAddSizesTest, ELLUtilsTest )

LAMA_AUTO_TEST_CASE_CT( compressIATest, ELLUtilsTest, scai::lama )
LAMA_AUTO_TEST_CASE_CT( compressValuesTest, ELLUtilsTest, scai::lama )

LAMA_AUTO_TEST_CASE_CT( matrixMultiplyTest, ELLUtilsTest, scai::lama )
LAMA_AUTO_TEST_CASE_CT( getValueTest, ELLUtilsTest, scai ::lama )
LAMA_AUTO_TEST_CASE_CT( matrixAddTest, ELLUtilsTest, scai::lama )

LAMA_AUTO_TEST_CASE_CTT( getRowTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTT( scaleValueTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTT( getCSRValuesTest, ELLUtilsTest )
LAMA_AUTO_TEST_CASE_CTT( setCSRValuesTest, ELLUtilsTest )

/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END()