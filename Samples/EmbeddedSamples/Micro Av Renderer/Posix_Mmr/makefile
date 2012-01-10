
#
# Intel's UPnP MicroStack/1.0, Sample MakeFile */
# Intel Device Builder - Build#1.0.1068.25676
#

# List all object files here
OFILES = \
	ILibParsers.o\
	ILibHTTPClient.o\
	UpnpMicroStack.o\
	MicroMediaRenderer.o\
	Utility.o\
	PortFn_MsTime.o\
	HttpPlaylistParser.o\
	MyString.o\
	Main.o 

# Compiler command name
CC = gcc

# Compiler flags applied to all files
# Optional flags: -D_VERBOSE -D_DEBUG -D_DEBUG_CODECWRAPPER -D_TEMPDEBUG
# If using the sample codec-wrapper-emulator, then you need the _UNIX #define.
# -g puts debug symbols
#
CFLAGS = -Os -Wall -D_POSIX -D_DEBUG -D_UNIX -I./Posix_Mmr -I../ -I./ -I./DeviceBuilder/Posix -I../DeviceBuilder/Posix -g -lpthread
 

# Search paths
VPATH = src: ./ ../ ./Posix_Mmr ./DeviceBuilder/Posix ../DeviceBuilder/Posix 

# Name of the executable
EXENAME = RendererPosix_Mmr

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES)

# Macro rule for all object files.
$(OFILES) : \
	ILibParsers.h\
	ILibHTTPClient.h\
	UpnpMicroStack.h\
	MicroMediaRenderer.h\
	Utility.h\
	PortFn_MsTime.h\
	HttpPlaylistParser.h\
	MyString.h

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
