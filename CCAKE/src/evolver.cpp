#include "evolver.h"

using namespace ccake;

//Template instantiations
template class Evolver<1>;
template class Evolver<2>;
template class Evolver<3>;

template <unsigned int D>
Evolver<D>::Evolver(std::shared_ptr<Settings> settingsPtr_in,
                    std::shared_ptr<SystemState<D>> systemPtr_in)
                 : settingsPtr(settingsPtr_in), 
                   systemPtr(systemPtr_in){};

template <unsigned int D>
void Evolver<D>::allocate_cache()
{
    n_particles = systemPtr->n_particles;
    
    //Allocate on device memory for the system state before
    //evolution begins.
    evolver_cache = Cabana::AoSoA<EvolverCache, DeviceType, VECTOR_LENGTH>("cache", n_particles);
}

template <unsigned int D>
void Evolver<D>::execute_timestep(double dt, int rk_order,
                                  std::function<void(void)> time_derivatives_functional )
{

      switch ( rk_order )
      {
        case 2:
          advance_timestep_rk2( dt, time_derivatives_functional );
          cout << "RK2" << endl;
          break;
//        case 4:
//          advance_timestep_rk4( dt, time_derivatives_functional );
//          break;
        default:
          std::cerr << "Invalid Runge-Kutta order!" << std::endl;
          exit(8);
          break;
      }
    return;

}

//==============================================================================
// this routine is used to initialize quantities prior to RK evolution
template <unsigned int D>
void Evolver<D>::set_current_timestep_quantities()
{

  CREATE_VIEW(device_, systemPtr->cabana_particles);

  auto shv = Cabana::slice<evolver_cache_info::viscous_shear>(evolver_cache);
  auto u = Cabana::slice<evolver_cache_info::four_velocity>(evolver_cache);
  auto r = Cabana::slice<evolver_cache_info::position>(evolver_cache);
  auto s = Cabana::slice<evolver_cache_info::specific_entropy>(evolver_cache);
  auto Bulk = Cabana::slice<evolver_cache_info::Bulk_pressure>(evolver_cache);
  //auto E0 = Cabana::slice<evolver_cache_info::E0>(evolver_cache);
  auto fill_cache = KOKKOS_LAMBDA (const int is, const int ia)
  {
    for (int i=1; i<D+1; i++)
    for (int j=1; j<D+1; j++)
      shv.access(is, ia,i-1,j-1) = device_hydro_spacetime_matrix.access(is, ia, hydro_info::shv, i, j);

    for(int idir = 0; idir < D; ++idir)
    {
      u.access(is, ia, idir) = device_hydro_vector.access(is, ia, ccake::hydro_info::u,idir);
      r.access(is, ia, idir) = device_position.access(is, ia, idir);
    }
    s.access(is, ia) = device_specific_density.access(is, ia, densities_info::s);
    Bulk.access(is, ia) = device_hydro_scalar.access(is, ia, hydro_info::Bulk);
    //E0(iparticle) = device_contribution_to_total_E(iparticle);
  };
  Cabana::simd_parallel_for(*(systemPtr->simd_policy), fill_cache, "fill_cache");
}

//==========================================================================
template <unsigned int D>
void Evolver<D>:: advance_timestep_rk2( double dt, 
                                        std::function<void(void)> time_derivatives_functional )
{
      double t0      = systemPtr->t;
      //double E0      = systemPtr->Ez;
      Kokkos::View<int, MemorySpace> E0("E0");
      Kokkos::View<int, MemorySpace> E("E");
      Kokkos::deep_copy(E0, systemPtr->Ez);
      Kokkos::deep_copy(E, systemPtr->Ez);
      // initialize quantities at current time step
      set_current_timestep_quantities();
 
      ////////////////////////////////////////////
      //    first step
      ////////////////////////////////////////////
      formatted_output::report("RK(n=2) evolution, step 1");

      // compute derivatives
      time_derivatives_functional();

  	  //Create views for the device
      CREATE_VIEW(device_, systemPtr->cabana_particles);
      auto shv0 = Cabana::slice<evolver_cache_info::viscous_shear>(evolver_cache);
      auto u0 = Cabana::slice<evolver_cache_info::four_velocity>(evolver_cache);
      auto r0 = Cabana::slice<evolver_cache_info::position>(evolver_cache);
      auto s0 = Cabana::slice<evolver_cache_info::specific_entropy>(evolver_cache);
      auto Bulk0 = Cabana::slice<evolver_cache_info::Bulk_pressure>(evolver_cache);
      //auto E0 = Cabana::slice<evolver_cache_info::E0>(evolver_cache);

      auto update_rk2_step1 = KOKKOS_CLASS_LAMBDA(int iparticle){
        for (int idir=0; idir<D; ++idir){
          device_position(iparticle, idir) = r0(iparticle,idir) + 0.5*dt*device_hydro_vector(iparticle, hydro_info::v, idir);
          device_hydro_vector(iparticle, hydro_info::u, idir) = u0(iparticle,idir) + 0.5*dt*device_hydro_vector(iparticle, hydro_info::du_dt, idir);
          for (int jdir=0; jdir<D; ++jdir){
            device_hydro_spacetime_matrix(iparticle, hydro_info::shv, idir+1, jdir+1) 
              = shv0(iparticle,idir,jdir) + 0.5*dt*device_hydro_space_matrix(iparticle, hydro_info::dshv_dt, idir, jdir);
          }
        }
        device_specific_density(iparticle, densities_info::s) = s0(iparticle) + 0.5*dt*device_d_dt_spec(iparticle, densities_info::s);
        device_hydro_scalar(iparticle, hydro_info::Bulk) = Bulk0(iparticle) + 0.5*dt*device_hydro_scalar(iparticle, hydro_info::dBulk_dt);
        
        // regulate negative entropy if necessary
        if ( REGULATE_NEGATIVE_S && device_specific_density(iparticle, densities_info::s) < 0.0 )
        {
          device_specific_density(iparticle, densities_info::s) = 0.5*(s0(iparticle)+1.0);
        }

        // regulate large entropy if necessary
        if ( REGULATE_LARGE_S && device_specific_density(iparticle,densities_info::s) > 10.0*s0(iparticle) )
        {
          device_specific_density(iparticle, densities_info::s) = 2.0*s0(iparticle);
        }

          device_contribution_to_total_Ez(iparticle) = E0() + 0.5*dt*device_contribution_to_total_dEz(iparticle);
      };
      Kokkos::parallel_for("update_rk2_step1", n_particles, update_rk2_step1);
      Kokkos::fence();
      //systemPtr->Ez = E0 + 0.5*dt*systemPtr->dEz;
      systemPtr->t  = t0 + 0.5*dt;
      
      //Update hydro time on device
      Kokkos::parallel_for("update_hydro_time", n_particles, KOKKOS_CLASS_LAMBDA(int iparticle){
        device_hydro_scalar(iparticle,ccake::hydro_info::t) = systemPtr->t;
      });
      Kokkos::fence();

      ////////////////////////////////////////////
      //    second step
      ////////////////////////////////////////////
      formatted_output::report("RK(n=2) evolution, step 2");

      // compute derivatives
      time_derivatives_functional();

      auto update_rk2_step2 = KOKKOS_CLASS_LAMBDA(int iparticle){
        for (int idir=0; idir<D; ++idir){
          device_position(iparticle, idir) = r0(iparticle,idir) + dt*device_hydro_vector(iparticle, hydro_info::v, idir);
          device_hydro_vector(iparticle, hydro_info::u, idir) = u0(iparticle,idir) + dt*device_hydro_vector(iparticle, hydro_info::du_dt, idir);
          for (int jdir=0; jdir<D; ++jdir){
            device_hydro_spacetime_matrix(iparticle, hydro_info::shv, idir+1, jdir+1) 
              = shv0(iparticle,idir,jdir) + dt*device_hydro_space_matrix(iparticle, hydro_info::dshv_dt, idir, jdir);
          }
        }
        device_specific_density(iparticle, densities_info::s) = s0(iparticle) + dt*device_d_dt_spec(iparticle, densities_info::s);
        device_hydro_scalar(iparticle, hydro_info::Bulk) = Bulk0(iparticle) + dt*device_hydro_scalar(iparticle, hydro_info::dBulk_dt);


        
        // regulate negative entropy if necessary
        if ( REGULATE_NEGATIVE_S && device_specific_density(iparticle, densities_info::s) < 0.0 )
        {
          device_specific_density(iparticle, densities_info::s) = 0.5*(s0(iparticle)+1.0);
        }

        // regulate large entropy if necessary
        if ( REGULATE_LARGE_S && device_specific_density(iparticle,densities_info::s) > 10.0*s0(iparticle) )
        {
          device_specific_density(iparticle, densities_info::s) = 2.0*s0(iparticle);
        }

          device_contribution_to_total_Ez(iparticle) = E0() + dt*device_contribution_to_total_dEz(iparticle);
      };
      Kokkos::parallel_for("update_rk2_step2", n_particles, update_rk2_step2);
      Kokkos::fence();
      //systemPtr->Ez = E0 + dt*systemPtr->dEz;
      systemPtr->t  = t0 + dt;
      
      //Update hydro time on device
      Kokkos::parallel_for("update_hydro_time", n_particles, KOKKOS_CLASS_LAMBDA(int iparticle){
        device_hydro_scalar(iparticle,ccake::hydro_info::t) = systemPtr->t;
      });
      Kokkos::fence();


      return;
}