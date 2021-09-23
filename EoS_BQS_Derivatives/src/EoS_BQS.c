/* 
	Copyright (c) 2019, Paolo Parotto and Jamie Stafford, 
	Department of Physics, University of Houston, Houston, TX 77204, US.
*/

/*
	This file produces a Taylor expanded EoS using Lattice QCD coefficients at muB=0.
*/ 

#define NRANSI

/* Import standard libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "../include/nrD.h"
#include "../include/nrutilD.h"
#include "../include/export_to_hdf.h"
#include "../include/nm_solver.h"

/* Import additional library for many variables used in the program. */
#include "../include/Variables.h"
#include "../include/Functions.h"

/* Strangeness neutrality. */
#define NSN 2

/* Time variables for timing. */
clock_t start, end;
double cpu_time_used;

/* Functions whose zeroes we are seeking for strangeness neutrality. */
void funcvSN(int n,double x[],double f[]){
	f[1] = StrDensTaylor(Tval,muBval,x[1],x[2]);
	f[2] = ChDensTaylor(Tval,muBval,x[1],x[2]) - 0.4*BarDensTaylor(Tval,muBval,x[1],x[2]);
} 

/*double mapf(double x, double a, double b)
{
	return log(fabs((x-a)/(b-x)));
}

double imapf(double t, double a, double b)
{
	return a + (b-a)/(1.0+exp(-t));
}

void get_seed_points(double a, double b, int n, double result[])
{
	for (int ii = 1; ii < n-1; ii+=1) result[ii] = mapf(a + (b-a)*ii/(n-1.0), a, b);
	result[0] = mapf(a+1e-10, a, b);
	result[n-1] = mapf(b-1e-10, a, b);
}*/

/* The main body of the program. */
int main(int argc, char *argv[])
{
	char buff[FILENAME_MAX];

	/* Define time variables to measure time elapsed creating the tables.*/
	time_t start, stop;

	/* All vectors and matrices are initialized. */
	/* Vectors for coefficients. */
	// Order 0
	CHI000PAR=vector(1,21);
	// Order 2
	// Diagonal 
	CHI200PAR=vector(1,21); CHI020PAR=vector(1,21); CHI002PAR=vector(1,21); 
	// Mixed
	CHI110PAR=vector(1,21);	CHI101PAR=vector(1,21);	CHI011PAR=vector(1,21);	
	// Order 4
	// Diagonal
	CHI400PAR=vector(1,21);	CHI040PAR=vector(1,21);	CHI004PAR=vector(1,21);
	// Mixed 31
	CHI310PAR=vector(1,21);	CHI301PAR=vector(1,21);	CHI031PAR=vector(1,21);	
	// Mixed 13
	CHI130PAR=vector(1,21);	CHI103PAR=vector(1,21); CHI013PAR=vector(1,21);	
   // Mixed 22
   CHI220PAR=vector(1,21); CHI202PAR=vector(1,21); CHI022PAR=vector(1,21);	
   // Mied 112
   CHI211PAR=vector(1,21);	CHI121PAR=vector(1,21); CHI112PAR=vector(1,21);
	/* Vectors for coefficients at low T. */
	// Order 0
	CHI000PAR_ABC=vector(1,3);
	// Order 2
	// Diagonal 
	CHI200PAR_ABC=vector(1,3); CHI020PAR_ABC=vector(1,3); CHI002PAR_ABC=vector(1,3); 
	// Mixed
	CHI110PAR_ABC=vector(1,3);	CHI101PAR_ABC=vector(1,3);	CHI011PAR_ABC=vector(1,3);	
	// Order 4
	// Diagonal
	CHI400PAR_ABC=vector(1,3);	CHI040PAR_ABC=vector(1,3);	CHI004PAR_ABC=vector(1,3);
	// Mixed 31
	CHI310PAR_ABC=vector(1,3);	CHI301PAR_ABC=vector(1,3);	CHI031PAR_ABC=vector(1,3);	
	// Mixed 13
	CHI130PAR_ABC=vector(1,3);	CHI103PAR_ABC=vector(1,3); CHI013PAR_ABC=vector(1,3);	
   // Mixed 22
   CHI220PAR_ABC=vector(1,3); CHI202PAR_ABC=vector(1,3); CHI022PAR_ABC=vector(1,3);	
   // Mied 112
   CHI211PAR_ABC=vector(1,3);	CHI121PAR_ABC=vector(1,3); CHI112PAR_ABC=vector(1,3);


	/* Matrix for coefficients: 22 coefficients, 21 parameters each. */
	parMatrix=matrix(1,22,1,21);
	
	/* For strangeness neutrality */
	xSN=vector(1,NSN);
 	fSN=vector(1,NSN);
	
	/* Assign the name of the main folder where the program lives and the files we wish to import are located.*/
	getcwd(buff,FILENAME_MAX);
	printf("Current working directory is: \n\n%s \n\n",buff);

   /* Start clock for importing lists */
  	start = clock();

	/* Parametrization parameters are read from the user-input file, and saved. */
//	printf("argc = %d\n", argc);
//	printf("argv[1] = %s\n", argv[1]);
  	FILE *ParametersIn = fopen(argv[1], "r");
  	if (ParametersIn == 0){
  		fprintf(stderr,"failed to open paremeters file\n");
  		exit(1);
  	}
  	for(i=1;fscanf(ParametersIn,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", &A0, &A1, &A2, &A3, &A4, &A5, &A6, &A7, 
  	                                                                                &A8, &A9, &B0, &B1, &B2, &B3, &B4, &B5, &B6, &B7, &B8, &B9, &C0) !=EOF; i++){
       parMatrix[i][1] = A0;  	    parMatrix[i][2] = A1;  	    parMatrix[i][3] = A2;  	    parMatrix[i][4] = A3;
  	    parMatrix[i][5] = A4;  	    parMatrix[i][6] = A5;  	    parMatrix[i][7] = A6;  	    parMatrix[i][8] = A7;
  	    parMatrix[i][9] = A8;  	    parMatrix[i][10] = A9;
  	    parMatrix[i][11] = B0;  	    parMatrix[i][12] = B1;  	    parMatrix[i][13] = B2;  	    parMatrix[i][14] = B3;
  	    parMatrix[i][15] = B4;  	    parMatrix[i][16] = B5;  	    parMatrix[i][17] = B6;  	    parMatrix[i][18] = B7;
  	    parMatrix[i][19] = B8;  	    parMatrix[i][20] = B9;
  	    parMatrix[i][21] = C0;
  	    
  	    //printf("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",A0,A1,A2,A3,A4,A5,A6,A7,B0,B1,B2,B3,B4,B5,B6,B7,C0);
  	}
  	fclose(ParametersIn);	
  	
 
	// Vectors for coefficients.   
	// All vectors are filled with parameters. 
	// Order 0
  	for(i=1;i<=21;i++) CHI000PAR[i] = parMatrix[1][i];
  	// Order 2
	// Diagonal 
  	for(i=1;i<=21;i++) CHI200PAR[i] = parMatrix[2][i];
  	for(i=1;i<=21;i++) CHI020PAR[i] = parMatrix[3][i];
	for(i=1;i<=21;i++) CHI002PAR[i] = parMatrix[4][i];
	// Mixed
	for(i=1;i<=21;i++) CHI110PAR[i] = parMatrix[5][i];
  	for(i=1;i<=21;i++) CHI101PAR[i] = parMatrix[6][i];
  	for(i=1;i<=21;i++) CHI011PAR[i] = parMatrix[7][i];
	// Order 4
   	// Diagonal	
   	for(i=1;i<=21;i++) CHI400PAR[i] = parMatrix[8][i];
  	for(i=1;i<=21;i++) CHI040PAR[i] = parMatrix[9][i];
  	for(i=1;i<=21;i++) CHI004PAR[i] = parMatrix[10][i];
  	// Mixed 31
  	for(i=1;i<=21;i++) CHI310PAR[i] = parMatrix[11][i];
  	for(i=1;i<=21;i++) CHI301PAR[i] = parMatrix[12][i];
  	for(i=1;i<=21;i++) CHI031PAR[i] = parMatrix[13][i];
  	// Mixed 13
  	for(i=1;i<=21;i++) CHI130PAR[i] = parMatrix[14][i];
  	for(i=1;i<=21;i++) CHI103PAR[i] = parMatrix[15][i];
  	for(i=1;i<=21;i++) CHI013PAR[i] = parMatrix[16][i];
  	// Mixed 22
  	for(i=1;i<=21;i++) CHI220PAR[i] = parMatrix[17][i];
  	for(i=1;i<=21;i++) CHI202PAR[i] = parMatrix[18][i];
  	for(i=1;i<=21;i++) CHI022PAR[i] = parMatrix[19][i];
  	// Mixed 211
  	for(i=1;i<=21;i++) CHI211PAR[i] = parMatrix[20][i];
  	for(i=1;i<=21;i++) CHI121PAR[i] = parMatrix[21][i];
  	for(i=1;i<=21;i++) CHI112PAR[i] = parMatrix[22][i];
  	
	T_min_matching = 70.0;
  	set_lowT_parameters(CHI000PAR, CHI000PAR_ABC);
  	set_lowT_Mod_parameters(CHI200PAR, CHI200PAR_ABC);
  	set_lowT_parameters(CHI020PAR, CHI020PAR_ABC);
  	set_lowT_parameters(CHI002PAR, CHI002PAR_ABC);
  	set_lowT_parameters(CHI110PAR, CHI110PAR_ABC);
  	set_lowT_parameters(CHI101PAR, CHI101PAR_ABC);
  	set_lowT_parameters(CHI011PAR, CHI011PAR_ABC);
  	set_lowT_parameters(CHI400PAR, CHI400PAR_ABC);
  	set_lowT_parameters(CHI040PAR, CHI040PAR_ABC);
  	set_lowT_parameters(CHI004PAR, CHI004PAR_ABC);
  	set_lowT_parameters(CHI310PAR, CHI310PAR_ABC);
  	set_lowT_parameters(CHI301PAR, CHI301PAR_ABC);
  	set_lowT_parameters(CHI031PAR, CHI031PAR_ABC);
  	set_lowT_parameters(CHI130PAR, CHI130PAR_ABC);
  	set_lowT_parameters(CHI103PAR, CHI103PAR_ABC);
  	set_lowT_parameters(CHI013PAR, CHI013PAR_ABC);
  	set_lowT_parameters(CHI220PAR, CHI220PAR_ABC);
  	set_lowT_parameters(CHI202PAR, CHI202PAR_ABC);
  	set_lowT_parameters(CHI022PAR, CHI022PAR_ABC);
  	set_lowT_parameters(CHI211PAR, CHI211PAR_ABC);
  	set_lowT_parameters(CHI121PAR, CHI121PAR_ABC);
  	set_lowT_parameters(CHI112PAR, CHI112PAR_ABC);

//if (1) exit(-1);

  	/* Create folder for coefficients checks. */
	mkdir("Coefficients_Check", S_IRWXU | S_IRWXG | S_IRWXO);
	chdir("Coefficients_Check");
  	

  	// Print values of all coefficients and derivatives thereof wrt T. (To check that everything is in order.)
  	FILE *CHIS = fopen("All_Chis.dat","w");
  	FILE *DCHISDT = fopen("All_DChisDT.dat","w");
  	FILE *D2CHISDT2 = fopen("All_D2ChisDT2.dat","w");
  	for(i=30;i<=800;i+=1){
  	    fprintf(CHIS,"%d    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf\n",
  	                i,CHI000(i),CHI200(i),CHI020(i),CHI002(i),CHI110(i),CHI101(i),CHI011(i),CHI400(i),CHI040(i),CHI004(i),CHI310(i),CHI301(i),CHI031(i),CHI130(i),CHI103(i),CHI013(i),
  	                CHI220(i),CHI202(i),CHI022(i),CHI211(i),CHI121(i),CHI112(i));
  	    fprintf(DCHISDT,"%d    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf\n",
  	                i,DCHI000DT(i),DCHI200DT(i),DCHI020DT(i),DCHI002DT(i),DCHI110DT(i),DCHI101DT(i),DCHI011DT(i),DCHI400DT(i),
  	                DCHI040DT(i),DCHI004DT(i),DCHI310DT(i),DCHI301DT(i),DCHI031DT(i),DCHI130DT(i),DCHI103DT(i),DCHI013DT(i),
  	                DCHI220DT(i),DCHI202DT(i),DCHI022DT(i),DCHI211DT(i),DCHI121DT(i),DCHI112DT(i));
  	    fprintf(D2CHISDT2,"%d    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf    %lf\n",
 	                i,D2CHI000DT2(i),D2CHI200DT2(i),D2CHI020DT2(i),D2CHI002DT2(i),D2CHI110DT2(i),D2CHI101DT2(i),D2CHI011DT2(i),D2CHI400DT2(i),
  	                D2CHI040DT2(i),D2CHI004DT2(i),D2CHI310DT2(i),D2CHI301DT2(i),D2CHI031DT2(i),D2CHI130DT2(i),D2CHI103DT2(i),D2CHI013DT2(i),
  	                D2CHI220DT2(i),D2CHI202DT2(i),D2CHI022DT2(i),D2CHI211DT2(i),D2CHI121DT2(i),D2CHI112DT2(i));            
  	}
  	fclose(CHIS);  	fclose(DCHISDT);  fclose(D2CHISDT2);

	chdir(buff);
  	
  	
	/*int run_density_solver = 0;
	if ( run_density_solver )
	for (int irun = 0; irun <= 2; irun+=1)
	{
		double eIn = 973.563, BIn = -0.316059, SIn = 0.323859, QIn = 1.06384;	// (MeV,1,1,1)/fm^3
		double densities[4] = {eIn, BIn, SIn, QIn};
		double sols[4] = {158.0, -437.0, -112.0, 437.0};		// MeV
		double minima[4] = {155.0, -450.0, -125.0, 425.0};		// MeV
		double maxima[4] = {160.0, -425.0, -100.0, 450.0};		// MeV

		// find the solution
		if (irun==1) solve(densities, sols);
		else if (irun==2)
		{
			start = clock();

			const int n_seeds_per_dimension = 5;	// includes endpoints
			const double dT_seed = (maxima[0] - minima[0])/(n_seeds_per_dimension-1);
			const double dmuB_seed = (maxima[1] - minima[1])/(n_seeds_per_dimension-1);
			const double dmuS_seed = (maxima[2] - minima[2])/(n_seeds_per_dimension-1);
			const double dmuQ_seed = (maxima[3] - minima[3])/(n_seeds_per_dimension-1);

			double seedT[4], seedmuB[4], seedmuS[4], seedmuQ[4];

			get_seed_points(minima[0], maxima[0], n_seeds_per_dimension, seedT);
			get_seed_points(minima[1], maxima[1], n_seeds_per_dimension, seedmuB);
			get_seed_points(minima[2], maxima[2], n_seeds_per_dimension, seedmuS);
			get_seed_points(minima[3], maxima[3], n_seeds_per_dimension, seedmuQ);
			
			int attempts = 0;
			double Tseed = 0.0, muBseed = 0.0, muSseed = 0.0, muQseed = 0.0;
			for (int ii = 1; ii < n_seeds_per_dimension-1; ii+=1)
			for (int jj = 1; jj < n_seeds_per_dimension-1; jj+=1)
			for (int kk = 1; kk < n_seeds_per_dimension-1; kk+=1)
			for (int ll = 1; ll < n_seeds_per_dimension-1; ll+=1)
			{
				Tseed = seedT[ii]; muBseed = seedmuB[jj]; muSseed = seedmuS[kk]; muQseed = seedmuQ[ll];
				double seeds[4] = {Tseed, muBseed, muSseed, muQseed};
				solve2(densities, sols, minima, maxima, seeds);
				attempts += 1;
				if (sols[0] > 0.0) // success
					goto success;
			}
			success:
				end = clock();
				cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
				printf("Finished calculating in %lf seconds.\n", cpu_time_used);	

				printf("Found solution in %d attempts!\n", attempts);
				printf("Seed point: %lf %lf %lf %lf\n",
						imapf(Tseed, minima[0], maxima[0]),
						imapf(muBseed, minima[1], maxima[1]),
						imapf(muSseed, minima[2], maxima[2]),
						imapf(muQseed, minima[3], maxima[3]));
		}
		double Tsol = sols[0], muBsol = sols[1], muSsol = sols[2], muQsol = sols[3];
		Tval = Tsol; muBval = muBsol; muSval = muSsol; muQval = muQsol;
		i = Tsol; j = muBsol; l = muSsol; k = muQsol;	// Q and S reversed
		
		
		printf("Input:\n");
		printf("eIn = %15.8f\n", eIn);
		printf("BIn = %15.8f\n", BIn);
		printf("SIn = %15.8f\n", SIn);
		printf("QIn = %15.8f\n", QIn);
		printf("Solution:\n");
		printf("Tsol = %15.8f\n", Tsol);
		printf("muBsol = %15.8f\n", muBsol);
		printf("muSsol = %15.8f\n", muSsol);
		printf("muQsol = %15.8f\n", muQsol);
		fflush(stdout);
		
		double POut = Tsol*Tsol*Tsol*Tsol*PressTaylor(Tsol, muBsol, muQsol, muSsol);
		double sOut = Tsol*Tsol*Tsol*EntrTaylor(Tsol, muBsol, muQsol, muSsol);
		double BOut = Tsol*Tsol*Tsol*BarDensTaylor(Tsol, muBsol, muQsol, muSsol);
		double SOut = Tsol*Tsol*Tsol*StrDensTaylor(Tsol, muBsol, muQsol, muSsol);
		double QOut = Tsol*Tsol*Tsol*ChDensTaylor(Tsol, muBsol, muQsol, muSsol);
		POut /= 197.327*197.327*197.327;
		sOut /= 197.327*197.327*197.327;
		BOut /= 197.327*197.327*197.327;
		SOut /= 197.327*197.327*197.327;
		QOut /= 197.327*197.327*197.327;
		double eOut = sOut*Tsol - POut + muBsol*BOut + muQsol*QOut + muSsol*SOut;
		
		printf("Check:\n");
		printf("POut = %15.8f\n", POut);
		printf("sOut = %15.8f\n", sOut);
		printf("eOut = %15.8f\n", eOut);
		printf("BOut = %15.8f\n", BOut);
		printf("SOut = %15.8f\n", SOut);
		printf("QOut = %15.8f\n", QOut);
		fflush(stdout);
		
		//if (irun==1) exit(-1);
		
		
		//Thermodynamics
		PressVal = PressTaylor(i,j,k,l);
		EntrVal = EntrTaylor(i,j,k,l);
		BarDensVal = BarDensTaylor(i,j,k,l);
		StrDensVal = StrDensTaylor(i,j,k,l);
		ChDensVal = ChDensTaylor(i,j,k,l);
		EnerDensVal = EntrVal - PressVal 
				+ muBval/Tval*BarDensVal 
				+ muQval/Tval*ChDensVal 
				+ muSval/Tval*StrDensVal;
		SpSoundVal = SpSound(Tval,muBval,muQval,muSval);
		            
		//Second Order Derivatives
		D2PB2 = P2B2(i,j,k,l);
		D2PQ2 = P2Q2(i,j,k,l);
		D2PS2 = P2S2(i,j,k,l);
		
		D2PBQ = P2BQ(i,j,k,l);
		D2PBS = P2BS(i,j,k,l);
		D2PQS = P2QS(i,j,k,l);
		
		D2PTB = P2TB(i,j,k,l);
		D2PTQ = P2TQ(i,j,k,l);
		D2PTS = P2TS(i,j,k,l);
		D2PT2 = P2T2(i,j,k,l);
		
		printf("vals: %lf  %lf  %lf  %lf  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", Tval, muBval, muQval, muSval, PressVal, EntrVal, 
		        BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
		printf("derivs: %lf  %lf  %lf  %lf  %3.12f  %3.12f %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f%  3.12f\n", Tval, muBval, muQval, muSval, D2PB2, D2PQ2, D2PS2, D2PBQ, D2PBS, D2PQS,
				D2PTB, D2PTQ, D2PTS, D2PT2);
		
		printf("********************************************************************************\n\n");
		
		fflush(stdout);
		
//		}
	}
	if ( run_density_solver ) exit(-1);
*/

	// DON'T CHANGE DIRECTORIES UNTIL EVERYTHING IS READ AND WRITTEN
  	/* Create folder for thermodynamic quantities. */
	mkdir("Thermodynamics", S_IRWXU | S_IRWXG | S_IRWXO);
	chdir("Thermodynamics");


//	for (double T0 = 0.5; T0 < T_min_matching+20.001; T0+=0.5)
//	{
//		if (T0<T_min_matching)
//			printf("%lf %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f\n", T0,
//				lowT_D2CHI000DT2(T0), lowT_D2CHI200DT2(T0), lowT_D2CHI020DT2(T0), lowT_D2CHI002DT2(T0),
//				lowT_D2CHI110DT2(T0), lowT_D2CHI101DT2(T0), lowT_D2CHI011DT2(T0), lowT_D2CHI400DT2(T0),
//				lowT_D2CHI040DT2(T0), lowT_D2CHI004DT2(T0), lowT_D2CHI310DT2(T0), lowT_D2CHI301DT2(T0),
//				lowT_D2CHI031DT2(T0), lowT_D2CHI130DT2(T0), lowT_D2CHI103DT2(T0), lowT_D2CHI013DT2(T0),
//				lowT_D2CHI220DT2(T0), lowT_D2CHI202DT2(T0), lowT_D2CHI022DT2(T0), lowT_D2CHI211DT2(T0),
//				lowT_D2CHI121DT2(T0), lowT_D2CHI112DT2(T0) );
//				/*lowT_DCHI000DT(T0), lowT_DCHI200DT(T0), lowT_DCHI020DT(T0), lowT_DCHI002DT(T0),
//				lowT_DCHI110DT(T0), lowT_DCHI101DT(T0), lowT_DCHI011DT(T0), lowT_DCHI400DT(T0),
//				lowT_DCHI040DT(T0), lowT_DCHI004DT(T0), lowT_DCHI310DT(T0), lowT_DCHI301DT(T0),
//				lowT_DCHI031DT(T0), lowT_DCHI130DT(T0), lowT_DCHI103DT(T0), lowT_DCHI013DT(T0),
//				lowT_DCHI220DT(T0), lowT_DCHI202DT(T0), lowT_DCHI022DT(T0), lowT_DCHI211DT(T0),
//				lowT_DCHI121DT(T0), lowT_DCHI112DT(T0) );*/
//				/*lowT_CHI000(T0), lowT_CHI200(T0), lowT_CHI020(T0), lowT_CHI002(T0),
//				lowT_CHI110(T0), lowT_CHI101(T0), lowT_CHI011(T0), lowT_CHI400(T0),
//				lowT_CHI040(T0), lowT_CHI004(T0), lowT_CHI310(T0), lowT_CHI301(T0),
//				lowT_CHI031(T0), lowT_CHI130(T0), lowT_CHI103(T0), lowT_CHI013(T0),
//				lowT_CHI220(T0), lowT_CHI202(T0), lowT_CHI022(T0), lowT_CHI211(T0),
//				lowT_CHI121(T0), lowT_CHI112(T0) );*/
//
//		else
//			printf("%lf %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f %15.12f "
//				"%15.12f %15.12f %15.12f %15.12f %15.12f\n", T0,
//				D2CHI000DT2(T0), D2CHI200DT2(T0), D2CHI020DT2(T0), D2CHI002DT2(T0),
//				D2CHI110DT2(T0), D2CHI101DT2(T0), D2CHI011DT2(T0), D2CHI400DT2(T0),
//				D2CHI040DT2(T0), D2CHI004DT2(T0), D2CHI310DT2(T0), D2CHI301DT2(T0),
//				D2CHI031DT2(T0), D2CHI130DT2(T0), D2CHI103DT2(T0), D2CHI013DT2(T0),
//				D2CHI220DT2(T0), D2CHI202DT2(T0), D2CHI022DT2(T0), D2CHI211DT2(T0),
//				D2CHI121DT2(T0), D2CHI112DT2(T0) );
//				/*DCHI000DT(T0), DCHI200DT(T0), DCHI020DT(T0), DCHI002DT(T0),
//				DCHI110DT(T0), DCHI101DT(T0), DCHI011DT(T0), DCHI400DT(T0),
//				DCHI040DT(T0), DCHI004DT(T0), DCHI310DT(T0), DCHI301DT(T0),
//				DCHI031DT(T0), DCHI130DT(T0), DCHI103DT(T0), DCHI013DT(T0),
//				DCHI220DT(T0), DCHI202DT(T0), DCHI022DT(T0), DCHI211DT(T0),
//				DCHI121DT(T0), DCHI112DT(T0) );*/
//				/*CHI000(T0), CHI200(T0), CHI020(T0), CHI002(T0),
//				CHI110(T0), CHI101(T0), CHI011(T0), CHI400(T0),
//				CHI040(T0), CHI004(T0), CHI310(T0), CHI301(T0),
//				CHI031(T0), CHI130(T0), CHI103(T0), CHI013(T0),
//				CHI220(T0), CHI202(T0), CHI022(T0), CHI211(T0),
//				CHI121(T0), CHI112(T0) );*/
//	}
//	exit(-1);


	// generates tables for input to Jaki's original v-suphydro2
	for (double T0 = 1.0; T0 < 49.999; T0+=0.1)
	{
		PressVal    = PressTaylor(   T0, 0, 0, 0 );
		EntrVal     = EntrTaylor(    T0, 0, 0, 0 );
		EnerDensVal = EntrVal - PressVal;
		double dwds = T0+EntrVal*T0*T0*T0/(197.327*197.327*197.327
										*DEntrDTTaylor(T0, 0, 0, 0)); //T+s/(ds/dT)
		printf("%lf %15.12f %15.12f %15.12f %15.12f\n", T0,
				0.001*EnerDensVal*T0*T0*T0*T0/(197.327*197.327*197.327),
				0.001*PressVal*T0*T0*T0*T0/(197.327*197.327*197.327),
				EntrVal*T0*T0*T0/(197.327*197.327*197.327), dwds/197.327 );	

	}
	for (double T0 = 50.0; T0 < 800.001; T0+=0.1)
	{
		PressVal    = PressTaylor(   T0, 0, 0, 0 );
		EntrVal     = EntrTaylor(    T0, 0, 0, 0 );
		EnerDensVal = EntrVal - PressVal;
		double dwds = T0+EntrVal*T0*T0*T0/(197.327*197.327*197.327
										*DEntrDTTaylor(T0, 0, 0, 0)); //T+s/(ds/dT)
		printf("%lf %15.12f %15.12f %15.12f %15.12f\n", T0,
				0.001*EnerDensVal*T0*T0*T0*T0/(197.327*197.327*197.327),
				0.001*PressVal*T0*T0*T0*T0/(197.327*197.327*197.327),
				EntrVal*T0*T0*T0/(197.327*197.327*197.327), dwds/197.327 );	

	}
	exit(-1);


	// add a loop to generate toy initial conditions
	for (double x0 = -5.0; x0 <= 5.000001; x0 += 0.05)
	for (double y0 = -5.0; y0 <= 5.000001; y0 += 0.05)
	{
		double T0 = 500.0*exp(-0.1*(x0*x0+y0*y0));
		//double T0 = 500.0;
		//double muB0 = 20.0, muQ0 = 30.0, muS0 = 40.0;
		double muB0 = 0.0, muQ0 = 0.0, muS0 = 0.0;

		//Thermodynamics
		PressVal    = PressTaylor(   T0, muB0, muQ0, muS0 );
		EntrVal     = EntrTaylor(    T0, muB0, muQ0, muS0 );
		BarDensVal  = BarDensTaylor( T0, muB0, muQ0, muS0 );
		StrDensVal  = StrDensTaylor( T0, muB0, muQ0, muS0 );
		ChDensVal   = ChDensTaylor(  T0, muB0, muQ0, muS0 );
		EnerDensVal = EntrVal - PressVal
					+ muB0/T0*BarDensVal
					+ muQ0/T0*ChDensVal
					+ muS0/T0*StrDensVal;
		//if ( T0 >= 30.0 )
		{
			printf("eBSQ: %lf %lf %15.12f %15.12f %15.12f %15.12f\n", x0, y0,
				0.001*EnerDensVal*T0*T0*T0*T0/(197.327*197.327*197.327),
				BarDensVal*T0*T0*T0/(197.327*197.327*197.327),
				StrDensVal*T0*T0*T0/(197.327*197.327*197.327),
				ChDensVal*T0*T0*T0/(197.327*197.327*197.327) );
			printf("s: %lf %lf %15.12f\n", x0, y0,
				EntrVal*T0*T0*T0/(197.327*197.327*197.327));
		}
	}
	if (1) exit(-1);


	// for HDF arrays
	//long long gridLength = 69090879;
	//long long gridLength = 138181758;
	long long gridLength = 0;
	long long gridWidth  = 11;
	long long gridWidthD = 14;
	long long gridEntry  = 0;

	// set T and mu_i ranges
	const int Tmin = 30, Tmax = 200, DeltaT = 5;
	const int muBmin = -450, muBmax = 450, DeltamuB = 10;
	const int muQmin = -450, muQmax = 450, DeltamuQ = 10;
	const int muSmin = -450, muSmax = 450, DeltamuS = 10;

	// set HDF array lengths
	for(i=Tmin;i<=Tmax;i+=DeltaT)
	for(j=muBmin;j<=muBmax;j+=DeltamuB)
	for(k=muQmin;k<=muQmax;k+=DeltamuQ)
	for(l=muSmin;l<=muSmax;l+=DeltamuS)
		gridLength++;

	printf("Total gridLength = %ld\n",gridLength);
	fflush(stdout);

	double **quantityArray, **derivativeArray;

	quantityArray   = malloc(gridLength * sizeof *quantityArray);
    derivativeArray = malloc(gridLength * sizeof *derivativeArray);
	for (long long i=0; i<gridLength; i++)
	{
		quantityArray[i]   = malloc(gridWidth * sizeof *quantityArray[i]);
        derivativeArray[i] = malloc(gridWidthD * sizeof *derivativeArray[i]);
	}

//	double quantityArray[gridLength][gridWidth];
//	double derivativeArray[gridLength][gridWidthD];

	int use_staggered_grid = 0;	// for testing interpolation accuracy
	if (use_staggered_grid) printf("Running on staggered grid!\n");
	
	/* (Unconstrained) thermodynamics for all T, muB, muS, muQ. */  	
  	FILE *All_Therm_Taylor = fopen("EoS_Taylor_AllMu.dat","w");
	FILE *All_Therm_Der = fopen("EoS_Taylor_AllMu_Derivatives.dat","w");
    for(i=Tmin;  i<=Tmax;  i+=DeltaT){
    for(j=muBmin;j<=muBmax;j+=DeltamuB){
    for(k=muQmin;k<=muQmax;k+=DeltamuQ){
    for(l=muSmin;l<=muSmax;l+=DeltamuS){
					Tval = i; muBval = j;  muQval = k; muSval = l;
					if (use_staggered_grid)
					{
						Tval = i+0.5*DeltaT;
						muBval = j+0.5*DeltamuB;
						muQval = k+0.5*DeltamuQ;
						muSval = l+0.5*DeltamuS;
					}
					if (Tval>Tmax || muBval > muBmax || muSval > muSmax || muQval > muQmax) continue;
					
					//Thermodynamics
					PressVal = PressTaylor(Tval, muBval, muQval, muSval);
					EntrVal = EntrTaylor(Tval, muBval, muQval, muSval);
					BarDensVal = BarDensTaylor(Tval, muBval, muQval, muSval);
					StrDensVal = StrDensTaylor(Tval, muBval, muQval, muSval);
					ChDensVal = ChDensTaylor(Tval, muBval, muQval, muSval);
					EnerDensVal = EntrVal - PressVal 
							+ muBval/Tval*BarDensVal 
							+ muQval/Tval*ChDensVal 
							+ muSval/Tval*StrDensVal;
					SpSoundVal = SpSound(Tval, muBval, muQval, muSval);
					            
					//Second Order Derivatives
					D2PB2 = P2B2(Tval, muBval, muQval, muSval);
					D2PQ2 = P2Q2(Tval, muBval, muQval, muSval);
					D2PS2 = P2S2(Tval, muBval, muQval, muSval);
					
					D2PBQ = P2BQ(Tval, muBval, muQval, muSval);
					D2PBS = P2BS(Tval, muBval, muQval, muSval);
					D2PQS = P2QS(Tval, muBval, muQval, muSval);
					
					D2PTB = P2TB(Tval, muBval, muQval, muSval);
					D2PTQ = P2TQ(Tval, muBval, muQval, muSval);
					D2PTS = P2TS(Tval, muBval, muQval, muSval);
					D2PT2 = P2T2(Tval, muBval, muQval, muSval);
					
					fprintf(All_Therm_Taylor,"%lf  %lf  %lf  %lf  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", Tval, muBval, muQval, muSval, PressVal, EntrVal, 
					        BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
					fprintf(All_Therm_Der,"%lf  %lf  %lf  %lf  %3.12f  %3.12f %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f%  3.12f\n", Tval, muBval, muQval, muSval, D2PB2, D2PQ2, D2PS2, D2PBQ, D2PBS, D2PQS,
							D2PTB, D2PTQ, D2PTS, D2PT2);

					if ( !use_staggered_grid ) continue;	// ignore HDF since then grid is wrong

					quantityArray[gridEntry][0]  = Tval;
					quantityArray[gridEntry][1]  = muBval;
					quantityArray[gridEntry][2]  = muQval;
					quantityArray[gridEntry][3]  = muSval;
					quantityArray[gridEntry][4]  = PressVal;
					quantityArray[gridEntry][5]  = EntrVal;
					quantityArray[gridEntry][6]  = BarDensVal;
					quantityArray[gridEntry][7]  = StrDensVal;
					quantityArray[gridEntry][8]  = ChDensVal;
					quantityArray[gridEntry][9]  = EnerDensVal;
					quantityArray[gridEntry][10] = SpSoundVal;
					
					derivativeArray[gridEntry][0]  = Tval;
					derivativeArray[gridEntry][1]  = muBval;
					derivativeArray[gridEntry][2]  = muQval;
					derivativeArray[gridEntry][3]  = muSval;
					derivativeArray[gridEntry][4]  = D2PB2;
					derivativeArray[gridEntry][5]  = D2PQ2;
					derivativeArray[gridEntry][6]  = D2PS2;
					derivativeArray[gridEntry][7]  = D2PBQ;
					derivativeArray[gridEntry][8]  = D2PBS;
					derivativeArray[gridEntry][9]  = D2PQS;
					derivativeArray[gridEntry][10] = D2PTB;
					derivativeArray[gridEntry][11] = D2PTQ;
					derivativeArray[gridEntry][12] = D2PTS;
					derivativeArray[gridEntry][13] = D2PT2;
					
					gridEntry = gridEntry + 1;
           	    }
            }	    
	    }
	}    
	fclose(All_Therm_Taylor);
    fclose(All_Therm_Der);

	// write to HDF	
	export_to_HDF(quantityArray,"quantityFile.h5",gridLength,gridWidth);
        export_to_HDF(derivativeArray,"derivFile.h5",gridLength,gridWidthD);




//	/* (Unconstrained) thermodynamics for muS = muQ = 0. */
//  	FILE *AllTherm_No_QS = fopen("AllTherm_No_QS_Taylor.dat","w");
//  	for(i=30;i<=800;i+=1){
//  	    for(j=0;j<=450;j+=1){
//  	        k = 0; l = 0;
//  	        Tval = i; muBval = j;
//  	        muQval = k; muSval = l;
//
//  	        PressVal = PressTaylor(i,j,k,l);
//   	     	EntrVal = EntrTaylor(i,j,k,l);
//	        BarDensVal = BarDensTaylor(i,j,k,l);
//	        StrDensVal = StrDensTaylor(i,j,k,l);
//	        ChDensVal = ChDensTaylor(i,j,k,l);
//	        EnerDensVal = EntrVal - PressVal + muBval/Tval*BarDensVal + muQval/Tval*ChDensVal+ muSval/Tval*StrDensVal;
//  	        SpSoundVal = SpSound(Tval,muBval,muQval,muSval);
//
//	        fprintf(AllTherm_No_QS,"%d  %d  %d  %d  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", i, j, k, l, PressVal, EntrVal,
//	                        BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
//	    }
//	}
//	fclose(AllTherm_No_QS);
//

	// /* Thermodynamics for Strangeness neutrality. */
 //  	FILE *AllTherm_StrNeutr = fopen("AllTherm_StrNeutr_Taylor.dat","w");
 //  	for(i=30;i<=800;i+=1){
 //  		for(j=0;j<=450;j+=1){
 //  			Tval = i; muBval = j;
	//     	xSN[1] = 0.0; xSN[2] = 0.0;	
	//     	newt(xSN,NSN,&check,funcvSN);
	// 		funcvSN(NSN,xSN,fSN);
	//     	muQval = xSN[1]; muSval = xSN[2];
	        
	//     	PressVal = PressTaylor(Tval,muBval,muQval,muSval);
 //   	   		EntrVal = EntrTaylor(Tval,muBval,muQval,muSval);
	//     	BarDensVal = BarDensTaylor(Tval,muBval,muQval,muSval);
	//     	StrDensVal = StrDensTaylor(Tval,muBval,muQval,muSval);
	//     	ChDensVal = ChDensTaylor(Tval,muBval,muQval,muSval);
	//    		EnerDensVal = EntrVal - PressVal + muBval/Tval*BarDensVal + muQval/Tval*ChDensVal+ muSval/Tval*StrDensVal;	
	//     	SpSoundVal = SpSound(Tval,muBval,muQval,muSval);
	   
	//     	fprintf(AllTherm_StrNeutr,"%f  %f  %f  %f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", Tval, muBval, muQval, muSval, PressVal, EntrVal, 
	//                         BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
	//     }
	// }    
	// fclose(AllTherm_StrNeutr);
	

	/* Thermodynamics for muB/T = constant and Strangeness Neutrality. */
	// FILE *AllTherm_StrNeutr_muBTConst = fopen("AllTherm_StrNeutr_muBTConst_Taylor.dat","w");
	// for(Rat=0.5;Rat<=3;Rat+=0.5){  
	//    	for(i=30;i<=800;i+=1){
	//   		if(Rat*i < 451){    
 //  	    		Tval = i; muBval = Rat*Tval;
	//         	xSN[1] = 0.0; xSN[2] = 0.0;	
	//         	newt(xSN,NSN,&check,funcvSN);
	// 	    	funcvSN(NSN,xSN,fSN);
	//         	if (check) printf("Convergence problems.\n");		        
	// 	    	muQval = xSN[1]; muSval = xSN[2];
	            
	//         	PressVal = PressTaylor(Tval,muBval,muQval,muSval);
 //   	      		EntrVal = EntrTaylor(Tval,muBval,muQval,muSval);
	//         	BarDensVal = BarDensTaylor(Tval,muBval,muQval,muSval);
	//         	StrDensVal = StrDensTaylor(Tval,muBval,muQval,muSval);
	//         	ChDensVal = ChDensTaylor(Tval,muBval,muQval,muSval);
	//         	EnerDensVal = EntrVal - PressVal + muBval/Tval*BarDensVal + muQval/Tval*ChDensVal+ muSval/Tval*StrDensVal;
	//         	SpSoundVal = SpSound(Tval,muBval,muQval,muSval);
	         
	//         	fprintf(AllTherm_StrNeutr_muBTConst,"%f %f  %f  %f  %f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", Rat, Tval, muBval, muQval, muSval, PressVal, EntrVal, 
	//                         BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
	//         }
	//     }
	// }    
	// fclose(AllTherm_StrNeutr_muBTConst);

	
	/* Thermodynamics for MuB/T = constant and muS = muQ = 0. */
 //  	FILE *AllTherm_NoQS_muBTConst = fopen("AllTherm_NoQS_muBTConst_Taylor.dat","w");
	// for(Rat=0.5;Rat<=3;Rat+=0.5){  
	//    	for(i=30;i<=800;i+=1){
 //    		if(Rat*i < 451){    
 //  	        	Tval = i; muBval = Rat*Tval;
	//         	muQval = 0.0; muSval = 0.0;
	            
	//         	PressVal = PressTaylor(Tval,muBval,muQval,muSval);
 //   	      		EntrVal = EntrTaylor(Tval,muBval,muQval,muSval);
	//         	BarDensVal = BarDensTaylor(Tval,muBval,muQval,muSval);
	//         	StrDensVal = StrDensTaylor(Tval,muBval,muQval,muSval);
	//         	ChDensVal = ChDensTaylor(Tval,muBval,muQval,muSval);
	//         	EnerDensVal = EntrVal - PressVal + muBval/Tval*BarDensVal + muQval/Tval*ChDensVal+ muSval/Tval*StrDensVal;
	//         	SpSoundVal = SpSound(Tval,muBval,muQval,muSval);
	         
	//         	fprintf(AllTherm_NoQS_muBTConst,"%lf %lf  %lf  %lf  %lf  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f  %3.12f\n", Rat, Tval, muBval, muQval, muSval, PressVal, EntrVal, 
	//                         BarDensVal, StrDensVal, ChDensVal, EnerDensVal, SpSoundVal);
	//         }
	//     }
	// }    
	// fclose(AllTherm_NoQS_muBTConst);
	
	chdir(buff);

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Finished calculating in %lf seconds.\n", cpu_time_used);	
	
	return 0;
}


#undef NRANSI
