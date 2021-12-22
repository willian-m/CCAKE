#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "particle.h"

////////////////////////////////////////////////////////////////////////////////
// Default constructor
Particle::Particle()
{
  s_an = 0.0;
  Imat.identity();
}
////////////////////////////////////////////////////////////////////////////////

// Overloaded constructor with initial fields
Particle::Particle(vector<double> &fields)
{
  Imat.identity();
  r.x[0]  = fields[0];
  r.x[1]  = fields[1];
  e_sub   = fields[2];
  rhoB_an = fields[3];
  rhoS_an = fields[4];
  rhoQ_an = fields[5];
  u.x[0]  = fields[6];
  u.x[1]  = fields[7];
  if ( fields.size() > 8 ) // passing in shear tensor initialization as well
  {
    double pi11 = fields[8];
    double pi22 = fields[9];
    double pi12 = fields[10];
    double pi33 = fields[11];
    shv.x[0][0] = 0.0;
    shv.x[0][1] = 0.0;
    shv.x[0][2] = 0.0;
    shv.x[1][0] = 0.0;
    shv.x[1][1] = pi11;
    shv.x[1][2] = pi12;
    shv.x[2][0] = 0.0;
    shv.x[2][1] = pi12;
    shv.x[2][2] = pi22;
    shv33       = pi33;
  }
  s_an    = 0.0;
}

Particle::Particle( const Particle& p )
{
  Imat.identity();
  r.x[0]  = p.r.x[0];
  r.x[1]  = p.r.x[1];
  e_sub   = p.e_sub;
  rhoB_an = p.rhoB_an;
  rhoS_an = p.rhoS_an;
  rhoQ_an = p.rhoQ_an;
  u.x[0]  = p.u.x[0];
  u.x[1]  = p.u.x[1];
  shv     = p.shv;
  shv33   = p.shv33;
  s_an    = p.s_an;
  eosPtr  = p.eosPtr;
}



////////////////////////////////////////////////////////////////////////////////
void Particle::set_EquationOfStatePtr( EquationOfState * eosPtr_in )
{
  eosPtr = eosPtr_in;
}

////////////////////////////////////////////////////////////////////////////////
void Particle::set_SettingsPtr(Settings * settingsPtr_in)
{
  settingsPtr = settingsPtr_in;
}

////////////////////////////////////////////////////////////////////////////////
double Particle::locate_phase_diagram_point_eBSQ(// previously s_out
                 double e_In, double rhoB_In, double rhoS_In, double rhoQ_In )
{
  double current_sVal = s();
  if ( Freeze == 5 )
    return current_sVal;
  else
  {
    // default: use particle's current location as initial guess
    eosPtr->tbqs( thermo.T, thermo.muB, thermo.muQ, thermo.muS );
    double sVal = eosPtr->s_out( e_In, rhoB_In, rhoS_In, rhoQ_In );

    if ( sVal > 0.0 )
      thermo.set(*eosPtr);
    else
      Freeze = 5; // new label for (totally decoupled) particles which go outside grid

    return sVal;
  }
}


////////////////////////////////////////////////////////////////////////////////
double Particle::locate_phase_diagram_point_eBSQ(double e_In)// previously s_out
{
  double current_sVal = s();
  if ( Freeze == 5 )
    return current_sVal;
  else
  {
    // default: use particle's current location as initial guess
    eosPtr->tbqs( thermo.T, 0.0, 0.0, 0.0 );
    double sVal = eosPtr->s_out( e_In, 0.0, 0.0, 0.0 );

    if ( sVal > 0.0 )
      thermo.set(*eosPtr);
    else
      Freeze = 5; // new label for (totally decoupled) particles which go outside grid

    return sVal;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Particle::locate_phase_diagram_point_sBSQ(// previously update_s
                 double s_In, double rhoB_In, double rhoS_In, double rhoQ_In )
{
  if ( Freeze != 5 )
  {
    if ( abs(r.x[0]) < 0.00001 && abs(r.x[1]) < 0.00001 )
    cout << "CHECK SBSQ(1): " << s_In << "   " << rhoB_In << "   " << rhoS_In
        << "   " << rhoQ_In << endl;
    // default: use particle's current location as initial guess
    eosPtr->tbqs( thermo.T, thermo.muB, thermo.muQ, thermo.muS );
    bool update_s_success = eosPtr->update_s( s_In, rhoB_In, rhoS_In, rhoQ_In );

    if ( update_s_success )
      thermo.set(*eosPtr);
    else
      Freeze = 5; // new label for (totally decoupled) particles which go outside grid

    if ( abs(r.x[0]) < 0.00001 && abs(r.x[1]) < 0.00001 )
    cout << "CHECK SBSQ(2): " << s_In << "   " << rhoB_In << "   " << rhoS_In
        << "   " << rhoQ_In << endl;
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
void Particle::locate_phase_diagram_point_sBSQ(double s_In) // previously update_s
{
  if ( Freeze != 5 )
  {
    // default: use particle's current location as initial guess
    eosPtr->tbqs( thermo.T, 0.0, 0.0, 0.0 );
    bool update_s_success = eosPtr->update_s( s_In, 0.0, 0.0, 0.0 );

    if ( update_s_success )
      thermo.set(*eosPtr);
    else
      Freeze = 5; // new label for (totally decoupled) particles which go outside grid
  }
  return;
}



////////////////////////////////////////////////////////////////////////////////
double Particle::gamcalc()
{
    return sqrt( Norm2(u) + 1.0 );
}


////////////////////////////////////////////////////////////////////////////////
void Particle::frzcheck( double tin, int &count, int N )
{
  if ( Freeze == 5 )
  {
    count++;
    return;
  }
  else if ( Freeze == 0 )
  {
    if ( T() <= freezeoutT )
    {
      Freeze = 1;
      frz2.t = tin;
    }
  }
  else if ( Freeze == 1 )
  {
    if ( btrack == -1 )
    {
      count += 1;
      Freeze = 3;
      frz1.t = tin;
    }
    else if ( T()>frz1.T )
    {
      Freeze = 1;
      frz2.t = tin;
    }
    else if( T() <= freezeoutT )
    {
      count += 1;
      Freeze = 3;
      frz1.t = tin;
    }
    else
    {
      Freeze=0;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//  Computes gamma and velocity
void Particle::calcbsq(double tin)
{
  gamma           = gamcalc();
  v               = (1.0/gamma)*u;
  double s_lab    = eta/gamma/tin;
  double rhoB_lab = rhoB_sub/gamma/tin;
  double rhoS_lab = rhoS_sub/gamma/tin;
  double rhoQ_lab = rhoQ_sub/gamma/tin;
  qmom            = ( (e()+p())*gamma/sigma )*u;
/*//THIS IS THE OLD ROUGH VERSION, JUST USE THE EVOLUTION EQUATIONS INSTEAD
	double rhoB_in2 = B*sigma/(gamma*sigmaweight);
	double rhoS_in2 = S*sigma/(gamma*sigmaweight);
	double rhoQ_in2 = Q*sigma/(gamma*sigmaweight);
	rhoB_an = rhoB_in2;
	rhoS_an = rhoS_in2;
	rhoQ_an = rhoQ_in2;
//cout << "\t - finding EoS solution for sBSQ: " << tin << "   " << s_lab << "   "
//		<< rhoB_in2 << "   " << rhoS_in2 << "   " << rhoQ_in2 << endl;
	locate_phase_diagram_point_sBSQ( s_lab, rhoB_in2, rhoS_in2, rhoQ_in2 );
*/
  // using *_sub quantities (which have been smoothed) in order to be consistent
  // with s_in2 = eta/(gamma*t) and evaluation of rest of EoM's quantities (e.g.,
  // sigma) which are also smoothed
	locate_phase_diagram_point_sBSQ( s_lab, rhoB_lab, rhoS_lab, rhoQ_lab );
}



////////////////////////////////////////////////////////////////////////////////
void Particle::return_bsqsv_A()
{
    eta_o_tau = (settingsPtr->using_shear) ? setas/stauRelax : 0.0;

	// THIS NEEDS TO BE CHECKED/FIXED, SPECIFICALLY WHEN INCLUDING MORE BETA-DOT TERMS
    Agam  = w() - dwds()*(s()+ bigPI/T() )- zeta/tauRelax
            - dwdB() * rhoB() - dwdS() * rhoS() - dwdQ() * rhoQ();

    Agam2 = ( Agam - eta_o_tau/3.0 - dwdsT1*shv.x[0][0] ) / gamma;
    Ctot  = C + eta_o_tau*(1/g2-1);

}



////////////////////////////////////////////////////////////////////////////////
double Particle::Bsub()
{
  // make sure this quantity is set
  uu = u*u;

  if ( !settingsPtr->using_shear )
    return 0.0;
  else
  {
    // these quantities will all be zero without shear
    mini( pimin, shv );
    piu         = rowp1(0,shv)*u;
    piutot      = piu+transpose(piu);
    double bsub = 0.0;
    double pig  = shv.x[0][0]/g2;

    for (int i=0; i<=1; i++)
    for (int j=0; j<=1; j++)
      bsub += gradU.x[i][j] * ( pimin.x[i][j] + pig*uu.x[j][i]
                                - ( piu.x[i][j] + piu.x[j][i] ) / gamma );
    return bsub;
  }
}

////////////////////////////////////////////////////////////////////////////////
Matrix<double,2,2> Particle::Msub(int i)
{
  piu                     = rowp1(0,shv)*u;
  Matrix<double,2,2> msub = Agam2*uu + Ctot*gamma*Imat - (1+4/3./g2)*piu
                            + dwdsT1*transpose(piu) + gamma*pimin;
  return msub;
}


////////////////////////////////////////////////////////////////////////////////
Matrix<double,2,2> Particle::dpidtsub()
{
  Matrix<double,2,2> vsub;

  for (int i=0; i<=1; i++)
  for (int j=0; j<=1; j++)
  for (int k=0; k<=1; k++)
    vsub.x[i][j] += ( u.x[i]*pimin.x[j][k] + u.x[j]*pimin.x[i][k] )*du_dt.x[k];

  return vsub;
}

////////////////////////////////////////////////////////////////////////////////
void Particle::bsqsvsigset( double tin, int i )
{
  // from svsigset
  g2           = gamma*gamma;
  g3           = gamma*g2;
  gt           = gamma*tin;
  double dwdsT = dwds()/T();
  dwdsT1       = 1 - dwds()/T();
  sigl         = dsigma_dt/sigma - 1/tin;
  gradU        = gamma*gradV+g3*(v*(v*gradV));
  bigPI        = Bulk*sigma/gt;
  C            = w()+ bigPI;
  return_bsqsv_A();
  Btot         = ( Agam*gamma + 2.0*eta_o_tau/3.0*gamma )*sigl
                + bigPI/tauRelax + dwdsT*( gt*shv33 + Bsub() );
  check        = sigl;
}

////////////////////////////////////////////////////////////////////////////////
void Particle::setvisc( int etaconst, double bvf, double svf, double zTc,
                        double sTc, double sig, int type )
{
  setas=s()*svf;

  stauRelax=5*setas/w();
  if (!settingsPtr->using_Gubser && stauRelax < 0.005) stauRelax = 0.005;

  if (abs(bvf) > 1e-6)
  {
    cerr << "You need to replace setvisc!!!" << endl;
    exit(1);
  }
  zeta = 0;
  tauRelax = 1;
}



////////////////////////////////////////////////////////////////////////////////
void Particle::sets(double tin2, bool is_first_timestep)
{
    gamma = gamcalc();
    if (settingsPtr->using_Gubser_with_shear && is_first_timestep)
    {
      shv.x[0][1] = 1./gamma*inner(u,colp1(1,shv));
      shv.x[0][2] = 1./gamma*inner(u,colp1(2,shv));
      shv.x[1][0] = shv.x[0][1];
      shv.x[2][0] = shv.x[0][2];
      
      setvar();
      //cout << "Sanity check: " << 1./gamma/gamma*con(uu,pimin) << "   "
      //      << shv.x[1][1] + shv.x[2][2] + tin2*shv33 << endl;

      //shv.x[0][0] = shv.x[1][1] + shv.x[2][2] + tin2*shv33;
      
      // go back to Jaki's default
      shv.x[0][0]=1./gamma/gamma*con(uu,pimin);
      shv33=(shv.x[0][0]-shv.x[1][1]-shv.x[2][2])/tin2;
    }
    else
    {
      shv.x[2][1]=shv.x[1][2];
      shv.x[0][1]=1./gamma*inner(u,colp1(1,shv));
      shv.x[0][2]=1./gamma*inner(u,colp1(2,shv));
      shv.x[1][0]=shv.x[0][1];
      shv.x[2][0]=shv.x[0][2];

      setvar();
      shv.x[0][0]=1./gamma/gamma*con(uu,pimin);
      shv33=(shv.x[0][0]-shv.x[1][1]-shv.x[2][2])/tin2;
    }
}


////////////////////////////////////////////////////////////////////////////////
void Particle::setvar()
{
    mini(pimin,shv);
    uu=u*u;
}




////////////////////////////////////////////////////////////////////////////////
// NEW FUNCTIONS TO CLEAN UP EQUATIONS_OF_MOTION BELOW THIS LINE
////////////////////////////////////////////////////////////////////////////////

void Particle::set_vartheta(double t) { vartheta = dsigma_dt/sigma - 1.0/t; }


////////////////////////////////////////////////////////////////////////////////
double Particle::get_Theta_force() { return -vartheta/gamma; }


////////////////////////////////////////////////////////////////////////////////
double Particle::get_Pi_force(double t)
{
  bigPI = Bulk*sigma/(gamma*t);
  return vartheta*(zeta/tauRelax + bigPI) - bigPI/(gamma*gamma*stauRelax);
}


////////////////////////////////////////////////////////////////////////////////
double Particle::get_aleph_force(double t)
{
  double aleph_force = -gamma*t*shv33;
  for (int i = 1; i < 3; i++)
  for (int j = 1; j < 3; j++)
    aleph_force += ( (shv.x[i][0]*u.x[j] + shv.x[0][j]*u.x[i])/gamma
                      - shv.x[i][j] - shv.x[0][0]*u.x[i]*u.x[j]/(gamma*gamma) )
                    * gradU.x[i][j];
  return aleph_force;
}


////////////////////////////////////////////////////////////////////////////////
Vector<double,2> Particle::get_Theta_mass()
{
  return v; // just the non-relativistic velocity
}


////////////////////////////////////////////////////////////////////////////////
Vector<double,2> Particle::get_Pi_mass()
{
  return (-(zeta/tauRelax + bigPI)/gamma)*v;
}


////////////////////////////////////////////////////////////////////////////////
Vector<double,2> Particle::get_aleph_mass()
{
  return shv.x[0][0]*v - colp1(0, shv);
}




