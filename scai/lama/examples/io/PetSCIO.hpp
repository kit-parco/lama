/**
 * @file PetSCIO.hpp
 *
 * @license
 * Copyright (c) 2009-2016
 * Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 * for Fraunhofer-Gesellschaft
 *
 * This file is part of the SCAI framework LAMA.
 *
 * LAMA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * LAMA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with LAMA. If not, see <http://www.gnu.org/licenses/>.
 *
 * Other Usage
 * Alternatively, this file may be used in accordance with the terms and
 * conditions contained in a signed written agreement between you and
 * Fraunhofer SCAI. Please contact our distributor via info[at]scapos.com.
 * @endlicense
 *
 * @brief Structure that contains IO routines for PETSC
 * @author Thomas Brandes
 * @date 10.06.2016
 */

#pragma once

#include "CRTPFileIO.hpp"

namespace scai
{

namespace lama
{

class PetSCIO : public CRTPFileIO<PetSCIO>
{

public:

    /** File suffix is used to decide about choice of output class */

    virtual std::string getVectorFileSuffix() const
    {
        return ".psc";
    }

    virtual std::string getMatrixFileSuffix() const
    {
        return ".psc";
    }

    /** Query if formatted or binary IO is supported 
     *
     *  @param[in] binary if true query support for binary, if false query support for formatted
     */

    virtual bool isSupported( const bool binary ) const;

public:
 
    /** Typed version of writeStorage (formatted)
     *
     *  This method must be available for implementation of
     *  CRTPFileIO::writeStorage
     */

    template<typename ValueType>
    static void writeStorageFormatted(
        const MatrixStorage<ValueType>& storage,
        const std::string& fileName )
    __attribute( ( noinline ) );

    /** Typed version of the writeStorage (binary) */

    template<typename ValueType>
    static void writeStorageBinary(
        const MatrixStorage<ValueType>& storage,
        const std::string& fileName,
        const common::scalar::ScalarType dataType,
        const common::scalar::ScalarType iaType,
        const common::scalar::ScalarType jaType )
    __attribute( ( noinline ) );

    /** Typed version of readStorage */

    template<typename ValueType>
    static void readStorageTyped(
        MatrixStorage<ValueType>& storage,
        const std::string& fileName )
    __attribute( ( noinline ) );

    /** Typed version of the writeArray (binary) */

    template<typename ValueType>
    static void writeArrayBinary(
        const hmemo::HArray<ValueType>& array,
        const std::string& fileName,
        const common::scalar::ScalarType dataType )
    __attribute( ( noinline ) );

    /** Typed version of writeArray (formatted) */

    template<typename ValueType>
    static void writeArrayFormatted(
        const hmemo::HArray<ValueType>& array,
        const std::string& fileName )
    __attribute( ( noinline ) );

    /** Typed version of readArray */

    template<typename ValueType>
    static void readArrayTyped(
        hmemo::HArray<ValueType>& array,
        const std::string& fileName )
    __attribute( ( noinline ) );

    SCAI_LOG_DECL_STATIC_LOGGER( logger );  //!< logger for IO class

};

}

}