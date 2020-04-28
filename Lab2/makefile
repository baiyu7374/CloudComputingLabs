cc = g++
CXXFLAGS=-Wextra -std=c++11 -pthread -lstdc++fs
prom = $(patsubst %.cpp, %, $(P))

source = $(P)

$(prom): $(source)
	$(cc) $(CXXFLAGS) -o $(prom) $(source)
