#compiler name
cc :=gcc

#remove command
RM := rm -rf

#source files
SOURCES :=database.c fs.c parsing.c
SRCS :=$(addprefix src/,$(SOURCES))

#object files
OBJS :=$(addprefix build/,$(SOURCES:.c=.o))

#main target
main: $(OBJS)
    $(CC) $^ -o $@

%.o: %.c
    $(CC) -c $< -o $@
 

.PHONY: clean
clean:
    $(RM) *.o main