/**
 * @file TypeTraits.hpp
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
 * @brief Type specific implementations for arithmetic value types.
 * @author Thomas Brandes
 * @date 17.11.2015
 */

#pragma once

#include <scai/common/ScalarType.hpp>

#include <cmath>
#include <cstdlib>

namespace scai
{

namespace common
{

/**
 * @brief The template class TypeTraits determines type-specific implementations.
 *
 * @tparam T The type of any arithmetic value type used
 */
template<typename ValueType>
class TypeTraits
{
public:

    /** Square root function for ValueType
     *
     *  In contrary to the routine of cmath it will be possible to
     *  use always the same name for the routine. 
     *
     *  \code
     *    ValueType x = sqrt ( y );                          // might not work always correctly
     *    ValueType x = TypeTraits<ValueType>::sqrt ( y );   // this is guaranteed to work
     *  \endcode
     */
    static inline ValueType sqrt( ValueType val )
    {
        // this is a good default but might be different for some value types

        return sqrt( val );
    }

    /** Absolute value function for ValueType
     *
     *  In contrary to the routine of cmath it will be possible to
     *  use always the same name for the routine. 
     */
    static inline ValueType abs( ValueType val )
    {
        return abs( val );
    }

    static inline ValueType conj( ValueType val )
    {
        return val;   
    }

    /** Get value-specific epsilon for comparison. */

    static inline ValueType getEps()
    {
        return std::numeric_limits<ValueType>::epsilon();
    }

    /** Get type-specific precision to be used for comparison in matrix-vector operations */

    static inline ValueType small()
    {
        return getEps();
    }

    /** Get maximal value of a ValueType, used for min, max reductions on arrays. */

    static inline ValueType getMax()
    {
        return std::numeric_limits<ValueType>::max();
    }

    /** Get maximal value of a ValueType, used for min, max reductions on arrays. */

    static inline ValueType getMin()
    {
        return - std::numeric_limits<ValueType>::max();
    }

    /**
     * @brief Corresponding type value of enum ScalarType.
    */
    static const scalar::ScalarType stype = scalar::UNKNOWN;

    /**
     * @brief Return name of the type for identification.
     *
     * Note: this routine returns more meaningful names than typeinfo( T ).name()
     */
    static inline const char* id()
    {
        return scalar2str( scalar::UNKNOWN );
    }
};

/** Type specific traits for IndexType */

template<>
class TypeTraits<IndexType>
{
public:

    static inline IndexType abs( IndexType x )
    {
        return ::abs( x );
    }

    static inline IndexType conj( IndexType x )
    {
        return x;
    }

    static inline IndexType getEps()
    {
        return 0;
    }

    static inline IndexType small()
    {
        return 0;
    }

    static inline IndexType getMax()
    {
        return std::numeric_limits<IndexType>::max();
    }

    static inline IndexType getMin()
    {
        return - std::numeric_limits<IndexType>::max();
    }

    static const scalar::ScalarType stype = scalar::INDEX_TYPE;

    static inline const char* id()
    {
        return scalar2str( scalar::INDEX_TYPE );
    }
};

/** Type specific traits for long double */

template<>
class TypeTraits<long double>
{
public:

    static inline long double sqrt( long double x )
    {
        return ::sqrtl( x );
    }
    static inline long double abs( long double x )
    {
        return ::fabsl( x );
    }

    static inline long double conj( long double x )
    {
        return x;
    }

    static inline long double getEps()
    {
        return std::numeric_limits<long double>::epsilon();
    }

    static inline long double small()
    {
        return 1e-8L;
    }

    static inline long double getMax()
    {
        return std::numeric_limits<long double>::max();
    }

    static inline long double getMin()
    {
        return - std::numeric_limits<long double>::max();
    }

    static const scalar::ScalarType stype = scalar::LONG_DOUBLE;

    static inline const char* id()
    {
        return scalar2str( scalar::LONG_DOUBLE );
    }
};

/** Type specific traits for double */

template<>
class TypeTraits<double>
{
public:

    static inline double sqrt( double x )
    {
        return ::sqrt( x );
    }
    static inline double abs( double x )
    {
        return ::fabs( x );
    }
    static inline double conj( double x )
    {
        return x;
    }
    static inline double getEps()
    {
        return std::numeric_limits<double>::epsilon();
    }

    static inline double small()
    {
        return 1e-5;
    }

    static inline double getMax()
    {
        return std::numeric_limits<double>::max();
    }
    static inline double getMin()
    {
        return - std::numeric_limits<double>::max();
    }

    static const scalar::ScalarType stype = scalar::DOUBLE;

    static inline const char* id()
    {
        return scalar2str( scalar::DOUBLE );
    }
};

/** Type specific traits for float */

template<>
class TypeTraits<float>
{
public:

    static inline float sqrt( float x )
    {
        return ::sqrtf( x );
    }
    static inline float abs( float x )
    {
        return ::fabsf( x );
    }
    static inline float conj( float x )
    {
        return x;
    }
    static inline float getEps()
    {
        return std::numeric_limits<float>::epsilon();
    }

    static inline float small()
    {
        return 1e-3f;
    }

    static inline float getMax()
    {
        return std::numeric_limits<float>::max();
    }
    static inline float getMin()
    {
        return - std::numeric_limits<float>::max();
    }

    static const scalar::ScalarType stype = scalar::FLOAT;

    static inline const char* id()
    {
        return scalar2str( stype );
    }
};

#ifdef SCAI_COMPLEX_SUPPORTED

/** Type specific traits for complex(float) */

template<>
class TypeTraits<ComplexFloat>
{
public:

    static inline ComplexFloat sqrt( ComplexFloat x )
    {
        return scai::common::sqrt( x );
    }
    static inline ComplexFloat abs( ComplexFloat x )
    {
        return scai::common::abs( x );
    }

    static inline ComplexFloat conj( ComplexFloat x )
    {
        return ComplexFloat( x.real(), -x.imag() );
    }
    static inline ComplexFloat getEps()
    {
        return std::numeric_limits<float>::epsilon();
    }

    static inline ComplexFloat small()
    {
        return ComplexFloat( 1e-3f );
    }

    static inline ComplexFloat getMax()
    {
        return std::numeric_limits<float>::max();
    }
    static inline ComplexFloat getMin()
    {
        return 0;
    }

    static const scalar::ScalarType stype = scalar::COMPLEX;

    static inline const char* id()
    {
        return "ComplexFloat";
    }
};

/** Type specific traits for complex(double) */

template<>
class TypeTraits<ComplexDouble>
{
public:

    static inline ComplexDouble sqrt( ComplexDouble x )
    {
        return scai::common::sqrt( x );
    }

    static inline ComplexDouble conj( ComplexDouble x )
    {
        return ComplexDouble( x.real(), -x.imag() );
    }

    static inline ComplexDouble abs( ComplexDouble x )
    {
        return scai::common::abs( x );
    }

    static inline ComplexDouble getEps()
    {
        return std::numeric_limits<double>::epsilon();
    }

    static inline ComplexDouble small()
    {
        return ComplexDouble( 1e-5 );
    }

    static inline ComplexDouble getMax()
    {
        return std::numeric_limits<double>::epsilon();
    }

    static inline ComplexDouble getMin()
    {
        return 0;
    }

    static const scalar::ScalarType stype = scalar::DOUBLE_COMPLEX;

    static inline const char* id()
    {
        return scalar2str( scalar::DOUBLE_COMPLEX );
    }
};

/** Type specific traits for complex(long double) */

template<>
class TypeTraits<ComplexLongDouble>
{
public:

    static inline ComplexLongDouble sqrt( ComplexLongDouble x )
    {
        return scai::common::sqrt( x );
    }

    static inline ComplexLongDouble abs( ComplexLongDouble x )
    {
        return scai::common::abs( x );
    }

    static inline ComplexLongDouble conj( ComplexLongDouble x )
    {
        return ComplexLongDouble( x.real(), -x.imag() );
    }

    static inline ComplexLongDouble getEps()
    {
        return std::numeric_limits<long double>::epsilon();
    }

    static inline ComplexLongDouble small()
    {
        return ComplexLongDouble( 1e-8L );
    }

    static inline ComplexLongDouble getMax()
    {
        return std::numeric_limits<long double>::max();
    }

    static inline ComplexLongDouble getMin()
    {
        return 0;
    }

    static const scalar::ScalarType stype = scalar::LONG_DOUBLE_COMPLEX;

    static inline const char* id()
    {
        return scalar2str( stype );
    }
};

#endif

/** For convenience and for compatibility make own routine of getScalarType */

template<typename ValueType> inline scalar::ScalarType getScalarType()
{
    return TypeTraits<ValueType>::stype;
}

}  // namespace common

}  // namespace scai
