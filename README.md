# OpenGL-minimal-renderer
Project focused on basic rendering and real-time geometric transformations

BASELINE IMPLEMENTATION:
The initial working implementation includes two simple functions, a main with update logic and a draw function to depict the shape each frame.
At the beginning it only included a basic static triangle, using LearnOpenGL's basic shaders, then I decided to create two triangles to form a square and add movement: the first implementation only had linear uniform movement through the screen (along axis x) without boundaries nor acceleration; I then decided to add some more dynamics with a bouncing effect, I moved the translations to axis y and added an acceleration factor which increases when coming down and decreases when going up, also reversing movement when the window's boundaries are met.

IDENTIFIED ISSUES:
Because this implementation started with a static image, where data was sent one single time to GPU, the draw function used each frame still includes this logic and the compilation of shaders, which would make the program inefficient. I then decided to separate the two components so that it doesn't get repeated each frame: to allow transformations to happen I decided to use transformation matrices.
