#author Matthias Noack<Ma.Noack@tu-cottbus.de>

SRC_FILES = \
	board/LocalBoard.cc \
	board/BoardServer.cc \
	board/BoardServerMPISimple.cc \
	board/BoardServerMPIAdvanced.cc \
	client/LifeClient.cc \
	client/LifeClientMPISimple.cc \
	client/LifeClientMPIAdvanced.cc \
	gui/DrawingWindow.cc \
	gui/BoardDrawingWindow.cc \
	misc/Log.cc \
	net/UDPNetwork.cc \
	net/TCPNetwork.cc \


SRC_SERVER = main_server.cc
SRC_CLIENT = main_client.cc
SRC_LOCAL = main_local.cc
SRC_MPI = main_mpi.cc

TMP_DIR = ./tmp/
BIN_DIR = ./bin/
SRC_DIR = ./src/
DOC_DIR = ./doc/
INC_DIR = ./include/
DEP_DIR = $(TMP_DIR)
# configure file ending, .cc or .c++
CXX_FILE_ENDING = .cc

# configure compiler
#ASM = g++
#CXX = g++
#LD	= g++
ASM = mpic++
CXX = mpic++
LD = mpic++

CXXFLAGS = -g -Wall -std=c++11
ifeq ($(DEBUG),1)
CXXFLAGS += -DDEBUG_MODE
endif

INCLUDES = -I$(INC_DIR)

XLIBDIR   = /usr/X11R6/lib/
XLIBS   = -L$(XLIBDIR) -lX11
PTHREADLIBS = -lpthread
BOOSTLIBDIR = -/home/vpsi1/boost/lib
BOOSTLIB = -L$(BOOSTLIBDIR) -lboost_program_options
LIBS = $(XLIBS) $(PTHREADLIBS) $(BOOSTLIB)

TARGET_SERVER = $(BIN_DIR)server
TARGET_CLIENT = $(BIN_DIR)client
TARGET_LOCAL = $(BIN_DIR)local
TARGET_MPI = $(BIN_DIR)mpi

######## end of configureable part ########
SRCS = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_FILES)))
OBJS_SERVER = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_SERVER)))
OBJS_CLIENT = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_CLIENT)))
OBJS_LOCAL = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_LOCAL)))
OBJS_MPI = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.o, $(SRC_MPI)))
DEPS = $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_FILES)))
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_SERVER))) 
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_CLIENT))) 
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_LOCAL))) 
DEPS += $(addprefix $(TMP_DIR), $(subst $(CXX_FILE_ENDING),.d, $(SRC_MPI))) 
TMP_SUBDIRS = $(sort $(dir $(OBJS)))
.PHONY: clean all depend

all: depend $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_LOCAL) $(TARGET_MPI)

$(TARGET_SERVER): $(OBJS) $(OBJS_SERVER)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_SERVER) $(LIBS) -o $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJS) $(OBJS_CLIENT)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_CLIENT) $(LIBS) -o $(TARGET_CLIENT)

$(TARGET_LOCAL): $(OBJS) $(OBJS_LOCAL)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_LOCAL) $(LIBS) -o $(TARGET_LOCAL)

$(TARGET_MPI): $(OBJS) $(OBJS_MPI)
	$(LD) $(LDFLAGS) $(OBJS) $(OBJS_MPI) $(LIBS) -o $(TARGET_MPI)

$(TMP_SUBDIRS) :
	mkdir -p $@

$(TMP_DIR)%.o : $(SRC_DIR)%$(CXX_FILE_ENDING) $(TMP_SUBDIRS)
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(shell mkdir -p $(BIN_DIR))

depend: $(SRCS) $(TMP_SUBDIRS)
	$(CXX) -MD -E $(CXXFLAGS) $(SRCS) $(INCLUDES) > /dev/null
	mv *.d $(DEP_DIR)

server: $(TARGET_SERVER)

client: $(TARGET_CLIENT)

local: $(TARGET_LOCAL)

mpi: $(TARGET_MPI)

clean:
	rm -rf $(OBJS) $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_LOCAL) $(OBJS_MPI) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_LOCAL) $(TARGET_MPI) $(DEPS) $(TMP_SUBDIRS)

# generated dependencies
-include $(DEPS)

