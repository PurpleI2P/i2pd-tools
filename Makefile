UNAME = $(shell uname -s)

I2PD_PATH = i2pd
I2PD_LIB = libi2pd.a

LIBI2PD_PATH = $(I2PD_PATH)/libi2pd
LIBI2PD_CLIENT_PATH = $(I2PD_PATH)/libi2pd_client
CXX ?= g++
FLAGS = -Wall -std=c++11 -Wno-misleading-indentation

ifeq ($(UNAME),Linux)
	FLAGS += -g
else ifeq ($(UNAME),Darwin)
	FLAGS += -g
else ifeq ($(UNAME),FreeBSD)
	FLAGS += -g
else
# Win32
	FLAGS += -Os -D_MT -DWIN32 -D_WINDOWS -DWIN32_LEAN_AND_MEAN
	BOOST_SUFFIX = -mt
endif

INCFLAGS = -I$(LIBI2PD_PATH) -I$(LIBI2PD_CLIENT_PATH)
CXXFLAGS = $(FLAGS)
LDFLAGS = 
LIBS = $(I2PD_PATH)/$(I2PD_LIB) -lboost_system$(BOOST_SUFFIX) -lboost_date_time$(BOOST_SUFFIX) -lboost_filesystem$(BOOST_SUFFIX) -lboost_program_options$(BOOST_SUFFIX) -lssl -lcrypto -lz

ifeq ($(UNAME),Linux)
	LIBS += -lrt -lpthread
else ifeq ($(UNAME),Darwin)
	LIBS += -lpthread
	LDFLAGS += -L/usr/local/opt/openssl@1.1/lib -L/usr/local/lib 
	INCFLAGS += -I/usr/local/opt/openssl@1.1/include -I/usr/local/include
else ifeq ($(UNAME),FreeBSD)
	LIBS += -lthr -lpthread
	LDFLAGS += -L/usr/local/lib 
	INCFLAGS += -I/usr/local/include
else
# Win32
	LIBS += -lws2_32 -lwsock32 -lgdi32 -liphlpapi -lstdc++ -lpthread
	LDFLAGS += -s -Wl,-Bstatic -static-libgcc -static-libstdc++
endif

all: $(I2PD_LIB) keygen keyinfo famtool routerinfo regaddr regaddr_3ld vain i2pbase64 offlinekeys b33address regaddralias

routerinfo: routerinfo.o $(I2PD_LIB)
	$(CXX) -o routerinfo routerinfo.o $(LDFLAGS) $(LIBS)

keygen: keygen.o $(I2PD_LIB)
	$(CXX) -o keygen keygen.o $(LDFLAGS) $(LIBS)

keyinfo: keyinfo.o $(I2PD_LIB)
	$(CXX) -o keyinfo keyinfo.o $(LDFLAGS) $(LIBS)

famtool: famtool.o $(I2PD_LIB)
	$(CXX) -o famtool famtool.o $(LDFLAGS) $(LIBS)

regaddr: regaddr.o $(I2PD_LIB)
	$(CXX) -o regaddr regaddr.o $(LDFLAGS) $(LIBS)

regaddr_3ld: regaddr_3ld.o $(I2PD_LIB)
	$(CXX) -o regaddr_3ld regaddr_3ld.o $(LDFLAGS) $(LIBS)

vain: vanitygen.o $(I2PD_LIB)
	$(CXX) -o vain vanitygen.o $(LDFLAGS) $(LIBS)

i2pbase64: i2pbase64.o $(I2PD_LIB)
	$(CXX) -o i2pbase64 i2pbase64.o $(LDFLAGS) $(LIBS)

offlinekeys: offlinekeys.o $(I2PD_LIB)
	$(CXX) -o offlinekeys offlinekeys.o $(LDFLAGS) $(LIBS)

b33address: b33address.o $(I2PD_LIB)
	$(CXX) -o b33address b33address.o $(LDFLAGS) $(LIBS)

regaddralias: regaddralias.o $(I2PD_LIB)
	$(CXX) -o regaddralias regaddralias.o $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES: .c .cc .C .cpp .o

$(I2PD_LIB):
	$(MAKE) -C $(I2PD_PATH) mk_obj_dir $(I2PD_LIB)

%.o: %.cpp $(I2PD_LIB)
	$(CXX) -o $@ -c $(CXXFLAGS) $(INCFLAGS) $<

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean-i2pd:
	$(MAKE) -C $(I2PD_PATH) clean

clean-obj:
	rm -f $(wildcard *.o)

clean-bin:
	rm -f b33address famtool i2pbase64 keygen keyinfo offlinekeys regaddr regaddr_3ld regaddralias routerinfo vain

clean: clean-i2pd clean-obj clean-bin

.PHONY: all
.PHONY: count
.PHONY: clean-i2pd
.PHONY: clean-obj
.PHONY: clean-bin
.PHONY: clean
