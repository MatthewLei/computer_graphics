GCC_OPTIONS=-Wall -pedantic -I../include
GL_OPTIONS=-framework OpenGL -framework GLUT
COPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

all: prog

prog: initShader.o simpleProgram.o
	g++ $(GL_OPTIONS) -o prog initShader.o simpleProgram.o

initShader.o: initShader.cpp
	g++ $(GCC_OPTIONS) -c initShader.cpp

simpleProgram.o: simpleProgram.cpp
	g++ $(GCC_OPTIONS) -c simpleProgram.cpp

run:
	./prog sphere42NS.obj.txt teapotNS.obj.txt bunnyNS.obj.txt

clean:
	rm initShader.o
	rm simpleProgram.o
	rm prog
