# Cloth Simulation with compute shaders

# Build
```shell
git clone https://github.com/SampoImmonen/Cloth-Simulation && cd Cloth-Simulation
git submodule update --init --recursive
mkdir build && cd build
cmake .. && cmake --build .
```

# Features:
1. Simple PBR shader with normal maps and shadows
2. Trapezoid integrator for cloth simulation
3. File explorer to change material textures (Only on Linux)
4. Visualize cloth normals with geometry shader

# Controls
1. F to apply constant force to cloth
2. A,S,D,W, Mouse and scroll to move camera


# Resources:
1. Integrators for simulation (https://www.embeddedrelated.com/showarticle/474.php)
2. OpenGL 4 Shading Language Cookbook (https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition
)
![alt text](./media/screenshot2.png)