EXENAME=miditext
DEFINES=$(options) HAVE_W32API_H __WXMSW__ _UNICODE WXUSINGDLL NOPCH

VERSION_MAJOR=$(shell lua -e "print(os.date('%Y')+0)")
VERSION_MINOR=$(shell lua -e "print(os.date('%m')+0)")
VERSION_BUILD_MAJOR=$(shell lua -e "print(os.date('%d')+0)")
VERSION_BUILD_MINOR=$(shell lua -e "print(os.date('%H')+0)")
VERSION_STRING=$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD_MAJOR)

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
WINDRESFLAGS=$(addprefix -D,$(DEFINES)) -I"$(CPATH)" -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_BUILD_MAJOR=$(VERSION_BUILD_MAJOR) -DVERSION_BUILD_MINOR=$(VERSION_BUILD_MINOR)
CXXFLAGS=-std=gnu++17 -Wextra -mthreads $(addprefix -D,$(DEFINES)) -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_BUILD_MAJOR=$(VERSION_BUILD_MAJOR) -DVERSION_BUILD_MINOR=$(VERSION_BUILD_MINOR) -DVERSION_STRING=\"$(VERSION_STRING)\"
LDFLAGS=-lwxbase33u -lwxmsw33u_core -lws2_32 -lbass -lbass_fx -lbassmidi -lbassmix -lbassenc -lbassenc_mp3 -lbassenc_ogg -lbassenc_flac -lbassenc_opus -lboost_regex -lfmt -llua -lUniversalSpeech -liphlpapi -mthreads -mthreads -mwindows

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

