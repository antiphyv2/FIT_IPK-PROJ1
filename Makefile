PROJ=proj1
CFLAGS=-std=c++20 -Wall -Wextra
CC=g++
RM=rm -f

SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

$(PROJ) : $(OBJS)

clean :
	$(RM) *.o $(PROJ) 


