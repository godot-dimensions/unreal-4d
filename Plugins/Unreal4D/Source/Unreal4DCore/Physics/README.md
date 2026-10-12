# Unreal 4D Physics

This folder contains classes for 4D physics shapes, bodies, and simulation.

`UPhysicsServer4DWorldSubsystem::QueryPointCollision` queries registered colliders at a world-space 4D point and appends `FCollisionManifold4D` results. Mesh components register automatically and participate when `EnableCollision` is set, using `CollisionMesh4D` when supplied.

The result's `Point` and `Normal` use world 4D coordinates. `Penetration` preserves the signed surface distance in the collider's local coordinates, with negative values inside.
