# OpenGL-minimal-particle-collision
Project developed to learn how particle simulation works, understand the basics of such simulations and motion based on physics. Evolution of a minimal renderer originally focused solely on simulating uniformly accelerated motion into a simple force-based particle collision system.

---

## SCOPE
- Simulating a particle collision system, with gravity, space boundaries, and collision logic.
- Developing the previously implemented project and turning predefined motion into a force based one, thus introducing masses.

---

## CODE STRUCTURE
- initialization: glfw and glad initialization, window creation, shape creation by building vertices and buffers, shader compilation.
- updateParticle: update logic that applies gravity and modifies the force, velocity, and position of each particle, resolving world boundaries as well.
- checkCollision: function used to determine whether two particles are colliding, and therefore require resolving the collision.
- resolveCollision: collision logic, modifies the involved particles' position and velocity as a result of the collision.
- draw: uses the initialized vertices, VAO, and shader to render the shape.

---

## DEVELOPMENT HISTORY
- baseline implementation: static shape, used to learn OpenGL fundamentals (VBOs, VAOs, shaders).
- uniformly accelerated motion: changed the shape to a rectangle and added a gradient, manually added offset and velocity to explore simple motion.
- separation of initialization and update logic, optimization: introduced glm and transformation matrix to update the position without changing the vertices and avoiding unnecessary data flow from CPU to GPU.
- introducing particles, gravity, and collision: defining a new structure to represent a particle, changed the motion to a force based one and introduced the concept of mass. Added collision logic.

- multiple particles: added a variant of the project with random generation of multiple particles.

---

## DECISIONAL CHOICES
- changed the shape to a circle to represent particles for simplicity when detecting and resolving collisions
- considered a global constant for restitution (assuminng all particles made of the same material and not considering different materials and how they interact, focusing only on momentum conservation and gravity simulation)

---

## LIMITATIONS AND FUTURE WORK
- the current implementation isn't optimal when checking for possible collisions, the time complexity is in fact O(n^2) where n is the number of particles. One possible optimization to lower complexity could be to select only pairs of particles that COULD be colliding, by checking their position on the axes (for example if the space they occupy on one axis overlaps with another particle).
Reference: https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics4collisiondetection/2017%20Tutorial%204%20-%20Collision%20Detection.pdf
- rotation and different restitutions could be considered for future work and further simulations

---

## ADDITIONAL NOTES
- By using deltaTime instead of frame by frame updates the animation is no longer dependent on frame rate.
- Non-commutativity of matrix-vector multiplication can be noticed by swapping the transformation matrix and position vector: the final animation no longer showcases a "bouncing" shape and is visibly different.
- OpenGL uses column first matrices, so I initially used the built in transposition because I am used to row first matrices, but this caused problems once I added particles (when transposing my translation matrix became wrong and produced unexpected shapes). Thus, in the end I opted to build the matrix correctly without having to transpose.
- Introduced some numerical stability techniques used in real solvers, such as gradual resolution of collisions.
