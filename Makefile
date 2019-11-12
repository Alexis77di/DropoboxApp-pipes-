#Makefile
OBJS = mirror_client.o list.o
SOURCE = mirror_client.cpp list.cpp 
HEADER = list.h 
OUT = mirror_client
FLAGS = -g -c

# -g option enables debugging mode
# -c flag generates object code for separate files

all: $(OBJS)
	g++ -g mirror_client.o list.o -o $(OUT)

# create / compile the individual files separately
mirror_client.o: mirror_client.cpp
	$(CC) $(FLAGS) mirror_client.cpp

list.o: list.cpp
	$(CC) $(FLAGS) list.cpp


# cleaning
clean:
	rm -f $(OBJS) $(OUT)

# accounting
count:
	wc $(SOURCE) $(HEADER)

