UNAME = $(shell uname -s)

I2PD_PATH = i2pd
LIBI2PD_PATH = $(I2PD_PATH)/libi2pd
LIBI2PD_CLIENT_PATH = $(I2PD_PATH)/libi2pd_client
CXX = g++
FLAGS = -Wall -std=c++11 -Wno-misleading-indentation

ifeq ($(UNAME),Linux)
	FLAGS += -g
else
# Win32
	FLAGS += -Os -D_MT -DWIN32 -D_WINDOWS -DWIN32_LEAN_AND_MEAN
	BOOST_SUFFIX = -mt
endif

INCFLAGS = -I$(LIBI2PD_PATH) -I$(LIBI2PD_CLIENT_PATH) -I$(I2PD_PATH)
CXXFLAGS = $(FLAGS)
LDFLAGS = -Wl,-rpath,/usr/local/lib
LIBS = $(I2PD_PATH)/libi2pd.a -lboost_system$(BOOST_SUFFIX) -lboost_date_time$(BOOST_SUFFIX) -lboost_filesystem$(BOOST_SUFFIX) -lboost_program_options$(BOOST_SUFFIX) -lssl -lcrypto -lz

ifeq ($(UNAME),Linux)
	LIBS += -lrt -lpthread
else
# Win32
	LIBS += -lws2_32 -lwsock32 -lgdi32 -liphlpapi -lstdc++ -lpthread
	LDFLAGS += -Wl,-Bstatic -static-libgcc -static-libstdc++
endif

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
	$(CXX) -o vain vanitygen.o $(LDFLAGS) -mavx $(LIBS)

$(OBJECTS): libi2pd.a

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

$(I2PD_LIB):
	$(MAKE) -C $(I2PD_PATH) mk_obj_dir $(I2PD_LIB)

%.o: %.cpp libi2pd.a
	$(CXX) -o $@ -c $(CXXFLAGS) $(INCFLAGS) $<

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean-i2pd:
	$(MAKE) -C $(I2PD_PATH) clean

clean-obj:
	rm -f $(OBJECTS)

clean-bin:
	rm -f keyinfo keygen famtool regaddr routerinfo vain

clean: clean-i2pd clean-obj clean-bin

.PHONY: all
.PHONY: count
.PHONY: clean
