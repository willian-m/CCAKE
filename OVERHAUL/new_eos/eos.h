#pragma once

//#include "read_in_hdf/read_in_hdf.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multiroots.h>

#include <fstream>
#include <string>

#include "eos_delaunay/eos_delaunay.h"

using std::string;


class EquationOfState
{
friend class InputOutput;

public:
    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC METHODS

    //Constructors:
    EquationOfState(string quantityFile, string derivFile);

    EquationOfState();
    void init();
    void init(string quantityFile, string derivFile);
    void init_grid_ranges_only(string quantityFile, string derivFile);
    void tbqs(double setT, double setmuB, double setmuQ, double setmuS);

    //getter functions for the quantities of interest at the current tbs/tbqs
    double T()   const;     //temperature
    double muB() const;   //baryon chemical potential
    double muQ() const;   //charge chemical potential
    double muS() const;   //strangeness chemical potential

    double p()   const;     //pressure density
    double s()   const;     //entropy density
    double B()   const;     //baryon density
    double S()   const;     //strangeness density
    double Q()   const;     //charge density
    double e()   const;     //energy density
    double cs2() const;   //speed of sound
    double w()   const;     //enthalpy

    double dwds();
    double dwdB();  //enthalpy derivatives **These still have not been checked**
    double dwdS();
    double dwdQ();

    void eosin(std::string type);
    double A();

    double efreeze(double TFO);
    double sfreeze(double TFO);

    double cs2out(double Tt, double muBin, double muQin, double muSin);
    double cs2out(double Tt);
    double wfz(double Tt, double muBin, double muQin, double muSin);
    double wfz(double Tt);
    double s_terms_T(double Tt); 

    //Rootfinder functions:
    bool update_s(double sin, double Bin, double Sin, double Qin);
    bool update_s(double sin);
    double s_out(double ein, double Bin, double Sin, double Qin);
    double s_out(double ein);

private:

    ////////////////////////////////////////////////////////////////////////////
    // PRIVATE MEMBERS

    //the current position in (T, muB, muQ, muS) initialized by tbqs()
    vector<double> tbqsPosition;

    //Rootfinding method used **THIS CAN BE CHANGED DEPENDING ON EOS
    const gsl_multiroot_fsolver_type *TYPE = gsl_multiroot_fsolver_hybrids;

    // string to hold input filenames
    string quantity_file = "";
    string deriv_file    = "";

    double pVal          = 0.0;
    double entrVal       = 0.0;
    double BVal          = 0.0;
    double SVal          = 0.0;
    double QVal          = 0.0;
    double eVal          = 0.0;
    double cs2Val        = 0.0;

    double db2           = 0.0;
    double ds2           = 0.0;
    double dq2           = 0.0;
    double dt2           = 0.0;
    double dbdq          = 0.0;
    double dbds          = 0.0;
    double dsdq          = 0.0;
    double dtdb          = 0.0;
    double dtds          = 0.0;
    double dtdq          = 0.0; //second derivative of pressure wrt i and j 
                                //where didj =: (d^2p)/(didj) or di2 = (d^2p)/((di)^2)

    double maxMuB        = 0.0;
    double minMuB        = 0.0;
    double maxMuQ        = 0.0;
    double minMuQ        = 0.0;
    double maxMuS        = 0.0;
    double minMuS        = 0.0;
    double maxT          = 0.0;
    double minT          = 0.0; //EOS range used for rootfinder checks

    ////////////////////////////////////////////////////////////////////////////
    // ROUTINES NEEDED FOR COMPUTING THERMODYNAMIC DERIVATIVES
    double dentr_dt();
    double dentr_dmub();
    double dentr_dmuq();
    double dentr_dmus();
    double db_dt();
    double db_dmub();
    double db_dmuq();
    double db_dmus();
    double ds_dt();
    double ds_dmub();
    double ds_dmuq();
    double ds_dmus();
    double dq_dt();
    double dq_dmub();
    double dq_dmuq();
    double dq_dmus();
    double calc_term_1();
    double calc_term_2(string i_char);
    double calc_term_3(string i_char);
    double calc_term_4(string j_char, string i_char);
    double deriv_mult_aTm_1b(gsl_vector* a, gsl_matrix* m, gsl_vector* b);

    
    ////////////////////////////////////////////////////////////////////////////
    // ROUTINES FOR DEBUGGING (uncomment to call)
    //void check_EoS_derivatives();
    //void get_toy_thermo(double point[], double thermodynamics[]);
    

    ////////////////////////////////////////////////////////////////////////////
    // ROUTINES TO FIND (T,muX) COORDINATES OF (e,rhoX) POINT
    // - for using the root-finding functionality
    Rootfinder rf;
    // - for using a Delaunay interpolation
    eos_delaunay e_delaunay;
    eos_delaunay entr_delaunay;

};
