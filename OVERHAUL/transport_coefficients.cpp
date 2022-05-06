#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "eos.h"
#include "kernel.h"
#include "matrix.h"
#include "particle.h"
#include "settings.h"
#include "transport_coefficients.h"

using std::string;
using std::vector;

//==============================================================================
//==============================================================================
// Getter functions for eta, zeta, and their relaxation times
double TransportCoefficients::getEta()      { return eta();      }
double TransportCoefficients::getZeta()     { return zeta();     }
double TransportCoefficients::getTauShear() { return tauShear(); }
double TransportCoefficients::getTauBulk()  { return tauBulk();  }

//==============================================================================
//==============================================================================
// INITIALIZE THE TRANSPORT COEFFICIENTS

//===============================
// batch initialization
void TransportCoefficients::initialize( const string & mode )
{
  if ( mode == "default" )
    initialize( "default", "default", "default", "default" );
  else if ( mode == "Gubser" )
    initialize( "constant", "Gubser", "NoBulk", "default" );
  else
  {
    std::cout << "TransportCoefficients::mode value = "
              << mode << " not supported!\n";
    exit(8);
  }
}


//===============================
// explicit (individual) initialization
void TransportCoefficients::initialize( const string & etaType_in,
                                        const string & tauShearType_in,
                                        const string & zetaType_in,
                                        const string & tauBulkType_in )
{
  // Set shear viscosity
  initialize_eta( etaType_in );

  // Set shear relaxation
  initialize_tauShear( tauShearType_in );

  // Set bulk viscosity
  initialize_zeta( zetaType_in );

  // Set bulk relaxation
  initialize_tauBulk( tauBulkType_in );
}


//==============================================================================
void TransportCoefficients::initialize_eta(string & etaType_in)
{
  // set chosen eta parameterization
  etaType = etaType_in;

  // assign corresponding function
  if (etaType == "default")
  {
    eta = [this]{ return default_eta(); };
  }
  else if (etaType == "constant")
  {
    eta_T_OV_w_IN = stod(etaOption);
    eta = [this]{ return constEta(); };
  }
  else if (etaType == "JakiParam")
  {
    eta = [this]{ return JakiParam(); };
  }
  else if (etaType == "LinearMus")
  {
    eta = [this]{ return LinearMusParam(); };
  }
  else if (etaType == "interpolate")
  {
    //use etaOption to find directory of table, then
    // execute interpolation
    eta = [this]{ return InterpolantWrapper(); };
  }
  else if (etaType == "NoShear")
  {
    eta = [this]{ return NoShear(); };
  }
  else
  {
    cout << "Shear viscosity specification not recognized. Now exiting.\n";
    exit(1);
  }
}


//==============================================================================
void TransportCoefficients::initialize_tauShear(string & tauShearType_in)
{
  tauShearType = tauShearType_in;

  if (tauShearType == "default")
  {
    tauShear = [this]{ return default_tauShear(); };
  }
  else if (tauShearType == "minVal")
  {
    tauShear = [this]{ return tauShearMinval(); };
  }
  else if (tauShearType == "Gubser")
  {
    /* these consistency checks maybe should be done in settings.h? */
    if (etaType != "constant")
    {
      std::cout << "Shear viscosity must be constant for Gubser. "
              "Check Input_Parameters.  Now exiting.\n";
      exit(1);
    }
    if (zetaType != "NoBulk")
    {
      std::cout << "Bulk viscosity must be zero"
                   " for Gubser. Check Input_Parameters.  "
                   " Now exiting.\n";
      exit(1);
    }
    tauShear = [this]{ return tauShearGubser(); };
  }
  else
  {
    cout << "Tau shear specification not recognized. Now exiting.\n";
    exit(1);
  }
}


//==============================================================================
void TransportCoefficients::initialize_zeta(string & zetaType_in)
{
  zetaType     = zetaType_in;

  if (zetaType == "default")
  {
    zeta = [this]{ return default_zeta(); };
  }
  else if (zetaType == "DNMR")
  {
    zeta = [this]{ return zeta_DNMR_LeadingMass(); };
  }
  else if (zetaType == "NoBulk")
  {
    zeta = [this]{ return NoBulk(); };
  }
  else if (zetaType == "interpolate")
  {
    //use zetaOption to find directory of table, then
    // execute interpolation
    zeta = [this]{ return InterpolantWrapper(); };
  }
  else
  {
    cout << "Bulk viscosity specification not recognized. Now exiting.\n";
    exit(1);
  }
}


//==============================================================================
void TransportCoefficients::initialize_tauBulk(string & tauBulkType_in)
{
  tauBulkType  = tauBulkType_in;

  if (tauBulkType == "default")
  {
    tauBulk = [this]{ return default_tauBulk(); };
  }
  else if (tauBulkType == "DNMR")
  {
    tauBulk = [this]{ return tauBulk_DNMR_LeadingMass(); };
  }
  else 
  {
    cout << "Tau bulk specification not recognized. Now exiting.\n";
    exit(1);   
  }
}




//==============================================================================
//==============================================================================
// Possible function choices for eta

//===============================
double TransportCoefficients::default_eta()
{
  double eta_over_s = 0.20;
  return therm.s * eta_over_s;
}

//===============================
double TransportCoefficients::constEta()
{
  double w = therm.w;
  double T = therm.T;
  return eta_T_OV_w_IN*(w/T);
}

//===============================
double TransportCoefficients::JakiParam()
{
  //picked the easiest one with functional dependence
  // parameters hardcoded for now.. just to see how it works

  double T     = therm.T;
  double s     = therm.s;
  double TC    = 155.0/hbarc_MeVfm; // 173.9/197.3
  double z     = pow(0.66*T/TC,2);
  double alpha = 33./(12.*PI)*(z-1)/(z*log(z));
  return s*(0.0416762/pow(alpha,1.6)+ 0.0388977/pow(T,5.1) );
}

//===============================
double TransportCoefficients::LinearMusParam()
{
  // parameters hardcoded for now.. just to see how it works
  double etaBase = 0.08;
  double muSlope = 0.0033;
  double muB = therm.muB;
  double muS = therm.muS;
  double muQ = therm.muQ;
  double w = therm.w;
  double T = therm.T;

  return (etaBase + muSlope*(muB + muS + muQ))*(w/T);
}

//===============================
double TransportCoefficients::InterpolantWrapper() { return 0.0; }

//===============================
double TransportCoefficients::NoShear() { return 0.0; }


//==============================================================================
//==============================================================================
// Possible function choices for shear relaxation

//===============================
double TransportCoefficients::default_tauShear() { return std::max( 5.0*eta()/therm.w, 0.005 ); }

//===============================
double TransportCoefficients::tauShearGubser() { return (5.0*eta())/therm.w; }

//===============================
double TransportCoefficients::tauShearMinval() { return std::max( (5.0*eta())/therm.w, 0.001 ); }



//==============================================================================
//==============================================================================
// Possible function choices for zeta


//===============================
double TransportCoefficients::default_zeta()
{
  double zeta_over_s = 0.005;
  return therm.s * zeta_over_s;
}

//===============================
double TransportCoefficients::zeta_DNMR_LeadingMass()
{
    //add this in later.. for now no bulk
    return 0.0;
}

//===============================
double TransportCoefficients::NoBulk() { return 0.0; }

//==============================================================================
//==============================================================================
// Possible function choices for bulk relaxation

//===============================
double TransportCoefficients::default_tauBulk()
{
  return std::max( 5.0*zeta()/(pow((1.0-therm.cs2),2.0)*therm.w), 0.1 );
}

//===============================
double TransportCoefficients::tauBulk_DNMR_LeadingMass() { return 0.0; }
