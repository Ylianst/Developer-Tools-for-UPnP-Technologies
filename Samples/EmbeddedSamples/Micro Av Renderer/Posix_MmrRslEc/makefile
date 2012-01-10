
#
# Intel's UPnP MicroStack/1.0, Sample MakeFile */
# Intel Device Builder - Build#1.0.1068.25676
#

# List all object files here
OFILES = \
	Emulator_Methods.o\
	ILibParsers.o\
	ILibWebClient.o\
	ILibWebServer.o\
	ILibAsyncSocket.o\
	ILibAsyncServerSocket.o\
	UpnpMicroStack.o\
	MicroMediaRenderer.o\
	CodecWrapper.o\
	RendererStateLogic.o\
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
# If enabling the TRANSITION state, then you need the _USE_TRANSIT_STATE #define.
# If enabling M3U caching, then you need the _M3U_BUFFER #define.
# -g puts debug symbols
#
CFLAGS = -Os -Wall -D_POSIX -D_DEBUG -D_M3U_BUFFER -D_UNIX -I./Posix_MmrRslEc -I../Posix_MmrRslEc -I../ -I./ -I./DeviceBuilder/Posix -I../DeviceBuilder/Posix -g -lpthread
 

# Search paths
VPATH = src: ./ ../ ./Posix_MmrRslEc ../Posix_MmrRslEc ./DeviceBuilder/Posix ../DeviceBuilder/Posix 

# Name of the executable
EXENAME = RendererPosix_MmrRslEc

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES)

# Macro rule for all object files.
$(OFILES) : \
	Emulator_Methods.h\
	ILibParsers.h\
	ILibWebClient.h\
	ILibWebServer.h\
	ILibAsyncSocket.h\
	ILibAsyncServerSocket.h\
	UpnpMicroStack.h\
	MicroMediaRenderer.h\
	CodecWrapper.h\
	RendererStateLogic.h\
	Utility.h\
	PortFn_MsTime.h\
	HttpPlaylistParser.h\
	MyString.h

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
