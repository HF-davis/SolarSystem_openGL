# SolarSystem_openGL
Nosotros podemos crear nuestro mundo con openGL

# Depedencias
## Librerias en c++
* glew.h
* glfw3.h
* soil2.h

Podemos usar opencv para cargar las texturas de los objetos, tambien podemos usar SOIL.h, una libreia de c++
# Corriendo en ubuntu
este proyecto funciona completamente con la libreria SOIL.h, tambien se puede usar con opencv, pero seria cuestion de hacer cambios en el código
# para correr usando SOIL.h usa:
* g++ -Wall *.cpp -lm -lopencv_core -lopencv_highgui -lopencv_imgcodecs `pkg-config --libs --cflags opencv4` -lGL -lGLU -lglut -lGLEW -lsoil2-debug -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -I/usr/local/local/include/opencv4/ -o main.out; ./main.out | more
# para correr usando opencv usa:
* g++ -Wall *.cpp -lm -lopencv_core -lopencv_highgui -lopencv_imgcodecs `pkg-config --libs --cflags opencv4` -lGL -lGLU -lglut -lGLEW -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -I/usr/local/local/include/opencv4/ -o main.out; ./main.out | more
