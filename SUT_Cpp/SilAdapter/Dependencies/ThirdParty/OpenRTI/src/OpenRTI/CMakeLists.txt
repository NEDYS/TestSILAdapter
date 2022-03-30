include(CheckIncludeFiles)
# include(CheckSymbolExists)
# include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

check_include_files(cstdint OpenRTI_HAVE_CSTDINT)
check_include_files(stdint.h OpenRTI_HAVE_STDINT_H)
check_include_files(inttypes.h OpenRTI_HAVE_INTTYPES_H)
if(NOT WIN32)
  set(CMAKE_REQUIRED_LIBRARIES dl)
  check_cxx_source_compiles("
#define _GNU_SOURCE
#include <dlfcn.h>
int main(int argc, char* argv[])
{
  Dl_info info;
  dladdr((void*)main, &info);
  return 0;
}
" OpenRTI_HAVE_DLADDR)
  set(CMAKE_REQUIRED_LIBRARIES)
  if(OpenRTI_HAVE_DLADDR)
    set(DL_LIBRARIES dl)
  endif()
endif()
check_cxx_source_compiles("
#include <alloca.h>
int main(int argc, char* argv[])
{
  void* data = alloca(17);
  return 0;
}
" OpenRTI_HAVE_ALLOCA)

# sigh, since we do network traffic, we should know that ...
include(TestBigEndian)
test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
  set(OpenRTI_HOST_IS_BIG_ENDIAN 1)
else(BIG_ENDIAN)
  set(OpenRTI_HOST_IS_LITTLE_ENDIAN 1)
endif(BIG_ENDIAN)

if(NOT WIN32)
  set(CMAKE_REQUIRED_LIBRARIES pthread)
check_cxx_source_compiles("
#define _GNU_SOURCE 1
#include <pthread.h>
int main(int argc, char* argv[])
{
  pthread_setname_np(pthread_self(), \"example\");
}
" OpenRTI_HAVE_PTHREAD_SETNAME_NP)
  set(CMAKE_REQUIRED_LIBRARIES)
  if(OpenRTI_HAVE_DLADDR)
    set(DL_LIBRARIES dl)
  endif()
endif()

# vismth: this won't work at all if the build directory is outside of the OpenRTI source tree ...
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenRTIConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/OpenRTIConfig.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(LINUX)
  add_definitions(-D_XOPEN_SOURCE=600)
  add_definitions(-D_DEFAULT_SOURCE=1)
elseif(UNIX)
  add_definitions(-D_XOPEN_SOURCE_EXTENDED=1)
elseif(WIN32)
  # for now, silence some win32 warnings - want to review that later ...
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  # We need XP/SP2 We are currently bound to the GetAddrInfo call introduced
  # in that version.
  add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7)
endif()


# A bunch of sources that are operating system dependent.
if(WIN32)
  set(OpenRTI_OS_SOURCES
    win32/ErrnoWin32.h
    win32/NetworkStatisticsWin32.h
    win32/PerformanceCounterStopWatch.h
    win32/SocketAddressPrivateDataWin32.h
    win32/SocketPrivateDataWin32.h
    win32/DynamicModuleWin32.cpp
    win32/ErrnoWin32.cpp
    win32/NetworkStatisticsWin32.cpp
    win32/PerformanceCounterStopWatch.cpp
    win32/SocketAddressWin32.cpp
    win32/SocketDataWin32.cpp
    win32/SocketEventDispatcherWin32.cpp
    win32/SocketPacketWin32.cpp
    win32/SocketPipeWin32.cpp
    win32/SocketServerPipeWin32.cpp
    win32/SocketServerTCPWin32.cpp
    win32/SocketServerWin32.cpp
    win32/SocketStreamWin32.cpp
    win32/SocketTCPWin32.cpp
    win32/SocketUDPWin32.cpp
    win32/SocketWin32.cpp
    win32/ThreadLocalWin32.cpp
    win32/ThreadWin32.cpp
    OpenRTI.rc
  )
else(WIN32)
  set(OpenRTI_OS_SOURCES
    posix/ErrnoPosix.h
    posix/NetworkStatisticsPosix.h
    posix/PerformanceCounterStopWatchPosix.h
    posix/SocketAddressPrivateDataPosix.h
    posix/SocketPrivateDataPosix.h
    posix/ThreadPrivateDataPosix.h
    posix/DynamicModulePosix.cpp
    posix/NetworkStatisticsPosix.cpp
    posix/PerformanceCounterStopWatchPosix.cpp
    posix/SocketAddressPosix.cpp
    posix/SocketDataPosix.cpp
    posix/SocketEventDispatcherPosix.cpp
    posix/SocketPacketPosix.cpp
    posix/SocketPipePosix.cpp
    posix/SocketPosix.cpp
    posix/SocketServerPipePosix.cpp
    posix/SocketServerPosix.cpp
    posix/SocketServerTCPPosix.cpp
    posix/SocketStreamPosix.cpp
    posix/SocketTCPPosix.cpp
    posix/SocketUDPPosix.cpp
    posix/ThreadLocalPosix.cpp
    posix/ThreadPosix.cpp
  )
endif(WIN32)

# We use expat as xml parser, if not on the system, use our own probaby outdated version
include(FindEXPAT)
if(EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIRS})
else(EXPAT_FOUND)
  # Additions for expat
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xml)
  add_definitions(-DXML_STATIC)
  add_definitions(-DHAVE_MEMMOVE)

  set(OpenRTI_EXPAT_SOURCES
    xml/xmlparse.c
    xml/xmlrole.c
    xml/xmltok.c
    xml/xmltok_impl.c
    xml/xmltok_ns.c
  )
endif(EXPAT_FOUND)

# bkd: disabled ZLIB support. In case of reintegration, note that SAB CLA has to ship the according zlib licence  
#include(FindZLIB)
#if(ZLIB_FOUND)
## ems: disabled (unused) ZLIB support due to a bug in the implementation:
## -------------------- 8< --------------------
## /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/ZLibProtocolLayer.cpp: In constructor OpenRTI::ZLibProtocolLayer::ZLibProtocolLayer():
## /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/ZLibProtocolLayer.cpp:190:50: error: invalid new-expression of abstract class type OpenRTI::ZLibProtocolLayer::ProtocolSocket
##    _protocolSocket(new ProtocolSocket(Z_BEST_SPEED))
##                                                   ^
## /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/ZLibProtocolLayer.cpp:27:40: note:   because the following virtual functions are pure within OpenRTI::ZLibProtocolLayer::ProtocolSocket:
##  class OPENRTI_LOCAL ZLibProtocolLayer::ProtocolSocket : public AbstractProtocolSocket {
##                                         ^~~~~~~~~~~~~~
## In file included from /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/AbstractProtocolLayer.h:23:0,
##                  from /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/NestedProtocolLayer.h:23,
##                  from /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/ZLibProtocolLayer.h:23,
##                  from /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/ZLibProtocolLayer.cpp:20:
## /mnt/d/vfs/CANoe/feature/13.0_DistSim_HLA/Projects_Source/OpenRTI/src/OpenRTI/AbstractProtocolSocket.h:40:19: note:     virtual OpenRTI::ssize_t OpenRTI::AbstractProtocolSocket::sendBufferSize() const
##    virtual ssize_t sendBufferSize() const = 0;
##                    ^~~~~~~~~~~~~~
## -------------------- >8 --------------------
#  message(WARNING "ZLIB is found but ignored due to a bug in ZLibProtocolLayer implementation. cf. src/OpenRTI/CMakeLists.txt for details.")
##  include_directories(${ZLIB_INCLUDE_DIRS})
##  add_definitions(-DOPENRTI_HAVE_ZLIB)
##  set(OpenRTI_ZLIB_SOURCES
##    ZLibProtocolLayer.cpp
##  )
#else()
#  set(OpenRTI_ZLIB_SOURCES "")
#endif()

set(OpenRTI_SOURCES
    AbsTimeout.cpp
    AbsTimeout.h
    AbstractConnect.h
    AbstractLogicalTime.h
    AbstractMessage.cpp
    AbstractMessage.h
    AbstractMessageDispatcher.h
    AbstractMessageEncoding.cpp
    AbstractMessageEncoding.h
    AbstractMessageQueue.h
    AbstractMessageReceiver.h
    AbstractMessageSender.cpp
    AbstractMessageSender.h
    AbstractNetworkStatistics.h
    AbstractNotificationHandle.h
    AbstractProtocolLayer.cpp
    AbstractProtocolLayer.h
    AbstractProtocolSocket.cpp
    AbstractProtocolSocket.h
    AbstractServer.cpp
    AbstractServer.h
    AbstractServerNode.cpp
    AbstractServerNode.h
    AbstractSocketEvent.cpp
    AbstractSocketEvent.h
    Ambassador.h
    Atomic.h
    Attributes.cpp
    Attributes.h
    Buffer.h
    Clock.h
    Condition.h
    ContentHandler.cpp
    ContentHandler.h
    DebugNew.h
    DecodeDataStream.h
    DefaultErrorHandler.cpp
    DefaultErrorHandler.h
    DynamicModule.h
    EncodeDataStream.h
    ErrorHandler.cpp
    ErrorHandler.h
    Exception.cpp
    Exception.h
    ExpatXMLReader.cpp
    ExpatXMLReader.h
    Export.h
    FDD1516EFileReader.cpp
    FDD1516EFileReader.h
    FEDFileReader.cpp
    FEDFileReader.h
    FOMStringModuleBuilder.h
    Federate.cpp
    Federate.h
    FederateHandleLowerBoundTimeStampMap.h
    Handle.h
    HandleAllocator.h
    ImmediateCallbackDispatcher.cpp
    ImmediateCallbackDispatcher.h
    InitialClientStreamProtocol.cpp
    InitialClientStreamProtocol.h
    InitialServerStreamProtocol.cpp
    InitialServerStreamProtocol.h
    InitialStreamProtocol.cpp
    InitialStreamProtocol.h
    InternalAmbassador.cpp
    InternalAmbassador.h
    InternalTimeManagement.cpp
    InternalTimeManagement.h
    IntrusiveList.h
    IntrusiveUnorderedMap.h
    LeafServerThread.cpp
    LeafServerThread.h
    LogStream.cpp
    LogStream.h
    Message.cpp
    Message.h
    MessageEncodingRegistry.cpp
    MessageEncodingRegistry.h
    MessageQueue.h
    MomTypes.h
    Mutex.h
    NestedProtocolLayer.cpp
    NestedProtocolLayer.h
    NetworkServer.cpp
    NetworkServer.h
    NetworkStatistics.cpp
    NetworkStatistics.h
    OpenRTIVersion.h
    Options.cpp
    Options.h
    ParenthesesReader.cpp
    ParenthesesReader.h
    PooledMessageList.h
    ProtocolSocketEvent.cpp
    ProtocolSocketEvent.h
    Rand.h
    RangeBounds.h
    Referenced.h
    Region.h
    RegionSet.cpp
    RegionSet.h
    ScopeLock.h
    ScopeUnlock.h
    ServerConfigContentHandler.cpp
    ServerConfigContentHandler.h
    ServerModel.cpp
    ServerModel.h
    ServerNode.cpp
    ServerNode.h
    ServerOptions.h
    SharedPtr.h
    SingletonPtr.h
    Socket.h
    SocketAddress.h
    SocketData.h
    SocketEventDispatcher.cpp
    SocketEventDispatcher.h
    SocketPacket.h
    SocketPipe.h
    SocketServer.h
    SocketServerAcceptEvent.cpp
    SocketServerAcceptEvent.h
    SocketServerPipe.h
    SocketServerTCP.h
    SocketStream.h
    SocketTCP.h
    SocketUDP.h
    StreamBufferProtocol.cpp
    StreamBufferProtocol.h
    StringUtils.cpp
    StringUtils.h
    TemplateTimeManagement.h
    TestLib.h
    Thread.h
    ThreadLocal.h
    ThreadServer.cpp
    ThreadServer.h
    TightBE1MessageEncoding.cpp
    TightBE1MessageEncoding.h
    TimeManagement.h
    TranslateTypes.cpp
    TranslateTypes.h
    TranslateTypes.h
    Types.h
    URL.cpp
    URL.h
    ValueImplementation.h
    VariableLengthData.h
    VariableLengthDataTuple.cpp
    VariableLengthDataTuple.h
    WeakPtr.h
    WeakReferenced.cpp
    WeakReferenced.h
    XMLReader.cpp
    XMLReader.h
    #ZLibProtocolLayer.h
    dprintf.cpp
    dprintf.h
    encoding/BasicDataElements.cpp
    encoding/BasicDataElements.h
    encoding/DataElement.cpp
    encoding/DataElement.h
    encoding/Encoding.h
    encoding/EncodingConfig.h
    encoding/EncodingExceptions.h
    encoding/HLAfixedArray.cpp
    encoding/HLAfixedArray.h
    encoding/HLAfixedRecord.cpp
    encoding/HLAfixedRecord.h
    encoding/HLAhandle.cpp
    encoding/HLAhandle.h
    encoding/HLAopaqueData.cpp
    encoding/HLAopaqueData.h
    encoding/HLAvariableArray.cpp
    encoding/HLAvariableArray.h
    encoding/HLAvariantRecord.cpp
    encoding/HLAvariantRecord.h
    mom/AbstractFederateMetrics.h
    mom/MomFederate.cpp
    mom/MomFederate.h
    mom/MomFederateMetrics.cpp
    mom/MomFederateMetrics.h
    mom/MomFederation.cpp
    mom/MomFederation.h
    mom/MomInteractions.cpp
    mom/MomInteractions.h
    mom/MomManager.cpp
    mom/MomManager.h
    mom/MomServer.cpp
    mom/MomServer.h
    mom/MomTypes.h
    stdafx.h
    xml/amigaconfig.h
    xml/ascii.h
    xml/asciitab.h
    xml/expat.h
    xml/expat_external.h
    xml/iasciitab.h
    xml/internal.h
    xml/latin1tab.h
    xml/macconfig.h
    xml/nametab.h
    xml/utf8tab.h
    xml/winconfig.h
    xml/xmlrole.h
    xml/xmltok.h
    xml/xmltok_impl.h
  ${OpenRTI_OS_SOURCES}
  ${OpenRTI_EXPAT_SOURCES}
  #${OpenRTI_ZLIB_SOURCES}
)

if(OPENRTI_BUILD_SHARED)
  add_library(OpenRTI SHARED ${OpenRTI_SOURCES})
  set_property(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL)
  set_property(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL_EXPORTS)
# ems: disable CMake so library versioning
#  set_property(TARGET OpenRTI PROPERTY VERSION 1.0.0)
#  set_property(TARGET OpenRTI PROPERTY SOVERSION 1)
else()
  add_library(OpenRTI STATIC ${OpenRTI_SOURCES})
  openrti_target_position_independent(OpenRTI)
endif()
set_property(TARGET OpenRTI PROPERTY OUTPUT_NAME OpenRTI)
if(EXPAT_FOUND)
  openrti_target_link_private_libraries(OpenRTI ${EXPAT_LIBRARIES})
endif(EXPAT_FOUND)
#if(ZLIB_FOUND)
#  openrti_target_link_private_libraries(OpenRTI ${ZLIB_LIBRARIES})
#endif()
if(WIN32)
  openrti_target_link_private_libraries(OpenRTI ws2_32)
else(WIN32)
  if(SUNOS)
    openrti_target_link_private_libraries(OpenRTI socket)
    openrti_target_link_private_libraries(OpenRTI nsl)
  endif()
  if (NOT ANDROID)
    openrti_target_link_private_libraries(OpenRTI pthread)
    if (NOT MACOS)
      openrti_target_link_private_libraries(OpenRTI rt)
    endif()
  endif()
  openrti_target_link_private_libraries(OpenRTI m)
endif(WIN32)
if(DL_LIBRARIES)
  openrti_target_link_private_libraries(OpenRTI ${DL_LIBRARIES})
endif(DL_LIBRARIES)

install(TARGETS OpenRTI EXPORT OpenRTITargets
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
