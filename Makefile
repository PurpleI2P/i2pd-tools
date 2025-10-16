UNAME := $(shell uname -s)

I2PD_PATH := i2pd
I2PD_LIB := libi2pd.a

LIBI2PD_PATH := $(I2PD_PATH)/libi2pd
LIBI2PD_CLIENT_PATH := $(I2PD_PATH)/libi2pd_client

CXX ?= g++
CXXFLAGS := -Wall -std=c++17 -O2
INCFLAGS := -I$(LIBI2PD_PATH) -I$(LIBI2PD_CLIENT_PATH)
DEFINES := -DOPENSSL_SUPPRESS_DEPRECATED

LDFLAGS := 
LDLIBS := $(I2PD_PATH)/$(I2PD_LIB) -lboost_program_options$(BOOST_SUFFIX) -lssl -lcrypto -lz

ifeq ($(UNAME),Linux)
    CXXFLAGS += -g
    LDLIBS += -lrt -lpthread
else ifeq ($(UNAME),Darwin)
    CXXFLAGS += -g
    LDLIBS += -lpthread

    ifeq ($(shell test -d /opt/homebrew && echo "true"),true)
        BREW_PREFIX := /opt/homebrew
    else
        BREW_PREFIX := /usr/local
    endif

    INCFLAGS += -I$(BREW_PREFIX)/include -I$(BREW_PREFIX)/opt/openssl@3/include
    LDFLAGS += -L$(BREW_PREFIX)/lib -L$(BREW_PREFIX)/opt/openssl@3/lib
    # Используем динамические библиотеки, без указания .a
    LDLIBS += -lboost_system -lboost_program_options
else ifeq ($(UNAME),FreeBSD)
    CXXFLAGS += -g
    LDLIBS += -lthr -lpthread
    LDFLAGS += -L/usr/local/lib
    INCFLAGS += -I/usr/local/include
else
    # Windows
    CXXFLAGS += -Os -fPIC -msse
    DEFINES += -DWIN32_LEAN_AND_MEAN
    LDFLAGS += -L/clang64/lib
    INCFLAGS += -I/clang64/include
    BOOST_SUFFIX =
    LDLIBS += -lwsock32 -lws2_32 -liphlpapi -lpthread
endif

# -------------------------
# Targets
# -------------------------
all: $(I2PD_LIB) vain keygen keyinfo famtool routerinfo regaddr regaddr_3ld i2pbase64 offlinekeys b33address regaddralias x25519 verifyhost autoconf

vain: vain.o $(I2PD_LIB)
	$(CXX) -o vain $(LDFLAGS) vain.o $(LDLIBS)

autoconf: autoconf.o $(I2PD_LIB)
	$(CXX) -o autoconf $(DEFINES) $(LDFLAGS) autoconf.o $(LDLIBS)

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

# -------------------------
# Object compilation
# -------------------------
.SUFFIXES: .c .cc .C .cpp .o

$(I2PD_LIB):
	$(MAKE) -C $(I2PD_PATH) mk_obj_dir $(I2PD_LIB)

%.o: %.cpp $(I2PD_LIB)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCFLAGS) -c -o $@ $<

# -------------------------
# Cleanup
# -------------------------
count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean-i2pd:
	$(MAKE) -C $(I2PD_PATH) clean

clean-obj:
	rm -f $(wildcard *.o)

stripall:
	strip b33address famtool i2pbase64 keygen keyinfo offlinekeys regaddr regaddr_3ld regaddralias routerinfo x25519 verifyhost vain autoconf

builddir:
	mkdir -p build
	mv b33address famtool i2pbase64 keygen keyinfo offlinekeys regaddr regaddr_3ld regaddralias routerinfo x25519 verifyhost vain autoconf build/ || true

clean-bin:
	rm -f b33address famtool i2pbase64 keygen keyinfo offlinekeys regaddr regaddr_3ld regaddralias routerinfo x25519 verifyhost vain autoconf

clean: clean-i2pd clean-obj clean-bin

.PHONY: all count clean-i2pd clean-obj clean-bin clean

