CXX = g++
WTP_SRC = src
WTP_INCLUDE = inc
COMMON_SRC = common
IPP_PATH = /opt/ipp/6.1.0.039/ia32
IPP_INCLUDE_DIR = $(IPP_PATH)/include
IPP_STATIC_INCLUDE_DIR = $(IPP_PATH)/tools/staticlib
IPP_STATIC_LIB_DIR = $(IPP_PATH)/lib


OBJECTS = WtpStatus.o WtpCommand.o Misc.o WtpException.o WtpImage.o Port.o ProtocolManager.o USBPortLinux.o UARTPortLinux.o TimParser.o CommandlineParser.o TimLib.o WtpDownloadApp.o \


CXXFLAGS = -Wno-deprecated -D LINUX=1 -D TOOLS_GUI=0 -D IPPV6=1 -D TRUSTED=1 -D DEV_DEBUG=0 -I $(COMMON_SRC)

WTP_CXXFLAGS = $(CXXFLAGS) -I $(WTP_INCLUDE)

#LIBS = $(IPP_STATIC_LIB_DIR)/libippcpmerged.a $(IPP_STATIC_LIB_DIR)/libippcore.a

WtpDownload: $(OBJECTS)
		$(CXX) $(WTP_CXXFLAGS) -o WtpDownload_linux $(OBJECTS) $(LIBS)
		-rm *.o

WtpDownloadApp.o: $(WTP_SRC)/WtpDownloadApp.cpp $(WTP_INCLUDE)/WtpDownloadApp.h
		$(CXX) $(WTP_CXXFLAGS) -o WtpDownloadApp.o -c $(WTP_SRC)/WtpDownloadApp.cpp

WtpException.o:	$(WTP_SRC)/WtpException.cpp $(WTP_INCLUDE)/WtpException.h
		$(CXX) $(WTP_CXXFLAGS) -o WtpException.o -c $(WTP_SRC)/WtpException.cpp

WtpStatus.o: 	$(COMMON_SRC)/WtpStatus.cpp 	$(COMMON_SRC)/WtpStatus.h
		$(CXX) $(WTP_CXXFLAGS) -o WtpStatus.o -c $(COMMON_SRC)/WtpStatus.cpp

Misc.o: 	$(WTP_SRC)/Misc.cpp 		$(WTP_INCLUDE)/Misc.h
		$(CXX) $(WTP_CXXFLAGS) -o Misc.o -c $(WTP_SRC)/Misc.cpp

WtpCommand.o:	$(COMMON_SRC)/WtpCommand.cpp 	$(COMMON_SRC)/WtpCommand.h
		$(CXX) $(WTP_CXXFLAGS) -o WtpCommand.o -c $(COMMON_SRC)/WtpCommand.cpp

WtpImage.o:	$(WTP_SRC)/WTPImage.cpp		$(WTP_INCLUDE)/WtpImage.h
		$(CXX) $(WTP_CXXFLAGS) -o WtpImage.o -c $(WTP_SRC)/WTPImage.cpp

Port.o:		$(WTP_SRC)/Port.cpp		$(WTP_INCLUDE)/Port.h
		$(CXX) $(WTP_CXXFLAGS) -o Port.o -c $(WTP_SRC)/Port.cpp

ProtocolManager.o: $(WTP_SRC)/ProtocolManager.cpp $(WTP_INCLUDE)/ProtocolManager.h
		$(CXX) $(WTP_CXXFLAGS) -o ProtocolManager.o -c $(WTP_SRC)/ProtocolManager.cpp

USBPortLinux.o:	$(WTP_SRC)/USBPortLinux.cpp	$(WTP_INCLUDE)/USBPortLinux.h
		$(CXX) $(WTP_CXXFLAGS) -o USBPortLinux.o -c $(WTP_SRC)/USBPortLinux.cpp

UARTPortLinux.o: $(WTP_SRC)/UARTPortLinux.cpp $(WTP_INCLUDE)/UARTPortLinux.h
		$(CXX) $(WTP_CXXFLAGS) -o UARTPortLinux.o -c $(WTP_SRC)/UARTPortLinux.cpp

TimParser.o:	$(WTP_SRC)/TimParser.cpp	$(WTP_INCLUDE)/TimParser.h
		$(CXX) $(WTP_CXXFLAGS) -o TimParser.o -c $(WTP_SRC)/TimParser.cpp

#Security.o:	$(WTP_SRC)/Security.cpp		$(WTP_INCLUDE)/Security.h
#		$(CXX) $(WTP_CXXFLAGS) -o Security.o -c $(WTP_SRC)/Security.cpp

#RSAKeys.o:	$(WTP_SRC)/RSAKeys.cpp		$(WTP_INCLUDE)/RSAKeys.h
#		$(CXX) $(WTP_CXXFLAGS) -o RSAKeys.o -c $(WTP_SRC)/RSAKeys.cpp

TimLib.o:	$(COMMON_SRC)/TimLib.cpp	$(COMMON_SRC)/TimLib.h
		$(CXX) $(CXXFLAGS) -o TimLib.o -c $(COMMON_SRC)/TimLib.cpp

CommandlineParser.o: $(WTP_SRC)/CommandlineParser.cpp $(WTP_INCLUDE)/CommandlineParser.h
		$(CXX) $(WTP_CXXFLAGS) -o CommandlineParser.o -c $(WTP_SRC)/CommandlineParser.cpp
