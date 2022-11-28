CXX=gcc
CXXFLAGS= -Werror 

all: mytee mync

mytee: mytee.c
	$(CXX) $(CXXFLAGS) $^ -o $@

mync: mync.c
	$(CXX) $(CXXFLAGS) $^ -o $@




%.o: %.c
	$(CXX) $(CXXFLAGS) $^ -o $@
%.so: %.o
	$(CXX) -shared -o $@ $<

clean:
	rm -f *.o mytee mync
