/**
 * @file OpenMPELLUtils.cpp
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
 * @brief Implementation of ELL utilities with OpenMP
 * @author Thomas Brandes
 * @date 04.07.2012
 * @since 1.0.0
 */

// hpp
#include <scai/lama/openmp/OpenMPELLUtils.hpp>
#include <scai/lama/openmp/OpenMP.hpp>

// others
#include <scai/lama/LAMAInterface.hpp>
#include <scai/lama/LAMAInterfaceRegistry.hpp>

// macros
#include <scai/lama/macros/unused.hpp>

// tracing
#include <scai/tracing.hpp>

// boost
#include <scai/common/bind.hpp>
#include <boost/preprocessor.hpp>

// stl
#include <set>
#include <map>
#include <cmath>

namespace scai
{

namespace lama
{

using std::abs;

using scai::common::getScalarType;

/* ------------------------------------------------------------------------------------------------------------------ */

SCAI_LOG_DEF_LOGGER( OpenMPELLUtils::logger, "OpenMP.ELLUtils" )

/* ------------------------------------------------------------------------------------------------------------------ */

IndexType OpenMPELLUtils::countNonEmptyRowsBySizes( const IndexType sizes[], const IndexType numRows )
{
    IndexType counter = 0;

    #pragma omp parallel for reduction( +:counter )

    for( IndexType i = 0; i < numRows; ++i )
    {
        if( sizes[i] > 0 )
        {
            counter++;
        }
    }

    SCAI_LOG_INFO( logger, "#non-zero rows = " << counter << ", counted by sizes" )

    return counter;
}

/* ------------------------------------------------------------------------------------------------------------------ */

void OpenMPELLUtils::setNonEmptyRowsBySizes(
    IndexType rowIndexes[],
    const IndexType numNonEmptyRows,
    const IndexType sizes[],
    const IndexType numRows )
{
    IndexType counter = 0;

    // Note: this routine is not easy to parallelize, no offsets for rowIndexes available

    for( IndexType i = 0; i < numRows; ++i )
    {
        if( sizes[i] > 0 )
        {
            rowIndexes[counter] = i;
            counter++;
        }
    }

    SCAI_ASSERT_EQUAL_DEBUG( counter, numNonEmptyRows )

    SCAI_LOG_INFO( logger, "#non-zero rows = " << counter << ", set by sizes" )
}

/* ------------------------------------------------------------------------------------------------------------------ */

bool OpenMPELLUtils::hasDiagonalProperty( const IndexType numDiagonals, const IndexType ellJA[] )
{
    SCAI_LOG_INFO( logger, "hasDiagonalProperty, #numDiagonals = " << numDiagonals )

    if( numDiagonals == 0 )
    {
        return false;
    }

    bool diagonalProperty = true;

    #pragma omp parallel for reduction( && : diagonalProperty )

    for( IndexType i = 0; i < numDiagonals; ++i )
    {
        if( !diagonalProperty )
        {
            continue;
        }

        if( ellJA[i] != i )
        {
            diagonalProperty = false;
        }
    }

    return diagonalProperty;
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType,typename OtherValueType>
void OpenMPELLUtils::scaleValue(
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    ValueType ellValues[],
    const OtherValueType values[] )
{
    SCAI_LOG_INFO( logger,
                   "scaleValue<" << getScalarType<ValueType>() << ", " << getScalarType<OtherValueType>() << ">" << ", #numRows = " << numRows )

    #pragma omp parallel for schedule( LAMA_OMP_SCHEDULE )

    for( IndexType i = 0; i < numRows; i++ ) //rows
    {
        for( IndexType jj = 0; jj < ellSizes[i]; jj++ ) //elements in row
        {
            IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
            ellValues[pos] *= static_cast<ValueType>( values[i] );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void OpenMPELLUtils::check(
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType numColumns,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const char* msg )
{
    SCAI_LOG_INFO( logger,
                   "check # numRows = " << numRows << ", numValuesPerRow = " << numValuesPerRow << ", numColumns = " << numColumns )

    if( numRows > 0 )
    {
        bool integrityIA = true;
        bool integrityJA = true;

        #pragma omp parallel for reduction( && : integrityIA, integrityJA ) schedule( LAMA_OMP_SCHEDULE )

        for( IndexType i = 0; i < numRows; i++ )
        {
            if( ellSizes[i] >= 0 && ellSizes[i] <= numValuesPerRow )
            {
                for( IndexType jj = 0; jj < ellSizes[i]; jj++ )
                {
                    IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                    IndexType j = ellJA[pos];
                    integrityJA = integrityJA && ( 0 <= j && j < numColumns );
                }
            }
            else
            {
                integrityIA = false;
            }
        }

        SCAI_ASSERT_ERROR( integrityIA, msg << ": ellSizes: at least one value out of range" )
        SCAI_ASSERT_ERROR( integrityJA, msg << ": ellJA: at least one value out of range" )
    }
    else
    {
        SCAI_ASSERT_EQUAL_ERROR( 0, numValuesPerRow )
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
ValueType OpenMPELLUtils::absMaxVal(
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    const ValueType values[] )
{
    ValueType maxValue = static_cast<ValueType>( 0.0 );

    #pragma omp parallel
    {
        ValueType threadVal = static_cast<ValueType>( 0.0 );

        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType i = 0; i < numRows; ++i )
        {
            for( IndexType jj = 0; jj < ellSizes[i]; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                ValueType val = abs( values[pos] );

                if( val > threadVal )
                {
                    threadVal = val;
                }

                // SCAI_LOG_TRACE( logger, "absMaxVal, val[" << i << ", " << jj << "] = " << val )
            }
        }

        #pragma omp critical
        {
            SCAI_LOG_DEBUG( logger, "absMaxVal, threadVal = " << threadVal << ", maxVal = " << maxValue )

            if( threadVal > maxValue )
            {
                maxValue = threadVal;
            }
        }
    }

    SCAI_LOG_DEBUG( logger, "absMaxVal, maxVal = " << maxValue )

    return maxValue;
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType,typename OtherValueType>
void OpenMPELLUtils::getRow(
    OtherValueType row[],
    const IndexType i,
    const IndexType numRows,
    const IndexType numColumns,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType values[] )
{
    SCAI_LOG_DEBUG( logger, "get row #i = " << i )

    #pragma omp parallel for schedule( LAMA_OMP_SCHEDULE )

    for( IndexType j = 0; j < numColumns; ++j )
    {
        row[j] = 0.0;
    }

    #pragma omp parallel for schedule( LAMA_OMP_SCHEDULE )

    for( IndexType jj = 0; jj < ellSizes[i]; ++jj )
    {
        IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
        row[ellJA[pos]] = static_cast<OtherValueType>( values[pos] );
    }
}

template<typename ValueType,typename OtherValueType>
OtherValueType OpenMPELLUtils::getValue(
    const IndexType i,
    const IndexType j,
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[] )
{
    SCAI_LOG_TRACE( logger, "get value i = " << i << ", j = " << j )

    for( IndexType jj = 0; jj < ellSizes[i]; ++jj )
    {
        IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );

        if( ellJA[pos] == j )
        {
            return static_cast<OtherValueType>( ellValues[pos] );
        }
    }

    return 0.0;
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ELLValueType,typename CSRValueType>
void OpenMPELLUtils::getCSRValues(
    IndexType csrJA[],
    CSRValueType csrValues[],
    const IndexType csrIA[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ELLValueType ellValues[] )
{
    SCAI_LOG_INFO( logger,
                   "get CSRValues<" << getScalarType<ELLValueType>() << ", " << getScalarType<CSRValueType>() << ">" << ", #rows = " << numRows )

    // parallelization possible as offset array csrIA is available

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL->CSR_values" )

        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType i = 0; i < numRows; i++ )
        {
            IndexType rowSize = ellSizes[i];
            IndexType offset = csrIA[i];

            // just make sure that csrIA and ellSizes really fit with each other

            SCAI_ASSERT_EQUAL_DEBUG( csrIA[i] + rowSize, csrIA[i + 1] )

            for( IndexType jj = 0; jj < rowSize; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                csrJA[offset + jj] = ellJA[pos];
                csrValues[offset + jj] = static_cast<CSRValueType>( ellValues[pos] );
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ELLValueType,typename CSRValueType>
void OpenMPELLUtils::setCSRValues(
    IndexType ellJA[],
    ELLValueType ellValues[],
    const IndexType ellSizes[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType csrIA[],
    const IndexType csrJA[],
    const CSRValueType csrValues[] )
{
    SCAI_LOG_INFO( logger,
                   "set CSRValues<" << getScalarType<ELLValueType>() << ", " << getScalarType<CSRValueType>() << ">" << ", #rows = " << numRows << ", #values/row = " << numValuesPerRow )

    // parallelization possible as offset array csrIA is available

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL<-CSR_values" )

        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType i = 0; i < numRows; i++ )
        {
            IndexType rowSize = ellSizes[i];
            IndexType offset = csrIA[i];
            IndexType j = 0; // will be last column index

            for( IndexType jj = 0; jj < rowSize; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                j = csrJA[offset + jj];
                ellJA[pos] = j;
                ellValues[pos] = static_cast<ELLValueType>( csrValues[offset + jj] );
            }

            // fill up the remaining entries with something useful

            for( IndexType jj = rowSize; jj < numValuesPerRow; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                ellJA[pos] = j; // last used column index
                ellValues[pos] = 0.0; // zero entry
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::fillELLValues(
    IndexType ellJA[],
    ValueType ellValues[],
    const IndexType ellSizes[],
    const IndexType numRows,
    const IndexType numValuesPerRow )
{
    SCAI_LOG_INFO( logger, "fill ELLValues<" << getScalarType<ValueType>() )

    #pragma omp parallel
    {
        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType i = 0; i < numRows; i++ )
        {
            IndexType rowSize = ellSizes[i];

            IndexType j = 0; // will be last column index

            if( rowSize > 0 && rowSize < numValuesPerRow )
            {
                IndexType pos = ellindex( i, rowSize - 1, numRows, numValuesPerRow );
                j = ellJA[pos];
            }

            // fill up the remaining entries with something useful

            for( IndexType jj = rowSize; jj < numValuesPerRow; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                ellJA[pos] = j; // last used column index
                ellValues[pos] = 0.0; // zero entry
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::compressIA(
    const IndexType IA[],
    const IndexType JA[],
    const ValueType ellValues[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const ValueType eps,
    IndexType newIA[] )
{
    SCAI_LOG_INFO( logger, "compressIA with eps = " << eps )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < numRows; i++ )
        {
            IndexType length = IA[i];

            for( IndexType jj = 0; jj < IA[i]; jj++ )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );

                if( JA[pos] == i )
                {
                    continue;
                }

                if( abs( ellValues[pos] ) <= eps )
                {
                    length--;
                }
            }

            newIA[i] = length;
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::compressValues(
    const IndexType IA[],
    const IndexType JA[],
    const ValueType values[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const ValueType eps,
    const IndexType newNumValuesPerRow,
    IndexType newJA[],
    ValueType newValues[] )
{
    SCAI_LOG_INFO( logger, "compressValues with eps = " << eps )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < numRows; i++ )
        {
            IndexType gap = 0;

            for( IndexType j = 0; j < IA[i]; j++ )
            {
                IndexType pos = ellindex( i, j, numRows, numValuesPerRow );

                if( abs( values[pos] ) <= eps && JA[pos] != i )
                {
                    gap++;
                    continue;
                }

                IndexType newpos = ellindex( i, j - gap, numRows, newNumValuesPerRow );
                newValues[newpos] = values[pos];
                newJA[newpos] = JA[pos];
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void OpenMPELLUtils::matrixMultiplySizes(
    IndexType cSizes[],
    const IndexType aNumRows,
    const IndexType UNUSED( aNumColumns ),
    const IndexType bNumRows,
    const bool UNUSED( diagonalProperty ),
    const IndexType aSizes[],
    const IndexType aJA[],
    const IndexType aNumValuesPerRow,
    const IndexType bSizes[],
    const IndexType bJA[],
    const IndexType bNumValuesPerRow )
{
    SCAI_LOG_INFO( logger, "matrixMultiplySizes with numRows A = " << aNumRows << " and numRows B = " << bNumRows )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < aNumRows; i++ )
        {
            std::set<IndexType> newElements;
            std::pair<std::set<IndexType>::iterator,bool> ret;
            IndexType length = 0;

            for( IndexType j = 0; j < aSizes[i]; j++ )
            {
                IndexType posA = ellindex( i, j, aNumRows, aNumValuesPerRow );
                IndexType jj = aJA[posA];

                for( IndexType k = 0; k < bSizes[jj]; k++ )
                {
                    IndexType posB = ellindex( jj, k, bNumRows, bNumValuesPerRow );
                    IndexType kk = bJA[posB];
                    ret = newElements.insert( kk );

                    if( ret.second == true )
                    {
                        length++;
                    }
                }
            }

            cSizes[i] = length;
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::matrixMultiply(
    IndexType cJA[],
    ValueType cValues[],
    const IndexType cSizes[],
    const IndexType cNumValuesPerRow,
    const IndexType aNumRows,
    const IndexType UNUSED( aNumColumns ),
    const IndexType bNumRows,
    const bool UNUSED( diagonalProperty ),
    const ValueType alpha,
    const IndexType aSizes[],
    const IndexType aJA[],
    const ValueType aValues[],
    const IndexType aNumValuesPerRow,
    const IndexType bSizes[],
    const IndexType bJA[],
    const ValueType bValues[],
    const IndexType bNumValuesPerRow )
{
    SCAI_LOG_INFO( logger, "matrix multiply with numRows A = " << aNumRows << " and numRows B = " << bNumRows )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < aNumRows; i++ )
        {
            std::set<IndexType> jaRow;
            std::map<IndexType,ValueType> valuesRow;
            std::pair<std::set<IndexType>::iterator,bool> ret;

            for( IndexType j = 0; j < aSizes[i]; j++ )
            {
                IndexType posA = ellindex( i, j, aNumRows, aNumValuesPerRow );
                IndexType jj = aJA[posA];

                for( IndexType k = 0; k < bSizes[jj]; k++ )
                {
                    IndexType posB = ellindex( jj, k, bNumRows, bNumValuesPerRow );
                    IndexType kk = bJA[posB];
                    ret = jaRow.insert( kk );
                    ValueType mult = alpha * aValues[posA] * bValues[posB];

                    if( ret.second == true )
                    {
                        valuesRow.insert( std::pair<IndexType,ValueType>( kk, mult ) );
                    }
                    else
                    {
                        valuesRow[kk] += mult;
                    }
                }
            }

            std::set<IndexType>::iterator jaIter;
            typename std::map<IndexType,ValueType>::iterator valuesIter;

            jaIter = jaRow.begin();
            valuesIter = valuesRow.begin();

            for( IndexType j = 0; j < cSizes[i]; j++ )
            {
                // note: cNumRows == aNumRows
                IndexType posC = ellindex( i, j, aNumRows, cNumValuesPerRow );
                cJA[posC] = *jaIter;
                cValues[posC] = ( *valuesIter ).second;
                jaIter++;
                valuesIter++;
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void OpenMPELLUtils::matrixAddSizes(
    IndexType cSizes[],
    const IndexType m,
    const IndexType UNUSED( n ),
    const bool UNUSED( diagonalProperty ),
    const IndexType aSizes[],
    const IndexType aJA[],
    const IndexType aNumValuesPerRow,
    const IndexType bSizes[],
    const IndexType bJA[],
    const IndexType bNumValuesPerRow )
{
    SCAI_LOG_INFO( logger, "matrixAddSizes A + B, #rows = " << m )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < m; i++ )
        {
            std::set<IndexType> iaRow;
            std::pair<std::set<IndexType>::iterator,bool> ret;
            IndexType length = 0;

            for( IndexType j = 0; j < aSizes[i]; j++ )
            {
                IndexType posA = ellindex( i, j, m, aNumValuesPerRow );
                iaRow.insert( aJA[posA] );
                length++;
            }

            for( IndexType j = 0; j < bSizes[i]; j++ )
            {
                IndexType posB = ellindex( i, j, m, bNumValuesPerRow );
                ret = iaRow.insert( bJA[posB] );

                if( ret.second == true )
                {
                    length++;
                }
            }

            cSizes[i] = length;
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::matrixAdd(
    IndexType cJA[],
    ValueType cValues[],
    const IndexType cSizes[],
    const IndexType cNumValuesPerRow,
    const IndexType m,
    const IndexType UNUSED( n ),
    const bool UNUSED( diagonalProperty ),
    const ValueType alpha,
    const IndexType aSizes[],
    const IndexType aJA[],
    const ValueType aValues[],
    const IndexType aNumValuesPerRow,
    const ValueType beta,
    const IndexType bSizes[],
    const IndexType bJA[],
    const ValueType bValues[],
    const IndexType bNumValuesPerRow )
{
    SCAI_LOG_INFO( logger, "matrixAdd C = " << alpha << " * A + " << beta << " * B, #rows = " << m )

    #pragma omp parallel
    {
        #pragma omp for

        for( IndexType i = 0; i < m; i++ )
        {
            std::set<IndexType> jaRow;
            std::map<IndexType,ValueType> valuesRow;
            std::pair<std::set<IndexType>::iterator,bool> ret;

            for( IndexType j = 0; j < aSizes[i]; j++ )
            {
                IndexType posA = ellindex( i, j, m, aNumValuesPerRow );
                jaRow.insert( aJA[posA] );
                valuesRow.insert( std::pair<IndexType,ValueType>( aJA[posA], aValues[posA] ) );
            }

            for( IndexType j = 0; j < bSizes[i]; j++ )
            {
                IndexType posB = ellindex( i, j, m, bNumValuesPerRow );
                ret = jaRow.insert( bJA[posB] );

                if( ret.second == true )
                {
                    valuesRow.insert( std::pair<IndexType,ValueType>( bJA[posB], beta * bValues[posB] ) );
                }
                else
                {
                    valuesRow[bJA[posB]] += beta * bValues[posB];
                }
            }

            std::set<IndexType>::iterator jaIter;
            typename std::map<IndexType,ValueType>::iterator valuesIter;

            jaIter = jaRow.begin();
            valuesIter = valuesRow.begin();

            for( IndexType j = 0; j < cSizes[i]; j++ )
            {
                // Note: cNumRows == aNumRows
                IndexType posC = ellindex( i, j, m, cNumValuesPerRow );
                cJA[posC] = *jaIter;
                cValues[posC] = ( *valuesIter ).second;
                jaIter++;
                valuesIter++;
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::jacobi(
    ValueType solution[],
    const IndexType numRows,
    const IndexType ellNumValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    const ValueType oldSolution[],
    const ValueType rhs[],
    const ValueType omega,
    class SyncToken* syncToken )
{
    SCAI_LOG_INFO( logger,
                   "jacobi<" << getScalarType<ValueType>() << ">" << ", #rows = " << numRows << ", omega = " << omega )

    if( syncToken != NULL )
    {
        SCAI_LOG_ERROR( logger, "jacobi called asynchronously, not supported here" )
    }

    const ValueType oneMinusOmega = static_cast<ValueType>( 1.0 - omega );
    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.jacobi" )
        #pragma omp for schedule(LAMA_OMP_SCHEDULE)

        for( IndexType i = 0; i < numRows; i++ )
        {
            ValueType temp = rhs[i];
            IndexType pos = ellindex( i, 0, numRows, ellNumValuesPerRow );
            ValueType diag = ellValues[pos]; //getDiagonal

            for( IndexType j = 1; j < ellSizes[i]; j++ )
            {
                pos = ellindex( i, j, numRows, ellNumValuesPerRow );
                temp -= ellValues[pos] * oldSolution[ellJA[pos]];
            }

            if( omega == 1.0 )
            {
                solution[i] = temp / diag;
            }
            else if( omega == 0.5 )
            {
                solution[i] = omega * ( temp / diag + oldSolution[i] );
            }
            else
            {
                solution[i] = omega * ( temp / diag ) + oneMinusOmega * oldSolution[i];
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::jacobiHalo(
    ValueType solution[],
    const IndexType numRows,
    const ValueType diagonal[],
    const IndexType ellNumValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    const IndexType rowIndexes[],
    const IndexType numNonEmptyRows,
    const ValueType oldSolution[],
    const ValueType omega,
    class SyncToken* syncToken )
{
    if( syncToken != NULL )
    {
        SCAI_LOG_WARN( logger, "jacobi called asynchronously, not supported here" )
    }

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.jacobiHalo" )

        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType ii = 0; ii < numNonEmptyRows; ++ii )
        {
            IndexType i = ii; // rowIndexes == NULL stands for all rows

            if( rowIndexes )
            {
                i = rowIndexes[ii];
            }

            ValueType temp = 0.0;

            for( IndexType jj = 0; jj < ellSizes[i]; jj++ )
            {
                IndexType pos = ellindex( i, jj, numRows, ellNumValuesPerRow );
                temp += ellValues[pos] * oldSolution[ellJA[pos]];
            }

            const ValueType diag = diagonal[i];

            solution[i] -= temp * ( omega / diag );
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::normalGEMV(
    ValueType result[],
    const ValueType alpha,
    const ValueType x[],
    const ValueType beta,
    const ValueType y[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    SyncToken* syncToken )
{
    SCAI_LOG_INFO( logger,
                   "normalGEMV<" << getScalarType<ValueType>() << ", #threads = " << omp_get_max_threads() << ">, result[" << numRows << "] = " << alpha << " * A( ell, #maxNZ/row = " << numValuesPerRow << " ) * x + " << beta << " * y " )

    if( numValuesPerRow == 0 )
    {
        COMMON_THROWEXCEPTION( "normalGEMV should not have been called, no entries" )

        // only compute: result = beta * y
    }

    if( syncToken )
    {
        SCAI_LOG_WARN( logger, "Host: asynchronous execution by task should be done at higher level" )
    }

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.normalGEMV" )

        #pragma omp for schedule(LAMA_OMP_SCHEDULE)

        for( IndexType i = 0; i < numRows; ++i )
        {
            ValueType temp = 0.0;

            for( IndexType jj = 0; jj < ellSizes[i]; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                IndexType j = ellJA[pos];
                SCAI_LOG_TRACE( logger,
                                "temp += dataAccess[i + jj * numRows] * xAccess[j];, jj = " << jj << ", j = " << j )
                SCAI_LOG_TRACE( logger, ", dataAccess[i + jj * numRows] = " << ellValues[ pos ] )
                SCAI_LOG_TRACE( logger, ", xAccess[j] = " << x[ j ] )
                temp += ellValues[pos] * x[j];
            }

            SCAI_LOG_TRACE( logger, "row = " << i << ", temp = " << temp )

            if( static_cast<ValueType>( 0 ) == beta )
            {
                // must be handled separately as y[i] might be uninitialized

                result[i] = alpha * temp;
            }
            else if( static_cast<ValueType>( 1 ) == alpha )
            {
                result[i] = temp + beta * y[i];
            }
            else
            {
                result[i] = alpha * temp + beta * y[i];
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::sparseGEMV(
    ValueType result[],
    const ValueType alpha,
    const ValueType x[],
    const IndexType numRows,
    const IndexType numValuesPerRow,
    const IndexType numNonZeroRows,
    const IndexType rowIndexes[],
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    SyncToken* syncToken )
{
    if( syncToken )
    {
        SCAI_LOG_WARN( logger, "Host: asynchronous execution by task should be done at higher level" )
    }

    SCAI_LOG_INFO( logger,
                   "sparseGEMV<" << getScalarType<ValueType>() << ">, n = " << numRows << ", nonZeroRows = " << numNonZeroRows << ", alpha = " << alpha )

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.sparseGEMV" )

        #pragma omp for schedule( LAMA_OMP_SCHEDULE )

        for( IndexType ii = 0; ii < numNonZeroRows; ++ii )
        {
            IndexType i = rowIndexes[ii];

            //result is not initialized for performance reasons
            ValueType temp = 0.0;

            for( IndexType jj = 0; jj < ellSizes[i]; ++jj )
            {
                IndexType pos = ellindex( i, jj, numRows, numValuesPerRow );
                IndexType j = ellJA[pos];
                temp += ellValues[pos] * x[j];
            }

            if( 1 == alpha )
            {
                result[i] += temp;
            }
            else
            {
                result[i] += alpha * temp;
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::normalGEVM(
    ValueType result[],
    const ValueType alpha,
    const ValueType x[],
    const ValueType beta,
    const ValueType y[],
    const IndexType numRows,
    const IndexType numColumns,
    const IndexType UNUSED(numValuesPerRow),
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    SyncToken* syncToken )
{
    SCAI_LOG_INFO( logger,
                   "normalGEVM<" << getScalarType<ValueType>() << ", #threads = " << omp_get_max_threads() << ">, result[" << numColumns << "] = " << alpha << " * x * A + " << beta << " * y " )

    if( syncToken )
    {
        COMMON_THROWEXCEPTION( "asynchronous execution should be done by LAMATask before" )
    }

    //#pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.normalGEVM" )

        //#pragma omp for schedule(LAMA_OMP_SCHEDULE)
        for( IndexType i = 0; i < numColumns; ++i )
        {
            ValueType temp = 0.0;

            for( IndexType j = 0; j < numRows; ++j )
            {
                for( IndexType k = 0; k < ellSizes[j]; ++k )
                {
                    if( ellJA[k * numRows + j] == i )
                    {
                        SCAI_LOG_TRACE( logger, "temp += dataAccess[k * numRows + j] * xAccess[j]; j = " << j )
                        SCAI_LOG_TRACE( logger, ", dataAccess[k * numRows + j] = " << ellValues[ k * numRows + j ] )
                        SCAI_LOG_TRACE( logger, ", xAccess[j] = " << x[ j ] )

                        temp += ellValues[k * numRows + j] * x[j];
                    }
                }
            }

            SCAI_LOG_TRACE( logger, "column = " << i << ", temp = " << temp )

            if( 0.0 == beta )
            {
                // must be handled separately as y[i] might be uninitialized

                result[i] = alpha * temp;
            }
            else if( 1.0 == alpha )
            {
                result[i] = temp + beta * y[i];
            }
            else
            {
                result[i] = alpha * temp + beta * y[i];
            }
        }
    }

    if( SCAI_LOG_TRACE_ON( logger ) )
    {
        std::cout << "NormalGEVM: result = ";

        for( IndexType i = 0; i < numColumns; ++i )
        {
            std::cout << " " << result[i];
        }

        std::cout << std::endl;
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

template<typename ValueType>
void OpenMPELLUtils::sparseGEVM(
    ValueType result[],
    const ValueType alpha,
    const ValueType x[],
    const IndexType UNUSED(numRows),
    const IndexType numColumns,
    const IndexType UNUSED(numValuesPerRow),
    const IndexType numNonZeroRows,
    const IndexType rowIndexes[],
    const IndexType ellSizes[],
    const IndexType ellJA[],
    const ValueType ellValues[],
    SyncToken* syncToken )
{
    SCAI_LOG_INFO( logger,
                   "sparseGEVM<" << getScalarType<ValueType>() << ", #threads = " << omp_get_max_threads() << ">, result[" << numColumns << "] = " << alpha << " * x * A " )

    if( syncToken )
    {
        COMMON_THROWEXCEPTION( "asynchronous execution should be done by LAMATask before" )
    }

    #pragma omp parallel
    {
        SCAI_REGION( "OpenMP.ELL.sparseGEVM" )

        #pragma omp for schedule(LAMA_OMP_SCHEDULE)

        for( IndexType i = 0; i < numColumns; ++i )
        {
            ValueType temp = 0.0;

            for( IndexType jj = 0; jj < numNonZeroRows; ++jj )
            {
                IndexType j = rowIndexes[jj];

                for( IndexType k = 0; k < ellSizes[j]; ++k )
                {
                    if( ellJA[k * numNonZeroRows + j] == i )
                    {
                        SCAI_LOG_TRACE( logger, "temp += dataAccess[k * numNonZeroRows + j] * xAccess[j]; i = " << j )
                        SCAI_LOG_TRACE( logger,
                                        ", dataAccess[k * numNonZeroRows + j] = " << ellValues[ k * numNonZeroRows + j ] )
                        SCAI_LOG_TRACE( logger, ", xAccess[j] = " << x[ j ] )

                        temp += ellValues[k * numNonZeroRows + j] * x[j];
                    }
                }

            }

            if( 1 == alpha )
            {
                result[i] += temp;
            }
            else
            {
                result[i] += alpha * temp;
            }
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------ */

void OpenMPELLUtils::setInterface( ELLUtilsInterface& ELLUtils )
{
    LAMA_INTERFACE_REGISTER( ELLUtils, countNonEmptyRowsBySizes )
    LAMA_INTERFACE_REGISTER( ELLUtils, setNonEmptyRowsBySizes )
    LAMA_INTERFACE_REGISTER( ELLUtils, hasDiagonalProperty )
    LAMA_INTERFACE_REGISTER( ELLUtils, check )

    LAMA_INTERFACE_REGISTER( ELLUtils, matrixMultiplySizes )
    LAMA_INTERFACE_REGISTER( ELLUtils, matrixAddSizes )

#define LAMA_ELL_UTILS2_REGISTER(z, J, TYPE )                                       \
    LAMA_INTERFACE_REGISTER_TT( ELLUtils, getRow, TYPE, ARITHMETIC_TYPE##J )        \
    LAMA_INTERFACE_REGISTER_TT( ELLUtils, getValue, TYPE, ARITHMETIC_TYPE##J )      \
    LAMA_INTERFACE_REGISTER_TT( ELLUtils, scaleValue, TYPE, ARITHMETIC_TYPE##J )    \
    LAMA_INTERFACE_REGISTER_TT( ELLUtils, setCSRValues, TYPE, ARITHMETIC_TYPE##J )  \
    LAMA_INTERFACE_REGISTER_TT( ELLUtils, getCSRValues, TYPE, ARITHMETIC_TYPE##J )  \

#define LAMA_ELL_UTILS_REGISTER(z, I, _)                                            \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, absMaxVal, ARITHMETIC_TYPE##I )            \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, compressIA, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, compressValues, ARITHMETIC_TYPE##I )       \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, matrixAdd, ARITHMETIC_TYPE##I )            \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, matrixMultiply, ARITHMETIC_TYPE##I )       \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, normalGEMV, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, normalGEVM, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, sparseGEMV, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, sparseGEVM, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, jacobi, ARITHMETIC_TYPE##I )               \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, jacobiHalo, ARITHMETIC_TYPE##I )           \
    LAMA_INTERFACE_REGISTER_T( ELLUtils, fillELLValues, ARITHMETIC_TYPE##I )        \
    \
    BOOST_PP_REPEAT( ARITHMETIC_TYPE_CNT,                                           \
                     LAMA_ELL_UTILS2_REGISTER,                                      \
                     ARITHMETIC_TYPE##I )                                           \

    BOOST_PP_REPEAT( ARITHMETIC_TYPE_CNT, LAMA_ELL_UTILS_REGISTER, _ )

#undef LAMA_ELL_UTILS_REGISTER
#undef LAMA_ELL_UTILS2_REGISTER

}

/* --------------------------------------------------------------------------- */
/*    Static registration of the ELLUtils routines                             */
/* --------------------------------------------------------------------------- */

bool OpenMPELLUtils::registerInterface()
{
    LAMAInterface& interface = LAMAInterfaceRegistry::getRegistry().modifyInterface( hmemo::context::Host );
    setInterface( interface.ELLUtils );
    return true;
}

/* --------------------------------------------------------------------------- */
/*    Static initialiazion at program start                                    */
/* --------------------------------------------------------------------------- */

bool OpenMPELLUtils::initialized = registerInterface();

} /* end namespace lama */

} /* end namespace scai */