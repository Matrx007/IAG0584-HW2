CXX = gcc
CXXFLAGS = -Wall -g
LDFLAGS = 

SRC = src/
OBJ = $(SRC:.c=.o)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -rf $(OBJ) $(EXEC)