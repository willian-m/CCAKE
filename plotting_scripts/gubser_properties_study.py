#!/usr/bin/env python
import sys
import os
import matplotlib as mpl
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np

time_list=['1.00', '2.00', '3.00', '4.00', '5.00', '6.00']
dpi=150

def read_sol(analytic_sol_folder:str)->list:
    df_list = []
    for t in time_list:
        inp_path = os.path.join(analytic_sol_folder,'tau='+t+'.txt')
        df = pd.read_table(inp_path,names=
                      ['x', 'y', 'eta', 'e', 'rhoB', 'rhoS', 'rhoQ', 
                       'ux', 'uy', 'ueta', 'Bulk', 'pixx', 'pixy', 'pixeta', 
                       'piyy', 'pyeta', 'pietaeta'], sep=" ", engine='python',
                       header=1)
        #Attach additional properties
        df.tau=t
        t_squared = float(t)**2

        #polar coordinates
        df['r'] = np.sqrt(df['x']**2+df['y']**2)
        df['phi'] = np.arctan2(df['y'],df['x'])
        df['p'] = df['e']/3 #Pressure

        #Computes missing pi^{\mu\nu} components
        df['u0'] = np.sqrt(1+df['ux']**2+df['uy']**2)
        df['pitx'] = (df['pixx']*df['ux']+df['pixy']*df['uy'])/df['u0']
        df['pity'] = (df['pixy']*df['ux']+df['piyy']*df['uy'])/df['u0']
        df['pitt'] = (df['pitx']*df['ux']+df['pity']*df['uy'])/df['u0']
        df['pizz'] = (df['pixx']+df['piyy']-df['pitt'])/t_squared

        df['pi_norm'] = np.sqrt(df['pitt']**2+df['pixx']**2+df['piyy']**2
                                +(t_squared*df['pizz'])**2
                                +2*(df['pixy']**2 - df['pitx']**2
                                    - df['pity']**2) )
        df['reynolds'] = df['pi_norm']/df['p']

        df['T00'] = df['e']*(4*df['u0']**2-1)/3
        df['T01'] = df['e']*4*df['u0']*df['ux']/3
        df['T02'] = df['e']*4*df['u0']*df['uy']/3
        df['T11'] = df['e']*(4*df['ux']**2-1)/3
        df['T12'] = df['e']*4*df['ux']*df['uy']/3
        df['T22'] = df['e']*(4*df['uy']**2-1)/3
        df_list.append(df)
    return df_list

def plot_ratios(df_list:list):
    fig_pi, ax_pi = plt.subplot_mosaic(
        [['pi11', 'pi12', 'cbar'],['R','pi22','cbar']],
        figsize=np.array([1080,1080])/dpi,
        width_ratios=[1.,1.,0.1]
        )

    #Line styles shenanigans
    #cmap = plt.get_cmap('winter_r',len(time_list))
    cmap = plt.get_cmap('Paired',len(time_list))

    for ii,df in enumerate(df_list):
        tau = df.tau
        df_d = df.query('abs(phi - 3.14159/4) < 1e-2')
        #df_d = df.query('abs(y) < 1e-2')

        ax_pi['pi11'].plot(df_d['r'],np.abs(df_d['pixx']/df_d['T11']),
                            color=cmap(ii))
        ax_pi['pi12'].plot(df_d['r'],np.abs(df_d['pixx']/df_d['T12']),
                            color=cmap(ii))
        ax_pi['pi22'].plot(df_d['r'],np.abs(df_d['pixx']/df_d['T22']),
                            color=cmap(ii))

        ax_pi['R'].plot(df_d['r'],df_d['pi_norm']/df_d['p'],
                            color=cmap(ii))
        #ax_pi['pi21'].plot([],[] ,color=cmap(ii),
        #                   label=r'$\tau = '+tau+r'$ fm/c')


    ax_pi['pi11'].set_ylabel(r"$|\pi^{xx}/T^{xx}|$")
    ax_pi['pi12'].set_ylabel(r"$|\pi^{xy}/T^{xy}|$")
    ax_pi['pi22'].set_ylabel(r"$|\pi^{yy}/T^{yy}|$")
    ax_pi['R'].set_ylabel(r"$\mathcal{R}^{-1}$")
    ax_pi['R'].set_xlabel(r"$r$ (fm)")
    ax_pi['pi22'].set_xlabel(r"$r$ (fm)")

    ax_pi['pi11'].set_ylim(0,1e+2)
    ax_pi['pi12'].set_ylim(0,1e+2)
    ax_pi['pi22'].set_ylim(0,1e+2)
    ax_pi['R'].set_ylim(-.1,5)

    for ax_key in ax_pi.keys():
        if 'cbar' in ax_key:
            continue
        ax_pi[ax_key].tick_params(axis='both', which='both', labelsize=12)
        ax_pi[ax_key].xaxis.set_ticks(np.linspace(0,10,11))
        ax_pi[ax_key].xaxis.label.set_size(12)
        ax_pi[ax_key].yaxis.label.set_size(12)
        ax_pi[ax_key].set_yscale('symlog')
        ax_pi[ax_key].axhline(1, color='black', ls='--', lw=2)
        ax_pi[ax_key].set_xlim(0,7)

    #ax_pi['pi21'].set_ylim(2,9)
    #ax_pi['pi21'].legend(loc='upper right', frameon=False, fontsize=12)
    tau_list = [float(df.tau) for df in df_list]
    tau_min = tau_list[0]
    tau_max = tau_list[-1]
    delta_time = tau_list[1]-tau_list[0]
    boundaries = np.arange(tau_min-delta_time/2,
                                      tau_max+3*delta_time/2, delta_time)

    norm = mpl.colors.Normalize(vmin=tau_min,vmax=tau_max)
    sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    plt.colorbar(sm, ticks=tau_list, label=r'$\tau$ (fm/c)',
                 cax = ax_pi['cbar'],
                 boundaries=boundaries)

    fig_pi.savefig('pi.png',dpi=300)

def map_reynolds(df_list:list):
    fig_rey, ax_rey = plt.subplot_mosaic(
        [['tau1.00', 'tau2.00', 'tau3.00', 'cbar'],
            ['tau4.00', 'tau5.00', 'tau6.00', 'cbar']],
        figsize=np.array([1080*3/2,1080])/dpi,
        width_ratios=[1.,1.,1.,0.1]
        )

    rey_max = 0
    rey_min = 1e+99
    for ii,df in enumerate(df_list):
        tau = df.tau
        rey_max = max(rey_max,df['reynolds'].max())
        rey_min = min(rey_min,df['reynolds'].min())

    for ii,df in enumerate(df_list):
        tau = df.tau

        tcf = ax_rey['tau'+tau].tricontourf(df['x'],df['y'],
                                      df['pi_norm']/df['p'],levels=100,
                                      vmax=rey_max,vmin=rey_min,cmap='plasma')
        ax_rey['tau'+tau].set_title(r'$\tau = '+tau+r'$ fm/c')
        ax_rey['tau'+tau].set_aspect('equal')
        ax_rey['tau'+tau].set_xlabel(r'$x$ (fm)')
        ax_rey['tau'+tau].set_ylabel(r'$y$ (fm)')

    norm = mpl.colors.Normalize(vmin=rey_min,vmax=rey_max)
    cmap = cmap = tcf.get_cmap()
    sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    plt.colorbar(sm, label=r'$\mathcal{R}^{-1}$',
                 cax = ax_rey['cbar'])

    ax_rey['cbar'].axhline(1, color='black', ls='--', lw=2)


    fig_rey.savefig('rey.png',dpi=300)


if __name__ == '__main__':
    mpl.rcParams.update({'font.size': 12})
    mpl.rcParams.update({'axes.labelsize': 12})
    mpl.rcParams.update({'xtick.labelsize': 12})
    mpl.rcParams.update({'ytick.labelsize': 12})
    mpl.rcParams.update({'legend.fontsize': 12})
    mpl.rcParams.update({'figure.autolayout': True})
    mpl.rcParams.update({'text.usetex': True})
    analytical_folder = sys.argv[1]
    df_list = read_sol(analytical_folder)
    plot_ratios(df_list)
    map_reynolds(df_list)
