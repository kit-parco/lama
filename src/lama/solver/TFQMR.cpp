/**
 * @file TFQMR.cpp
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
 * @brief TFQMR.cpp
 * @author 
 * @date 13.05.2015
 * @since
 */

// hpp
#include <lama/solver/TFQMR.hpp>
// others
#include <lama/expression/VectorExpressions.hpp>
#include <lama/expression/MatrixExpressions.hpp>
#include <lama/expression/MatrixVectorExpressions.hpp>

#include <lama/norm/L2Norm.hpp>
#include <lama/norm/MaxNorm.hpp>

#include <lama/DenseVector.hpp>

#include <lama/solver/criteria/ResidualStagnation.hpp>


#include <limits>

namespace lama{ 

LAMA_LOG_DEF_LOGGER( TFQMR::logger, "Solver.TFQMR" )

TFQMR::TFQMR( const std::string& id )
    : IterativeSolver(id){}


TFQMR::TFQMR( const std::string& id, LoggerPtr logger )
    : IterativeSolver(id ,logger){}

TFQMR::TFQMR( const TFQMR& other )
    : IterativeSolver( other ){}



TFQMR::TFQMRRuntime::TFQMRRuntime()
    : IterativeSolverRuntime(){}

TFQMR::~TFQMR(){}

TFQMR::TFQMRRuntime::~TFQMRRuntime(){}

void TFQMR::initialize( const Matrix& coefficients ){
    LAMA_LOG_DEBUG(logger, "Initialization started for coefficients = "<< coefficients)

    Solver::initialize( coefficients );
 	TFQMRRuntime& runtime = getRuntime();

    runtime.mAlpha  = 0.0;
    runtime.mBeta   = 0.0;
    runtime.mC      = 0.0;
    runtime.mEta    = 0.0;
    runtime.mTheta  = 0.0;

    Scalar::ScalarType type = coefficients.getValueType();
    
    runtime.mVecD.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );
    runtime.mInitialR.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );
    runtime.mVecVEven.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );
    runtime.mVecVOdd.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );
    runtime.mVecW.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );
    runtime.mVecZ.reset( Vector::createVector( type, coefficients.getDistributionPtr() ) );

    runtime.mVecD->setContext( coefficients.getContextPtr() );   
    runtime.mInitialR->setContext( coefficients.getContextPtr() );
    runtime.mVecVEven->setContext( coefficients.getContextPtr() );
    runtime.mVecVOdd->setContext( coefficients.getContextPtr() );
    runtime.mVecW->setContext( coefficients.getContextPtr() );
    runtime.mVecZ->setContext( coefficients.getContextPtr() );
    
}


void TFQMR::solveInit( Vector& solution, const Vector& rhs ){
    TFQMRRuntime& runtime = getRuntime();

    runtime.mRhs = &rhs;
    runtime.mSolution = &solution;

    if ( runtime.mCoefficients->getNumRows() != runtime.mRhs->size() ){
        LAMA_THROWEXCEPTION(
            "Size of rhs vector " << *runtime.mRhs << " does not match column size of matrix " << *runtime.mCoefficients );
    }

    if ( runtime.mCoefficients->getNumColumns() != solution.size() ){
        LAMA_THROWEXCEPTION(
            "Size of solution vector " << solution << " does not match row size of matrix " << *runtime.mCoefficients );
    }

    if ( runtime.mCoefficients->getColDistribution() != solution.getDistribution() ){
        LAMA_THROWEXCEPTION(
            "Distribution of lhs " << solution << " = " << solution.getDistribution() << " does not match (row) distribution of " << *runtime.mCoefficients << " = " << runtime.mCoefficients->getColDistribution() );
    }

    if ( runtime.mCoefficients->getDistribution() != runtime.mRhs->getDistribution() ){
        LAMA_THROWEXCEPTION(
            "Distribution of old Solution " << *runtime.mRhs << " = " << runtime.mRhs->getDistribution() << " does not match (row) distribution of " << *runtime.mCoefficients << " = " << runtime.mCoefficients->getDistribution() );
    }


    // Initialize
    this->getResidual();   

    const Matrix& A = *runtime.mCoefficients;

    Vector* initialR = (*runtime.mResidual).copy();
    Vector* mVecVEven = (*runtime.mResidual).copy();
    Vector* mVecW = (*runtime.mResidual).copy();

    runtime.mInitialR.reset( initialR );
    runtime.mVecVEven.reset( mVecVEven );
    runtime.mVecW.reset( mVecW );


    *runtime.mVecZ = A * (*runtime.mResidual);
    *runtime.mVecD *= (0.0);                   

    L2Norm norm;
    runtime.mTau = norm.apply(*runtime.mInitialR);
    runtime.mRhoOld= runtime.mTau * runtime.mTau;



  
    runtime.mSolveInit = true;
}

void TFQMR::iterationEven(){
	TFQMRRuntime& runtime = getRuntime();

	const Vector& vecZ		= *runtime.mVecZ;
	const Vector& initialR	= *runtime.mInitialR;
	const Vector& vecVEven 	= *runtime.mVecVEven;
		  Vector& vecVOdd	= *runtime.mVecVOdd;
	const Scalar& rho 		= runtime.mRhoOld;
	
	const Scalar dotProduct	= vecZ.dotProduct( initialR );

	Scalar& alpha = runtime.mAlpha;	

	alpha = rho / dotProduct;
	vecVOdd  = vecVEven - alpha*vecZ;

}

void TFQMR::iterationOdd(){
	TFQMRRuntime& runtime = getRuntime();

	const Matrix& A 		= *runtime.mCoefficients;
	const Vector& vecW		= *runtime.mVecW;
	const Vector& vecVOdd 	= *runtime.mVecVOdd;
	   Vector& vecVEven 	= *runtime.mVecVEven;
	        Scalar& rhoOld	= runtime.mRhoOld;	  	  
	Scalar& rhoNew 	= runtime.mRhoNew;
	Scalar& beta		= runtime.mBeta;
	Vector& vecZ = *runtime.mVecZ;

	const Vector& initialR = *runtime.mInitialR;

	rhoNew 	= vecW.dotProduct(initialR);
	beta   	= rhoNew / rhoOld;
	vecVEven = vecW + beta* vecVOdd;
	vecZ *= beta;
	vecZ = beta * A * vecVOdd + beta * vecZ;
	vecZ = A * vecVEven + vecZ;
    rhoOld = rhoNew;
}	



void TFQMR::iterate(){
    TFQMRRuntime& runtime	= getRuntime();
	const IndexType& iteration = runtime.mIterations; 
		  L2Norm norm;

    const Matrix& A 		= *runtime.mCoefficients;
    	  Vector& vecW 		= *runtime.mVecW;
    	  Vector& vecD 		= *runtime.mVecD;
	  Vector& solution      = *runtime.mSolution;
    const Scalar& alpha 	= runtime.mAlpha;
	  Scalar& c             = runtime.mC;
          Scalar& eta 		= runtime.mEta;
    	  Scalar& theta 	= runtime.mTheta;
    	  Scalar& tau 		= runtime.mTau;

    const Vector* vecVp;

    // if iteration = even -> need mVecVEven, else need mVecVOdd    
    if( (iteration % 2) == 0 ) vecVp = &(*runtime.mVecVEven);
    else vecVp = &(*runtime.mVecVOdd);


    const Vector& vecV = *vecVp;

    if( (iteration % 2) == 0 && iteration > 0 )
        iterationOdd();

    if( (iteration % 2) == 0 )
    	iterationEven();

    vecW = vecW - alpha * A * vecV;

    Scalar tempScal = theta*theta*eta/alpha;
    vecD = vecV + tempScal * vecD;				
    theta = norm.apply(vecW)/tau;
    c = 1.0 / sqrt( 1.0 + theta*theta);
    tau = tau* theta * c;

    eta = c*c*alpha;
    solution = solution + eta * vecD;
}

void TFQMR::setStoppingCriterion( const CriterionPtr criterion ){
    Scalar eps = std::numeric_limits<double>::epsilon()*5;                // NOT ABSTRACT.
    IndexType lookback = 2;

    NormPtr norm = NormPtr( new MaxNorm() );
    CriterionPtr rs( new ResidualStagnation( norm, lookback, eps ));

    LAMA_ASSERT_ERROR( criterion, "Criterion defined is NULL." )
    LAMA_LOG_INFO( logger, "Criteria " << *criterion << " defined." )

    mCriterionRootComponent = ( criterion ||  rs );
}

SolverPtr TFQMR::copy(){
    return SolverPtr( new TFQMR( *this ) );
}

TFQMR::TFQMRRuntime& TFQMR::getRuntime(){
    return mTFQMRRuntime;
}

const TFQMR::TFQMRRuntime& TFQMR::getConstRuntime() const{
    return mTFQMRRuntime;
}

} /* namespace lama */
