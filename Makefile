PROJ=ipk24chat-client
CFLAGS=-std=c++20
CC=g++
RM=rm -f

SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

$(PROJ) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	$(RM) *.o $(PROJ) 


