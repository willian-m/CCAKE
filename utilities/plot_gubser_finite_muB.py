#!/usr/bin/env python
# -*- coding: utf-8 -*-
#Author: Willian M. Serenone

import os
import sys
from matplotlib import pyplot as plt
import pandas as pd

#Plot poarameters
plot_title="Gubser flow at finite $\mu_B$ - Conformal EoS"


def check_args():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <sim_folder> <analytical_folder> <output.png>")
        sys.exit(1)

def load_data(fname_analytical, fname_sim):
    cols_sim = ["id", "tau", "x", "y", "p", "T", "muB", "muS", "muQ", "e", 
                "rhoB", "rhoS", "rhoQ", "s", "smoothed_s", "specific_s", "sigma",
                "norm_spec_s", "stauRelax", "bigtheta", "??", "??2", "shv00", 
                "shv11", "shv22", "shv12", "tau2*shv33", "vx", "vy", "gamma", 
                "Freeze", "EOS"]
    cols_analytical = ["x", "y", "eta", "e", "rhoB", "rhoS", "rhoQ", "ux",
                       "uy", "ueta", "Pi", "pixx", "pixy", "pixeta", "piyy", "piyeta", "pietaeta"]
    data_analytical = pd.read_table(fname_analytical, sep=" ", names=cols_analytical, header=0)


    data_sim = pd.read_csv(fname_sim, sep=" ", names=cols_sim, header=0)
    data_sim["ux"] = data_sim["vx"]*data_sim["gamma"]
    data_sim["uy"] = data_sim["vy"]*data_sim["gamma"]
    data_sim["e"] = data_sim["e"]/1000 #Convert to GeV/fm^3
    data_sim["shv11"] = data_sim["shv11"]*.197
    data_sim["shv22"] = data_sim["shv22"]*.197
    data_sim["tau2*shv33"] = data_sim["tau2*shv33"]*.197/data_sim["tau"]**2

    return data_analytical, data_sim

def get_folder_names(sim_folder, analytical_folder, time,dt):
    itime = int((time-1.)/dt)
    fname_analytical = f"{analytical_folder}/tau={time:4.2f}.txt"
    fname_sim = f"{sim_folder}/system_state_{itime}.dat"
    skip = False
    print(fname_analytical, fname_sim)

    if (not os.path.exists(fname_analytical)) or (not os.path.exists(fname_sim)):
        skip = True
    return fname_analytical, fname_sim, skip

def create_figure():
    fig, ax = plt.subplots(2, 3, figsize=(15, 8),
                           sharex=True, sharey=False, 
                           gridspec_kw={"hspace": 0.0, "wspace": 0.4})
    #Set labels size
    for i in range(2):
        for j in range(3):
            ax[i][j].tick_params(axis='both', which='major', labelsize=15)
            ax[i][j].xaxis.label.set_size(15)
            ax[i][j].yaxis.label.set_size(15)
            ax[i][j].set_xlim(-4,4)

    #Set labels
    ax[0][0].set_ylabel(r"$\varepsilon$ (GeV/fm$^3$)")
    ax[0][0].set_xlabel(r"$x$ (fm)")
    ax[0][1].set_ylabel(r"$\rho_B$ (fm$^{-3}$)")
    ax[0][1].set_xlabel(r"$x$ (fm)")
    ax[0][2].set_ylabel(r"$u^x$")
    ax[0][2].set_xlabel(r"$x$ (fm)")
    ax[1][0].set_ylabel(r"$\pi^{xx}$ (GeV/fm$^3$)")
    ax[1][0].set_xlabel(r"$x$ (fm)")
    ax[1][1].set_ylabel(r"$\pi^{yy}$ (GeV/fm$^3$)")
    ax[1][1].set_xlabel(r"$x$ (fm)")
    ax[1][2].set_ylabel(r"$\tau^2 \pi^{\eta\eta}$ (GeV/fm$^3$)")
    ax[1][2].set_xlabel(r"$x$ (fm)")

    ax[0][2].axhline(0, color="black", linestyle="--", linewidth=2)

    #Give figure a title
    fig.suptitle(plot_title, fontsize=20)

    return fig, ax

def plot_data(ax, data_analytical, data_sim):
    ana_cutted=data_analytical.query("abs(y) < 0.01")
    sim_cutted=data_sim.query("abs(y) < 0.01")
    ax[0][0].plot(ana_cutted["x"], ana_cutted["e"], label="Analytical",color="blue")
    ax[0][0].plot(sim_cutted["x"], sim_cutted["e"], label="Simulation",
                  color="red",linestyle="--")
    ax[0][1].plot(ana_cutted["x"], ana_cutted["rhoB"], label="Analytical",color="blue")
    ax[0][1].plot(sim_cutted["x"], sim_cutted["rhoB"], label="Simulation",
                  color="red",linestyle="--")
    ax[0][2].plot(ana_cutted["x"], ana_cutted["ux"], label="Analytical",color="blue")
    ax[0][2].plot(sim_cutted["x"], sim_cutted["ux"], label="Simulation",
                  color="red",linestyle="--")
    ax[1][0].plot(ana_cutted["x"], ana_cutted["pixx"], label="Analytical",color="blue")
    ax[1][0].plot(sim_cutted["x"], sim_cutted["shv11"], label="Simulation",
                  color="red",linestyle="--")
    ax[1][1].plot(ana_cutted["x"], ana_cutted["piyy"], label="Analytical",color="blue")
    ax[1][1].plot(sim_cutted["x"], sim_cutted["shv22"], label="Simulation",
                  color="red",linestyle="--")
    ax[1][2].plot(ana_cutted["x"], ana_cutted["pietaeta"], label="Analytical",color="blue")
    ax[1][2].plot(sim_cutted["x"], sim_cutted["tau2*shv33"], label="Simulation",
                  color="red",linestyle="--")


def main():
    check_args()
    sim_folder = sys.argv[1]
    analytical_folder = sys.argv[2]
    output_file = sys.argv[3]

    list_times = [1.0, 1.1, 1.2, 1.3, 1.4]#, 1.5, 1.6]#, 1.7]
    fig, ax = create_figure()
    for t in list_times:
        #Load the output files
        dt=0.01/2
        fname_analytical, fname_sim, skip = get_folder_names(sim_folder, analytical_folder, t,dt)
        if skip:
            continue
        data_analytical, data_sim = load_data(fname_analytical, fname_sim)

        plot_data(ax, data_analytical, data_sim)
    fig.savefig(output_file)


if __name__ == "__main__":
    main()
