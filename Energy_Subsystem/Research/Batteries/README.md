# Batteries

---

# Battery Statistics

## Key battery Statistics from Data Table

[Data Table](Batteries%208f78c407abed451abff6fc5c740e9061/Data%20Table%207e34b73d0088425a82d25aca824f3ff2.csv)

Charging guidelines: Constant current charging, then constant voltage charging up to 3.6V

If we have constant voltage charging, do we need constant voltage discharging as well?

When do we actually need rapid charging current? Because at the end we still need to take quite a while for constant voltage charging to reach the full capacity.

## Battery Statistics from Initial Testing

Testing criteria:

- Constant current charging and discharging of 0.5C or 250mA
- Minimum voltage of 2.5V and maximum voltage of 3.6V

[Couple Relevant Data](Batteries%208f78c407abed451abff6fc5c740e9061/Couple%20Relevant%20Data%20c951c3ae95f34b7c8ca7476f6547319b.csv)

---

# State of Charge Estimation

- Battery Modelling

    ## Unscented kalmant filter

    - Non linear lookup tables that we characterise the battery cell (look at previous video before Battery Cell Balancing and SOC Estimation)

    ## Modelling Batteries

    1. Equivalent circuit (accounts for environmental conditions and SoC)
    2. Parametrize equivalent circuit
    3. Refine estimation (more elaborate models) 

    Typical equivalent circuit

    - Voltage source associated with open circuit potential
    - SEries resistor ionic conductivity of seperator and electrolyte
    - 1+(say 3?)  resistor and capacitor pairs that mimic the diffusional process involved in the interaction and interpolation and deinterploation of lithium ions in and out of the porous electrodes
    - Chemical the electrochemical and dissipative phenomena, environmental conditions
    - Or maybe just assume 25 degree temperature

    Parameter Estimation 

    - Series resistance to account for the instantaneous portion of the recovery
    - Combined RC pair explains the exponential portion
    - Correct circuit OCV
    - Different response at different SoC levels
    - **Simulink design optimisation (A Vector of values, each corresponding to different SoC)**

    ## Hard to estimate because

    - Like it is hard to estimate water flow with a lot of ripple
    - Open circuit voltage: load current pulls the voltage down

Percentage of total charge at which the battery is currently at.

Considering the charge and discharge profile of the battery, we can obtain the SOC by

1. OCV estimation
2. Coulomb counting
    1. Disadvantage: es not work with cells with a flat discharge profile.

Temperature and Voltage within designated ranges to ensure minimal degradation (prevent premature degradation)

Are we interested in the SoC in terms of capacity, or SoC in terms of energy? Soc E is determined by the area of the graph on the voltage vs SOC video

How do we ensure that we charge till the maximum of the battery? Charge with constant current then constant voltage then wait till the current drops to very low?

Can we assume that SOC is 0% when the voltage is at 2.5V (which is when we stop discharging)?

Is the total SOC difference (which defines the capacity) 20mAh (500-480) ? Why is the minimum rated capacity 480mAh? Or how do we actually define the minimum rated capacity? Do we look at Whr or Ahr Capacity?

I understand how we use the open circuit voltage lookup, by looking at the charge and discharge profiles that we have done during preliminary testing. However how do we actually use that to estimate SoC?

Why can't we switch on multiple relays at the same time? For example if we are removing all the cells from the circuit, for example, if we remove all cells from the circuit it is still safe right?

Why do we connect the battery in a Buck (reverse Boost) configuration, rather than a Boost configuration? Is it only because we need the current data from the batteries?

OCV vs SoC curve depends on SoH (conflicting opinions)

### Voltage Method

Non linear function of open circuit voltage, only when battery is fully at rest (or slow charge and discharge)

Isn't open circuit voltage always 3.3V? Use that to determine initial state of charge

### Coulomb Counting

- Depends on accurate SoH and precision of current measurement
- Measuring current (total coulomb) flowing in and out of battery - give one change in SoC (if SoH and initial capacity is known)

    Coloumb count between two instances will indeed be a good measure of energy added or removed, but will only represent SoC only if the initial SoC is good

    Change in SoC = Charge pumped in/out of battery/ (Capacity * SoH)

    Charge pumped in/out is coloumb count * electronic charge, or **integration of current over time**

No linear relationship between SoC and Voltage (only when current is zero, **open circuit voltage**) Voltage depends on rate of charging (current)

## SOH

$SoC_{new} = SoC_{old} + \Delta SoC$ but SoH needs to be known as delta SoC is dependent on SoH

- Repeated charging and discharging will reduce accuracy as error builds up
- SoH keeps decreasing over time

To determine SoH

- Completely discharge it and then charge it (track open circuit voltage) TOTALLY
- Then discharge **slowly** and carry out the coulomb count (such that it is almost like open circuit voltage)
- Indicates the maximum charge the battery can currently hold; compare it with past data to determine SoH

Alternative method: internal resistance

- As battery electrodes deteriorate, its capacity to deliver current also reduces
- Internal resistance of a cell indicates the ability to deliver current
- Difference between initial and current initial resistance helps estimate SoH

Self discharge of battery (while on shelf)

---

# Battery Balancing

Things to consider

- Nominal voltage (V), Battery Capacity (Ah)
- Region of operation (40-80% of SOC)
- Discharge Load and time?

Charge with constant current, then constant voltage.

High charge rate only partially charges the battery up to some low SoC, and beyond that we need constant voltage charging

## Cell Balancing

Weakest cell affects the amount of charge that can be drawn from the entire system

Strongest cell affects the extent to which the system can be be charged

### Dissipative (Passive) Discharging

- Discharge to resistors in parallel with the individual cell
- Balancing: relay (transistors) to selective switch the bypass branch of bleed resistors on and off
- Divert part of the charging current, thereby slowing the rate of charge they are connected to, allowing cells above/ below to catach up.
- Temperature controlled charging? Assuming the heating is only a result of ohmic heating at the resistors

**"MODEL CORRELATION TECHNIQUE" - Characterising the battery _ READ THIS**

Hottest cell will degrade faster

### Active Balancing 🙅‍♂️

Distribute excess charge from some cells and redistribute it in other cells