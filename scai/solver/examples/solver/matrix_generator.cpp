/**
 * @file matrix_generator.cpp
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
 * @brief Example program that generates matrices and writes them to a file
 * @author Thomas Brandes
 * @date 15.05.2013
 * @since 1.0.0
 */

// Define levels for assertion, logging and tracing

#include <scai/lama.hpp>

#include <scai/lama/DenseVector.hpp>
#include <scai/lama/Scalar.hpp>
#include <scai/lama/expression/all.hpp>
#include <scai/lama/StorageIO.hpp>
#include <scai/lama/matrix/CSRSparseMatrix.hpp>
#include <scai/lama/matutils/MatrixCreator.hpp>

#include <iostream>

using namespace scai::lama;
using namespace scai::dmemo;
using namespace std;

void replaceStencil( std::string& str, const std::string& stencil )
{
    size_t i = str.find( "%s" );

    if ( i != string::npos )
    {
        str.replace( i, 2, stencil );
    }
}

void printUsage( const char* prog_name )
{
    cout << "Usage: " << prog_name << " <filename> <dim> <stencilType> <dimX> [ <dimY> [ <dimZ> ] ]" << endl;
    cout << "         filename : name of the output file for matrix, vector" << endl;
    cout << "           filename = <id>.mtx -> generates matrix market format, <id>_v.mtx for vector" << endl;
    cout << "           filename = <id>     -> generates binary format, <id>.frm for matrix, <id>.frv for vector" << endl;
    cout << "           %s in filename is replaced with stencil values, e.g. 2D5P_100_100" << endl;
    cout << "         dim = 1, 2, 3  is dimension of stencil" << endl;
    cout << "         stencilType = 3 (for dim = 1) " << endl;
    cout << "         stencilType = 5, 9 (for dim = 2) " << endl;
    cout << "         stencilType = 7, 19, 27 (for dim = 3) " << endl;
}

int main( int argc, char* argv[] )
{
    CommunicatorPtr comm = Communicator::getCommunicator();

    int myRank = comm->getRank();

    std::string filename;
    
    IndexType dimension = 1;
    IndexType stencilType = 3;
    IndexType dimX = 1;
    IndexType dimY = 1;
    IndexType dimZ = 1;

    if ( argc >= 5 )
    {
        filename = argv[1];
        sscanf( argv[2], "%d", &dimension );
        sscanf( argv[3], "%d", &stencilType );
        sscanf( argv[4], "%d", &dimX );
        if ( argc >= 6 ) sscanf( argv[5], "%d", &dimY );
        if ( argc >= 7 ) sscanf( argv[6], "%d", &dimZ );
    }
    else
    {
        if ( myRank == 0 )
        {
            printUsage( argv[0] );
        }
        return -1;
    }

    cout << "Generate poisson file " << filename << 
            ", dim = " << dimension << ", stencilType = " << stencilType << endl;

    if ( !MatrixCreator<double>::supportedStencilType( dimension, stencilType ) )
    {
        if ( myRank == 0 )
        {
            cout << "Unsupported stencilType " << stencilType << " for dim = " << dimension << endl;
        }
        return -1;
    }

    if ( argc != ( dimension + 4 ) )
    {
        if ( myRank == 0 )
        {
            cout << "Missing values for dim = " << dimension 
                 << ", argc = " << argc << ", expected " << ( dimension + 3 ) << endl;
        }
        return -1;
    }

    // Generate name for the stencil 

    ostringstream stencilName;

    stencilName << dimension << "D" << stencilType << "P_" << dimX;
    if ( dimension > 1 ) 
    {
        stencilName << "_" << dimY;
    }
    if ( dimension > 2 ) 
    {
        stencilName << "_" << dimZ;
    }

    cout << "Stencil is : " << stencilName.str() << endl;

    // replace %s in filename with stencil description

    replaceStencil( filename, stencilName.str() );

    CSRSparseMatrix<double> m;

    MatrixCreator<double>::buildPoisson( m, dimension, stencilType, dimX, dimY, dimZ );

    DenseVector<double> lhs( m.getDistributionPtr(), 1.0 );
    DenseVector<double> rhs( m * lhs );

    cout << "m = " << m << endl;
    cout << "m has diagonal property = " << m.hasDiagonalProperty() << endl;
    cout << "lhs = " << lhs << endl;
    cout << "rhs = " << rhs << endl;

    cout << endl;
    cout << "Solution vector x = ( 1.0, ..., 1.0 ) assumed" << endl;
    cout << "Write matrix and rhs vector to file" << endl;

    if ( _StorageIO::hasSuffix( filename, ".mtx" ) )
    {
        std::string vectorFilename = filename;

        // replace . with _v. 

        vectorFilename.replace( vectorFilename.length() - 4, 1, "_v." );

        m.writeToFile( filename, File::MATRIX_MARKET );
        rhs.writeToFile( vectorFilename, File::MATRIX_MARKET );

        cout << "Written matrix to matrix market file " << filename  << endl;
        cout << "Written rhs vector to matrix market file " << vectorFilename << endl;
    }
    else
    {
        m.writeToFile( filename, File::BINARY );
        rhs.writeToFile( filename, File::BINARY );

        cout << "Written matrix to header file " << filename << ".frm and binary file " << filename << ".amg" << endl;
        cout << "Written rhs vector to header file " << filename << ".frv and binary file " << filename << ".vec" << endl;
    }
}
