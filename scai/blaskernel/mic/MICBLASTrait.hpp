/**
 * @file MICBLASTrait.hpp
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
 * @brief Definitions for MKL interface on mic
 * @author  Eric Stricker
 * @date 21.01.2016
 * @since 2.0.0
 */

#pragma once

// macros
#define MIC_BLAS_NAME( name, prefix ) prefix##name

#define MIC_BLAS_CALL( name, prefix, ... )	\
		SCAI_MIC_CALL( CUBLAS_BLAS_NAME( name, prefix ), __VAR_ARGS__ )

// external
#include <mkl.h>

namespace scai {

namespace blaskernel {

class COMMON_DLL_IMPORTEXPORT MICBLASTrait
{
public:
	typedef int BLASIndexType;
	typedef char BLASTrans;
};

} /* end namespace blaskernel */

} /* end namespace scai */