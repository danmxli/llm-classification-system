CXX = g++
CXXFLAGS = -std=c++11
LDFLAGS = -lcurl
INCLUDES = -I./include

all: llm_socket.cpp main.cpp trienode.cpp trie.cpp illegal_exception.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) llm_socket.cpp main.cpp trienode.cpp trie.cpp illegal_exception.cpp $(LDFLAGS) -o a.out

clean:
	rm -f a.out