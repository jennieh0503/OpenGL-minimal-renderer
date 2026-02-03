# OpenGL-minimal-renderer
Project focused on basic rendering and real-time geometric transformations.
The rendered image is a colored gradient rectangle which moves using real-time updates and simulating a uniformly accelerated movement along axis y.

---

## SCOPE
Simulating movement with a minimal OpenGL program using uniform transformations and transformation matrices. I decided to apply my theoretical knowledge of physics, geometry and linear algebra to real-time graphics, experimenting with the differences between frame dependent animation and time dependent ones.

---

## CODE STRUCTURE
- initialization: glfw and glad initialization, window creation, shape creation by building vertices and buffers, shader compilation
- update: update logic applied each frame, it modifies the offset and velocity parameters using delta-time. These parameters are then used to create the tranformation matrix that will be passed to the shader in order to move the shape. Updates and transformation matrices are all computed by the CPU: the GPU is only in charge of the matrix*vector multiplication that allows to calculate the final vertices positions that need to be rendered.
- draw: uses the initialized vertices, VAO, and shader to render the shape.

---

## DEVELOPMENT HISTORY
- rendered a simple static triangle, with shaders code based on Learn OpenGL basic shaders.
- added a baseline implementation of movement using offset/translation that updates each iteration of the main loop, initially without acceleration or boundaries, then with these features added (see not optimized branch).
- added another triangle to form a rectangular shape.

Such implementation was not optimal since it required vertices to be sent each frame to the GPU, thus:
- I separated the initialization, update and draw functions. To avoid modifying vertices and sending them to the GPU each frame I implemented a transformation matrix, which would allow to add other movements in the future easily as well, and consequently changed the shaders' initial codes.
- While modifying shaders I also experimented with data-flow through shaders by adding a gradient to my shape (color dependent on y position).

---

## ADDITIONAL NOTES
- By using deltaTime instead of frame by frame updates the animation is no longer dependent on frame rate.
- Non-commutativity of matrix-vector multiplication can be noticed by swapping the transformation matrix and position vector: the final animation no longer showcases a "bouncing" shape and is visibly different.
