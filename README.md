# Description
A fluid simulation based on [Fluid Simulation course notes](https://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf) by Bridson and Müller-Fischer.

Screen space renderer is done according to [Screen Space Fluid Rendering For Games](http://developer.download.nvidia.com/presentations/2010/gdc/Direct3D_Effects.pdf) by Simon Green.

Pressure solver is done according to Appendix B2 of [Introduction to the Conjugate Gradient Method Without the Agonizing Pain](https://www.cs.cmu.edu/~quake-papers/painless-conjugate-gradient.pdf) by Jonathan Richard Shewchuk.


I am not very good at math, but I understood just enough to get it working.
It runs on CPU and is far from efficient.

Video: https://youtu.be/Yq4zrmvY-To


# Compiling
If you're on Ubuntu install libglew-dev, libsdl2-dev, libglm-dev and run make.

To run, launch build/sim.out.
