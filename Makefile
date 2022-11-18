#author Matthias Noack<Ma.Noack@tu-cottbus.de>

SRC_FILES = \
	board/LocalBoard.cc \
	board/BoardServer.cc \
	gui/DrawingWindow.cc \
	gui/BoardDrawingWindow.cc \
	misc/Log.cc \
	net/UDPNetwork.cc \


SRC_SERVER = server.cc
SRC_CLIENT = client.cc
SRC_LOCAL = main.cc

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

CXXFLAGS = -g -Wall -std=c++11
ifeq ($(DEBUG),1)
CXXFLAGS += -DDEBUG_MODE
endif

INCLUDES = -I$(INC_DIR)

XLIBDIR   = /usr/X11R6/lib/
XLIBS   = -L$(XLIBDIR) -lX11
PTHREADLIBS = -lpthread
BOOSTLIBS = -lboost_program_options
LIBS = $(XLIBS) $(PTHREADLIBS) $(BOOSTLIBS)

TARGET_SERVER = $(BIN_DIR)server
TARGET_CLIENT = $(BIN_DIR)client
TARGET_LOCAL = $(BIN_DIR)main

######## end of configureable part ########
SRCS = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_FILES)))
OBJS_SERVER = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_SERVER)))
OBJS_CLIENT = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_CLIENT)))
OBJS_LOCAL = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_LOCAL)))
DEPS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_FILES)))
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_SERVER))) 
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_CLIENT))) 
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_LOCAL))) 
TMP_SUBDIRS = $(sort $(dir $(OBJS)))
.PHONY: clean all depend

all: depend $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_LOCAL)

$(TARGET_SERVER): $(OBJS) $(OBJS_SERVER)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_SERVER) $(LIBS) -o $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJS) $(OBJS_CLIENT)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_CLIENT) $(LIBS) -o $(TARGET_CLIENT)

$(TARGET_LOCAL): $(OBJS) $(OBJS_LOCAL)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_LOCAL) $(LIBS) -o $(TARGET_LOCAL)

$(TMP_SUBDIRS) :
	mkdir -p $@

$(TMP_DIR)%.o : $(SRC_DIR)%$(CXX_FILE_ENDING) $(TMP_SUBDIRS)
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

depend: $(SRCS) $(TMP_SUBDIRS)
	$(CXX) -MD -E $(CXXFLAGS) $(SRCS) $(INCLUDES) > /dev/null
	mv *.d $(DEP_DIR)

server: $(TARGET_SERVER)

client: $(TARGET_CLIENT)

local: $(TARGET_LOCAL)

clean:
	rm -rf $(OBJS) $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_LOCAL) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_LOCAL) $(DEPS) $(TMP_SUBDIRS)

# generated dependencies
-include $(DEPS)

