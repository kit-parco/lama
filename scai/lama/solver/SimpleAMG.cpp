/**
 * @file SimpleAMG.cpp
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
 * @brief SimpleAMG.cpp
 * @author Jiri Kraus
 * @date 27.10.2011
 * @since 1.0.0
 */

// hpp
#include <scai/lama/solver/SimpleAMG.hpp>

// others
#include <scai/lama/solver/shared_lib.hpp>
#include <scai/lama/solver/SingleGridSetup.hpp>

#include <scai/lama/expression/MatrixVectorExpressions.hpp>
#include <scai/lama/Settings.hpp>

// tracing
#include <scai/tracing.hpp>
#include <omp.h>

#include <cstdlib>
#include <iomanip>

using namespace scai::hmemo;

namespace scai
{

namespace lama
{

SCAI_LOG_DEF_LOGGER( SimpleAMG::logger, "Solver.IterativeSolver.SimpleAMG" )
SCAI_LOG_DEF_LOGGER( SimpleAMG::SimpleAMGRuntime::logger, "Solver.IterativeSolver.SimpleAMG.SimpleAMGRuntime" )

SimpleAMG::SimpleAMG( const std::string& id )
    : IterativeSolver( id ), mMaxLevels( 25 ), mMinVarsCoarseLevel( 100 ), mSmootherContext(
          Context::getContextPtr( context::Host ) )
{
    SCAI_LOG_INFO( logger, "SimpleAMG, id = " << id << " created, no logger" )
}

SimpleAMG::SimpleAMG( const std::string& id, LoggerPtr logger )
    : IterativeSolver( id, logger ), mMaxLevels( 25 ), mMinVarsCoarseLevel( 100 ), mSmootherContext(
          Context::getContextPtr( context::Host ) )
{
    SCAI_LOG_INFO( SimpleAMG::logger, "SimpleAMG, id = " << id << " created, with logger" )
}

SimpleAMG::SimpleAMG( const SimpleAMG& other )
    : IterativeSolver( other ), mMaxLevels( other.mMaxLevels ), mMinVarsCoarseLevel(
          other.mMinVarsCoarseLevel ), mSmootherContext( other.mSmootherContext )
{
}

SimpleAMG::SimpleAMGRuntime::SimpleAMGRuntime()
    : IterativeSolverRuntime(), mSetup(), mCurrentLevel( 0 ), mLibHandle( 0 ), mHostOnlyLevel(
          std::numeric_limits<IndexType>::max() ), mHostOnlyVars( 0 ), mReplicatedLevel(
          std::numeric_limits<IndexType>::max() )
{
}

SimpleAMG::~SimpleAMG()
{
}

SimpleAMG::SimpleAMGRuntime::~SimpleAMGRuntime()
{
    common::shared_ptr<AMGSetup>& amgSetup = mSetup;

    if( mLibHandle != 0 )
    {
        SCAI_LOG_INFO( logger, "~SimpleAMG, now release AMGSetup in lib" )

        typedef void (*lama_releaseAMGSetup)( scai::lama::AMGSetup* );
        lama_releaseAMGSetup funcHandle = NULL;
        getFunctionHandle( funcHandle, reinterpret_cast<LAMA_LIB_HANDLE_TYPE&>( mLibHandle ),"lama_releaseAMGSetup" );

        if( funcHandle )
        {
            funcHandle( amgSetup.get() );
        }
        else
        {
            SCAI_LOG_WARN( logger, "Failed to locate function lama_releaseAMGSetup" )
        }

        SCAI_LOG_INFO( logger, "~SimpleAMG, AMGSetup has been released" )

        //TODO: feeLibHandle call dlclose this leads to a crash in case of
        //      scalapack mkl + intel mpi
        //freeLibHandle( reinterpret_cast<LAMA_LIB_HANDLE_TYPE&>( mLibHandle ) );

        SCAI_LOG_INFO( logger, "~SimpleAMG, library has been released" )
    }
    else
    {
        SCAI_LOG_INFO( logger, "~SimpleAMG, no library has been used" )
    }
}

void SimpleAMG::initialize( const Matrix& coefficients )
{
    SCAI_REGION( "Solver.SimpleAMG.initialize" )

    SCAI_LOG_DEBUG( logger, "initialize AMG, coefficients matrix = " << coefficients )

    SimpleAMGRuntime& runtime = getRuntime();

    common::shared_ptr<AMGSetup>& amgSetup = runtime.mSetup;

    if( amgSetup.get() == 0 )
    {
        //try to load libAMGSetup.so

        std::string amgSetupLibrary;

        const Communicator& comm = coefficients.getDistribution().getCommunicator();

        // comm: so it is sufficient if only root has set the environment variable

        bool isSet = Settings::getEnvironment( amgSetupLibrary, "LAMA_AMG_SETUP_LIBRARY", comm );

        if( isSet )
        {
            typedef scai::lama::AMGSetup* (*lama_createAMGSetup)();
            lama_createAMGSetup funcHandle = NULL;

            int error = loadLibAndGetFunctionHandle( funcHandle, reinterpret_cast<LAMA_LIB_HANDLE_TYPE&>( runtime.mLibHandle ), amgSetupLibrary.c_str(), "lama_createAMGSetup" );

            if( error == 0 && runtime.mLibHandle != 0 && funcHandle != 0 )
            {
                amgSetup.reset( funcHandle() );
            }
            else
            {
                SCAI_LOG_WARN( logger,
                               "Failed to load lib " << amgSetupLibrary << ", func lama_createAMGSetup" << ": error = " << error << ", lib handle = " << runtime.mLibHandle << ", func handle = " << funcHandle )
            }
        }
        else
        {
            SCAI_LOG_WARN( logger, "LAMA_AMG_SETUP_LIBRARY not set, take SingleGridSetup" )
        }

        if( amgSetup.get() == 0 )
        {
            amgSetup.reset( new SingleGridSetup() );
        }
    }

    amgSetup->setMaxLevels( mMaxLevels );
    amgSetup->setMinVarsCoarseLevel( mMinVarsCoarseLevel );
    amgSetup->setHostOnlyLevel( runtime.mHostOnlyLevel );
    amgSetup->setHostOnlyVars( runtime.mHostOnlyVars );
    amgSetup->setReplicatedLevel( runtime.mReplicatedLevel );
    amgSetup->setCoarseLevelSolver( mCoarseLevelSolver );
    amgSetup->setSmoother( mSmoother );

    logSetupSettings();

    amgSetup->initialize( coefficients );

    logSetupInfo();

    logSolverInfo();

    logSetupDetails();

    if( mSmootherContext )
    {
        for( IndexType level = 0; level < (IndexType) amgSetup->getNumLevels() - 1; ++level )
        {
            amgSetup->getSmoother( level ).setContext( mSmootherContext );
        }
    }

    IterativeSolver::initialize( coefficients );

    totalSmootherTime = 0.0;
    totalTransferTime = 0.0;
    totalResidualTime = 0.0;
    totalIterations = 0;
}

void SimpleAMG::iterate()
{
    SCAI_REGION( "Solver.SimpleAMG.iterate" )

    cycle();
    totalIterations++;
}

double SimpleAMG::getAverageSmootherTime() const
{
    return ( totalSmootherTime / totalIterations );
}

double SimpleAMG::getAverageTransferTime() const
{
    return ( totalTransferTime / totalIterations );
}

double SimpleAMG::getAverageResidualTime() const
{
    return ( totalResidualTime / totalIterations );
}

void SimpleAMG::setMaxLevels( unsigned int levels )
{
    mMaxLevels = levels;
}

void SimpleAMG::setMinVarsCoarseLevel( unsigned int vars )
{
    mMinVarsCoarseLevel = vars;
}

const Matrix& SimpleAMG::getGalerkin( unsigned int level )
{
    return getRuntime().mSetup->getGalerkin( level );
}

const Matrix& SimpleAMG::getRestriction( unsigned int level )
{
    return getRuntime().mSetup->getRestriction( level );
}

const Matrix& SimpleAMG::getInterpolation( unsigned int level )
{
    return getRuntime().mSetup->getInterpolation( level );
}

Vector& SimpleAMG::getSolutionVector( unsigned int level )
{
    return getRuntime().mSetup->getSolutionVector( level );
}

Vector& SimpleAMG::getRhsVector( unsigned int level )
{
    return getRuntime().mSetup->getRhsVector( level );
}

Solver& SimpleAMG::getSmoother( unsigned int level )
{
    return getRuntime().mSetup->getSmoother( level );
}

Solver& SimpleAMG::getCoarseLevelSolver()
{
    return getRuntime().mSetup->getCoarseLevelSolver();
}

void SimpleAMG::setSmootherContext( ContextPtr smootherContext )
{
    mSmootherContext = smootherContext;
}

void SimpleAMG::setHostOnlyLevel( IndexType hostOnlyLevel )
{
    getRuntime().mHostOnlyLevel = hostOnlyLevel;
}

void SimpleAMG::setHostOnlyVars( IndexType hostOnlyVars )
{
    getRuntime().mHostOnlyVars = hostOnlyVars;
}

void SimpleAMG::setReplicatedLevel( IndexType replicatedLevel )
{
    getRuntime().mReplicatedLevel = replicatedLevel;
}

void SimpleAMG::setCoarseLevelSolver( SolverPtr solver )
{
    SCAI_LOG_DEBUG( logger, "Defined smoother for all level " << *solver )
    mCoarseLevelSolver = solver;
}

void SimpleAMG::setSmoother( SolverPtr solver )
{
    SCAI_LOG_DEBUG( logger, "Defined smoother for all level " << *solver )
    mSmoother = solver;
}

unsigned int SimpleAMG::getNumLevels()
{
    return getRuntime().mSetup->getNumLevels();
}

void SimpleAMG::cycle()
{
    // go via pointers because of const rhs on finest grid

    SimpleAMGRuntime& runtime = getRuntime();

    SCAI_REGION_N( "Solver.SimpleAMG.cycle", runtime.mCurrentLevel )

    // dereferences to current level solution + rhs
    common::shared_ptr<AMGSetup>& amgSetup = runtime.mSetup;

    const Vector* curRhsPtr = runtime.mRhs;
    Vector* curSolutionPtr = 0;

    if( runtime.mCurrentLevel == 0 )
    {
        curSolutionPtr = &( runtime.mSolution.getReference() );
    }
    else
    {
        curSolutionPtr = &( amgSetup->getSolutionVector( runtime.mCurrentLevel ) );
        curRhsPtr = &( amgSetup->getRhsVector( runtime.mCurrentLevel ) );
    }

    Vector& curSolution = ( *curSolutionPtr );

    const Vector& curRhs = ( *curRhsPtr );

    //no more Smoothers we are on the coareste level
    if( runtime.mCurrentLevel >= amgSetup->getNumLevels() - 1 )
    {
        amgSetup->getCoarseLevelSolver().solve( curSolution, curRhs );
    }
    else
    {
        const Matrix& curGalerkin = amgSetup->getGalerkin( runtime.mCurrentLevel );
        const Matrix& curRestriction = amgSetup->getRestriction( runtime.mCurrentLevel );
        const Matrix& curInterpolation = amgSetup->getInterpolation( runtime.mCurrentLevel );
        Vector& curTmpRhs = amgSetup->getTmpResVector( runtime.mCurrentLevel );
        Vector& curCoarseSolution = amgSetup->getSolutionVector( runtime.mCurrentLevel + 1 );
        Vector& curCoarseRhs = amgSetup->getRhsVector( runtime.mCurrentLevel + 1 );
        Solver& curSmoother = amgSetup->getSmoother( runtime.mCurrentLevel );

        // PreSmoothing
        SCAI_LOG_DEBUG( logger, "Pre smoothing on level "<< runtime.mCurrentLevel )
        double smootherStartTime = omp_get_wtime();
        curSmoother.solve( curSolution, curRhs );
        totalSmootherTime += omp_get_wtime() - smootherStartTime;

        // Restrict residual to next coarser grid
        // and initialize solution
        SCAI_LOG_DEBUG( logger, "curTmpRhs=curRhs - curGalerkin * curSolution on level "<< runtime.mCurrentLevel )
        double residualStartTime = omp_get_wtime();
        curTmpRhs = curRhs - curGalerkin * curSolution;
        totalResidualTime += omp_get_wtime() - residualStartTime;
        SCAI_LOG_DEBUG( logger, "curCoarseRhs = curRestriction * curTmpRhs on level "<< runtime.mCurrentLevel )
        double transferStartTime = omp_get_wtime();
        curCoarseRhs = curRestriction * curTmpRhs;
        curCoarseSolution = 0.0;
        totalTransferTime += omp_get_wtime() - transferStartTime;

        ++runtime.mCurrentLevel;

        cycle();

        --runtime.mCurrentLevel;

        SCAI_LOG_DEBUG( logger,
                        "curSolution = curSolution + curInterpolation * curCoarseSolution on level "<< runtime.mCurrentLevel )
        transferStartTime = omp_get_wtime();
        curSolution = curSolution + curInterpolation * curCoarseSolution;
        totalTransferTime += omp_get_wtime() - transferStartTime;

        SCAI_LOG_DEBUG( logger, "Post smoothing on level "<< runtime.mCurrentLevel )
        smootherStartTime = omp_get_wtime();
        curSmoother.solve( curSolution, curRhs );
        totalSmootherTime += omp_get_wtime() - smootherStartTime;
    }
}

void SimpleAMG::logSetupSettings()
{
    if( mLogger->getLogLevel() < LogLevel::solverInformation )
    {
        return;
    }

    if( getRuntime().mSetup.get() != 0 )
    {
        mLogger->logMessage( LogLevel::solverInformation, "Running SimpleAMG.\n" );
        mLogger->logNewLine( LogLevel::solverInformation );
        mLogger->logMessage( LogLevel::solverInformation, "Setup Components:\n" );
        mLogger->logMessage( LogLevel::solverInformation, "=================\n" );

        std::stringstream outputCouplingPredicate;
        outputCouplingPredicate << getRuntime().mSetup->getCouplingPredicateInfo() << "\n";
        mLogger->logMessage( LogLevel::solverInformation, outputCouplingPredicate.str() );

        std::stringstream outputColoring;
        outputColoring << getRuntime().mSetup->getColoringInfo() << "\n";
        mLogger->logMessage( LogLevel::solverInformation, outputColoring.str() );

        std::stringstream outputInterpolation;
        outputInterpolation << getRuntime().mSetup->getInterpolationInfo() << "\n";

        mLogger->logMessage( LogLevel::solverInformation, outputInterpolation.str() );
        mLogger->logNewLine( LogLevel::solverInformation );

        std::stringstream outputRunning;
        outputRunning << "Running Setup (maxLevels=" << mMaxLevels << ", mMinVarsCoarseLevel=" << mMinVarsCoarseLevel
                      << ")...\n";
        mLogger->logMessage( LogLevel::solverInformation, outputRunning.str() );
    }
    else
    {
        mLogger->logMessage( LogLevel::solverInformation, "Running Gauss-Seidel.\n" );
    }
}

void SimpleAMG::logSetupInfo()
{
    if( mLogger->getLogLevel() < LogLevel::solverInformation )
    {
        return;
    }

    mLogger->logMessage( LogLevel::solverInformation, "Setup done.\n" );
    mLogger->logType( LogLevel::solverInformation, "Number of levels\t: ", getRuntime().mSetup->getNumLevels() );
    mLogger->logNewLine( LogLevel::solverInformation );

    if( mLogger->getLogLevel() < LogLevel::advancedInformation )
    {
        return;
    }

    for( unsigned int i = 0; i < getRuntime().mSetup->getNumLevels(); ++i )
    {
        if( i == 0 )
        {
            std::stringstream output1;
            output1 << "Operator Matrix Hierarchy:\n";
            mLogger->logMessage( LogLevel::advancedInformation, output1.str() );
            std::stringstream output2;
            output2 << "Lvl    #Rows    #Cols  #Entries Average\n";
            mLogger->logMessage( LogLevel::advancedInformation, output2.str() );
        }

        std::stringstream output;
        double averageNumValues = static_cast<double>( getRuntime().mSetup->getGalerkin( i ).getNumValues() )
                                  / getRuntime().mSetup->getGalerkin( i ).getNumRows();
        output << std::setw( 3 ) << i << std::setw( 9 ) << getRuntime().mSetup->getGalerkin( i ).getNumRows()
               << std::setw( 9 ) << getRuntime().mSetup->getGalerkin( i ).getNumColumns() << std::setw( 10 )
               << getRuntime().mSetup->getGalerkin( i ).getNumValues() << "  " << std::setw( 6 ) << std::fixed
               << std::setprecision( 1 ) << averageNumValues << "\n";
        mLogger->logMessage( LogLevel::advancedInformation, output.str() );
    }

    mLogger->logNewLine( LogLevel::advancedInformation );

    for( unsigned int i = 0; i < getRuntime().mSetup->getNumLevels() - 1; ++i )
    {
        if( i == 0 )
        {
            std::stringstream output1;
            output1 << "Interpolation Matrix Hierarchy:\n";
            mLogger->logMessage( LogLevel::advancedInformation, output1.str() );
            std::stringstream output2;
            output2 << "Lvl    #Rows    #Cols  #Entries Average\n";
            mLogger->logMessage( LogLevel::advancedInformation, output2.str() );
        }

        std::stringstream output;
        double averageNumValues = static_cast<double>( getRuntime().mSetup->getInterpolation( i ).getNumValues() )
                                  / getRuntime().mSetup->getInterpolation( i ).getNumRows();
        output << std::setw( 3 ) << i << std::setw( 9 ) << getRuntime().mSetup->getInterpolation( i ).getNumRows()
               << std::setw( 9 ) << getRuntime().mSetup->getInterpolation( i ).getNumColumns()
               << std::setw( 10 ) << getRuntime().mSetup->getInterpolation( i ).getNumValues() << "  "
               << std::setw( 6 ) << std::fixed << std::setprecision( 1 ) << averageNumValues << "\n";
        mLogger->logMessage( LogLevel::advancedInformation, output.str() );
    }

    mLogger->logNewLine( LogLevel::advancedInformation );

}

void SimpleAMG::logSolverInfo()
{
    if( mLogger->getLogLevel() < LogLevel::solverInformation )
    {
        return;
    }

    if( getRuntime().mSetup.get() != 0 )
    {
        mLogger->logNewLine( LogLevel::solverInformation );

        mLogger->logMessage( LogLevel::solverInformation, "Solution Components:\n" );
        mLogger->logMessage( LogLevel::solverInformation, "====================\n" );
        mLogger->logMessage( LogLevel::solverInformation, "Iteration Type : V-Cycle\n" );

        std::stringstream smootherInfo;
        smootherInfo << "Smoothing      : " << getRuntime().mSetup->getSmootherInfo() << "\n";
        mLogger->logMessage( LogLevel::solverInformation, smootherInfo.str() );

        std::stringstream coarseLevelSolverInfo;
        coarseLevelSolverInfo << "Coarse Level   : " << getRuntime().mSetup->getCoarseLevelSolverInfo() << "\n";
        mLogger->logMessage( LogLevel::solverInformation, coarseLevelSolverInfo.str() );

        mLogger->logNewLine( LogLevel::solverInformation );
    }
}

void SimpleAMG::logSetupDetails()
{
    if( mLogger->getLogLevel() < LogLevel::advancedInformation )
    {
        return;
    }

    double sizeVector = 0.0;
    double sizeInterpolation = 0.0;
    double sizeRestriction = 0.0;
    double sizeGalerkin = 0.0;
    double sizeInterpolationCSR = 0.0;
    double sizeRestrictionCSR = 0.0;
    double sizeGalerkinCSR = 0.0;

    for( unsigned int i = 0; i < getRuntime().mSetup->getNumLevels(); ++i )
    {
        // Vector
        if( i == 0 )
        {
            sizeVector += 2 * getRuntime().mSetup->getGalerkin( 0 ).getValueTypeSize() 
                          * getRuntime().mSetup->getGalerkin( 0 ).getNumRows();
        }
        else
        {
            sizeVector += getRuntime().mSetup->getSolutionVector( i ).getMemoryUsage();
            sizeVector += getRuntime().mSetup->getRhsVector( i ).getMemoryUsage();
        }

        if( i != getRuntime().mSetup->getNumLevels() - 1 )
        {
            sizeVector += getRuntime().mSetup->getTmpResVector( i ).getMemoryUsage();

            // Interpolation
            {
                sizeInterpolation += getRuntime().mSetup->getInterpolation( i ).getMemoryUsage();

                IndexType numIndexInterpolationCSR = getRuntime().mSetup->getInterpolation( i ).getNumValues()
                                                     + getRuntime().mSetup->getInterpolation( i ).getNumRows();

                sizeInterpolationCSR += numIndexInterpolationCSR * sizeof(IndexType);

                IndexType numValueInterpolationCSR = getRuntime().mSetup->getInterpolation( i ).getNumValues();

                size_t interpolationSizeType = getRuntime().mSetup->getInterpolation( i ).getValueTypeSize();

                sizeInterpolationCSR += numValueInterpolationCSR * interpolationSizeType;
            }

            // Restriction
            {
                sizeRestriction += getRuntime().mSetup->getRestriction( i ).getMemoryUsage();

                IndexType numIndexRestrictionCSR = getRuntime().mSetup->getRestriction( i ).getNumValues()
                                                   + getRuntime().mSetup->getRestriction( i ).getNumRows();

                sizeRestrictionCSR += numIndexRestrictionCSR * sizeof(IndexType);

                size_t restrictionSizeType = getRuntime().mSetup->getRestriction( i ).getValueTypeSize();

                IndexType numValueRestrictionCSR = getRuntime().mSetup->getRestriction( i ).getNumValues();
                sizeRestrictionCSR += numValueRestrictionCSR * restrictionSizeType;
            }
        }

        // Galerkin
        {
            sizeGalerkin += getRuntime().mSetup->getGalerkin( i ).getMemoryUsage();

            IndexType numIndexGalerkinCSR = getRuntime().mSetup->getGalerkin( i ).getNumValues()
                                            + getRuntime().mSetup->getGalerkin( i ).getNumRows();
            sizeGalerkinCSR += numIndexGalerkinCSR * sizeof(IndexType);

            size_t galerkinSizeType = getRuntime().mSetup->getGalerkin( i ).getValueTypeSize();

            IndexType numValueGalerkinCSR = getRuntime().mSetup->getGalerkin( i ).getNumValues();

            sizeGalerkinCSR += numValueGalerkinCSR * galerkinSizeType;
        }
    }

    int overheadInterpolation = static_cast<int>( 100 * sizeInterpolation / sizeInterpolationCSR ) - 100;
    int overheadRestriction = static_cast<int>( 100 * sizeRestriction / sizeRestrictionCSR ) - 100;
    int overheadGalerkin = static_cast<int>( 100 * sizeGalerkin / sizeGalerkinCSR ) - 100;
    int overhead = static_cast<int>( 100 * ( sizeInterpolation + sizeRestriction + sizeGalerkin )
                                     / ( sizeInterpolationCSR + sizeRestrictionCSR + sizeGalerkinCSR ) ) - 100;

    size_t cgSolverValueTypeSize = getRuntime().mSetup->getGalerkin( getRuntime().mSetup->getNumLevels() - 1 ).getValueTypeSize();

    double sizeCGSolver = static_cast<double>( cgSolverValueTypeSize
                          * getRuntime().mSetup->getGalerkin( getRuntime().mSetup->getNumLevels() - 1 ).getNumRows()
                          * getRuntime().mSetup->getGalerkin( getRuntime().mSetup->getNumLevels() - 1 ).getNumRows() );

    double sizeTotal = static_cast<double>( sizeVector + sizeInterpolation + sizeRestriction + sizeGalerkin
                                            + sizeCGSolver );

    double sizeTotalCSR = static_cast<double>( sizeVector + sizeInterpolationCSR + sizeRestrictionCSR + sizeGalerkinCSR
                          + sizeCGSolver );

    std::stringstream outputVector;
    outputVector << "Vector         " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                 << sizeVector / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                 << sizeVector / ( 1024 * 1024 ) << " MB" << std::endl;
    std::stringstream outputInterpolation;
    outputInterpolation << "Interpolation  " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                        << sizeInterpolationCSR / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed
                        << std::setprecision( 1 ) << sizeInterpolation / ( 1024 * 1024 ) << " MB" << std::setw( 4 )
                        << overheadInterpolation << "% " << std::endl;
    std::stringstream outputRestriction;
    outputRestriction << "Restriction    " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                      << sizeRestrictionCSR / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed
                      << std::setprecision( 1 ) << sizeRestriction / ( 1024 * 1024 ) << " MB" << std::setw( 4 )
                      << overheadRestriction << "% " << std::endl;
    std::stringstream outputGalerkin;
    outputGalerkin << "Galerkin       " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                   << sizeGalerkinCSR / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed
                   << std::setprecision( 1 ) << sizeGalerkin / ( 1024 * 1024 ) << " MB" << std::setw( 4 )
                   << overheadGalerkin << "% " << std::endl;
    std::stringstream outputCGSolver;
    outputCGSolver << "Coarse Inverse " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                   << sizeCGSolver / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                   << sizeCGSolver / ( 1024 * 1024 ) << " MB" << std::endl;
    std::stringstream outputTotal;
    outputTotal << "Total          " << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                << sizeTotalCSR / ( 1024 * 1024 ) << " MB" << std::setw( 8 ) << std::fixed << std::setprecision( 1 )
                << sizeTotal / ( 1024 * 1024 ) << " MB" << std::setw( 4 ) << overhead << "% " << std::endl;

    mLogger->logNewLine( LogLevel::advancedInformation );
    mLogger->logMessage( LogLevel::advancedInformation, "Memory needed for AMG hierarchy:\n" );
    mLogger->logMessage( LogLevel::advancedInformation, "==========================================\n" );
    mLogger->logMessage( LogLevel::advancedInformation, "                    CSR(ref.)  Actual  pad\n" );
    mLogger->logMessage( LogLevel::advancedInformation, outputVector.str() );
    mLogger->logMessage( LogLevel::advancedInformation, outputInterpolation.str() );
    mLogger->logMessage( LogLevel::advancedInformation, outputRestriction.str() );
    mLogger->logMessage( LogLevel::advancedInformation, outputGalerkin.str() );
    mLogger->logMessage( LogLevel::advancedInformation, outputCGSolver.str() );
    mLogger->logMessage( LogLevel::advancedInformation, "------------------------------------------\n" );
    mLogger->logMessage( LogLevel::advancedInformation, outputTotal.str() );
    mLogger->logNewLine( LogLevel::advancedInformation );
}

SimpleAMG::SimpleAMGRuntime& SimpleAMG::getRuntime()
{
    return mSimpleAMGRuntime;
}

const SimpleAMG::SimpleAMGRuntime& SimpleAMG::getConstRuntime() const
{
    return mSimpleAMGRuntime;
}

SolverPtr SimpleAMG::copy()
{
    return SolverPtr( new SimpleAMG( *this ) );
}

} /* end namespace lama */

} /* end namespace scai */