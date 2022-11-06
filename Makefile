#author Matthias Noack <Ma.Noack@tu-cottbus.de>

SRC_FILES = \
	misc/Log.cc \
	gui/DrawingWindow.cc \
	gui/BoardDrawer.cc \
	board/LocalBoard.cc \

	
SRC_MAIN = main.cc

TMP_DIR = ./tmp/
BIN_DIR = ./bin/
SRC_DIR = ./src/
DOC_DIR = ./doc/
INC_DIR = ./include/
DEP_DIR = $(TMP_DIR)
# configure file ending, .cc or .c++
CXX_FILE_ENDING = .cc

ASM	= g++
CXX	= g++
LD	= g++

CXXFLAGS = -g -Wall
#CXXFLAGS += -DDEBUG_MODE

INCLUDES = -I$(INC_DIR)

XLIBDIR   = /usr/X11R6/lib/
XLIBS   = -L$(XLIBDIR) -lX11
PTHREADLIBS = -lpthread
LIBS = $(XLIBS) $(PTHREADLIBS)

TARGET_MAIN = $(BIN_DIR)main

MKDIR_P = mkdir -p

######## end of configureable part ########
SRCS = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_FILES)))
OBJS_MAIN = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_MAIN)))
DEPS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_FILES)))
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_MAIN))) 
TMP_SUBDIRS = $(sort $(dir $(OBJS)))
.PHONY: clean all depend debug

all: depend directories $(TARGET_MAIN)

$(TARGET_MAIN):	$(OBJS) $(OBJS_MAIN)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_MAIN) $(LIBS) -o $(TARGET_MAIN)

$(TMP_SUBDIRS) :
	$(MKDIR_P) $@

$(TMP_DIR)%.o : $(SRC_DIR)%$(CXX_FILE_ENDING) $(TMP_SUBDIRS)
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

depend: $(SRCS) $(TMP_SUBDIRS)
	$(CXX) -MD -E $(CXXFLAGS) $(SRCS) $(INCLUDES) > /dev/null
	mv *.d $(DEP_DIR)

debug: CXXFLAGS += -DDEBUG_MODE -g
debug: all

main: directories $(TARGET_MAIN)

clean:
	rm -rf $(OBJS) $(OBJS_MAIN) $(TARGET_MAIN) $(DEPS) $(TMP_SUBDIRS)

directories:
	$(MKDIR_P) $(BIN_DIR)

# generated dependencies
-include $(DEPS)

