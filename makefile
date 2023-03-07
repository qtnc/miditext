EXENAME=miditext
DEFINES=$(options) HAVE_W32API_H __WXMSW__ _UNICODE WXUSINGDLL NOPCH

ifeq ($(OS),Windows_NT)
EXT_EXE=.exe
EXT_DLL=.dll
else
EXT_EXE=
EXT_DLL=.so
endif

ifeq ($(mode),release)
NAME_SUFFIX=
DEFINES += RELEASE
CXXOPTFLAGS=-s -O3
else
NAME_SUFFIX=d
DEFINES += DEBUG
CXXOPTFLAGS=-g
endif

EXECUTABLE=$(EXENAME)$(NAME_SUFFIX)$(EXT_EXE)
OBJDIR=obj$(NAME_SUFFIX)/

CXX=g++
WINDRES=windres
WINDRESFLAGS=$(addprefix -D,$(DEFINES)) -I"$(dir $(shell where $(WINDRES)))..\i686-w64-mingw32\include"
CXXFLAGS=-std=gnu++17 -Wextra $(addprefix -D,$(DEFINES)) -mthreads
LDFLAGS=-lwxbase31u -lwxmsw31u_core -lws2_32 -L. -lbass -lbass_fx -lbassmidi -lbassmix -lbassenc -lbassenc_mp3 -lbassenc_ogg -lbassenc_flac -lbassenc_opus -lboost-regex -lfmt -llua -lUniversalSpeech -liphlpapi -mthreads -mthreads -mwindows

SRCS=$(wildcard src/app/*.cpp) $(wildcard src/common/*.cpp)
RCSRCS=$(wildcard src/app/*.rc)
OBJS=$(addprefix $(OBJDIR),$(SRCS:.cpp=.o))
RCOBJS=$(addprefix $(OBJDIR)rsrc/,$(RCSRCS:.rc=.o))
PERCENT=%

all: $(EXECUTABLE)

.PHONY: $(EXECUTABLE)

clean:
	rm -r $(OBJDIR)

$(EXECUTABLE): $(RCOBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp $(wildcard %.hpp)
	mkdir.exe -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -c -o $@ $<

$(OBJDIR)rsrc/%.o: %.rc
	mkdir.exe -p $(dir $@)
	$(WINDRES) $(WINDRESFLAGS) -o $@ $<

