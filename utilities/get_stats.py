#!/usr/bin/env python
# -*- coding: utf-8 -*-
#Author: Willian M. Serenone

import os
import sys
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np

#Plot poarameters
plot_title="Gubser flow at finite $\mu_B$ - Conformal EoS"


def check_args():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <sim_folder>")
        sys.exit(1)

def load_data(fname_sim):
    cols_sim = ["id", "tau", "x", "y", "p", "T", "muB", "muS", "muQ", "e", 
                "rhoB", "rhoS", "rhoQ", "s", "smoothed_s", "specific_s", "sigma",
                "norm_spec_s", "stauRelax", "bigtheta", "??", "??2", "shv00", 
                "shv11", "shv22", "shv12", "tau2*shv33", "vx", "vy", "gamma", 
                "Freeze", "EOS"]
#    cols_analytical = ["x", "y", "eta", "e", "rhoB", "rhoS", "rhoQ", "ux",
#                       "uy", "ueta", "Pi", "pixx", "pixy", "pixeta", "piyy", "piyeta", "pietaeta"]
#    data_analytical = pd.read_table(fname_analytical, sep=" ", names=cols_analytical, header=0)


    data_sim = pd.read_csv(fname_sim, sep=" ", names=cols_sim, header=0)
    data_sim["ux"] = data_sim["vx"]*data_sim["gamma"]
    data_sim["uy"] = data_sim["vy"]*data_sim["gamma"]
    data_sim["e"] = data_sim["e"]/1000 #Convert to GeV/fm^3
    data_sim["shv11"] = data_sim["shv11"]*.197
    data_sim["shv22"] = data_sim["shv22"]*.197
    data_sim["tau2*shv33"] = data_sim["tau2*shv33"]*.197/data_sim["tau"]**2

    return data_sim

def get_folder_names(sim_folder, time,dt):
    itime = int((time-1.)/dt)
    #fname_analytical = f"{analytical_folder}/tau={time:4.2f}.txt"
    fname_sim = f"{sim_folder}/system_state_{itime}.dat"
    skip = False
    #print(fname_analytical, fname_sim)

    #if (not os.path.exists(fname_analytical)) or (not os.path.exists(fname_sim)):
    #    skip = True
    return fname_sim #, fname_sim, skip


def get_stat(sim_data):
    xmax = np.abs(sim_data['x']).max()
    ymax = np.abs(sim_data['y']).max()

    return xmax, ymax

def main():
    check_args()
    sim_folder = sys.argv[1]

    list_times = [1+i*.005 for i in range(263) ]
    xmax = 0
    ymax = 0
    for t in list_times:
        #Load the output files
        dt=0.005
        fname_sim = get_folder_names(sim_folder, t,dt)
        data_sim = load_data(fname_sim)
        x, y = get_stat(data_sim)
        xmax = max(x,xmax)
        ymax = max(y,ymax)

    print(xmax,ymax)
if __name__ == "__main__":
    main()
