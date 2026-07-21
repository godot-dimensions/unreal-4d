# Unreal 4D Physics Shapes

This folder contains classes for 4D physics shapes, which are used as part of simulated physics objects.

- The `Components` folder contains the 4D scene components that represent the shapes.
  - These extend `UShape4DComponent`, which extends `UPrimitive4DComponent`.
  - These contain the shape data classes, and provide them to an ancestor `UCollisionObject4DComponent` for use in the physics simulation.
  - If a `UShape4DComponent` does not have a `UCollisionObject4DComponent` ancestor, the shapes are considered global static solid shapes.

- The `Data` folder contains the 4D shape data classes, which are used to define the geometry of the shapes.
  - These extend `UShape4DData`, which extends `UObject`.
  - These are lightweight data classes that can be used portably without the scene component bloat.
