/**
 * @file ContextArray.hpp
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
 * @brief Common base class for dynamic array classes where the array data can be
 *        used in different contexts and where the data is moved implicitly
 *        when corresponding read/write accesses are required.
 *
 * @author Thomas Brandes, Jiri Krause
 * @date 03.07.2015
 */

#pragma once

// for dll_import
#include <common/config.hpp>

// base classes

// others
#include <memory/Context.hpp>
#include <memory/ContextDataManager.hpp>
#include <memory/Scalar.hpp>

// common
#include <common/Printable.hpp>
#include <common/Factory.hpp>

#include <vector>
#include <map>

/** Number of contexts that might be used in maximum. This number
 *  is used for reservation of entries but does not imply any restrictions.
 */

#define LAMA_MAX_CONTEXTS 4

namespace memory
{

// Forward declaration of friend classes.

template<typename ValueType>
class ReadAccess;

template<typename ValueType>
class WriteAccess;

/** Common base class for typed LAMAArray. 
 * 
 *  Base class provides also a factory for creating arrays.
 */

class COMMON_DLL_IMPORTEXPORT ContextArray: 

    public Printable, 
    public common::Factory<ScalarType, ContextArray*>
{
    // Member variables of this class

protected:

    IndexType mSize;        //!< number of entries for the context array, common for all contexts
    IndexType mValueSize;   //!< number of bytes needed for one data element

    bool constFlag;         //!< if true the array cannot be written

    mutable ContextDataManager mContextDataManager;  //!< takes control of accesses and allocations

public:

    virtual ~ContextArray()
    {
    }

    /**
     * @brief Query the value type of the array elements, e.g. DOUBLE or FLOAT.
     */
    virtual ScalarType getValueType() const = 0;

    using common::Factory<ScalarType, ContextArray*>::create;

    /**
     *  Each derived class must provide a create function. This will
     *  allow writing general routines that require temporary data.
     */

    virtual ContextArray* clone() = 0;

    /**
     * @brief Query the current size of the LAMA array, i.e. number of entries.
     *
     * @return the number of entries of the array.
     */
    inline IndexType size() const;

    /**
     * @brief Gets the first context where the data of this LAMAArray is available.
     *
     * If possible a context of the passed preferred type is returned.
     *
     * @param[in] preferredType the preferred type for the valid context.
     * @return                  a context there the data of this LAMAArray is available.
     * 
     * Note: NULL pointer is returned if no valid data is available
     */
    ContextPtr getValidContext( const ContextType preferredType = context::Host ) const;

    /**
     * @brief Prefetches the contents of the container to the passed context.
     *
     * @param[in] context  the context to prefetch to
     *
     * This method prefetches the contents of the container to the context.
     * If this valid at location nothing happens,if not a transfer from a valid location
     * to the passed location is started. Because the transfer is handled by LAMAArray and to
     * maintain the consistency of the container only one running transfer can exist at any
     * point in time. There for if two prefetches to two different invalid locations are
     * started one after the other the second transfer does not start before the first one is finished.
     */
    void prefetch( ContextPtr context );

    /**
     * @brief Query the capacity ( in number of elements ) at a certain context.
     */
    IndexType capacity( ContextPtr context ) const;
    
    /**
     * @brief Query if data is valid in a certain context
     */
    bool isValid( ContextPtr context ) const;

    /**
     * @brief resize the array
     * 
     * Changes the size of the array. No memory is freed if the size becomes smaller.
     * Reserves additional memory on all valid locations.
     */
    void resize( IndexType size );

    /**
     * @brief clear of an array is the same as resize 0 
     *
     * \code
     *   ContextArray& array = ...
     *   // this sequence of statements just invalidates all data
     *   IndexType size = array.size();
     *   array.clear();
     *   array.resize( 0 );
     * \endcode
     */

    void clear();

protected:

    explicit ContextArray( const IndexType n, const IndexType size ) : 

        mSize( n ), 
        mValueSize( size ), 
        constFlag( false )
    {
    }

    /** Complete handling to get read access for a certain context.
     *
     *  @param[in] context is the context where the read access is needed
     *
     *  For a read access it will be always guaranteed that a valid copy of the 
     *  data is available at the context.
     *
     * \returns index of context data array that contains the valid entry.
     */
    ContextDataIndex acquireReadAccess( ContextPtr context ) const;

    /** Release an acquired read access. */

    void releaseReadAccess( ContextDataIndex ref ) const;

    /** Complete handling to get write access for a certain context.
     *
     *  @param[in] context is the context where the write access is needed
     *  @param[in] keepFlag if true it will be guaranteed that a valid copy is at the context.
     *
     *  A write only access will set keepFlag = false.
     *
     * \returns index of context data array that contains the valid entry.
     */
    ContextDataIndex acquireWriteAccess( ContextPtr context, bool keepFlag );

    /** Release an acquired write access. */

    void releaseWriteAccess( ContextDataIndex );

    /** Query the capacity for a certain access. 
     *
     *  @param[in] index is the reference to the context data as the result of an acquired access.
     */

    IndexType capacity( ContextDataIndex index ) const;
};

/* ---------------------------------------------------------------------------------*/

inline IndexType ContextArray::size() const
{
    return mSize;
}

/* ---------------------------------------------------------------------------------*/

inline void ContextArray::prefetch( ContextPtr context )
{
    mContextDataManager.prefetch( context, mSize * mValueSize );
}

/* ---------------------------------------------------------------------------------*/

inline bool ContextArray::isValid( ContextPtr context ) const
{
    return mContextDataManager.isValid( context );
}

/* ---------------------------------------------------------------------------------*/

inline void ContextArray::resize( IndexType size )
{
    // resize on all valid locations

    mContextDataManager.resize( size * mValueSize, mSize * mValueSize );

    mSize = size;   
}

/* ---------------------------------------------------------------------------------*/

inline void ContextArray::clear()
{
    COMMON_ASSERT( !mContextDataManager.locked(), "Tried to clear a locked LAMAArray " << *this )

    mSize = 0;
}

/* ---------------------------------------------------------------------------------*/

inline IndexType ContextArray::capacity( ContextPtr context ) const
{
    // will return 0 if no data is available at the specified context

    return mContextDataManager.capacity( context ) / mValueSize;
}

/* ---------------------------------------------------------------------------------*/

inline IndexType ContextArray::capacity( ContextDataIndex index ) const
{
    const ContextData& entry = mContextDataManager[index];

    return static_cast<IndexType>( entry.capacity() / mValueSize );
}

/* ---------------------------------------------------------------------------------*/

inline ContextPtr ContextArray::getValidContext( const ContextType preferredType ) const
{
    return mContextDataManager.getValidContext( preferredType );
}

/* ---------------------------------------------------------------------------------*/

inline ContextDataIndex ContextArray::acquireReadAccess( ContextPtr context ) const
{
    size_t allocSize = mSize * mValueSize;
    size_t validSize = allocSize;                   // read access needs valid data in any case

    return mContextDataManager.acquireAccess( context, context::Read, allocSize, validSize );
}

/* ---------------------------------------------------------------------------------*/

inline void ContextArray::releaseReadAccess( ContextDataIndex index ) const
{
    mContextDataManager.releaseAccess( index, context::Read );
}

/* ---------------------------------------------------------------------------------*/

inline ContextDataIndex ContextArray::acquireWriteAccess( ContextPtr context, bool keepFlag )
{
    size_t allocSize = mSize * mValueSize;
    size_t validSize = keepFlag ? allocSize : 0 ;    // valid data only if keepFlag is set

    return mContextDataManager.acquireAccess( context, context::Write, allocSize, validSize );
}

/* ---------------------------------------------------------------------------------*/

inline void ContextArray::releaseWriteAccess( ContextDataIndex index )
{
    mContextDataManager.releaseAccess( index, context::Write );
}

}  // namespace 