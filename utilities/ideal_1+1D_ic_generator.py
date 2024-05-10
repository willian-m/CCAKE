#!/usr/bin/env python
# coding: utf-8
# author: Surkhab Kaur

import numpy as np
import pandas as pd
import sys
from tqdm import tqdm

def init_cond(tau):
    stepeta = 0.01
    stepx = 0.1
    stepy = 0.01
    xmax = 0.1
    xmin = -xmax
    ymax = 5
    ymin = -ymax
    etamax = 5
    etamin = -etamax
    eps0 = 1000
    etas = np.arange(-5,5.01,0.01).round(3)
    c_sq = 1/3
    a = 1
    tau0 = 1
    t0 = 0.6 * tau0
    eps = np.zeros(len(etas))
    u = np.zeros(len(etas))
    c1 = (1 - c_sq**2) / (4 * c_sq)
    c2 = (1 + c_sq)**2 * (4 * c_sq)
    fname=f"./ic_long_tau_{tau}.dat"
    with open(fname, "w") as f:
        f.write(f"#0 {stepx} {stepeta} {stepy} 0 {xmin} {etamin} {ymin}\n")
        for eta in tqdm(etas):
            eps = eps0 * (t0/tau0 + a*tau/tau0 * np.exp(eta))**(c1/a**2 - c2) * (t0/tau0 + tau/(a*tau0) * np.exp(-eta))**(c1*a**2 - c2)
            u = 0.5 / tau * (np.sqrt((t0*np.exp(-eta)+tau*a) / t0*np.exp(eta)+(tau/a)) - np.sqrt((t0*np.exp(eta)+(tau/a) / t0*np.exp(-eta)+tau*a)))
            f.write(f"0 0 {eta} {eps} 0 0 0 0 0 {u} 0 0 0 0 0 0 0\n")
        f.close()
    print(f'Initial conditions were successfully generated for tau = {tau}')


def main():
    if len(sys.argv) != 2:
        print("No argument provided. Please provide a value for tau. Example: "+sys.argv[0]+" 1.0")
    else:
        argument = float(sys.argv[1])
        init_cond(argument)

if __name__ == "__main__":
    main()
