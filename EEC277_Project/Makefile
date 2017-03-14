UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
all: main.cpp 
	g++ main.cpp -std=gnu++0x -ggdb -DDEBUG -Iinclude/ -o main.exe  -Iinclude/ -lglfw3 -lGLEW -lGL
endif
ifeq ($(UNAME), Darwin) # Mac OS
all: main.cpp 
	g++ -framework OpenGL main.cpp -std=c++11 -Iinclude/ -o main -lglfw -lglew 
endif

