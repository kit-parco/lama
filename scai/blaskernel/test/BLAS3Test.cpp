/**
 * @file BLAS3Test.cpp
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
 * @brief Contains tests for the blas3 methods.
 * @author: Bea Hornef
 * @date 17.7.2013
 * @since 1.0.0
 **/

// boost
#include <boost/test/unit_test.hpp>

// others
#include <scai/blaskernel/BLASKernelTrait.hpp>
#include <scai/hmemo.hpp>
#include <scai/kregistry/KernelContextFunction.hpp>

#include <scai/blaskernel/test/TestMacros.hpp>

using namespace scai::hmemo;

namespace scai
{
namespace blaskernel
{
namespace BLAS3Test
{

template<typename ValueType>
void gemmTest( ContextPtr loc )
{
    //  input
    //                            (  2.0 3.0 )
    // 17.0 * ( 1.0  2.0 -3.0 ) * ( -1.0 1.0 ) - 13.0 * ( 15.0 13.0 ) =  (-9.0 -1.0)
    //        ( 4.0  5.0 -6.0 )   (  4.0 5.0 )          ( 27.0 17.0 )    (-6.0  0.0)

    scai::kregistry::KernelTraitContextFunction<blaskernel::BLASKernelTrait::gemm<ValueType> > gemm;

    const ValueType alpha = 17.0;
    const ValueType beta = 13.0;
    const ValueType resultRowMajor[] =
    { -9.0, -1.0, -6.0, 0.0 };
    const ValueType resultColMajor[] =
    { -9.0, -6.0, -1.0, 0.0 };
    const IndexType n = 2;
    const IndexType m = 2;
    const IndexType k = 3;
    // CblasRowMajor and 2 x CblasNoTrans
    {
        const ValueType matrixA[] =
        { 1.0, 2.0, -3.0, 4.0, 5.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, 3.0, -1.0, 1.0, 4.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 13.0, 27.0, 17.0 };
        const IndexType lda = 3;
        const IndexType ldb = 2;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultRowMajor[i], rAmC[i] );
            }
        }
    }
    // CblasColMajor and 2 x CblasNoTrans
    {
        const ValueType matrixA[] =
        { 1.0, 4.0, 2.0, 5.0, -3.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, -1.0, 4.0, 3.0, 1.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 27.0, 13.0, 17.0 };
        const IndexType lda = 2;
        const IndexType ldb = 3;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultColMajor[i], rAmC[i] );
            }
        }
    }
    // CblasRowMajor, CblasNoTrans for A and CblasTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 2.0, -3.0, 4.0, 5.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, -1.0, 4.0, 3.0, 1.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 13.0, 27.0, 17.0 };
        const IndexType lda = 3;
        const IndexType ldb = 3;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasRowMajor, CblasNoTrans, CblasTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultRowMajor[i], rAmC[i] );
            }
        }
    }
    // CblasColMajor, CblasNoTrans for A and CblasTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 4.0, 2.0, 5.0, -3.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, 3.0, -1.0, 1.0, 4.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 27.0, 13.0, 17.0 };
        const IndexType lda = 2;
        const IndexType ldb = 2;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasColMajor, CblasNoTrans, CblasTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                  wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultColMajor[i], rAmC[i] );
            }
        }
    }
    // CblasRowMajor, CblasTrans for A and CblasNoTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 4.0, 2.0, 5.0, -3.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, 3.0, -1.0, 1.0, 4.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 13.0, 27.0, 17.0 };
        const IndexType lda = 2;
        const IndexType ldb = 2;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasRowMajor, CblasTrans, CblasNoTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultRowMajor[i], rAmC[i] );
            }
        }
    }
    // CblasColMajor, CblasTrans for A and CblasNoTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 2.0, -3.0, 4.0, 5.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, -1.0, 4.0, 3.0, 1.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 27.0, 13.0, 17.0 };
        const IndexType lda = 3;
        const IndexType ldb = 3;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasColMajor, CblasTrans, CblasNoTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultColMajor[i], rAmC[i] );
            }
        }
    }
    // CblasRowMajor, CblasTrans for A and CblasTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 4.0, 2.0, 5.0, -3.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, -1.0, 4.0, 3.0, 1.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 13.0, 27.0, 17.0 };
        const IndexType lda = 2;
        const IndexType ldb = 3;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasRowMajor, CblasTrans, CblasTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultRowMajor[i], rAmC[i] );
            }
        }
    }
    // CblasColMajor, CblasTrans for A and CblasTrans for B
    {
        const ValueType matrixA[] =
        { 1.0, 2.0, -3.0, 4.0, 5.0, -6.0 };
        const ValueType matrixB[] =
        { 2.0, 3.0, -1.0, 1.0, 4.0, 5.0 };
        ValueType matrixC[] =
        { 15.0, 27.0, 13.0, 17.0 };
        const IndexType lda = 3;
        const IndexType ldb = 2;
        const IndexType ldc = 2;
//        LAMAArray<ValueType> AmA( 6, matrixA );
//        LAMAArray<ValueType> AmB( 6, matrixB );
//        LAMAArray<ValueType> AmC( 4, matrixC );
        HArray<ValueType> AmA( 6 );
        initArray( AmA, matrixA, 6 );
        HArray<ValueType> AmB( 6 );
        initArray( AmB, matrixB, 6 );
        HArray<ValueType> AmC( 4 );
        initArray( AmC, matrixC, 4 );
        {
            SCAI_CONTEXT_ACCESS( loc );
            ReadAccess<ValueType> rAmA( AmA, loc );
            ReadAccess<ValueType> rAmB( AmB, loc );
            WriteAccess<ValueType> wAmC( AmC, loc );
            gemm[loc->getType()]( CblasColMajor, CblasTrans, CblasTrans, m, n, k, alpha, rAmA.get(), lda, rAmB.get(), ldb, beta,
                       wAmC.get(), ldc );
        }
        {
            ReadAccess<ValueType> rAmC( AmC );

            for ( int i = 0; i < 4; ++i )
            {
                BOOST_CHECK_EQUAL( resultColMajor[i], rAmC[i] );
            }
        }
    }
} // gemmTest

} /* end namespace BLAS3Test */

} /* end namespace blaskernel */

} /* end namespace scai */
/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE ( BLAS3Test )

SCAI_LOG_DEF_LOGGER( logger, "Test.BLAS3Test" )

LAMA_AUTO_TEST_CASE_CT( gemmTest, BLAS3Test, scai::blaskernel )
/* ------------------------------------------------------------------------------------------------------------------ */

BOOST_AUTO_TEST_SUITE_END()