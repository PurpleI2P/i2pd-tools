CXX = g++
CXXFLAGS = -g -Wall -std=c++11
OBJECTS = keygen.o
INCFLAGS = -I"i2pd"
LDFLAGS = -Wl,-rpath,/usr/local/lib 
LIBS = ../i2pd/libi2pd.a -lboost_system -lboost_date_time -lboost_filesystem -lboost_program_options -lssl -lcrypto -lpthread -lrt -lz 

all: keygen

keygen: $(OBJECTS)
	$(CXX) -o keygen $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

.cpp.o :
	$(CXX) -o $@ -c $(CXXFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o keygen

.PHONY: all
.PHONY: count
.PHONY: clean
