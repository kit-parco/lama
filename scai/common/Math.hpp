/*
 * math.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eschricker
 */

#pragma once

#include <scai/common/mic/MICCallable.hpp>
#include <scai/common/cuda/CUDACallable.hpp>

#include <cmath>
#include <cstdlib>

namespace scai
{

namespace common
{


#ifdef SCAI_COMPLEX_SUPPORTED
template<typename ValueType> class Complex;
#endif

struct Math
{
    /** Square root function for ValueType
     *
     *  In contrary to the routine of cmath it will be possible to
     *  use always the same name for the routine.
     *
     *  \code
     *    ValueType x = sqrt ( y );            // might not work always correctly
     *    ValueType x = Math::sqrt ( y );      // this is guaranteed to work for all arithmetic types
     *  \endcode
     */
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER float sqrt( const float& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER double sqrt( const double& x );

    static inline MIC_CALLABLE_MEMBER long double sqrt( const long double& x );

#ifdef SCAI_COMPLEX_SUPPORTED
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER Complex<float> sqrt( const Complex<float>& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER Complex<double> sqrt( const Complex<double>& x );

    static inline MIC_CALLABLE_MEMBER Complex<long double> sqrt( const Complex<long double>& x );
#endif

    /** Absolute value function for ValueType
     *
     *  In contrary to the routine of cmath it will be possible to
     *  use always the same name for the routine.
     */
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER int abs( const int& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER long abs( const long& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER long long abs( const long long& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER float abs( const float& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER double abs( const double& x );

    static inline MIC_CALLABLE_MEMBER long double abs( const long double& x );

#ifdef SCAI_COMPLEX_SUPPORTED
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER float abs( const Complex<float>& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER double abs( const Complex<double>& x );

    static inline MIC_CALLABLE_MEMBER long double abs( const Complex<long double>& x );
#endif

    /*
     * Computes the conjugated value of a given value
     */
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER float conj( const float& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER double conj( const double& x );

    static inline MIC_CALLABLE_MEMBER long double conj( const long double& x );

#ifdef SCAI_COMPLEX_SUPPORTED
    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER Complex<float> conj( const Complex<float>& x );

    static inline MIC_CALLABLE_MEMBER CUDA_CALLABLE_MEMBER Complex<double> conj( const Complex<double>& x );

    static inline MIC_CALLABLE_MEMBER Complex<long double> conj( const Complex<long double>& x );
#endif

};

float Math::sqrt( const float& x )
{
    return ::sqrtf( x );
}

double Math::sqrt( const double& x )
{
    return ::sqrt( x );
}

long double Math::sqrt( const long double& x )
{
    return ::sqrtl( x );
}

// -------------------------------- abs -----------------------------

int Math::abs( const int& x )
{
    return ::abs( x );
}

long Math::abs( const long& x )
{
    return ::labs( x );
}

long long Math::abs( const long long& x )
{
    return ::llabs( x );
}

float Math::abs( const float& x )
{
    return ::fabsf( x );
}

double Math::abs( const double& x )
{
    return ::fabs( x );
}

long double Math::abs( const long double& x )
{
    return ::fabsl( x );
}

// -------------------------------- conj -----------------------------

float Math::conj( const float& x )
{
    return x;
}

double Math::conj( const double& x )
{
    return x;
}

long double Math::conj( const long double& x )
{
    return x;
}

} /* end namespace common */

} /* end namespace scai */