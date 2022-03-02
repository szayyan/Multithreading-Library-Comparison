TARGET = MPvsX

IDIR = -I$(CURDIR)/Common -I$(CURDIR)/Hull -I$(CURDIR)/Boids -I$(CURDIR)/BreadthFirst -I$(CURDIR)/Util
CXXFLAGS = -g -O2 -Wall -Wno-reorder -fopenmp $(IDIR)
LIBS = -fopenmp -lsfml-graphics -lsfml-window -lsfml-system
LDFLAGS = -lGL -lGLU -lGLEW
SRC = $(wildcard Boids/*.cpp) \
      $(wildcard BreadthFirst/*.cpp) \
	  $(wildcard Common/*.cpp) \
	  $(wildcard Hull/*.cpp) \
	  $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, %.o, $(SRC))

.PHONY: default all clean

default: $(TARGET)

all: default

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	-rm -rf $(OBJECTS) $(TARGET)