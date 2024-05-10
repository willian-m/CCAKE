#!/usr/bin/env python
import sys
import os
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np

#time_list=['1.00', '1.20', '1.40', '1.60']#, '1.80']#, '2.2' ]
#time_list=['1.70','1.80', '1.90', '2.00', '2.50' ]
time_list=['1.00', '1.10', '1.20', '1.30', '1.40', '1.50', '1.60']#, '2.00' ]
#time_list=['1.00', '1.20', '1.40', '1.60', '1.80', '2.00', '2.50', '3.00' ]
#time_list=['1.90', '2.00', '2.10']

#time_list=['2.00', '3.00', '4.00', '5.00']
dpi=150
fig, ax = plt.subplots(2,3,figsize=np.array([1920,1920*2/3])/dpi, sharex=True,gridspec_kw={'wspace':.5})

def read_sol(analytic_sol_folder):
    fig_pi, ax_pi = plt.subplot_mosaic(
        [['pi11','pi12'],['pi21','pi22']],
        figsize=np.array([1080,1080])/dpi, sharex=True,gridspec_kw={'wspace':.5}
        )
    for t in time_list:
        inp_path = os.path.join(analytic_sol_folder,'tau='+t+'.txt')
        df = pd.read_table(inp_path,names=
                      ['x', 'y', 'eta', 'e', 'rhoB', 'rhoS', 'rhoQ', 
                       'ux', 'uy', 'ueta', 'Bulk', 'pixx', 'pixy', 'pixeta', 
                       'piyy', 'pyeta', 'pietaeta'], sep=" ", engine='python',
                       header=1)

        df = df.query('abs(y) < 1.5e-2')
        ax[0][0].plot(df['x'],df['e'],label=r'$\tau = '+t+r'$ fm/c',color='blue')
        ax[0][1].plot(df['x'],df['rhoB'],label=r'$\tau = '+t+r'$ fm/c',color='blue')
        ax[0][2].plot(df['x'],df['ux'],label=r'$\tau = '+t+r'$ fm/c',color='blue')
        ax[1][0].plot(df['x'],df['pixx'],label=r'$\tau = '+t+r'$ fm/c',color='blue')
        ax[1][1].plot(df['x'],df['pixy'],color='blue')
        ax[1][2].plot(df['x'],df['pietaeta'],label=r'$\tau = '+t+r'$ fm/c',color='blue')



def read_sim(sim_result_folder):
    dt=.001
    for t in time_list:
        col_names=['id','t','x', 'y', 'p','T','muB','muS','muQ','e',
                   'rhoB','rhoS','rhoQ','s','s_smoothed','s_specific',
                   'sigma','spec_s','stauRelax','bigTheta','??',
                   '??2','pi00','pi11','pi22','pi12','t2pi33','v1','v2',
                   'gamma','frz','eos']
        idx = int(np.round((float(t)-1)/dt)/100)
        inp_path = os.path.join(sim_result_folder,f'system_state_{idx}.dat')
        print(inp_path)
        df = pd.read_table(inp_path,
                           names=col_names,sep=' ',header=0)
        df['u1'] = df.loc[:,'v1']*df.loc[:,'gamma']
        df['u2'] = df.loc[:,'v2']*df.loc[:,'gamma']
        df['r'] = np.sqrt(df['x']**2+df['y']**2)
        df_query = df.query('abs(y) < 1.5e-2')
        ax[0][0].plot(df_query['x'],df_query['e']/1000, label=r'$\tau = '+t+r'$ fm/c',color='red',ls='--')
        ax[0][1].plot(df_query['x'],df_query['rhoB'],label=r'$\tau = '+t+r'$ fm/c',color='red',ls='--')
        ax[0][2].plot(df_query['x'],df_query['u1'],label=r'$\tau = '+t+r'$ fm/c',color='red',ls='--')
        ax[1][0].plot(df_query['x'],df_query['pi11']*.197,label=r'$\tau = '+t+r'$ fm/c',color='red',ls='--')
        ax[1][1].plot(df_query['x'],df_query['pi12']*.197,color='red',ls='--')
        ax[1][2].plot(df_query['x'],df_query['t2pi33']*.197/float(t)**2,label=r'$\tau = '+t+r'$ fm/c',color='red',ls='--')


    ax[1][0].set_xlabel("x (fm)")
    ax[1][1].set_xlabel("x (fm)")
    ax[1][2].set_xlabel("x (fm)")
    ax[0][0].set_ylabel(r"$\varepsilon$ (GeV)")
    ax[0][1].set_ylabel(r"$\rho_B$")
    ax[0][2].set_ylabel(r"$u^x$")
    ax[1][0].set_ylabel(r"$\pi^{xx}$")
    ax[1][1].set_ylabel(r"$\pi^{xy}$")
    ax[1][2].set_ylabel(r"$\pi^{\eta \eta}$")
    #ax[0][0].set_xlim(-7,7)
    #ax[0][1].set_xlim(-7,7)
    #ax[0][2].set_xlim(-7,7)

    # ax[0][0].set_ylim(0,.25)
    # ax[0][1].set_ylim(-2.25,2.25)
    ax[0][2].set_ylim(-2,2)
    #ax[1][0].set_ylim(-.45,.025)
    ax[1][1].set_ylim(-.1,.1)
    #ax[1][2].set_ylim(-.025,.45)

    ax[1][1].plot([],[],'b-',label='Analytic')
    ax[1][1].plot([],[],'r--',label='Simulation')
    ax[1][1].legend(loc='upper right', frameon=False)

if __name__ == '__main__':
    analytical_folder = sys.argv[1]
    simulation_folder = sys.argv[2]
    read_sol(analytical_folder)
    read_sim(simulation_folder)
    if (len(sys.argv) < 4):
        fig.savefig('test.png',dpi=300)
    else:
        fig.savefig(sys.argv[3],dpi=300)
