# IN PROGRESS

## Warning
Let's just get this out of the way. This is NOT a project for the faint of heart. A combination of mechanical aptitude, pinning electrical connectors, and a 3D printer are required to execute.
If you want a less advanced yet still capable version. Try the V2 winder based on commonly avaliable RC components.

With that out of the way. Let's begin.

## Background

On some level, this build is not so much an evolution, but a return to the original plan.
See, the very first computerized winder I had attempted used a pancake stepper and 12V power. But due to problems integrating the servo control and stepper libraries, this was quickly abandoned in favor of hobby RC components.
Now, with years more experience, and a better understanding of my inital failures, i'm back to make my ultimate CNC winder!

Heres the intended improvements.

* Increased Precision:

    The previous winder design utilized RC servos for traverse control. While this worked well, there are three major failing points.
    * 1: The servo has some measure of backlash, negating some precision.
    * 2: Using just the servo horn for traverse means complex Rotation-Linear position math.
    * 3: The largest failure. The servo traverse can only be controlled in 1 degree increments. With a 10mm arm length, this is 0.2mm per step.  Not precise enough for laying 42 gauge wire side by side.

    The new design uses a 2mm pitch lead screw, with 16 part microstepping. Or 3200 steps per revolution Which results in Linear precision of 0.000625mm.
    10X the required precision for layer to layer position of 44AWG wire (0.058mm with insulation). 

* Tension Measurement:
    A weakness of all previous designs was a reliance on "eye-caliper" measurements for tension. Otherwise known as trial and error.
    And knowing when the wire was close to it's breaking point was all but impossible. 
    To fix this problem, A HX711 chip paired to a 0-100g load cell was added for tension measurement. At minimum this provides a warning for break tension.

* Complete Offline Running:
    While previous versions were relatively easy to use. They were required to be connected to a computer for parameter adjustment.
    Perfectly acceptable for a hobbyist builder, With Arduino IDE being easy to access. But this is still less than ideal, and certainly not up to being an "ulitmate" winder.

* Preset and parameter Storage:
    This is part of the previous problem. While presets could be loaded into IDE and then selected via a selectable parameter in code. This still required modification to the code files any time a new pickup was desired.
    This new winder incorporates a small touch screen to allow parameter editing, and multiple presets than can be tweaked before deployment. (say a 5% underwind while maintaining all other specifications).



![alt text](<Photos/Winder Current State 9-14.JPG>)