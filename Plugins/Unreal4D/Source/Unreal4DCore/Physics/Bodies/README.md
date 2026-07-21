# Unreal 4D Physics Bodies

This folder contains classes for 4D physics root objects, such as areas and bodies.

- `UCollisionObject4DComponent`: Abstract base for 4D physics root objects.
  - `UArea4DComponent`: Non-solid 4D physics objects that can detect overlaps with other 4D physics objects.
  - `UPhysicsBody4DComponent`: Abstract base for solid 4D physics bodies.
    - `UDynamicBody4DComponent`: Bodies that have their movement driven by the physics engine and be affected by forces and impulses.
    - `UCharacterBody4DComponent`: Bodies that have their movement driven by scripted movement and can affect other solid 4D physics bodies.
    - `UStaticBody4DComponent`: Bodies that do not move and only exist to be obstacles for other solid 4D physics bodies.

Any `UShape4DComponent` component will search for its nearest ancestor `UCollisionObject4DComponent` and provide its shape data to it for use in the physics simulation. If a `UShape4DComponent` does not have a `UCollisionObject4DComponent` ancestor, the shapes are considered global static solid shapes.
