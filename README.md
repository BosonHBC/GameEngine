# GameEngine
GameEngine for eae6320 class

Create engine feature from sketches that implemented by John Paul (john-paul.ownby@eae.utah.edu)
Features:
1. Engine structures: Platform-specific implementation with platform-indenpent interface. We are using OpenGL and Direct3D as two different platform.
2. Create representation of geometry files, shading files.
3. Create core systems like game actor and rendering objects.
4. Minimize struct size to save memory.
5. How to comvert file to binary file in build time and load binary file in run time to save loading time.
6. Build Maya plugin to create our own geometry file representaion.
7. Use handle/manager model to do reference counting.
8. Use Lua to load normal files and binary files.
9. Create unreal-style user input binding
10. Create simple top-down shooter
