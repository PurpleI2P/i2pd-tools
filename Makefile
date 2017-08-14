I2PD_PATH = i2pd
LIBI2PD_PATH = $(I2PD_PATH)/libi2pd
LIBI2PD_CLIENT_PATH = $(I2PD_PATH)/libi2pd_client
CXX = g++
FLAGS = -g -Wall -std=c++11
INCFLAGS = -I$(LIBI2PD_PATH) -I$(LIBI2PD_CLIENT_PATH) -I$(I2PD_PATH)
CXXFLAGS = $(FLAGS) $(INCFLAGS)
LDFLAGS = -Wl,-rpath,/usr/local/lib
LIBS = $(I2PD_PATH)/libi2pd.a -lboost_system -lboost_date_time -lboost_filesystem -lboost_program_options -lssl -lcrypto -lpthread -lrt -lz

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
I2PD_LIB = libi2pd.a


all: keygen keyinfo famtool routerinfo regaddr vain

routerinfo: $(OBJECTS)
	$(CXX) -o routerinfo routerinfo.o $(LDFLAGS) $(LIBS)

keygen: $(OBJECTS)
	$(CXX) -o keygen keygen.o $(LDFLAGS) $(LIBS)

keyinfo: $(OBJECTS)
	$(CXX) -o keyinfo keyinfo.o $(LDFLAGS) $(LIBS)

famtool: $(OBJECTS)
	$(CXX) -o famtool famtool.o $(LDFLAGS) $(LIBS)

regaddr: $(OBJECTS)
	$(CXX) -o regaddr regaddr.o $(LDFLAGS) $(LIBS)

vain: $(OBJECTS)
	$(CXX) -o vain vanitygen.o $(LDFLAGS) $(LIBS)

$(OBJECTS): libi2pd.a

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

$(I2PD_LIB):
	$(MAKE) -C $(I2PD_PATH) mk_obj_dir $(I2PD_LIB)

%.o: %.cpp libi2pd.a
	$(CXX) -o $@ -c $(CXXFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean-i2pd:
	$(MAKE) -C $(I2PD_PATH) clean

clean-obj:
	rm -f $(OBJECTS)

clean-bin:
	rm -f keyinfo keygen famtool regaddr routerinfo

clean: clean-i2pd clean-obj clean-bin

.PHONY: all
.PHONY: count
.PHONY: clean
