#include "system_state.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>

//using namespace std;
using std::cout;
using std::endl;
using std::string;

#include "constants.h"
#include "vector.h"
#include "particle.h"
#include "runge_kutta.h"
#include "eos.h"
#include "Stopwatch.h"
#include "system_state.h"
#include "linklist.h"

using namespace constants;

////////////////////////////////////////////////////////////////////////////////
void SystemState::set_equation_of_state(EquationOfState & eos)
{
  eosPtr = &eos;
}

void SystemState::set_settings(Settings & settings)
{
  settingsPtr = &settings;
}



////////////////////////////////////////////////////////////////////////////////
void SystemState::initialize()  // formerly called "manualenter"
{
  double h, factor;
  double it0;
  int start, end;

  int df;


  /*

  //linklist.setv( fvisc );
  //linklist.eost   = settingsPtr->eostype;
  //linklist.cevent = 0;
  //std::cout << fvisc << " hydro, h=" << h <<  " dimensions=" << 2
  //          << " dt=" << dt << " QM fluc:  " << settingsPtr->qmf << "\n";

  //////////////////////////////////////////////////////////////////////////////
  // SET EQUATION OF STATE
  // rewrite by C. Plumberg: allow for different EOS format if using BSQ
  double efcheck = 0.0, sfcheck = 0.0;
  if ( settingsPtr->visc == 4 )	//if we're running BSQ (table is only option)
  {
    bool using_HDF = false;
    if (using_HDF)
    {
      string quantityFile   = ifolder + std::string("quantityFile.h5");
      string derivativeFile = ifolder + std::string("derivFile.h5");
      std::cout << "Using BSQ Equation of State table from: "
                << quantityFile << " and " << derivativeFile << "\n";

      eosPtr->init( quantityFile, derivativeFile );
    }
    else
    {
      string quantityFilename   = "EoS_Taylor_AllMu_T0_1200.dat";
      string derivativeFilename = "EoS_Taylor_AllMu_Derivatives_T0_1200.dat";
      string quantityFile       = ifolder + quantityFilename;
      string derivativeFile     = ifolder + derivativeFilename;
      std::cout << "Using BSQ Equation of State table from: "
                << quantityFile << " and " << derivativeFile << "\n";

      eosPtr->init( quantityFile, derivativeFile );
    }

    //eosPtr->eosin( eostype );			// does nothing!
    const double freeze_out_T_at_mu_eq_0
                  = 0.15/hbarc_GeVfm;	//1/fm
    efcheck       = EOS0.efreeze( freeze_out_T_at_mu_eq_0 );
    sfcheck       = EOS0.sfreeze( freeze_out_T_at_mu_eq_0 );
    //efcheck = 0.266112/0.1973;
    //sfcheck = 2.05743;

    std::cout << "efcheck = " << efcheck*hbarc_GeVfm << " GeV/fm^3\n";
    std::cout << "sfcheck = " << sfcheck << " 1/fm^3\n";
  }
  else
  {
    std::cerr << "This EoS model not currently supported!" << std::endl;
  }

  */

  linklist.efcheck = efcheck;
  linklist.sfcheck = sfcheck;
  linklist.fcount  = 0;
  linklist.average = 0;
  //       Start reading ICs          //

  int numpart, _Ntable3;

  //  cout << "setting up SPH" << endl;

  string ictype = "iccing";
  cout << "Initial conditions type: " << ictype << endl;

  linklist.gtyp=0;
  if ( ictype == "iccing" )
  {

    int count           = 1;
    vector<string>        filelist( count );

    int j               = 0;
    filelist[j]         = "./ic0.dat"; // only doing single event
    linklist.filenames  = filelist;
    linklist.fcount     = count;
    linklist.fnum       = linklist.start;
    
    linklist.initialize( it0, _Ntable3, h, particles, dt, numpart );

    cout << "number of sph particles=" << _Ntable3 << endl;
    linklist.gtyp=6;

  }

  /*if ( ictype == iccing )
  {
    linklist.updateIC();
    cout << "bsq optimization done" << endl;
    linklist.bsqsvfreezeset();
  }*/

  // formerly bsqsv_set in this loop
  for (auto & p : particles)
  {
    double gg = p.gamcalc();
    p.g2      = gg*gg;
    p.shv33   = 0.0;
  }

  return;
}
///////////////////////////////////////////////////////////////////////////////
//Dekra: BSQsimulation was moved from this location to BSQhydro and renamed to run
///////////////////////////////////////////////////////////////////////////////
//Start routines for checking conservation of energy density, entropy density and BSQ 
// charge densities at each time step of the simulation
///////////////////////////////////////
void SystemState::check_BSQ_energy_conservation()
{
  E=0.0;
  for ( auto & p : particles )
    E += ( p.C*p.g2 - p.eosPtr->p() - p.bigPI + p.shv.x[0][0] )
          *p.sigmaweight*t/p.sigma;

  if (linklist.first == 1)
  {
    linklist.first = 0;
    E0    = E;
  }

  return;
}
////////////////////////////////////////
void SystemState::check_BSQ_charge_conservation()
{
  Btotal = 0.0;
  Stotal = 0.0;
  Qtotal = 0.0;

  for ( auto & p : particles )
  {
    //Btotal += p.B;
    //Stotal += p.S;
    //Qtotal += p.Q;
    Btotal += p.rhoB_sub*p.rho_weight;
    Stotal += p.rhoS_sub*p.rho_weight;
    Qtotal += p.rhoQ_sub*p.rho_weight;
  }

  if (linklist.first==1)
  {
    Btotal0 = Btotal;
    Stotal0 = Stotal;
    Qtotal0 = Qtotal;
  }

	return;
}
///////////////////////////////////////
void SystemState::bsqsvconservation()
{
    bsqsvconservation_E();
    Etot  = E + Ez;
    Eloss = (E0-Etot)/E0*100;
    rk2   = 0;
}
///////////////////////////////////////
void SystemState::conservation_entropy()
{
  S=0.0;

  for (int i=0; i<_n; i++)
  {
    S += particles[i].eta_sigma*particles[i].sigmaweight;
    if (i==0)
    std::cout << "\t\t --> " << i << "   " << particles[i].eta_sigma << "   "
              << particles[i].sigmaweight << "   " << S << endl;
  }

  if (linklist.first==1)
    S0=S;
}
///////////////////////////////////////
void SystemState::conservation_BSQ()
{
    Btotal = 0.0;
    Stotal = 0.0;
    Qtotal = 0.0;

    for (int i=0; i<_n; i++)
	{
        //Btotal += particles[i].B;
        //Stotal += particles[i].S;
        //Qtotal += particles[i].Q;
        Btotal += particles[i].rhoB_sub*particles[i].rho_weight;
        Stotal += particles[i].rhoS_sub*particles[i].rho_weight;
        Qtotal += particles[i].rhoQ_sub*particles[i].rho_weight;
    }

    if (linklist.first==1)
    {
        Btotal0 = Btotal;
        Stotal0 = Stotal;
        Qtotal0 = Qtotal;
    }
	return;
}
///////////////////////////////////////
void SystemState::bsqsvconservation_E()
{

    E=0.;
    for (int i=0; i<_n; i++)
    {
      const auto & p = particles[i];

        E += ( p.C*p.g2 - p.eosPtr->p() - p.bigPI + p.shv.x[0][0] )
              / p.sigma*p.sigmaweight*t;
        if (i==0)
          std::cout << "E: " << i << "   " << t
              << "   " << p.eosPtr->T()
              << "   " << p.eosPtr->e()
              << "   " << p.C
              << "   " << p.g2
              << "   " << p.eosPtr->p()
              << "   " << p.bigPI
              << "   " << p.shv.x[0][0]
              << "   " << p.sigma
              << "   " << p.sigmaweight << endl;    }

    if (linklist.first==1)
    {
      linklist.first=0;
      E0=E;
    }
}
///////////////////////////////////////
void SystemState::bsqsvconservation_Ez()
{
  dEz=0.;

  double t2=t*t;
  for (int i=0; i<_n; i++)
  {
    const auto & p = particles[i];

    dEz += ( p.eosPtr->p() + p.bigPI + p.shv33*t2 ) / p.sigma*p.sigmaweight;

    if (false)
      std::cout << "dEz: " << i << "   " << t
        << "   " << p.eosPtr->p()
        << "   " << p.bigPI
        << "   " << p.shv33*t2
        << "   " << p.sigma
        << "   " << p.sigmaweight << endl;
  }
}






///////////////////////////////////////////////////////////////////////////////
void SystemState::set_current_timestep_quantities()
{
  etasigma0.resize(N);
  Bulk0.resize(N);

  u0.resize(N);
  r0.resize(N);

  shv0.resize(N);

  for (int i=0; i<N; ++i)
  {
    const auto & p = particles[i];
    u0[i]        = p.u;
    r0[i]        = p.r;
    etasigma0[i] = p.eta_sigma;
    Bulk0[i]     = p.Bulk;
    mini( shv0[i], p.shv );
  }
}
///////////////////////////////////////////////////////////////////////////////
void SystemState::get_derivative_halfstep(double dx)
{
  for (int i=0; i<N; ++i)
  {
    auto & p = particles[i];
    p.u            = u0[i]        + 0.5*dx*p.du_dt;
    p.r            = r0[i]        + 0.5*dx*p.v;
    p.eta_sigma    = etasigma0[i] + 0.5*dx*p.detasigma_dt;
    p.Bulk         = Bulk0[i]     + 0.5*dx*p.dBulk_dt;
    tmini( p.shv,    shv0[i]      + 0.5*dx*p.dshv_dt );
  }
}
///////////////////////////////////////////////////////////////////////////////
void SystemState::get_derivative_fullstep(double dx)
{
  for (int i=0; i<N; ++i)
  {
    auto & p = particles[i];
    p.u            = u0[i]        + dx*p.du_dt;
    p.r            = r0[i]        + dx*p.v;
    p.eta_sigma    = etasigma0[i] + dx*p.detasigma_dt;
    p.Bulk         = Bulk0[i]     + dx*p.dBulk_dt;
    tmini( p.shv,    shv0[i]      + dx*p.dshv_dt );
  }
}
//////////////////////////////////////////////////////////////////////////////