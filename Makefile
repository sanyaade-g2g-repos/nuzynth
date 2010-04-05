NAME = Nuzynth

CC = llvm-g++
SRCDIR = src
BINDIR = bin
OBJFILES := $(addprefix $(BINDIR)/,$(patsubst %.cpp,%.o,$(notdir $(wildcard $(SRCDIR)/*.cpp))))



UNAME := $(shell uname -s)

ifeq ($(UNAME),Darwin)
	ARCH = -arch i386
	BUNDLE = $(NAME).app
	TARGET = $(BUNDLE)/Contents/MacOS/$(NAME)
	CXXFLAGS = -Wall `/usr/local/bin/wx-config --cppflags` -O3 \
	                 -arch i386 -mmacosx-version-min=10.5
	LIBRARIES = `/usr/local/bin/wx-config --libs std,gl` \
	            `llvm-config --ldflags --libs` \
	            -lportaudio -lportmidi -lfftw3f \
	            -framework CoreAudio \
	            -framework AudioToolbox \
	            -framework AudioUnit \
	            -framework Carbon \
	            -framework CoreMIDI \
	            -arch i386 -mmacosx-version-min=10.5
endif

ifeq ($(UNAME),MINGW32)
	TARGET = $(NAME).exe
	CXXFLAGS = `/c/msys/1.0/wxbuild/wx-config --cxxflags` \
			   -I/c/msys/1.0/llvm/include \
			   -I/c/msys/1.0/portaudio/include 
	LIBRARIES = -mwindows `/c/msys/1.0/wxbuild/wx-config --libs gl` `/c/msys/1.0/llvm-configure/Release/bin/llvm-config --ldflags --libs` -L/c/msys/1.0/portaudio/lib/.libs -lportaudio -lfftw3f -limagehlp -lpsapi
endif



all: $(BINDIR) $(TARGET)

$(TARGET): $(OBJFILES) synthjit/SynthJIT.o
ifeq ($(UNAME),Darwin)
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	mkdir $(BUNDLE)/Contents
	mkdir $(BUNDLE)/Contents/MacOS/
else
	rm -f $(TARGET)
endif
	$(CC) -o $@ $^ $(LIBRARIES)

$(BINDIR): 
	mkdir $(BINDIR)

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $(BINDIR)/$*.o $(SRCDIR)/$*.cpp $(CXXFLAGS)

synthjit/SynthJIT.o: synthjit/SynthJIT.cpp synthjit/synth.c
	llvm-g++ -O3 -emit-llvm synthjit/synth.c -c -o synthjit/synth.bc $(ARCH)
	(echo "const char synth_bitcode[] = {"; od -txC   synthjit/synth.bc   | sed -e "s/^[0-9]*//" -e s"/ \([0-9a-f][0-9a-f]\)/0x\1,/g" -e"\$$d" | sed -e"\$$s/,$$/, 0x00};/") >synthjit/synth-data.h
	$(CC) -c -o synthjit/SynthJIT.o synthjit/SynthJIT.cpp $(CXXFLAGS) `llvm-config --cxxflags`


clean: 
	rm -rf $(BINDIR)
	rm -f $(TARGET)
	rm -rf $(BUNDLE)
	rm -f synthjit/SynthJIT.o
	rm -f synthjit/synth-data.h
	rm -f synthjit/synth.bc

.PHONY: clean all
