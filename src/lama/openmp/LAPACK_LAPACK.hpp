/**
 * @file LAPACK_LAPACK.hpp
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
 * @brief LAPACK_LAPACK.hpp
 * @author Lauretta Schubert
 * @date 02.07.2012
 * @since 1.0.0
 */
#ifndef LAMA_OPENMP_LAPACK_HPP_
#define LAMA_OPENMP_LAPACK_HPP_

// for dll_import
#include <lama/config.hpp>

// others
#include <lama/LAMATypes.hpp>
#include <lama/SyncToken.hpp>

#include <lama/openmp/BLASHelper.hpp>

// logging
#include <logging/logging.hpp>

namespace lama
{

class LAPACK_LAPACK
{
public:

    /** Implementation of BLASInterface::LAPACK::getrf by LAPACK. */

    template<typename ValueType>
    static IndexType getrf(
        const CBLAS_ORDER order,
        const IndexType m,
        const IndexType n,
        ValueType* const a,
        const IndexType lda,
        IndexType* const ipiv );

    /** Implementation of BLASInterface::LAPACK::getrf by LAPACK. */

    template<typename ValueType>
    static IndexType getri(
        const CBLAS_ORDER order,
        const IndexType n,
        ValueType* const A,
        const IndexType lda,
        IndexType* const ipiv );

    /** Implementation of BLASInterface::LAPACK::getinv by LAPACK. */

    template<typename ValueType>
    static void getinv( const IndexType n, ValueType* a, const IndexType lda );

    /** Implementation of BLASInterface::LAPACK::tptrs vi LAPACK. */

    template<typename ValueType>
    static IndexType tptrs(
        const CBLAS_ORDER order,
        const CBLAS_UPLO uplo,
        const CBLAS_TRANSPOSE trans,
        const CBLAS_DIAG diag,
        const IndexType n,
        const IndexType nrhs,
        const ValueType* AP,
        ValueType* B,
        const IndexType ldb );

    /** OpenMP implementation for BLASInterface::LAPACK::laswp */

    template<typename ValueType>
    static void laswp(
        const CBLAS_ORDER order,
        const IndexType n,
        ValueType* A,
        const IndexType lda,
        const IndexType k1,
        const IndexType k2,
        const IndexType* ipiv,
        const IndexType incx,
        SyncToken* syncToken );

    /** Routine that sets functions pointers belonging to LAPACK in a BLASInterface.
     *
     *  param[inout] BLASInterface struct to register all routines implemented in OpenMP
     *
     *  Note: this routine will make instantiations of the template routines.
     */

    static void setInterface( struct BLASInterface& BLAS );

private:

    LAMA_LOG_DECL_STATIC_LOGGER( logger )

    static    bool initialized;

    static bool registerInterface();

}; /* LAPACK_LAPACK */

}
/* namespace lama */

#endif // LAMA_OPENMP_LAPACK_HPP_