# RF 2D Field Solver

Did you ever route a PCB and needed to figure out how wide a trace needs to be to reach a specific impedance? This tool can calculate that for you.

I can already imagine the next question:
##### But aren't there a lot of free online calculators for that purpose already? Why would I need to simulate this?

Well yes, but actually no:

- Most available calculators are based on approximate formulas. Change the proportions to a something where these formulas are no longer valid, and you get a wrong result
- The results from several calculators actually differ by a surprising amount (see below)
- If you have a specific geometry which isn't covered by any of the calculators, you need a field solver. This could for example be gaps in the ground plane or multiple dielectrics between the trace and the reference plane

##### Will this tool solve all my problems with calculating impedances?

Ideally yes, but it is not that straightforward. In theory, a field solver should give you a perfect result. In practice, the accuracy will depend on the grid size and tolerance for the simulation. Getting these values right for an acceptable simulation time while still getting good results can be tricky.

## How to use the field solver

1. Configure the area size
2. Set up the elements (RF traces, GND planes, dielectric substrates)
3. Select simulation parameters (simulation grid resolution, tolerance, distance of Gauss integral from RF conductors)
4. Start the simulation
5. View the potential field and the calculated impedances

##### Predefined scenarios

The real use case is the simulation of non-standard elements. But creating the structure can take some time. If used for standard traces, several predefined scenarios are available to speed up creating the elements:

|     References      |                         Single Ended                         |                         Differential                         |
| :-----------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|     Microstrip      |     ![](Software/RF2DFieldSolver/images/microstrip.png)      | ![](Software/RF2DFieldSolver/images/microstrip_differential.png) |
| Coplanar Microstrip | ![](Software/RF2DFieldSolver/images/coplanar_microstrip.png) | ![](Software/RF2DFieldSolver/images/coplanar_microstrip_differential.png) |
|      Stripline      |      ![](Software/RF2DFieldSolver/images/stripline.png)      | ![](Software/RF2DFieldSolver/images/stripline_differential.png) |
| Coplanar Stripline  | ![](Software/RF2DFieldSolver/images/coplanar_stripline.png)  | ![](Software/RF2DFieldSolver/images/coplanar_stripline_differential.png) |

## How does the calculation work?

## Comparison to other calculators









