#ifndef SPH_WORKSTATION_H
#define SPH_WORKSTATION_H

#include "kernel.h"
#include "settings.h"
#include "system_state.h"

class SPHWorkstation
{
friend class InputOutput;

private:
  
  static constexpr int    VERBOSE        = 0;
  static constexpr double TOLERANCE      = 0.0;
  static constexpr bool   REGULATE_LOW_T = false;

  SystemState     * systemPtr            = nullptr;
  Settings        * settingsPtr          = nullptr;

  EquationOfState eos;


public:

  // default constructor/destructor
  SPHWorkstation(){}
  ~SPHWorkstation(){}

  // initialize pointers
  void set_SystemStatePtr( SystemState * systemPtr_in );
  void set_SettingsPtr( Settings * settingsPtr_in );

  // initialize workstation (now includes eos initialization)
  void initialize()
  {
    eos.init();

    settingsPtr->efcheck = eos.efreeze(settingsPtr->Freeze_Out_Temperature);
    settingsPtr->sfcheck = eos.sfreeze(settingsPtr->Freeze_Out_Temperature);
    systemPtr->efcheck   = settingsPtr->efcheck;
    systemPtr->sfcheck   = settingsPtr->sfcheck;

  }

  // routines for resetting quantities
  void reset_linklist() { systemPtr->linklist.reset(); }
  void reset_pi_tensor();

  void process_initial_conditions();
  void initialize_entropy_and_charge_densities();
  void initial_smoothing();


  // smoothing
  void smooth_fields( Particle & pa );
  void smooth_gradients( Particle & pa, double tin, int & count );

  // functions to apply action to all particles
  void smooth_all_particle_fields()
        { for ( auto & p : systemPtr->particles )
            smooth_fields(p); }

  void smooth_all_particle_gradients( int & count )
        { for ( auto & p : systemPtr->particles )
            smooth_gradients( p, systemPtr->t, count ); }

  void update_all_particle_thermodynamics()
        { for ( auto & p : systemPtr->particles )
            calcbsq( p ); }

  void update_all_particle_viscosities()
        { for ( auto & p : systemPtr->particles )
            p.setvisc( systemPtr->etaconst, systemPtr->bvf, systemPtr->svf,
                       systemPtr->zTc,      systemPtr->sTc, systemPtr->zwidth,
                       systemPtr->visc ); }

  void update_all_particle_fluid_quantities()
        { for ( auto & p : systemPtr->particles )
            p.update_fluid_quantities( systemPtr->t ); }

  void evaluate_all_particle_time_derivatives()
        { for ( auto & p : systemPtr->particles )
            p.evaluate_time_derivatives( systemPtr->t ); }


  int do_freezeout_checks();
  void update_all_particles_dsigma_dt();
  void update_freeze_out_lists();
  void finalize_freeze_out(int curfrz);


  // routines to edit particles directly
  double locate_phase_diagram_point_eBSQ(
          Particle & p, double e_In,
          double rhoB_In, double rhoS_In, double rhoQ_In );
  double locate_phase_diagram_point_eBSQ(
          Particle & p, double e_In );
  void locate_phase_diagram_point_sBSQ(
          Particle & p, double s_In,
          double rhoB_In, double rhoS_In, double rhoQ_In );
  void locate_phase_diagram_point_sBSQ(
          Particle & p, double s_In );

  void set_particle_thermo( Particle & p );
  void calcbsq( Particle & p );

  // freeze-out routines
  void bsqsvfreezeout(int curfrz);
  void bsqsvinterpolate(int curfrz);

  // misc. routine
  double gradPressure_weight(const int a, const int b);



  // Move this into a different namespace or something?
  // It feels like this should be organized separately
  void advance_timestep_rk2( double dt );
  void advance_timestep_rk4( double dt );
  void advance_timestep( double dt, int rk_order )
  {
    // turn on freeze-out flag initially
    systemPtr->cfon = 1;

    switch ( rk_order )
    {
      case 2:
        advance_timestep_rk2( dt );
        break;
      case 4:
        advance_timestep_rk4( dt );
        break;
      default:
        std::cerr << "Invalid Runge-Kutta order!" << std::endl;
        exit(8);
        break;
    }

    // set number of particles which have frozen out
    systemPtr->number_part = systemPtr->get_frozen_out_count();
//    std::cout << "Check termination conditions: "
//              << system.t << "   "
//              << settings.tend << "   "
//              << system.get_frozen_out_count() << "   "
//              << system.number_part << "   "
//              << system.n() << "   "
//              << ( system.t < settings.tend ) << "   "
//              << ( system.number_part < system.n() ) << endl;

    return;
  }

  //MOVE THIS TO ITS OWN CLASS USING TRAVIS' IMPROVEMENTS
  void get_time_derivatives();

  // decide whether to continue evolving
  bool continue_evolution()
  {
    return ( systemPtr->t < settingsPtr->tend )
            && ( systemPtr->number_part < systemPtr->n() );
  }

};



#endif