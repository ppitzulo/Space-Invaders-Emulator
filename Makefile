#OBJS specifies which files to compile as part of the project
OBJS = emulator.c cpu.c disassembler.c

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = emulator

#This is the target that compiles our executable
all : $(OBJS)
	gcc $(OBJS) -w -lSDL2 -o $(OBJ_NAME)