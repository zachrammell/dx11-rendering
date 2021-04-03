/* Start Header -------------------------------------------------------
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: README.txt
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS350_1
Author: Zach Rammell, zach.rammell
Creation date: 2/15/21
End Header --------------------------------------------------------*/

a. How to use parts of your user interface that is NOT specified in the assignment description.
  scroll wheel to zoom, right click and drag to move camera.
  b. Any assumption that you make on how to use the application that, if violated, might cause
the application to fail.
  The working directory of the application must contain the assets folder.
  assets/models/powerplant needs to be filled in with the ppsection folders (untarred)
  please use release mode
c. Which part of the assignment has been completed?
geometry primitives: Point, Triangle, Plane
bounding volumes: sphere, AABB
tests: 
[X] Sphere	Vs Sphere
[X] AABB	Vs Sphere
[X] Sphere	Vs	AABB
[X] AABB Vs	AABB

[X] Point	Vs Sphere
[X] Point	Vs AABB
[X] Point	Vs	Triangle
[X] Point	Vs	Plane

[X] Ray	Vs	Plane
[X] Ray	Vs	AABB
[X] Ray	Vs	Sphere
[X] Ray	Vs	Triangle
d. Which part of the assignment has NOT been completed (not done, not working, etc.) and
explanation on why those parts are not completed?
e. Where the relevant source codes (both C++ and shaders) for the assignment are located.
Specify the file path (folder name), file name, function name (or line number).
  in ./source/geometry/
  I wrote all code myself.
f. Which machine did you test your application on.
  my laptop
i. If on campus – put the machine name e.g. DIT1234
ii. If remote – specify OS, GPU, and OpenGL Driver version. Tip: You can use the
OpenGL Extensions Viewer to get this information readily.
  Windows 10 2004, Nvidia RTX 2060M
g. The number of hours you spent on the assignment, on a weekly basis
h. Any other useful information pertaining to the application
