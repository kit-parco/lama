###
 # @file CompilerVersion.cmake
 #
 # @license
 # Copyright (c) 2009-2013
 # Fraunhofer Institute for Algorithms and Scientific Computing SCAI
 # for Fraunhofer-Gesellschaft
 #
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included in
 # all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 # SOFTWARE.
 # @endlicense
 #
 # @brief Version variable defintions for the used compilers
 # @author Jan Ecker
 # @date 25.04.2013
 # @since 1.0.0
###

### GNU compiler

## C Compiler
if    ( CMAKE_COMPILER_IS_GNUCC )
    execute_process ( COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE _compiler_output )
    string ( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" GNUCC_COMPILER_VERSION ${_compiler_output} )
endif ( CMAKE_COMPILER_IS_GNUCC )

## CXX Compiler
if ( CMAKE_COMPILER_IS_GNUCXX )
    execute_process ( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE _compiler_output )
    string ( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" GNUCXX_COMPILER_VERSION ${_compiler_output} )
endif ( CMAKE_COMPILER_IS_GNUCXX )

### Intel compiler

## C Compiler
if    ( CMAKE_CC_COMPILER_ID MATCHES Intel )
    execute_process ( COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE _compiler_output )
    string ( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" IntelCC_COMPILER_VERSION ${_compiler_output} )
endif ( CMAKE_CC_COMPILER_ID MATCHES Intel )

## CXX Compiler
if    ( CMAKE_CXX_COMPILER_ID MATCHES Intel )
    execute_process ( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE _compiler_output )
    string ( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" IntelCXX_COMPILER_VERSION ${_compiler_output} )
endif ( CMAKE_CXX_COMPILER_ID MATCHES Intel )