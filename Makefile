UNAME = $(shell uname -s)

I2PD_PATH = i2pd
I2PD_LIB = libi2pd.a

LIBI2PD_PATH = $(I2PD_PATH)/libi2pd
LIBI2PD_CLIENT_PATH = $(I2PD_PATH)/libi2pd_client

CXX ?= g++
CXXFLAGS = -Wall -std=c++17
INCFLAGS = -I$(LIBI2PD_PATH) -I$(LIBI2PD_CLIENT_PATH)
DEFINES = -DOPENSSL_SUPPRESS_DEPRECATED

LDFLAGS = 
LDLIBS = $(I2PD_PATH)/$(I2PD_LIB) -lboost_system$(BOOST_SUFFIX) -lboost_date_time$(BOOST_SUFFIX) -lboost_filesystem$(BOOST_SUFFIX) -lboost_program_options$(BOOST_SUFFIX) -lssl -lcrypto -lz

ifeq ($(UNAME),Linux)
	CXXFLAGS += -g
else ifeq ($(UNAME),Darwin)
	CXXFLAGS += -g
else ifeq ($(UNAME),FreeBSD)
	CXXFLAGS += -g
else
# Win32
	CXXFLAGS += -Os -fPIC -msse
	DEFINES += -DWIN32_LEAN_AND_MEAN
	BOOST_SUFFIX = -mt
endif

ifeq ($(UNAME),Linux)
	LDLIBS += -lrt -lpthread
else ifeq ($(UNAME),Darwin)
	LDLIBS += -lpthread
	LDFLAGS += -L/usr/local/opt/openssl@1.1/lib -L/usr/local/lib
	INCFLAGS += -I/usr/local/opt/openssl@1.1/include -I/usr/local/include
else ifeq ($(UNAME),FreeBSD)
	LDLIBS += -lthr -lpthread
	LDFLAGS += -L/usr/local/lib
	INCFLAGS += -I/usr/local/include
else
# Win32
	LDLIBS += -lwsock32 -lws2_32 -liphlpapi -lpthread
	LDFLAGS += -s -static
endif

all: $(I2PD_LIB) keygen keyinfo famtool routerinfo regaddr regaddr_3ld vain i2pbase64 offlinekeys b33address regaddralias x25519 verifyhost

routerinfo: routerinfo.o $(I2PD_LIB)
	$(CXX) -o routerinfo $(LDFLAGS) routerinfo.o $(LDLIBS)

keygen: keygen.o $(I2PD_LIB)
	$(CXX) -o keygen $(DEFINES) $(LDFLAGS) keygen.o $(LDLIBS)

keyinfo: keyinfo.o $(I2PD_LIB)
	$(CXX) -o keyinfo $(DEFINES) $(LDFLAGS) keyinfo.o $(LDLIBS)

famtool: famtool.o $(I2PD_LIB)
	$(CXX) -o famtool $(DEFINES) $(LDFLAGS) famtool.o $(LDLIBS)

regaddr: regaddr.o $(I2PD_LIB)
	$(CXX) -o regaddr $(DEFINES) $(LDFLAGS) regaddr.o $(LDLIBS)

regaddr_3ld: regaddr_3ld.o $(I2PD_LIB)
	$(CXX) -o regaddr_3ld $(DEFINES) $(LDFLAGS) regaddr_3ld.o $(LDLIBS)

vain: vanitygen.o $(I2PD_LIB)
	$(CXX) -o vain $(DEFINES) $(LDFLAGS) vanitygen.o $(LDLIBS)

i2pbase64: i2pbase64.o $(I2PD_LIB)
	$(CXX) -o i2pbase64 $(DEFINES) $(LDFLAGS) i2pbase64.o $(LDLIBS)

offlinekeys: offlinekeys.o $(I2PD_LIB)
	$(CXX) -o offlinekeys $(DEFINES) $(LDFLAGS) offlinekeys.o $(LDLIBS)

b33address: b33address.o $(I2PD_LIB)
	$(CXX) -o b33address $(DEFINES) $(LDFLAGS) b33address.o $(LDLIBS)

regaddralias: regaddralias.o $(I2PD_LIB)
	$(CXX) -o regaddralias $(DEFINES) $(LDFLAGS) regaddralias.o $(LDLIBS)

x25519: x25519.o $(I2PD_LIB)
	$(CXX) -o x25519 $(DEFINES) $(LDFLAGS) x25519.o $(LDLIBS)

verifyhost: verifyhost.o $(I2PD_LIB)
	$(CXX) -o verifyhost $(DEFINES) $(LDFLAGS) verifyhost.o $(LDLIBS)

.SUFFIXES:
.SUFFIXES: .c .cc .C .cpp .o

$(I2PD_LIB):
	$(MAKE) -C $(I2PD_PATH) mk_obj_dir $(I2PD_LIB)

%.o: %.cpp $(I2PD_LIB)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCFLAGS) -c -o $@ $<

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean-i2pd:
	$(MAKE) -C $(I2PD_PATH) clean

clean-obj:
	rm -f $(wildcard *.o)

clean-bin:
	rm -f b33address famtool i2pbase64 keygen keyinfo offlinekeys regaddr regaddr_3ld regaddralias routerinfo vain x25519 verifyhost

clean: clean-i2pd clean-obj clean-bin

.PHONY: all
.PHONY: count
.PHONY: clean-i2pd
.PHONY: clean-obj
.PHONY: clean-bin
.PHONY: clean
