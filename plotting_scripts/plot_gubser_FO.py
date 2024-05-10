#!/usr/bin/env python
import sys
import os
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

def read_sim(sim_result_path):
    dt=.001
    col_names=['divEener','divE_x','divE_y', 'gsub', "uout_x", "uout_y",
                "swsub", "bulksub", "pit_t", "pi_xx", "pi_yy", "pi_zz", "pi_xy",
                "t", "x", "y", "s", "e", "T", "muB", "muS", "muQ", "w","cs2"]

    df = pd.read_table(sim_result_path,
                        names=col_names,sep=' ',header=0)

    #x = np.linspace(df['x'].min(), df['x'].max(), len(df['x'].unique()))
    #y = np.linspace(df['y'].min(), df['y'].max(), len(df['y'].unique()))
    #x, y = np.meshgrid(x, y)
    #Create 3D scatter plot of rsub_x, rsub_y, and rsub_z
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.view_init(elev=45, azim=45)
    scatter = ax.scatter(df['x'], df['y'], df['t'], c=df['T'],marker='o',s=1)

    ax.quiver(df['x'][::10], df['y'][::10], df['t'][::10],
              -df['divE_y'][::10], -df['divE_x'][::10], df['divEener'][::10],
              length=0.1, normalize=True)

    ax.set_xlabel('x [fm]')
    ax.set_ylabel('y [fm]')
    ax.set_zlabel(r'$\tau$ [fm]')

    threshold = 0.001  # adjust as needed
    df = df[np.abs(df['y']) < threshold]
    fig2, ax2 = plt.subplots()
    scatter = ax2.scatter(df['x'], df['t'], c=df['T'], marker='o', s=5)
    ax2.quiver(df['x'], df['t'], df['divE_x'], df['divEener'], scale=20)
    print(df['divEener']**2 - df['divE_y']**2-df['divE_x']**2)
    #print(df)
    #           normalize=True)
    ax2.set_xlabel('x [fm]')
    ax2.set_ylabel(r'$\tau$ [fm]')
    ax2.set_xlim(-2.2,2.2)


    cbar = plt.colorbar(scatter)
    cbar.set_label('T [GeV]')
    fig.savefig("FO_surface_gubser.png")
    fig2.savefig("FO_surface_gubser_y=0.png")


if __name__ == '__main__':
    simulation_folder = sys.argv[1]
    read_sim(simulation_folder)
    # if (len(sys.argv) < 4):
        # fig.savefig('test.png')
    # else:
        # fig.savefig(sys.argv[3])