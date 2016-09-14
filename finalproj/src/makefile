GCC_OPTIONS=-Wall -pedantic -I../include
GL_OPTIONS=-framework OpenGL -framework GLUT
COPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

all: prog

prog: initShader.o epicViewer.o
	g++ $(GL_OPTIONS) -o prog initShader.o epicViewer.o

initShader.o: initShader.cpp
	g++ $(GCC_OPTIONS) -c initShader.cpp

epicViewer.o: epicViewer.cpp
	g++ $(GCC_OPTIONS) -c epicViewer.cpp

run:
	./prog floor.obj.txt cow_statue.obj.txt cow.obj.txt walls.obj.txt

clean:
	rm initShader.o
	rm epicViewer.o
	rm prog
