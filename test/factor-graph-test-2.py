#! /usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

from fg import *

# example factor graph
################################################################################

def construct_fg(data):
    data = map(lambda x: [x], data)
    fg  = factor_graph_t()
    # factor graph inside the plate
    fg += normal_fnode_t("normal1")
    fg += normal_vnode_t("x")
    fg.link("normal1:output", "x")
    # replicate this graph n-1 times
    fg.replicate(len(data)-1)
    # construct graph outside the plate
    fg += normal_fnode_t("normal2", 0, 0.01)
    fg += normal_vnode_t("mu")
    fg += gamma_fnode_t ("gamma", 1, 2)
    fg += gamma_vnode_t ("tau")
    fg.link("normal2:output", "mu")
    fg.link("gamma:output",   "tau")
    # connect v2 and v3 to all factors f1
    # within the plate
    fg.link("normal1:mean",      "mu")
    fg.link("normal1:precision", "tau")
    # loop over all variable nodes v1
    for i, d in enumerate(data):
        fg.variable_node("x", i).condition([d])
    return fg

# utility
################################################################################

def plot_density(ax, d, x_limits, xlab="", ylab="", n=1001):
    x = np.linspace(x_limits[0], x_limits[1], num=n)
    y = map(lambda xp: d([xp]), x)
    ax.set_xlabel(xlab)
    ax.set_ylabel(ylab)
    return ax.plot(x,y)

def plot_fg(fg, data, bound):
    # obtain estimates
    e_mu  = fg["mu" ].moments(0)
    e_tau = fg["tau"].moments(1)
    d     = normal_distribution_t(e_mu, e_tau)
    d1    = fg["mu" ]
    d2    = fg["tau"]
    # plot result
    plt.clf()
    ax1 = plt.subplot2grid((3,2), (0,0), colspan=2)
    ax2 = plt.subplot2grid((3,2), (1,0))
    ax3 = plt.subplot2grid((3,2), (1,1))
    ax4 = plt.subplot2grid((3,2), (2,0), colspan=2)
    plot_density(ax1, d,  [ 0.5,  1.5], xlab="x", ylab="density")
    plot_density(ax2, d1, [ 0.9,  1.1], xlab=r'$\mu$', ylab="density")
    plot_density(ax3, d2, [50, 90], xlab=r'$\tau$')
    ax1.hist(data, 50, normed=1)
    ax4.plot(bound)
    ax4.set_xlabel("iteration")
    ax4.set_ylabel("bound")
    plt.tight_layout()
    #plt.savefig("factor-graph-test-2.png")
    plt.show()

# test
################################################################################

# generate some data
mu    = 1
sigma = 0.1
data  = np.random.normal(mu, sigma, 1000)

# construct and execute the factor graph
fg = construct_fg(data)
fg.init()
bound = fg()

plot_fg(fg, data, bound[1:])
