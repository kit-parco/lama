/**
 * @file lama/io/IOStream.cpp
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
 * @brief Implementation of methods for IOstream
 * @author Jan Ecker
 * @date 16.03.2016
 */

#include "IOStream.hpp"

namespace scai
{

namespace lama
{

SCAI_LOG_DEF_LOGGER( IOStream::logger, "FileIO.IOStream" )

IOStream::Endian IOStream::mMachineEndian = IOStream::_determineMachineEndian();

IOStream::IOStream( const std::string& filename, ios_base::openmode mode, Endian usedEndian )
{
    open( filename, mode, usedEndian );
}

void IOStream::closeCheck()
{
    std::streampos fsize = tellg();
    seekg( 0, std::ios::end );
    fsize = tellg() - fsize;

    if ( fsize > 0 )
    {
        SCAI_LOG_WARN( logger, "close " << mFileName << ", but remain " << fsize << " unread bytes" )
    }

    close();
}

void IOStream::open( const std::string& filename, ios_base::openmode mode, Endian usedEndian )
{
    mFileName = filename;

    std::fstream::open( filename.c_str(), mode );

    mUsedEndian = usedEndian;

    if ( !is_open() )
    {
        COMMON_THROWEXCEPTION( "Unable to open file " << mFileName )
    }
}

const std::string& IOStream::getFileName()
{
    SCAI_ASSERT( is_open(), "IOStream not opened" );

    return mFileName;
}

IOStream::Endian IOStream::_determineMachineEndian()
{
    int a = 1;
    char* ch = reinterpret_cast<char*>( &a );

    if ( static_cast<int>( *ch ) != 0 )
    {
        return LITTLE;
    }
    else
    {
        return BIG;
    }
}

void IOStream::endianConvert( char* out, const char* in, const IndexType n, const IndexType size )
{
    SCAI_LOG_INFO( logger, "endianConvert, n = " << n << ", size = " << size )

    size_t offs = 0;

    for ( IndexType i = 0; i < n; ++i )
    {
        char* out_s = out + offs;
        const char* in_s = in + offs;
        if ( size == 1 )
        {
            out_s[0] = in_s[0];
        }
        else if ( size == 2 )
        {
            out_s[0] = in_s[1];
            out_s[1] = in_s[0];
        }
        if ( size == 4 )
        {
            out_s[0] = in_s[3];
            out_s[1] = in_s[2];
            out_s[2] = in_s[1];
            out_s[3] = in_s[0];
        }
        else if ( size == 8 )
        {
            out_s[0] = in_s[7];
            out_s[1] = in_s[6];
            out_s[2] = in_s[5];
            out_s[3] = in_s[4];
            out_s[4] = in_s[3];
            out_s[5] = in_s[2];
            out_s[6] = in_s[1];
            out_s[7] = in_s[0];
        }
        else
        {
             COMMON_THROWEXCEPTION( "unsupported size for endianConvert, size = " << size )
        }
        offs += size;
    }
}

}

}