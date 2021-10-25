#include "kernel.h"

namespace kernel
{

  void set_kernel_parameters( double h )
  {
//    knorm       = 10/7./pi/(_h*_h);
//    kgrad       = -10.0/7./pi/pow(_h,3)*3/4.;
//    kgrad2      = 10.0/7./pi/pow(_h,3)/_h;
    knorm       = 10.0/(7.*pi*h*h);
    knorm2      = knorm*0.25;
    kgrad       = -15.0/(14.0*pi*h*h*h)
    kgrad2      = -10.0/(7.0*pi*h*h*h*h)

    return;
  }

  double kernel( Vector<double,2> a )
  {
    double r = Norm(a);
    double q = r/_h;

    if ( q >= 2.0 )
      return 0.0;
    if ( q >= 1.0 )
      return knorm2*(2.0-q)*(2.0-q)*(2.0-q);

    double qq=q*q;
    return knorm*(1 - 1.5*qq + 0.75*q*qq);
  }

  Vector<double,2> gradKernel( Vector<double,2> a )
  {
    Vector<double,2> tsubb;
    double r = Norm(a);
    double q = r/_h;

    if ( q >= 2.0 )
      return tsubb;
    if ( q >= 1.0 )
      return kgrad*(2.0-q)*(2.0-q)*a/r;

    return kgrad2*( -3.0+2.25*q )*a;
  }

}