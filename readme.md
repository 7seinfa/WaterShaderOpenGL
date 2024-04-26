This project was made as part of the CS3388 course. It includes createing an OpenGL environment, using VBOs and VAOs and buffers to render things, and using textures as well as shaders (vertex, tesselation, geometry, fragment).
There are parts of the code provided by the prof, and I have commented that out, but to summarize they include: the bitmap loading function, and the basic mesh generation function.

Attached are four code files: main.cpp, Shaders.hpp, PlaneMesh.hpp, and Functions.hpp
In addition, I have attached a screenshot of my waves as screenshot.png, as well as the two bmp files I used

main.cpp contains the main function, where the scene and camera are created, and where we create a PlaneMesh object
PlaneMesh.hpp contains the PlaneMesh, creatings the shaders, the buffers, the textures, and the draw function
Shaders.hpp contains the shader strings, I did this so I didn't have too many files
Functions.hpp contains the loadBMP function to load textures

