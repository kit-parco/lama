/**
 * @file OpenMPBLAS2.cpp
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
 * @brief Implementation of BLAS2 routines used in LAMA with own C++/OpenMP implementations.
 * @author Eric Schricker
 * @date 09.10.2013
 * @since 1.1.0
 */

// hpp
#include <scai/lama/openmp/OpenMPBLAS2.hpp>
#include <scai/lama/BLASInterface.hpp>
#include <scai/lama/LAMAInterfaceRegistry.hpp>

// macros
#include <scai/lama/macros/unused.hpp>

// boost
#include <boost/preprocessor.hpp>

namespace scai
{

namespace lama
{

SCAI_LOG_DEF_LOGGER( OpenMPBLAS2::logger, "OpenMP.BLAS2" )

/** gemv */

template<typename ValueType>
void OpenMPBLAS2::gemv(
    const CBLAS_ORDER order,
    const CBLAS_TRANSPOSE TransA,
    const IndexType M,
    const IndexType N,
    const ValueType alpha,
    const ValueType* A,
    const IndexType lda,
    const ValueType* X,
    const IndexType incX,
    const ValueType beta,
    ValueType* Y,
    const IndexType incY,
    SyncToken* syncToken )
{
    SCAI_LOG_INFO( logger,
                   "gemv<" << common::getScalarType<ValueType>()<< ">: M = " << M << ", N = " << N 
                      << ", LDA = " << lda << ", incX = " << incX << ", incY = " << incY 
                      << ", alpha = " << alpha << ", beta = " << beta )

    if( M == 0 )
    {
        return; // empty X, Y, A
    }

    // N == 0: empty A, but deal with X, Y, we can handle this here

    if( syncToken )
    {
        SCAI_LOG_WARN( logger, "no asynchronous execution for openmp possible at this level." )
    }

    IndexType RowMajorStrg;
    RowMajorStrg = 0;

    if( order == CblasColMajor )
    {
        if( TransA == CblasNoTrans )
        {
            //'N'
            // y = alpha * A * x + beta * y
            ValueType Z = 0.0;

            if( incX == 1 && incY == 1 )
            {
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )

                for( int i = 0; i < M; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < N; j++ )
                    {
                        Z += A[lda * j + i] * X[j];
                    }

                    Y[i] = Z * alpha + Y[i] * beta;
                }
            }
            else
            {
                //incX != 1 || incY != 1
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )
                for( int i = 0; i < M; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < N; j++ )
                    {
                        Z += A[lda * j + i] * X[j * incX];
                    }

                    Y[i * incY] = Z * alpha + Y[i * incY] * beta;
                }
            }

        }
        else if( TransA == CblasTrans )
        {
            //'T'
            // y = alpha * A^T * x + beta * y
            ValueType Z = 0.0;

            if( incX == 1 && incY == 1 )
            {
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )

                for( int i = 0; i < N; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < M; j++ )
                    {
                        Z += A[lda * i + j] * X[j];
                    }

                    Y[i] = Z * alpha + Y[i] * beta;
                }
            }
            else
            {
                //incX != 1 || incY != 1
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )
                for( int i = 0; i < N; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < M; j++ )
                    {
                        Z += A[lda * i + j] * X[j * incX];
                    }

                    Y[i * incY] = Z * alpha + Y[i * incY] * beta;
                }
            }

        }
        else if( TransA == CblasConjTrans )
        {
            //'C'
        }
        else
        {
            BLASHelper::XERBLA_cpu( RowMajorStrg, 2, "cblas_sgemv", "Illegal TransA setting, %d\n", TransA );
            RowMajorStrg = 0;
        }

    }
    else if( order == CblasRowMajor )
    {
        RowMajorStrg = 1;

        if( TransA == CblasNoTrans )
        {
            //'T'
            ValueType Z = 0.0;

            if( incX == 1 && incY == 1 )
            {
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )

                for( int i = 0; i < M; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < N; j++ )
                    {
                        Z += A[lda * i + j] * X[j];
                    }

                    Y[i] = Z * alpha + Y[i] * beta;
                }
            }
            else
            {
                //incX != 1 || incY != 1
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )
                for( int i = 0; i < M; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < N; j++ )
                    {
                        Z += A[lda * i + j] * X[j * incX];
                    }

                    Y[i * incY] = Z * alpha + Y[i * incY] * beta;
                }
            }

        }
        else if( TransA == CblasTrans )
        {
            //'N'
            ValueType Z = 0.0;

            if( incX == 1 && incY == 1 )
            {
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )

                for( int i = 0; i < N; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < M; j++ )
                    {
                        Z += A[lda * j + i] * X[j];
                    }

                    Y[i] = Z * alpha + Y[i] * beta;
                }
            }
            else
            {
                //incX != 1 || incY != 1
                #pragma omp parallel for private(Z) schedule( LAMA_OMP_SCHEDULE )
                for( int i = 0; i < N; i++ )
                {
                    Z = 0.0;

                    for( int j = 0; j < M; j++ )
                    {
                        Z += A[lda * j + i] * X[j * incX];
                    }

                    Y[i * incY] = Z * alpha + Y[i * incY] * beta;
                }
            }
        }
        else if( TransA == CblasConjTrans )
        {
            //TA = 'N'
        }
        else
        {
            BLASHelper::XERBLA_cpu( RowMajorStrg, 2, "cblas_sgemv", "Illegal TransA setting, %d\n", TransA );
            RowMajorStrg = 0;
            return;
        }

    }
    else
    {
        BLASHelper::XERBLA_cpu( RowMajorStrg, 1, "cblas_sgemv", "Illegal order setting, %d\n", order );
    }

    RowMajorStrg = 0;
    return;
}

/* --------------------------------------------------------------------------- */
/*     Template instantiations via registration routine                        */
/* --------------------------------------------------------------------------- */

void OpenMPBLAS2::setInterface( BLASInterface& BLAS )
{
    SCAI_LOG_INFO( logger, "set BLAS2 routines for OpenMP in Interface" )

#define LAMA_BLAS2_REGISTER(z, I, _)                                            \
    LAMA_INTERFACE_REGISTER_T( BLAS, gemv, ARITHMETIC_TYPE##I )                 \

    BOOST_PP_REPEAT( ARITHMETIC_TYPE_CNT, LAMA_BLAS2_REGISTER, _ )

#undef LAMA_BLAS2_REGISTER

    // all other routines are not used in LAMA yet
}

/* --------------------------------------------------------------------------- */
/*    Static registration of the BLAS2 routines                                */
/* --------------------------------------------------------------------------- */

bool OpenMPBLAS2::registerInterface()
{
    LAMAInterface& interface = LAMAInterfaceRegistry::getRegistry().modifyInterface( hmemo::context::Host );
    setInterface( interface.BLAS );
    return true;
}

/* --------------------------------------------------------------------------- */
/*    Static initialiazion at program start                                    */
/* --------------------------------------------------------------------------- */

bool OpenMPBLAS2::initialized = registerInterface();

} /* end namespace lama */

} /* end namespace scai */