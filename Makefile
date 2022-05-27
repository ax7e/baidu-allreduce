MPI_ROOT=/home/haozhewen/openmpi

# Check that MPI path exists.
ifeq ("$(wildcard $(MPI_ROOT))","")
$(error Could not find MPI in "$(MPI_ROOT)")
endif

CC:=mpicxx
LDFLAGS:=-L$(MPI_ROOT)/lib -lmpi -DOMPI_SKIP_MPICXX=
CFLAGS:=-std=c++17 -I$(MPI_ROOT)/include -I. -DOMPI_SKIP_MPICXX=
EXE_NAME:=allreduce-test
SRC:=$(wildcard *.cpp test/*.cpp)
OBJS:=$(SRC:.cpp=.o) 

all: $(EXE_NAME)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(EXE_NAME): $(OBJS)
	$(CC) -o $(EXE_NAME) $(LDFLAGS) $^ $(LDFLAGS)

test: $(EXE_NAME)
	$(EXE_NAME)

clean:
	rm -f *.o test/*.o $(EXE_NAME)
