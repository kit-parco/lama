/**
 * @file BiCG.cpp
 *
 * @license
 * Copyright (c) 2013
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
 * @brief BiCG.cpp
 * @author Lauretta Schubert
 * @date 03.07.2013
 * @since 1.1.0
 */

// hpp
#include <lama/solver/BiCG.hpp>

// others
#include <lama/DenseVector.hpp>
#include <lama/tracing.hpp>

#include <lama/expression/VectorExpressions.hpp>
#include <lama/expression/MatrixVectorExpressions.hpp>

#include <lama/storage/MatrixStorage.hpp>

namespace lama
{

LAMA_LOG_DEF_LOGGER( BiCG::logger, "Solver.IterativeSolver.BiCG" )

BiCG::BiCG( const std::string& id )
: CG( id )
{
}

BiCG::BiCG( const std::string& id, LoggerPtr logger )
: CG( id, logger )
{
}

BiCG::BiCG( const BiCG& other )
: CG( other )
{
}

BiCG::~BiCG()
{
}

BiCG::BiCGRuntime::BiCGRuntime()
    : CGRuntime(), mPScalar2( 0.0 ), mResidual2( 0 )
{
}

BiCG::BiCGRuntime::~BiCGRuntime()
{
}

void BiCG::initialize( const Matrix& coefficients )
{
    LAMA_REGION( "Solver.BiCG.initialize" )
    CG::initialize( coefficients );
    BiCGRuntime& runtime = getRuntime();

    runtime.mPScalar2 = 0.0;
    runtime.mTransposeA = coefficients.create();

    switch ( coefficients.getValueType() )
    {
    case Scalar::FLOAT:
    {
        runtime.mP2.reset( new DenseVector<float>( coefficients.getDistributionPtr() ) );
        runtime.mQ2.reset( new DenseVector<float>( coefficients.getDistributionPtr() ) );
        runtime.mZ2.reset( new DenseVector<float>( coefficients.getDistributionPtr() ) );

        if( runtime.mTransposeA->getMatrixKind() == Matrix::DENSE )
        {
            LAMA_THROWEXCEPTION( "Cannot transpose a dense matrix yet." )
        }
        else if( runtime.mTransposeA->getMatrixKind() == Matrix::SPARSE )
        {
            SparseMatrix<float>* m = dynamic_cast<SparseMatrix<float>* >( runtime.mTransposeA );
            m->assignTranspose( coefficients );
        }
        else
        {
            LAMA_THROWEXCEPTION( "Internal error: matrix is not dense or sparse." )
        }

        break;
    }
    case Scalar::DOUBLE:
    {
        runtime.mP2.reset( new DenseVector<double>( coefficients.getDistributionPtr() ) );
        runtime.mQ2.reset( new DenseVector<double>( coefficients.getDistributionPtr() ) );
        runtime.mZ2.reset( new DenseVector<double>( coefficients.getDistributionPtr() ) );

        if( runtime.mTransposeA->getMatrixKind() == Matrix::DENSE )
        {
            LAMA_THROWEXCEPTION( "Cannot transpose a dense matrix yet." )
        }
        else if( runtime.mTransposeA->getMatrixKind() == Matrix::SPARSE )
        {
            SparseMatrix<double>* m = dynamic_cast<SparseMatrix<double>* >( runtime.mTransposeA );
            m->assignTranspose( coefficients );
        }
        else
        {
            LAMA_THROWEXCEPTION( "Internal error: matrix is not dense or sparse." )
        }

        break;
    }
    default:
    {
        LAMA_THROWEXCEPTION( "Unsupported ValueType " << coefficients.getValueType() )
    }
    }

    // 'force' vector operations to be computed at the same location where coefficients reside
    runtime.mTransposeA->setContext( coefficients.getContextPtr() );
    runtime.mP2->setContext( coefficients.getContextPtr() );
    runtime.mQ2->setContext( coefficients.getContextPtr() );
    runtime.mZ2->setContext( coefficients.getContextPtr() );
}

void BiCG::iterate()
{
    LAMA_REGION( "Solver.BiCG.iterate" )

    BiCGRuntime& runtime = getRuntime();
    Scalar lastPScalar( runtime.mPScalar );
    Scalar& pScalar = runtime.mPScalar;
    Scalar alpha;
    Scalar beta;

    if ( this->getIterationCount() == 0 )
    {
        this->getResidual();
        this->getResidual2();
    }

    Vector& residual = *runtime.mResidual;
    Vector& residual2 = *runtime.mResidual2;
    const Matrix& A = *runtime.mCoefficients;
    const Matrix& transA = *runtime.mTransposeA;
    Vector& x = *runtime.mSolution;
    Vector& p = *runtime.mP;
    Vector& p2 = *runtime.mP2;
    Vector& q = *runtime.mQ;
    Vector& q2 = *runtime.mQ2;
    Vector& z = *runtime.mZ;
    Vector& z2 = *runtime.mZ2;

    LAMA_LOG_INFO( logger, "Doing preconditioning." )

    //BiCG implementation start
    if ( !mPreconditioner )
    {
        z = residual;
        z2 = residual2;
    }
    else
    {
        z = 0.0;
        mPreconditioner->solve( z, residual );
        z2 = 0.0;
        mPreconditioner->solve( z2, residual2 );
    }

    LAMA_LOG_INFO( logger, "Calculating pScalar." )
    pScalar = z2.dotProduct( z );
    LAMA_LOG_DEBUG( logger, "pScalar = " << pScalar )
    LAMA_LOG_INFO( logger, "Calculating p." )

    if ( this->getIterationCount() == 0 )
    {
        p = z;
        p2 = z2;
    }
    else
    {
        if ( lastPScalar.getValue<double>() == 0.0 )
        {
            beta = 0.0;
        }
        else
        {
            beta = pScalar / lastPScalar;
        }

        LAMA_LOG_DEBUG( logger, "beta = " << beta )
        p = z + beta * p;
        LAMA_LOG_TRACE( logger, "l2Norm( p ) = " << p.l2Norm() )
        p2 = z2 + beta * p2;
        LAMA_LOG_TRACE( logger, "l2Norm( p2 ) = " << p2.l2Norm() )
    }

    {
        LAMA_REGION( "Solver.BiCG.calc_q" )
        LAMA_LOG_INFO( logger, "Calculating q." )
        q = A * p;
        LAMA_LOG_TRACE( logger, "l2Norm( q ) = " << q.l2Norm() )
        q2 = transA * p2; //p2 * A;
        LAMA_LOG_TRACE( logger, "l2Norm( q2 ) = " << q2.l2Norm() )
    }

    LAMA_LOG_INFO( logger, "Calculating pqProd." )
    const Scalar pqProd = p2.dotProduct( q );
    LAMA_LOG_DEBUG( logger, "pqProd = " << pqProd )

    if ( pqProd.getValue<double>() == 0.0 )
    {
        alpha = 0.0;
    }
    else
    {
        alpha = pScalar / pqProd;
    }

    LAMA_LOG_DEBUG( logger, "alpha = " << alpha )
    {
        LAMA_LOG_INFO( logger, "Calculating x." )
        LAMA_REGION( "Solver.BiCG.update_x" )
        x = x + alpha * p;
        LAMA_LOG_TRACE( logger, "l2Norm( x ) = " << x.l2Norm() )
    }
    {
        LAMA_LOG_INFO( logger, "Updating residual." )
        LAMA_REGION( "Solver.BiCG.update_res" )
        residual = residual - alpha * q;
        LAMA_LOG_TRACE( logger, "l2Norm( residual ) = " << residual.l2Norm() )
        residual2 = residual2 - alpha * q2;
        LAMA_LOG_TRACE( logger, "l2Norm( residual2 ) = " << residual.l2Norm() )
    }
    //BiCG implementation end

    mBiCGRuntime.mSolution.setDirty( false );
}

const Vector& BiCG::getResidual2() const
{
    LAMA_LOG_DEBUG( logger, "getResidual2 of solver " << mId )

    const BiCGRuntime& runtime = getConstRuntime();
    LAMA_ASSERT_DEBUG( runtime.mCoefficients, "mCoefficients == NULL" )
    LAMA_ASSERT_DEBUG( runtime.mRhs, "mRhs == NULL" )

    //mLogger->logMessage(LogLevel::completeInformation,"Request for residual received.\n");

    if ( runtime.mSolution.isDirty() || !runtime.mResidual2.get() )
    {
        LAMA_LOG_DEBUG( logger, "calculating residual of = " << &(runtime.mSolution.getConstReference()) )
        if ( !runtime.mResidual2.get() )
        {
            runtime.mResidual2.reset( runtime.mRhs->create() );
        }

        //mLogger->logMessage(LogLevel::completeInformation,"Residual needs revaluation.\n");

        mLogger->startTimer( "ResidualTimer" );

        //*runtime.mResidual2 = ( *runtime.mRhs ) - ( runtime.mSolution.getConstReference() * ( *runtime.mCoefficients ) ) ;
        *runtime.mResidual2 = ( *runtime.mRhs ) - ( ( *runtime.mTransposeA ) * runtime.mSolution.getConstReference()) ;

        mLogger->stopTimer( "ResidualTimer" );
        mLogger->logTime( "ResidualTimer", LogLevel::completeInformation, "Revaluation of residual took [s]: " );
        mLogger->stopAndResetTimer( "ResidualTimer" );

        runtime.mSolution.setDirty( false );
    }

    return ( *runtime.mResidual2 );
}

SolverPtr BiCG::copy()
{
    return SolverPtr( new BiCG( *this ) );
}

BiCG::BiCGRuntime& BiCG::getRuntime()
{
    return mBiCGRuntime;
}

const BiCG::BiCGRuntime& BiCG::getConstRuntime() const
{
    return mBiCGRuntime;
}

} // namespace lama