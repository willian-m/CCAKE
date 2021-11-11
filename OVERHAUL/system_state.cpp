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
void SystemState::set_EquationOfStatePtr( EquationOfState * eosPtr_in )
{
  eosPtr = eosPtr_in;
}


void SystemState::set_SettingsPtr(Settings * settingsPtr_in)
{
  settingsPtr = settingsPtr_in;
}


////////////////////////////////////////////////////////////////////////////////
void SystemState::initialize()  // formerly called "manualenter"
{
  int start, end;
  int df;

  cout << __PRETTY_FUNCTION__ << ": " << endl;
  _n = particles.size();

  cout << "_n = " << _n << endl;

  t = settingsPtr->t0;

  cout << "t = " << t << endl;

  _h = settingsPtr->_h;

  settingsPtr->efcheck = eosPtr->efreeze(settingsPtr->Freeze_Out_Temperature);
  settingsPtr->sfcheck = eosPtr->sfreeze(settingsPtr->Freeze_Out_Temperature);
  efcheck = settingsPtr->efcheck;
  sfcheck = settingsPtr->sfcheck;

  std::cout << "efcheck = " << efcheck*hbarc_GeVfm << " GeV/fm^3\n";
  std::cout << "sfcheck = " << sfcheck << " 1/fm^3\n";

  freezeoutT = settingsPtr->Freeze_Out_Temperature;

  for (auto & p : particles)
  {
    p.set_EquationOfStatePtr( eosPtr );
    p.freezeoutT = freezeoutT;
  }

  linklist.efcheck = efcheck;
  linklist.sfcheck = sfcheck;
  linklist.fcount  = 0;
  linklist.average = 0;
  //       Start reading ICs          //

  //int numpart, _Ntable3;

  //  cout << "setting up SPH" << endl;
  return;
}



void SystemState::initialize_linklist()
{
  cout << "Initial conditions type: " << settingsPtr->IC_type << endl;

  if ( settingsPtr->IC_type == "ICCING" )
  {
    settingsPtr->gtyp   = 6;

    int count           = 1;
    vector<string>        filelist( count );

    int j               = 0;
    filelist[j]         = "./ic0.dat"; // only doing single event
    linklist.filenames  = filelist;
    linklist.fcount     = count;
    linklist.fnum       = linklist.start;
    
    cout << "Check 0: " << particles[0].r.x[0] << "   " << particles[0].r.x[1] << endl;

    int currently_frozen_out = number_part;
    linklist.initialize( settingsPtr->t0, particles.size(),
                         settingsPtr->_h, &particles, dt, currently_frozen_out );

    //cout << "number of sph particles=" << _Ntable3 << endl;
    linklist.gtyp=settingsPtr->gtyp;

  }
  else if ( settingsPtr->IC_type == "Gubser" )
  {
    settingsPtr->gtyp   = 7;

    int count           = 1;
    vector<string>        filelist( count );

    int j               = 0;
    filelist[j]         = "./ic0.dat"; // only doing single event
    linklist.filenames  = filelist;
    linklist.fcount     = count;
    linklist.fnum       = linklist.start;
    
    cout << "Check 0: " << particles[0].r.x[0] << "   " << particles[0].r.x[1] << endl;

    int currently_frozen_out = number_part;
    linklist.initialize( settingsPtr->t0, particles.size(),
                         settingsPtr->_h, &particles, dt, currently_frozen_out );

    //cout << "number of sph particles=" << _Ntable3 << endl;
    linklist.gtyp=settingsPtr->gtyp;
  }
  else
  {
    std::cerr << "Initial conditions type = " << settingsPtr->IC_type
              << " not supported!" << std::endl;
    exit(1);
  }


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
    E += ( p.C*p.g2 - p.p() - p.bigPI + p.shv.x[0][0] )
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
      auto & p = particles[i];

        E += ( p.C*p.g2 - p.p() - p.bigPI + p.shv.x[0][0] )
              / p.sigma*p.sigmaweight*t;
        if (i==0)
          std::cout << "E: " << i << "   " << t
              << "   " << p.T()
              << "   " << p.e()
              << "   " << p.C
              << "   " << p.g2
              << "   " << p.p()
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
    auto & p = particles[i];

    dEz += ( p.p() + p.bigPI + p.shv33*t2 ) / p.sigma*p.sigmaweight;

    if (false)
      std::cout << "dEz: " << i << "   " << t
        << "   " << p.p()
        << "   " << p.bigPI
        << "   " << p.shv33*t2
        << "   " << p.sigma
        << "   " << p.sigmaweight << endl;
  }
}






///////////////////////////////////////////////////////////////////////////////
void SystemState::set_current_timestep_quantities()
{
  N = _n;

  etasigma0.resize(N);
  Bulk0.resize(N);

  u0.resize(N);
  r0.resize(N);

  shv0.resize(N);

  for (int i=0; i<N; ++i)
  {
    auto & p = particles[i];

    u0[i]        = p.u;
    r0[i]        = p.r;
    etasigma0[i] = p.eta_sigma;
    Bulk0[i]     = p.Bulk;
    mini( shv0[i], p.shv );
  }
}


///////////////////////////////////////////////////////////////////////////////
void SystemState::get_derivative_step(double dx)
{
  N = _n;

  for (int i=0; i<N; ++i)
  {
    auto & p = particles[i];

    p.r            = r0[i]        + dx*p.v;

    if ( p.Freeze < 5 )
    {
      p.u            = u0[i]        + dx*p.du_dt;
      p.eta_sigma    = etasigma0[i] + dx*p.detasigma_dt;
      p.Bulk         = Bulk0[i]     + dx*p.dBulk_dt;
      tmini( p.shv,    shv0[i]      + dx*p.dshv_dt );
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
void SystemState::bsqsvfreezeout(int curfrz)
{
  cout << "CHECK BSQSVFREEZEOUT: " << frzc << "   " << tau << "   " << taup
        << "   " << taupp << "   " << cfon << endl;

  if (frzc==0)
  {
    taupp = t;
    frzc  = 1;
    for (auto & p : particles)
    {
      p.frz2.r       = p.r;
      p.frz2.u       = p.u;
      p.frz2.sigma   = p.sigma;
      p.frz2.T       = p.T();
      p.frz2.bulk    = p.bigPI;
      p.frz2.theta   = p.div_u + p.gamma/t;
      p.frz2.gradP   = p.gradP;
      p.frz2.shear   = p.shv;
      p.frz2.shear33 = p.shv33;
      p.frz2.inside  = p.inside;
    }

  }
  else if (frzc==1)
  {
    taup = t;
    frzc = 2;
    for (auto & p : particles)
    {
      p.frz1.r       = p.r;
      p.frz1.u       = p.u;
      p.frz1.sigma   = p.sigma;
      p.frz1.T       = p.T();
      p.frz1.bulk    = p.bigPI;
      p.frz1.theta   = p.div_u + p.gamma/t;
      p.frz1.gradP   = p.gradP;
      p.frz1.shear   = p.shv;
      p.frz1.shear33 = p.shv33;
      p.frz1.inside  = p.inside;
    }

    divTtemp.resize( curfrz );
    divT.resize( curfrz );
    gsub.resize( curfrz );
    uout.resize( curfrz );
    swsub.resize( curfrz );
    bulksub.resize( curfrz );
    shearsub.resize( curfrz );
    shear33sub.resize( curfrz );
    tlist.resize( curfrz );
    rsub.resize( curfrz );

    if ( curfrz > 0 )
      bsqsvinterpolate( curfrz );
    else
      cf = 0;
  }
  else
  {
    int i_local = 0;
    for (auto & p : particles)
    {
      if ( p.Freeze < 4 )
      {
        if ( ( p.btrack <= 3 ) && ( p.btrack > 0 ) )
        {
          p.fback4 = p.fback2;
          p.fback3 = p.fback;
          p.fback2 = p.frz2;
          p.fback  = p.frz1;
        }
        else if ( p.btrack == 0 )
        {
          if ( p.fback.gradP.x[0] != 0 )
          {
            p.frz2 = p.fback2;
            p.frz1 = p.fback;
          }
          else
          {
            p.frz2 = p.fback4;
            p.frz1 = p.fback3;
            cout << "back second"  << endl;
          }


          curfrz++;
          list.push_back( i_local );
          p.Freeze = 4;
          p.btrack = -1;
        }
      }

      i_local++;
    }

    tau = t;

    // resize vectors
    divTtemp.resize( curfrz );
    divT.resize( curfrz );
    gsub.resize( curfrz );
    uout.resize( curfrz );
    swsub.resize( curfrz );
    bulksub.resize( curfrz );
    shearsub.resize( curfrz );
    shear33sub.resize( curfrz );
    tlist.resize( curfrz );
    rsub.resize( curfrz );

    if ( curfrz > 0 )
      bsqsvinterpolate( curfrz );
    else
      cf = 0;


    //sets up the variables for the next time step
    for (auto & p : particles)
    {
      p.frz2         = p.frz1;

      p.frz1.r       = p.r;
      p.frz1.u       = p.u;
      p.frz1.sigma   = p.sigma;
      p.frz1.T       = p.T();
      p.frz1.bulk    = p.bigPI ;
      p.frz1.theta   = p.div_u+p.gamma/t;
      p.frz1.gradP   = p.gradP;
      p.frz1.shear   = p.shv;
      p.frz1.shear33 = p.shv33;
      p.frz1.inside  = p.inside;
    }

    taupp = taup;
    taup  = tau;
  }

  cfon = 0;
}
//////////////////////////////////////////////////////////////////////////////
void SystemState::bsqsvinterpolate(int curfrz)
{
  sFO.resize( curfrz, 0 );
  Tfluc.resize( curfrz, 0 );

  for (int j=0; j<curfrz; j++)
  {
    int i    = list[j];
    auto & p = particles[i];


    int swit = 0;
    if ( abs( p.frz1.T - freezeoutT ) < abs( p.frz2.T - freezeoutT ) )
      swit   = 1;
    else
      swit   = 2;


    double sigsub = 0.0, thetasub = 0.0, inside = 0.0;
    Vector<double,2> gradPsub;
    if ( swit == 1 )
    {
      if ( p.btrack != -1 )
        tlist[j]    = taup;
      else
        tlist[j]    = taup - dt;

      rsub[j]       = p.frz1.r;
      uout[j]       = p.frz1.u;
      bulksub[j]    = p.frz1.bulk;
      shearsub[j]   = p.frz1.shear;
      shear33sub[j] = p.frz1.shear33;

      gradPsub      = p.frz1.gradP;
      inside        = p.frz1.inside;
      sigsub        = p.frz1.sigma;
      thetasub      = p.frz1.theta;
      Tfluc[j]      = p.frz1.T;
    }
    else if ( swit == 2 )
    {
      if ( p.btrack != -1 )
        tlist[j]    = taupp;
      else
        tlist[j]    = taupp - dt;

      rsub[j]       = p.frz2.r;
      uout[j]       = p.frz2.u;
      bulksub[j]    = p.frz2.bulk;
      shearsub[j]   = p.frz2.shear;
      shear33sub[j] = p.frz2.shear33;

      gradPsub      = p.frz2.gradP;
      inside        = p.frz2.inside;
      sigsub        = p.frz2.sigma;
      thetasub      = p.frz2.theta;
      Tfluc[j]      = p.frz2.T;
    }
    else
    {
      cout << __PRETTY_FUNCTION__ << ": Not at freeze-out temperature" << endl;
    }

    //sFO[j]       = _p[0].EOSs_terms_T(Tfluc[j]);  // unnecessary since all EOS's identical
    sFO[j]       = p.eosPtr->s_terms_T( Tfluc[j] );

    gsub[j]      = sqrt( Norm2(uout[j]) + 1 );


    sigsub      /= gsub[j]*tlist[j];
    swsub[j]     = p.sigmaweight/sigsub;

    divT[j]      = (1.0/sFO[j])*gradPsub;
    divTtemp[j]  = -(1.0/(gsub[j]*sFO[j]))
                      *( cs2 * (wfz+bulksub[j]) * thetasub
                        - cs2*inside+inner(uout[j], gradPsub) );


    double insub = divTtemp[j]*divTtemp[j] - Norm2(divT[j]);
    double norm  = -sqrt(abs(insub));
    divTtemp[j] /= norm;
    divT[j]      = (1.0/norm)*divT[j];


    if ( divTtemp[j] == 1 )
    {
      cout << "track sph=" << p.btrack << " " << i << endl;
      cout << divTtemp[j] << " " << divT[j] << " " << norm << endl;
      cout << gradPsub << " " << thetasub << endl;
      cout << tlist[j] << " " << p.r << endl;
      cout << p.frz1.gradP << " " << p.frz2.gradP << endl;
      cout << p.frz1.T*197.3<< " " << p.frz2.T*197.3 << endl;
      getchar();
    }

    avgetasig += sFO[j]/sigsub;

    if(isnan(divTtemp[j]))
    {
      cout << "divtemp" << endl;
      cout << divTtemp[j] << " " << divT[j] << " " << norm << endl;
      cout << gradPsub << " " << thetasub << endl;
      cout << bulksub[j] << endl;
      cout << gsub[j] << endl;
      cout << tlist[j] << " " << p.r << endl;
      cout << p.frz1.T*0.1973<< " " << p.frz2.T*0.1973<< endl;
    }

    sFO[j]   *= pow(Tfluc[j]*0.1973, 3);
    Tfluc[j] *= 0.1973;

  }

  cf = curfrz;
}
///////////////////////////////////////////////////////////////////////////////////
