# List all object files here
OFILES = \
	CdsMediaClass.o\
	CdsMediaObject.o\
	CdsObjectToDidl.o\
	ILibParsers.o\
	ILibWebClient.o\
	ILibWebServer.o\
	ILibAsyncSocket.o\
	ILibAsyncServerSocket.o\
	UpnpMicroStack.o\
	MicroMediaServer.o\
	MyString.o\
	MimeTypes.o\
	PortingFunctions.o\
	mms.o 
	
# Compiler command name
CC = gcc

# Compiler flags applied to all files
# Optional flags: -D_VERBOSE -D_DEBUG -DSPAWN_BROWSE_THREAD -D_TEMPDEBUG
# -g puts debug symbols
# -DSPAWN_BROWSE_THREAD makes it so each browse thread spawns a thread (recommended only if browse request takes a long time)
#
CFLAGS = -Os -Wall -D_POSIX -D_DEBUG -I./Posix -I../ -I./ -I./DeviceBuilder/Posix -I../DeviceBuilder/Posix -g -lpthread

# Search paths
VPATH = src: ./ ../ ./Posix ./DeviceBuilder/Posix ../DeviceBuilder/Posix 

# Name of the executable
EXENAME = mms

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES)

# Macro rule for all object files.
$(OFILES) : \
	ILibParsers.h\
	ILibWebClient.h\
	ILibWebServer.h\
	ILibAsyncSocket.h\
	ILibAsyncServerSocket.h\
	UpnpMicroStack.h\
	MicroMediaServer.h\
	MyString.h\
	MimeTypes.h\
	PortingFunctions.h

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
