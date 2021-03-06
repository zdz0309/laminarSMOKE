/*-----------------------------------------------------------------------*\
|                                                                         |
|                    ╔═══╦═╗╔═╦═══╦╗╔═╦═══╗                               |
|                    ║╔═╗║║╚╝║║╔═╗║║║╔╣╔══╝                               | 
|   ╔╗╔══╦╗╔╦╦═╗╔══╦═╣╚══╣╔╗╔╗║║ ║║╚╝╝║╚══╗                               |
|   ║║║╔╗║╚╝╠╣╔╗╣╔╗║╔╩══╗║║║║║║║ ║║╔╗║║╔══╝                               |
|   ║╚╣╔╗║║║║║║║║╔╗║║║╚═╝║║║║║║╚═╝║║║╚╣╚══╗                               |
|   ╚═╩╝╚╩╩╩╩╩╝╚╩╝╚╩╝╚═══╩╝╚╝╚╩═══╩╝╚═╩═══╝                               |
|                                                                         |
|                                                                         |
|   Authors: A. Cuoci                                                     |
|                                                                         |
|   Contacts: Alberto Cuoci                                               |
|   email: alberto.cuoci@polimi.it                                        |
|   Department of Chemistry, Materials and Chemical Engineering           |
|   Politecnico di Milano                                                 |
|   P.zza Leonardo da Vinci 32, 20133 Milano (Italy)                      |
|                                                                         |
|-------------------------------------------------------------------------|
|                                                                         |
|   This file is part of laminarSMOKE solver.                             |
|                                                                         |
|   License                                                               |
|                                                                         |
|   Copyright(C) 2016, 2015, 2014 A. Cuoci                                |
|   laminarSMOKE is free software: you can redistribute it and/or modify  |
|   it under the terms of the GNU General Public License as published by  |
|   the Free Software Foundation, either version 3 of the License, or     |
|   (at your option) any later version.                                   |
|                                                                         |
|   laminarSMOKE is distributed in the hope that it will be useful,       |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of        |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
|   GNU General Public License for more details.                          |
|                                                                         |
|   You should have received a copy of the GNU General Public License     |
|   along with laminarSMOKE. If not, see <http://www.gnu.org/licenses/>.  |
|                                                                         |
\*-----------------------------------------------------------------------*/

{
	//- Initial conditions
	scalarField& TCells = T.internalField();
	scalarField& QCells = Q.internalField();
	const scalarField& rhoCells = rho.internalField();
	const scalarField& vCells = mesh.V();

	if (homogeneousReactions == true && odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_OPENSMOKE)
	{
		const unsigned int NC  = thermodynamicsMapXML->NumberOfSpecies();
		const unsigned int NEQ = thermodynamicsMapXML->NumberOfSpecies()+1;
		
		// Min and max values
		Eigen::VectorXd yMin(NEQ); for(unsigned int i=0;i<NEQ;i++) yMin(i) = 0.;  yMin(NC) = 200.;
		Eigen::VectorXd yMax(NEQ); for(unsigned int i=0;i<NEQ;i++) yMax(i) = 1.;  yMax(NC) = 6000.;
		Eigen::VectorXd y0(NEQ);
		Eigen::VectorXd yf(NEQ);

		Info <<" * Solving homogeneous chemistry (OpenSMOKE solver)... "<<endl;
		{			
			unsigned int counter = 0;
			
			double tStart = OpenSMOKE::OpenSMOKEGetCpuTime();
			forAll(TCells, celli)
			{
				//- Solving for celli:	
				{
					for(unsigned int i=0;i<NC;i++)
						y0(i) = Y[i].internalField()[celli];
					y0(NC) = TCells[celli];
					
					if (constPressureBatchReactor == true)
					{
						// Set reactor
						batchReactorHomogeneousConstantPressure.SetReactor(thermodynamicPressure);
						batchReactorHomogeneousConstantPressure.SetEnergyEquation(energyEquation);
						
						// Set initial conditions
						odeSolverConstantPressure().SetInitialConditions(t0, y0);

						// Additional ODE solver options
						if (celli == 0)
						{
							// Set linear algebra options
							odeSolverConstantPressure().SetLinearAlgebraSolver(odeParameterBatchReactorHomogeneous.linear_algebra());
							odeSolverConstantPressure().SetFullPivoting(odeParameterBatchReactorHomogeneous.full_pivoting());

							// Set relative and absolute tolerances
							odeSolverConstantPressure().SetAbsoluteTolerances(odeParameterBatchReactorHomogeneous.absolute_tolerance());
							odeSolverConstantPressure().SetRelativeTolerances(odeParameterBatchReactorHomogeneous.relative_tolerance());

							// Set minimum and maximum values
							odeSolverConstantPressure().SetMinimumValues(yMin);
							odeSolverConstantPressure().SetMaximumValues(yMax);
						}
						
						// Solve
						OdeSMOKE::OdeStatus status = odeSolverConstantPressure().Solve(tf);
						odeSolverConstantPressure().Solution(yf);
					}
					else
					{
						// Set reactor pressure
						batchReactorHomogeneousConstantVolume.SetReactor(vCells[celli], thermodynamicPressure, rhoCells[celli]);
						batchReactorHomogeneousConstantVolume.SetEnergyEquation(energyEquation);
						
						// Set initial conditions
						odeSolverConstantVolume().SetInitialConditions(t0, y0);

						// Additional ODE solver options
						if (celli == 0)
						{
							// Set linear algebra options
							odeSolverConstantVolume().SetLinearAlgebraSolver(odeParameterBatchReactorHomogeneous.linear_algebra());
							odeSolverConstantVolume().SetFullPivoting(odeParameterBatchReactorHomogeneous.full_pivoting());

							// Set relative and absolute tolerances
							odeSolverConstantVolume().SetAbsoluteTolerances(odeParameterBatchReactorHomogeneous.absolute_tolerance());
							odeSolverConstantVolume().SetRelativeTolerances(odeParameterBatchReactorHomogeneous.relative_tolerance());

							// Set minimum and maximum values
							odeSolverConstantVolume().SetMinimumValues(yMin);
							odeSolverConstantVolume().SetMaximumValues(yMax);
						}
						
						// Solve
						OdeSMOKE::OdeStatus status = odeSolverConstantVolume().Solve(tf);
						odeSolverConstantVolume().Solution(yf);
					}
				}
			
				// Check mass fractions
				normalizeMassFractions(yf, celli, massFractionsTol);

				// Assign mass fractions
				for(int i=0;i<NC;i++)
					Y[i].internalField()[celli] = yf(i);

				//- Allocating final values: temperature
				if (energyEquation == true)
					TCells[celli] = yf(NC);

				if (counter%(int(0.20*mesh.nCells())+1) == 0)
					Info <<"   Accomplished: " << counter << "/" << mesh.nCells() << endl;

				counter++;

				// Output
				if (runTime.outputTime())
				{
					if (constPressureBatchReactor == true)
					{
						QCells[celli] = batchReactorHomogeneousConstantPressure.QR();
						if (outputFormationRatesIndices.size() != 0)
						{
							for (int i=0;i<outputFormationRatesIndices.size();i++)
								FormationRates[i].internalField()[celli] = batchReactorHomogeneousConstantPressure.R()[outputFormationRatesIndices[i]+1] *
                                       	      		                                                   thermodynamicsMapXML->MW()[outputFormationRatesIndices[i]+1];
						}
					}
					else
					{
						QCells[celli] = batchReactorHomogeneousConstantVolume.QR();
						if (outputFormationRatesIndices.size() != 0)
						{
							for (int i=0;i<outputFormationRatesIndices.size();i++)
								FormationRates[i].internalField()[celli] = batchReactorHomogeneousConstantVolume.R()[outputFormationRatesIndices[i]+1] *
                                           	                                                           thermodynamicsMapXML->MW()[outputFormationRatesIndices[i]+1];
						}
					}
				}
			}
			double tEnd = OpenSMOKE::OpenSMOKEGetCpuTime();
			
			Info << "   Homogeneous chemistry solved in " << tEnd - tStart << " s  (" << (tEnd-tStart)/double(mesh.nCells())*1000. << " ms per reactor)" << endl;
		}
	}
	else if (homogeneousReactions == true && odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_CHEMEQ2)
	{
		const unsigned int NC  = thermodynamicsMapXML->NumberOfSpecies();
		const unsigned int NEQ = thermodynamicsMapXML->NumberOfSpecies();
		Eigen::VectorXd y0(NEQ);
		Eigen::VectorXd yf(NEQ);

		OpenSMOKE::OpenSMOKE_CHEMEQ2<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_CHEMEQ2>   odeSolverConstantPressure(chemeq2SolverConstantPressure);
		odeSolverConstantPressure.SetDimensions(NEQ);
		OpenSMOKE::OpenSMOKE_CHEMEQ2<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_CHEMEQ2>   odeSolverConstantVolume(chemeq2SolverConstantVolume);
		odeSolverConstantVolume.SetDimensions(NEQ);

		Info << " * Solving homogeneous chemistry (CHEMEQ2 solver)... " << endl;
		{			
			unsigned int counter = 0;
			
			double tStart = OpenSMOKE::OpenSMOKEGetCpuTime();
			forAll(TCells, celli)
			{
				//- Solving for celli:	
				{
					for(unsigned int i=0;i<NC;i++)
						y0(i) = Y[i].internalField()[celli];
					
					if (constPressureBatchReactor == true)
					{
						// Set reactor
						batchReactorHomogeneousConstantPressure.SetReactor(thermodynamicPressure);
						batchReactorHomogeneousConstantPressure.SetTemperature(TCells[celli], y0.data());
						batchReactorHomogeneousConstantPressure.SetEnergyEquation(energyEquation);
						
						// Set initial conditions
						odeSolverConstantPressure.SetInitialValues(t0, y0.data());

						// Additional ODE solver options
						if (celli == 0)
						{
							odeSolverConstantPressure.SetEpsilonMinimum(chemeq2_epsilonMinimum);
							odeSolverConstantPressure.SetEpsilonMaximum(chemeq2_epsilonMaximum );
							odeSolverConstantPressure.SetMaximumNumberSubIterations( chemeq2_subIterations);
							odeSolverConstantPressure.SetMinimumStep(chemeq2_dtMinimum);
							odeSolverConstantPressure.SetMinimumValue(chemeq2_yMinimum);
						}
						
						// Solve
						odeSolverConstantPressure.Solve(tf);

						// Recover solution
						odeSolverConstantPressure.Solution(yf.data());
						if (energyEquation == true)
							TCells[celli] = batchReactorHomogeneousConstantPressure.GetTemperature();
					}
					else
					{
						// Set reactor pressure
						batchReactorHomogeneousConstantVolume.SetReactor(vCells[celli], thermodynamicPressure, rhoCells[celli]);
						batchReactorHomogeneousConstantVolume.SetTemperature(TCells[celli], y0.data());
						batchReactorHomogeneousConstantVolume.SetEnergyEquation(energyEquation);

						// Set initial conditions
						odeSolverConstantVolume.SetInitialValues(t0, y0.data());

						// Additional ODE solver options
						if (celli == 0)
						{
							odeSolverConstantVolume.SetEpsilonMinimum(chemeq2_epsilonMinimum);
							odeSolverConstantVolume.SetEpsilonMaximum(chemeq2_epsilonMaximum );
							odeSolverConstantVolume.SetMaximumNumberSubIterations( chemeq2_subIterations);
							odeSolverConstantVolume.SetMinimumStep(chemeq2_dtMinimum);
							odeSolverConstantVolume.SetMinimumValue(chemeq2_yMinimum);
						}
						
						// Solve
						odeSolverConstantVolume.Solve(tf);

						// Recover solution
						odeSolverConstantVolume.Solution(yf.data());
						if (energyEquation == true)
							TCells[celli] = batchReactorHomogeneousConstantVolume.GetTemperature();
					}
				}
			
				// Check mass fractions
				normalizeMassFractions(yf.data(), celli, massFractionsTol, NC);

				// Assign mass fractions
				for(int i=0;i<NC;i++)
					Y[i].internalField()[celli] = yf(i);

				if (counter%(int(0.20*mesh.nCells())+1) == 0)
					Info <<"   Accomplished: " << counter << "/" << mesh.nCells() << endl;

				counter++;
			}
			double tEnd = OpenSMOKE::OpenSMOKEGetCpuTime();
			
			Info << "   Homogeneous chemistry solved in " << tEnd - tStart << " s  (" << (tEnd-tStart)/double(mesh.nCells())*1000. << " ms per reactor)" << endl;
		}
	}
	else if (homogeneousReactions == true && odeParameterBatchReactorHomogeneous.type() != OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_OPENSMOKE)
	{
		const unsigned int NC  = thermodynamicsMapXML->NumberOfSpecies();
		const unsigned int NEQ = thermodynamicsMapXML->NumberOfSpecies()+1;
		
		// Min and max values
		OpenSMOKE::OpenSMOKEVectorDouble y0(NEQ);
		OpenSMOKE::OpenSMOKEVectorDouble yf(NEQ);

		Info <<" * Solving homogeneous chemistry (external ODE solver)... "<<endl;
		{			
			unsigned int counter = 0;
			
			double tStart = OpenSMOKE::OpenSMOKEGetCpuTime();
			forAll(TCells, celli)
			{
				//- Solving for celli:	
				{
					for(unsigned int i=0;i<NC;i++)
						y0[i+1] = Y[i].internalField()[celli];
					y0[NC+1] = TCells[celli];
					
					if (constPressureBatchReactor == true)
					{
						// Set reactor pressure
						batchReactorHomogeneousConstantPressure.SetReactor(thermodynamicPressure);
						batchReactorHomogeneousConstantPressure.SetCheckMassFractions(true);
						batchReactorHomogeneousConstantPressure.SetEnergyEquation(energyEquation);
						
						if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DVODE)
						{
							#if OPENSMOKE_USE_DVODE == 1
							typedef OpenSMOKE::OpenSMOKE_DVODE<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_DVODE> Solver;
							SolveOpenSourceSolvers<Solver>(dvodeSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);
							#endif
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DLSODE)
						{
							#if OPENSMOKE_USE_ODEPACK == 1
							typedef OpenSMOKE::OpenSMOKE_DLSODE<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_DLSODE> Solver;
							SolveOpenSourceSolvers<Solver>(dlsodeSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);					
							#endif
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DLSODA)
						{
							#if OPENSMOKE_USE_ODEPACK == 1
							typedef OpenSMOKE::OpenSMOKE_DLSODA<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_DLSODA> Solver;
							SolveOpenSourceSolvers<Solver>(dlsodaSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DASPK)
						{
							#if OPENSMOKE_USE_DASPK == 1
							typedef OpenSMOKE::OpenSMOKE_DASPK<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_DASPK> Solver;
							SolveOpenSourceSolvers<Solver>(daspkSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);								
							#endif							
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_RADAU5)
						{
							#if OPENSMOKE_USE_RADAU == 1
							typedef OpenSMOKE::OpenSMOKE_RADAU<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_RADAU5> Solver;
							SolveOpenSourceSolvers<Solver>(radauSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);						
							#endif							
						}			
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_MEBDF)
						{
							#if OPENSMOKE_USE_MEBDF == 1
							typedef OpenSMOKE::OpenSMOKE_MEBDF<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_MEBDF> Solver;
							SolveOpenSourceSolvers<Solver>(mebdfSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}	
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_CVODE)
						{
							#if OPENSMOKE_USE_SUNDIALS == 1
							typedef OpenSMOKE::OpenSMOKE_CVODE_Sundials<OpenSMOKE::BatchReactorHomogeneousConstantPressure_ODE_CVODE> Solver;
							SolveOpenSourceSolvers<Solver>(cvodeSolverConstantPressure, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}																							
					}
					else
					{
						// Set reactor data
						batchReactorHomogeneousConstantVolume.SetReactor(vCells[celli], thermodynamicPressure, rhoCells[celli]);
						batchReactorHomogeneousConstantVolume.SetCheckMassFractions(true);
						batchReactorHomogeneousConstantVolume.SetEnergyEquation(energyEquation);
						
						if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DVODE)
						{
							#if OPENSMOKE_USE_DVODE == 1
							typedef OpenSMOKE::OpenSMOKE_DVODE<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_DVODE> Solver;
							SolveOpenSourceSolvers<Solver>(dvodeSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);
							#endif
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DLSODE)
						{
							#if OPENSMOKE_USE_ODEPACK == 1
							typedef OpenSMOKE::OpenSMOKE_DLSODE<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_DLSODE> Solver;
							SolveOpenSourceSolvers<Solver>(dlsodeSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);					
							#endif
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DLSODA)
						{
							#if OPENSMOKE_USE_ODEPACK == 1
							typedef OpenSMOKE::OpenSMOKE_DLSODA<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_DLSODA> Solver;
							SolveOpenSourceSolvers<Solver>(dlsodaSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_DASPK)
						{
							#if OPENSMOKE_USE_DASPK == 1
							typedef OpenSMOKE::OpenSMOKE_DASPK<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_DASPK> Solver;
							SolveOpenSourceSolvers<Solver>(daspkSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);								
							#endif							
						}
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_RADAU5)
						{
							#if OPENSMOKE_USE_RADAU == 1
							typedef OpenSMOKE::OpenSMOKE_RADAU<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_RADAU5> Solver;
							SolveOpenSourceSolvers<Solver>(radauSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);						
							#endif							
						}			
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_MEBDF)
						{
							#if OPENSMOKE_USE_MEBDF == 1
							typedef OpenSMOKE::OpenSMOKE_MEBDF<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_MEBDF> Solver;
							SolveOpenSourceSolvers<Solver>(mebdfSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}	
						else if (odeParameterBatchReactorHomogeneous.type() == OpenSMOKE::ODE_Parameters::ODE_INTEGRATOR_CVODE)
						{
							#if OPENSMOKE_USE_SUNDIALS == 1
							typedef OpenSMOKE::OpenSMOKE_CVODE_Sundials<OpenSMOKE::BatchReactorHomogeneousConstantVolume_ODE_CVODE> Solver;
							SolveOpenSourceSolvers<Solver>(cvodeSolverConstantVolume, t0, tf, y0, yf, odeParameterBatchReactorHomogeneous);							
							#endif							
						}		
					}
				}
				
				// Check mass fractions
				normalizeMassFractions(yf, celli, massFractionsTol);

				// Assign mass fractions
				for(int i=0;i<NC;i++)
					Y[i].internalField()[celli] = yf[i+1];

				//- Allocating final values: temperature
				if (energyEquation == true)
					TCells[celli] = yf[NC+1];

				if (counter%(int(0.20*mesh.nCells())+1) == 0)
					Info <<"   Accomplished: " << counter << "/" << mesh.nCells() << endl;

				counter++;

				// Output
				if (runTime.outputTime())
				{
					if (constPressureBatchReactor == true)
					{
						QCells[celli] = batchReactorHomogeneousConstantPressure.QR();
						if (outputFormationRatesIndices.size() != 0)
						{
							for (int i=0;i<outputFormationRatesIndices.size();i++)
								FormationRates[i].internalField()[celli] = batchReactorHomogeneousConstantPressure.R()[outputFormationRatesIndices[i]+1] *
                                       	      		                                                   thermodynamicsMapXML->MW()[outputFormationRatesIndices[i]+1];
						}
					}
					else
					{
						QCells[celli] = batchReactorHomogeneousConstantVolume.QR();
						if (outputFormationRatesIndices.size() != 0)
						{
							for (int i=0;i<outputFormationRatesIndices.size();i++)
								FormationRates[i].internalField()[celli] = batchReactorHomogeneousConstantVolume.R()[outputFormationRatesIndices[i]+1] *
                                           	                                                           thermodynamicsMapXML->MW()[outputFormationRatesIndices[i]+1];
						}
					}
				}
			}
			double tEnd = OpenSMOKE::OpenSMOKEGetCpuTime();
			
			Info << "   Homogeneous chemistry solved in " << tEnd - tStart << " s  (" << (tEnd-tStart)/double(mesh.nCells())*1000. << " ms per reactor)" << endl;
		}
	}
    
    Info<< " * T gas min/max (after chemistry) = " << min(T).value() << ", " << max(T).value() << endl;
}
