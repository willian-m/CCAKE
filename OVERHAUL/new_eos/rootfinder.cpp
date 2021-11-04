









double Rootfinder::s_out(double ein, double Bin, double Sin, double Qin)
{   //update the t and mu position based on input. Returns entropy if found, returns -1 if failed
    if (rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
	///////////////////////////
	if (accept_nearest_neighbor)
		std::cout << "Should not have made it here!" << std::endl;
	if (discard_unsolvable_charge_densities)
		return -1.0;//!!!!!!!!!!!!
	///////////////////////////

    double t0 = tbqsPosition[0];
    double mub0 = tbqsPosition[1];
    double muq0 = tbqsPosition[2];
    double mus0 = tbqsPosition[3];
    double t10 = t0*.2;
    double muB10 = mub0*.2;
    double muQ10 = muq0*.2;
    double muS10 = mus0*.2;

    //perturb T
    if(t0 + t10 > maxT) {
        tbqs(maxT - 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 + t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(t0 - t10 < minT) {
        tbqs(minT + 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 - t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturb mub
    if(mub0 + muB10 > maxMuB) {
        tbqs(t0, maxMuB - 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 + muB10, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(mub0 - muB10 < minMuB) {
        tbqs(t0, minMuB + 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 - muB10, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturn muq
    if(muq0 + muQ10 > maxMuQ) {
        tbqs(t0, mub0, maxMuQ - 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 + muQ10, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(muq0 - muQ10 < minMuQ) {
        tbqs(t0, mub0, minMuQ + 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 - muQ10, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturb mus
    if(mus0 + muS10 > maxMuS) {
        tbqs(t0, mub0, muq0, maxMuS - 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 + muS10);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(mus0 - muS10 < maxMuS) {


        tbqs(t0, mub0, muq0, minMuS + 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 - muS10);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //check mu = 0
    tbqs(t0, 0, 0, 0);
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    tbqs(t0, mub0, muq0, mus0);
    return -1;
}







bool update_s(double sin, double Bin, double Sin, double Qin)
{ //update the t and mu position based on input. Returns 1 if found, returns 0 if failed
    if (rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
	///////////////////////////
	if (accept_nearest_neighbor)
		std::cout << "Should not have made it here!" << std::endl;
	if (discard_unsolvable_charge_densities)
		return false;//!!!!!!!!!!!!
	///////////////////////////
    double t0 = tbqsPosition[0];
    double mub0 = tbqsPosition[1];
    double muq0 = tbqsPosition[2];
    double mus0 = tbqsPosition[3];
    double t10 = t0*.2;
    double muB10 = mub0*.2;
    double muQ10 = muq0*.2;
    double muS10 = mus0*.2;

    //perturb T
    if(t0 + t10 > maxT) {
        tbqs(maxT - 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 + t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(t0 - t10 < minT) {
        tbqs(minT + 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 - t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturb mub
    if(mub0 + muB10 > maxMuB) {
        tbqs(t0, maxMuB - 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 + muB10, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(mub0 - muB10 < minMuB) {
        tbqs(t0, minMuB + 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 - muB10, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturn muq
    if(muq0 + muQ10 > maxMuQ) {
        tbqs(t0, mub0, maxMuQ - 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 + muQ10, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(muq0 - muQ10 < minMuQ) {
        tbqs(t0, mub0, minMuQ + 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 - muQ10, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturb mus
    if(mus0 + muS10 > maxMuS) {
        tbqs(t0, mub0, muq0, maxMuS - 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 + muS10);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(mus0 - muS10 < maxMuS) {
        tbqs(t0, mub0, muq0, minMuS + 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 - muS10);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //check mu = 0
    tbqs(t0, 0, 0, 0);
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    tbqs(t0, mub0, muq0, mus0);
    return false;
}




////////////////////////////////////////////////////////////////////////////////
int rootfinder_fsbqs(const gsl_vector *x, void *params, gsl_vector *f)
{
    //x contains the next (T, muB, muS) coordinate to test
    vector<double> tbqsToEval(4);
    tbqsToEval[0] = gsl_vector_get(x,0);
    tbqsToEval[1] = gsl_vector_get(x,1);	// convert x into densevector so it
    tbqsToEval[2] = gsl_vector_get(x,2);	// can be a BSpline evaluation point
    tbqsToEval[3] = gsl_vector_get(x,3);


    double entrGiven, rhoBGiven, rhoQGiven, rhoSGiven, entr, rhoB, rhoQ, rhoS;
    entrGiven = ((rootfinder_parameters*)params)->eorEntGiven;
    rhoBGiven = ((rootfinder_parameters*)params)->rhoBGiven;            //given variables contain the target point
    rhoQGiven = ((rootfinder_parameters*)params)->rhoQGiven;
    rhoSGiven = ((rootfinder_parameters*)params)->rhoSGiven;
{
	double phase_diagram_point[4] = {tbqsToEval[0]*hbarc_MeVfm, tbqsToEval[1]*hbarc_MeVfm,
					 tbqsToEval[3]*hbarc_MeVfm, tbqsToEval[2]*hbarc_MeVfm};	// NOTE: S <<-->> Q swapped!!!
	double densities_at_point[4];
	get_sBSQ_densities(phase_diagram_point, densities_at_point);
	entr = densities_at_point[0];
	rhoB = densities_at_point[1];
	rhoS = densities_at_point[2];
	rhoQ = densities_at_point[3];

}

    gsl_vector_set(f, 0, (entr - entrGiven)); //f[0] contains (s(T,muB,muQ,muS) - sGiven)
    gsl_vector_set(f, 1, (rhoB - rhoBGiven)); //f[1] contains (rhoB(T,muB,muQ,muS) - rhoBGiven)
    gsl_vector_set(f, 2, (rhoQ - rhoQGiven)); //f[2] contains (rhoQ(T,muB,muQ,muS) - rhoQGiven)
    gsl_vector_set(f, 3, (rhoS - rhoSGiven)); //f[2] contains (rhoS(T,muB,muQ,muS) - rhoSGiven)

    return GSL_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
int rootfinder_febqs(const gsl_vector *x, void *params, gsl_vector *f)
{
    //x contains the next (T, muB, muQ, muS) coordinate to test
    vector<double> tbqsToEval(4);
    tbqsToEval[0] = gsl_vector_get(x,0);
    tbqsToEval[1] = gsl_vector_get(x,1);	// convert x into densevector so it
    tbqsToEval[2] = gsl_vector_get(x,2);	// can be a BSpline evaluation point
    tbqsToEval[3] = gsl_vector_get(x,3);

    double eGiven, rhoBGiven, rhoQGiven, rhoSGiven, e, rhoB, rhoQ, rhoS;
    eGiven = ((rootfinder_parameters*)params)->eorEntGiven;
    rhoBGiven = ((rootfinder_parameters*)params)->rhoBGiven;            //given variables contain the target point
    rhoQGiven = ((rootfinder_parameters*)params)->rhoQGiven;
    rhoSGiven = ((rootfinder_parameters*)params)->rhoSGiven;
{
	double phase_diagram_point[4] = {tbqsToEval[0]*hbarc_MeVfm, tbqsToEval[1]*hbarc_MeVfm,
					 tbqsToEval[3]*hbarc_MeVfm, tbqsToEval[2]*hbarc_MeVfm};	// NOTE: S <<-->> Q swapped!!!
	double densities_at_point[4];
	get_eBSQ_densities(phase_diagram_point, densities_at_point);
	e = densities_at_point[0]/hbarc_MeVfm;
	rhoB = densities_at_point[1];
	rhoS = densities_at_point[2];
	rhoQ = densities_at_point[3];
}

    gsl_vector_set(f, 0, (e - eGiven)); //f[0] contains (e(T,muB,muQ,muS) - eGiven)
    gsl_vector_set(f, 1, (rhoB - rhoBGiven)); //f[1] contains the (rhoB(T,muB,muQ,muS) - rhoBGiven)
    gsl_vector_set(f, 2, (rhoQ - rhoQGiven)); //f[2] contains the (rhoQ(T,muB,muQ,muS) - rhoQGiven)
    gsl_vector_set(f, 3, (rhoS - rhoSGiven)); //f[2] contains the (rho2(T,muB,muQ,muS) - rhoSGiven)

    return GSL_SUCCESS;
}


bool rootfinder4D(double e_or_s_Given, int e_or_s_mode,
						double rhoBGiven, double rhoSGiven, double rhoQGiven,
						double error, size_t steps)
{
    //declare x = (T, muB, muQ, muS)
    gsl_vector *x = gsl_vector_alloc(4);

	// use NMN method to estimate where to start the rootfinder
	// ( returns estimates in units of MeV )
	vector<double> T_muB_muQ_muS_estimates;
	constexpr bool use_normalized_trees = true;
	if ( e_or_s_mode==1 )
		e_delaunay.get_NMN_coordinates(
					{e_or_s_Given*hbarc_MeVfm, rhoBGiven, rhoSGiven, rhoQGiven},
					T_muB_muQ_muS_estimates, use_normalized_trees );
	else
		entr_delaunay.get_NMN_coordinates(
					{e_or_s_Given, rhoBGiven, rhoSGiven, rhoQGiven},
					T_muB_muQ_muS_estimates, use_normalized_trees );


    gsl_vector_set(x, 0, T());
    gsl_vector_set(x, 1, muB());
    gsl_vector_set(x, 2, muQ());
    gsl_vector_set(x, 3, muS());


    //initialize the rootfinder equation to the correct variable quantities
    bool isEntropy = false;
    if(e_or_s_mode == 0) {
        isEntropy = true;
    }
    rootfinder_parameters p;
	if(isEntropy) {
		p.set( e_or_s_Given, rhoBGiven, rhoQGiven, rhoSGiven);
	} else {
		p.set( e_or_s_Given, rhoBGiven, rhoQGiven, rhoSGiven);
	}

    //initialize multiroot solver
    gsl_multiroot_fsolver *solver;
    gsl_multiroot_function f;
    if(isEntropy) {
        f.f = &rootfinder_fsbqs;
    } else {
        f.f = &rootfinder_febqs;
    }
    f.n = 4;
    f.params = &p;

    solver = gsl_multiroot_fsolver_alloc(TYPE, 4);
    gsl_multiroot_fsolver_set(solver, &f, x);

    int status;
    size_t iter = 0;
	double previous_solver_step[4];

    do
	{
		for (int iSolverElem = 0; iSolverElem < 4; iSolverElem++)
			previous_solver_step[iSolverElem] = gsl_vector_get(solver->x, iSolverElem);

        ++iter;
        status = gsl_multiroot_fsolver_iterate(solver);

        if(VERBOSE > 5 && status)
		{
			if ( status == GSL_EBADFUNC && VERBOSE > 5 )
				std::cout << "Error: something went to +/-Inf or NaN!" << std::endl;
			else if ( status == GSL_ENOPROG && VERBOSE > 5 )
				std::cout << "Error: not making enough progress!" << std::endl;
            //break if the rootfinder gets stuck
			break;
        }

		//break if the rootfinder goes out of bounds
        if(gsl_vector_get(solver->x, 0) < minT)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (T < minT)!" << std::endl;
			status = -10;
			break;
        }
		else if(gsl_vector_get(solver->x, 0) > maxT)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (T > maxT)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 1) < minMuB)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuB < minMuB)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 1) > maxMuB)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuB > maxMuB)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 2) < minMuQ)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuQ < minMuQ)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 2) > maxMuQ)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuQ > maxMuQ)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 3) < minMuS)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuS < minMuS)!" << std::endl;
			status = -10;
			break;
        }
		else if (gsl_vector_get(solver->x, 3) > maxMuS)
		{
			if ( VERBOSE > 5 )
				std::cout << "Error: out-of-bounds (MuS > maxMuS)!" << std::endl;
			status = -10;
			break;
        }

        status = gsl_multiroot_test_residual(solver->f, error);

    } while (status == GSL_CONTINUE && iter < steps);

//std::cout << "Exited GSL loop" << endl;

    bool found = true; //to return variable
    if ( iter >= steps || status != 0 )
	{
		if ( status == GSL_EBADFUNC )
			std::cout << "Error: something went to +/-Inf or NaN!" << std::endl;
		else if ( status == GSL_ENOPROG )
			std::cout << "Error: not making enough progress!" << std::endl;
		else
			std::cout << "Check: " << iter << "   " << steps << "   " << status << std::endl;
        found = false;
    }


    if ( found )
	{
        tbqs( gsl_vector_get(solver->x, 0),
			  gsl_vector_get(solver->x, 1),
			  gsl_vector_get(solver->x, 2),
			  gsl_vector_get(solver->x, 3) );    //set T, muB, muQ, muS
    }
	


    //memory deallocation
    gsl_multiroot_fsolver_free(solver);
    gsl_vector_free(x);
    return found;
}
