# Perturb and Observe Algorithm Test File

## Aim 

The aim of this setup is merely to explore the efficiency and feasibility of implementing the PnO algorithm for the solar panels, and how well it can adapt to changing lighting conditions. 

## Setup

There are no additional libraries required for this setup. This test does not interact with the control interface.

Hardware-wise, connect a single solar cell to the B port of the SMPS, with the switch OFF.

## How this differs from the actual setup

Aside from the differences mentioned in the previous section, this file assumes connecting the panels to the B port. 

Since it is realistically difficult to swap the battery configuration (with the SMPS) during execution, for the rover setup, the solar panel array is connected to the A side, such that the PV to battery conversion is Buck.

In an ideal world, the power supplied by the PV panels is directly known, but here we assume that this value is approximately equal to that supplied to the batteries, after accounting for the efficiency losses. (Explain about the efficiency losses)

This is a justifiable compromise because monitoring the maximum amount of charging current for the cells is more important than obtaining an accurate estimation of the power supplied. Given that the voltage measurement at the port A is available, the PnO algorithm is still achievable, albeit the slightly inaccurate measurements.