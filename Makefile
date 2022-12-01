CXX=gcc
CXXFLAGS= -Werror 

all: mytee mync myshell myecho

mytee: mytee.c
	$(CXX) $(CXXFLAGS) $^ -o $@

mync: mync.c
	$(CXX) $(CXXFLAGS) $^ -o $@

myshell: myshell.c
	$(CXX) $(CXXFLAGS) $^ -o $@

myecho: myecho.c
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) $^ -o $@
%.so: %.o
	$(CXX) -shared -o $@ $<


#valgrind --tool=memcheck -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99 ./mync 2>&1 | { egrep "lost| at " || true; }
#valgrind --tool=memcheck -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99 ./mync 127.0.0.1 299934 2>&1 | { egrep "lost| at " || true; }
#valgrind --tool=memcheck -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99 --log-file=f2.txt ./myshell | { egrep "lost| at " || true; }

clean:
	rm -f *.o mytee mync myshell myecho
