# PV Panels

# The overall rationale of Maximum Power Point Tracking (MPPT)

- Varying due to irrandiance and temperature
    - Higher irradiance - better
    - Higher temperature - worse
- I-V curve is unique for a certain set of operating conditions (irradiance and temperature)
    - Adjust electrical load seen by PV module (PV cell)
- Aim: To identify the peak of PV curve, or IV curve knee
- Need to adapt to changing ambient conditions (temperature and irradiance), so we need to track changes in IV curve and track the MPP

# MPPT Techniques

Aim: track the knee, achieve a stable current and voltage, efficiently (in terms of energy consumption), and respond to ambient conditions quickly

## Indirect 🙅‍♂️

1. Fixed voltage method. Assumption: Higher MPP in winter, lower MPP for same level of irradiance
2. Fractional open circuit voltage. 

    $$v_{mpp} = k \times V_{oc}$$

    Assumption: Measure open circuit voltage to estimate maximum power point voltage as a fraction.

    k value not very precise: only in the MPP region

    Needs to (frequently) measure the open circuit voltage constantly during operation, which requires disconnecting the PV module and halting current temporarily - lost in production

    Pilot cell: receives the same illumination, and provides a good representation of the PV cell

## Direct "Hill climbing algorithms" ✅

### Perturb and Observe

1. Increase in voltage leads to increase in power, operating point is LEFT OF MPPT. Want to shift operating point (Vref)  to the right
2. Increase in voltage leads to decrease in power, operating point RIGHT OF MPPT. Want to shift to the left.
3. Eventually shifts to the MPPT
4. MPPT is never stable - use smaller perturbation steps. **Struggles in rapidly changing illumination conditions** (e.g. in between time sampling instances, then it will fail to converge). 
    1. Danger: perturbations might cause it to incorrectly estimate operating point (like thinking it is on the left when it is on the right)
5. Most widely used in the industry

![PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled.png](PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled.png)

### Incremental Conductance method:

Comparing incremental conductance (delta, or slope) and instantaneous conductance (just I/V)

- Eventually, the instantaneous and incremental conductance should be the same
- Low sampling intervals, less suspectible to changing illumination conditions
- Still not efficient when there are shades
- Hardware complexity - need a Buck boost DC DC converter

![PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%201.png](PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%201.png)

![PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%202.png](PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%202.png)

![PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%203.png](PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%203.png)

![PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%204.png](PV%20Panels%20536e2af6fc5044ba899e40522143e417/Untitled%204.png)