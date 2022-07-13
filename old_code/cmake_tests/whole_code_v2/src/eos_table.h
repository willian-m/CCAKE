#ifndef EOS_TABLE_H
#define EOS_TABLE_H

#include <algorithm>
#include <iterator>
#include <string>

#include "eos_delaunay/eos_delaunay.h"
#include "interpolatorND/interpolatorND.h"
#include "constants.h"
#include "eos_base.h"
#include "eos_header.h"

using namespace constants;

class EoS_table: public EoS_base
{
private:
  string equation_of_state_table_filename = "";

  static InterpolatorND<4> equation_of_state_table;

  void init_grid_ranges_only(string quantityFile, string derivFile);

  void get_eBSQ_densities_from_interpolator( double point[], double densities[] );
  void get_sBSQ_densities_from_interpolator( double point[], double densities[] );

  void get_eBSQ_safe( const double point[], double results[] );
  void get_sBSQ_safe( const double point[], double results[] );
  void get_full_thermo_safe( const double point[], double results[] );

  bool point_is_in_range_no_ext( double setT, double setmuB, double setmuQ, double setmuS );


public:
  // default constructor/destructor
  EoS_table(){}
  virtual ~EoS_table(){}

  // constructor from files containing tables
  EoS_table( string quantityFile, string derivFile );

  void get_eBSQ( double point[], double results[] );
  void get_sBSQ( double point[], double results[] );
  void get_full_thermo( double point[], double results[] );
};

#endif