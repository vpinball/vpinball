// license:GPLv3+

///////////////////////////////////////////////////////////////////////////////
// Remote Control plugin
//
// This plugin allows to use one instance of VPX running on a computer as a 
// controller for another instance of VPX running on another computer, on the
// same local area network. The use case is to allow to play in VR on a cabinet,
// while the cabinet computer is not powerful enough to feed the VR headset.

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
#include <chrono>
#include <thread>
#include <semaphore>
#include <mutex>
#include <atomic>
#include <cerrno>

// Shared logging
#include "plugins/LoggingPlugin.h"

namespace RemoteControl {

LPI_USE_CPP();
#define LOGD RemoteControl::LPI_LOGD_CPP
#define LOGI RemoteControl::LPI_LOGI_CPP
#define LOGW RemoteControl::LPI_LOGW_CPP
#define LOGE RemoteControl::LPI_LOGE_CPP

///////////////////////////////////////////////////////////////////////////////
// Minimal portable sockets
// Derived from https://github.com/simondlevy/CppSockets (MIT licensed)

// Windows
#ifdef _WIN32
   #pragma comment(lib, "ws2_32.lib")
   #define WIN32_LEAN_AND_MEAN
   #undef TEXT
   #include <winsock2.h>
   #include <ws2tcpip.h>

// Linux
#else
   #define sprintf_s snprintf
   typedef int SOCKET;
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netdb.h>
   #include <unistd.h>
   #include <arpa/inet.h>
   static const int INVALID_SOCKET = -1;
   static const int SOCKET_ERROR = -1;
#endif

class Socket
{
protected:
   SOCKET _sock;
   char _message[200];

   bool initWinsock(void)
   {
      #ifdef _WIN32
         WSADATA wsaData;
         int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
         if (iResult != 0)
         {
            sprintf_s(_message, sizeof(_message), "WSAStartup() failed with error: %d\n", iResult);
            return false;
         }
      #endif
      return true;
   }

   void cleanup(void)
   {
      #ifdef _WIN32
         WSACleanup();
      #endif
   }

   void inetPton(const char* host, struct sockaddr_in& saddr_in)
   {
      #ifdef _WIN32
         #ifdef _UNICODE
            InetPtonA(AF_INET, host, &(saddr_in.sin_addr.s_addr));
         #else
            InetPton(AF_INET, host, &(saddr_in.sin_addr.s_addr));
         #endif
      #else
         inet_pton(AF_INET, host, &(saddr_in.sin_addr));
      #endif
   }

   void setUdpTimeout(uint32_t msec)
   {
      #ifdef _WIN32
         setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&msec, sizeof(msec));
      #else
         struct timeval timeout;
         timeout.tv_sec = msec / 1000;
         timeout.tv_usec = (msec * 1000) % 1000000;
         setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
      #endif
   }

public:
   void closeConnection(void)
   {
      #ifdef _WIN32
         closesocket(_sock);
      #else
         close(_sock);
      #endif
   }

   char* getMessage(void) { return _message; }
};

class UdpSocket : public Socket
{
protected:
   struct sockaddr_in _si_other {};
   socklen_t _slen = sizeof(_si_other);

   void setupTimeout(uint32_t msec)
   {
      if (msec > 0)
         Socket::setUdpTimeout(msec);
   }

public:
   int sendData(const void* buf, size_t len)
   {
      return sendto(_sock, (const char*)buf, (int)len, 0, (struct sockaddr*)&_si_other, (int)_slen);
   }
   int receiveData(void* buf, size_t len)
   {
      return recvfrom(_sock, (char*)buf, (int)len, 0, (struct sockaddr*)&_si_other, &_slen);
   }
   static bool hasTimedOut()
   {
      #ifdef _WIN32
         return WSAGetLastError() == WSAETIMEDOUT;
      #else
      return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
   }
};

class UdpClientSocket final : public UdpSocket
{
public:
   UdpClientSocket(const char* host, const short port, const uint32_t timeoutMsec = 0)
   {
      // Initialize Winsock, returning on failure
      if (!initWinsock())
         return;

      // Create socket
      _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (_sock == SOCKET_ERROR)
      {
         sprintf_s(_message, sizeof(_message), "socket() failed");
         return;
      }

      // Setup address structure
      memset((char*)&_si_other, 0, sizeof(_si_other));
      _si_other.sin_family = AF_INET;
      _si_other.sin_port = htons(port);
      Socket::inetPton(host, _si_other);

      // Check for / set up optional timeout for receiveData
      UdpSocket::setUdpTimeout(timeoutMsec);
   }
};

class UdpServerSocket final : public UdpSocket
{
public:
   UdpServerSocket(const short port, const uint32_t timeoutMsec = 0)
   {
      // Initialize Winsock, returning on failure
      if (!initWinsock())
         return;

      // Create socket
      _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (_sock == INVALID_SOCKET)
      {
         sprintf_s(_message, sizeof(_message), "socket() failed");
         return;
      }

      // Prepare the sockaddr_in structure
      struct sockaddr_in server;
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = INADDR_ANY;
      server.sin_port = htons(port);

      // Bind
      if (bind(_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
      {
         sprintf_s(_message, sizeof(_message), "bind() failed");
         return;
      }

      // Check for / set up optional timeout for receiveData
      UdpSocket::setUdpTimeout(timeoutMsec);
   }
};



///////////////////////////////////////////////////////////////////////////////
// Remote Control implementation

const MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;

uint32_t endpointId;
unsigned int getVpxApiId, onGameStartId, onGameEndId, onUpdatePhysicsId, onPrepareFrameId, onActionEventId;

std::thread udpThread;
std::binary_semaphore msgReadySem { 0 };

constexpr uint16_t RemoteControlProtocolVersion = 1;

struct StateMsg
{
   uint16_t version = RemoteControlProtocolVersion;
   uint32_t sequence = 0; // Allows to drop reordered or duplicated messages
   VPXInputState state {};
};

enum RunMode
{
   RunModeNone,
   RunModeController,
   RunModePlayer
};
std::atomic<RunMode> runMode { RunMode::RunModeNone };

std::mutex stateMutex;
StateMsg inputState[2]; // Last state acquired from VPX (controller mode) or received from network (player mode)
int activeInputState = 0; // Only accessed under stateMutex
std::atomic<int> lastReceivedMsgId { 0 };
int lastProcessedMsgId = 0; // Only accessed on the API thread
enum class ConnectionState
{
   Unconnected, ConnectionMade, Connected, ConnectionLost
};
std::atomic<ConnectionState> connectionState { ConnectionState::Unconnected };

const char* runModeLiterals[] = { "Controller", "Player" };
MSGPI_ENUM_VAL_SETTING(runModeProp, "RunMode", "Mode", "Select between Controller and Player mode", true, 1, 2, runModeLiterals, 1);
MSGPI_INT_VAL_SETTING(portProp, "Port", "Port", "", true, 0, 0xFFFF, 0);
MSGPI_STRING_VAL_SETTING(hostProp, "Host", "Host", "", true, "", 1024);


static void onPrepareFrame(const unsigned int eventId, void* userData, void* eventData)
{
   switch (connectionState)
   {
   case ConnectionState::ConnectionMade:
      connectionState = ConnectionState::Connected;
      if (runMode == RunMode::RunModeController)
         vpxApi->PushNotification("Remote player connected", 5000);
      else if (runMode == RunMode::RunModePlayer)
         vpxApi->PushNotification("Remote controller connected", 1000);
      break;
   case ConnectionState::ConnectionLost:
      connectionState = ConnectionState::Unconnected;
      if (runMode == RunMode::RunModeController)
         vpxApi->PushNotification("Remote player disconnected", 5000);
      else if (runMode == RunMode::RunModePlayer)
      {
         // Release remotely driven inputs (all buttons released, plunger & nudge overrides dropped)
         VPXInputState releasedState {};
         releasedState.actionMask = 0xFFFFFFFFFFFFFFFFULL;
         releasedState.stateMask = 0;
         vpxApi->SetInputState(&releasedState);
         vpxApi->PushNotification("Remote controller disconnected", 1000);
      }
      break;
   default: break;
   }
}

static void onControllerActionEvent(const unsigned int eventId, void* userData, void* eventData)
{
   if (connectionState == ConnectionState::Connected)
   {
      VPXActionEvent* event = static_cast<VPXActionEvent*>(eventData);
      event->enableVPXProcessing = false; // Disable local processing
   }
}

static void onControllerUpdatePhysics(const unsigned int eventId, void* userData, void* eventData)
{
   // Gather input state and broadcast it to the server
   VPXInputState& lastState = inputState[activeInputState].state;
   VPXInputState& state = inputState[1 - activeInputState].state;
   state.actionMask = 0xFFFFFFFFFFFFFFFFULL; // Request all actions
   state.stateMask = 7; // Request plunger position, velocity and nudge
   vpxApi->GetInputState(&state);
   if (((lastState.actionState & lastState.actionMask) != (state.actionState & state.actionMask))
      || ((state.stateMask & 1) && (lastState.plungerPosition != state.plungerPosition))
      || ((state.stateMask & 2) && (lastState.plungerVelocity != state.plungerVelocity))
      || ((state.stateMask & 4) && (lastState.nudgeAccelerationX != state.nudgeAccelerationX))
      || ((state.stateMask & 4) && (lastState.nudgeAccelerationY != state.nudgeAccelerationY))
      || ((state.stateMask & 4) && (lastState.nudgeDisplacementX != state.nudgeDisplacementX))
      || ((state.stateMask & 4) && (lastState.nudgeDisplacementY != state.nudgeDisplacementY)))
   {
      // LOGI(">>> New InputState");
      std::lock_guard lock(stateMutex);
      activeInputState = 1 - activeInputState;
      msgReadySem.release();
   }
}

static void onPlayerUpdatePhysics(const unsigned int eventId, void* userData, void* eventData)
{
   // Process any pending message from the controller
   if (lastReceivedMsgId != lastProcessedMsgId)
   {
      std::lock_guard lock(stateMutex);
      // LOGI(std::format(">>> New InputState"));
      lastProcessedMsgId = lastReceivedMsgId;
      vpxApi->SetInputState(&inputState[activeInputState].state);
   }
}

static void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   lastReceivedMsgId = 0;
   lastProcessedMsgId = 0;
   connectionState = ConnectionState::Unconnected;
   if (runModeProp_Val == 1)
   {
      if (hostProp_Get()[0] == '\0' || portProp_Val == 0)
      {
         LOGE("RemoteControl plugin is configured as controller but Host and/or Port are not defined"s);
         return;
      }
      runMode = RunMode::RunModeController;
      LOGI("RemoteControl plugin started as controller (client mode, server ip is "s + hostProp_Get() + ':' + std::to_string(portProp_Val) + ')');
      udpThread = std::thread([]()
         {
            using namespace std::literals;
            StateMsg stateMsg;
            uint32_t sequence = 0;
            UdpClientSocket client(hostProp_Get(), portProp_Val, 500);
            while (runMode != RunMode::RunModeNone)
            {
               // Wait for a new input state, waking up periodically to send keep alive messages
               msgReadySem.try_acquire_for(500ms);
               {
                  std::lock_guard lock(stateMutex);
                  stateMsg = inputState[activeInputState];
                  stateMsg.sequence = ++sequence;
               }
               if (client.sendData(&stateMsg, sizeof(stateMsg)) != sizeof(stateMsg))
               {
                  LOGE("RemoteControl failed to send input state over network, stopping"s);
                  runMode = RunMode::RunModeNone;
                  break;
               }
               char ack;
               if (client.receiveData(&ack, 1) == 1) // Acked, therefore connected
               {
                  if (connectionState == ConnectionState::Unconnected)
                     connectionState = ConnectionState::ConnectionMade;
               }
               else // Timed out, therefore not connected
               {
                  if (connectionState == ConnectionState::Connected)
                     connectionState = ConnectionState::ConnectionLost;
               }
            }
            client.closeConnection();
         });
      msgApi->SubscribeMsg(endpointId, onActionEventId, onControllerActionEvent, nullptr);
      msgApi->SubscribeMsg(endpointId, onUpdatePhysicsId, onControllerUpdatePhysics, nullptr);
      msgApi->SubscribeMsg(endpointId, onPrepareFrameId, onPrepareFrame, nullptr);
   }
   else if (runModeProp_Val == 2)
   {
      if (portProp_Val == 0)
      {
         LOGE("RemoteControl plugin is configured as player but Port is not defined"s);
         return;
      }
      runMode = RunMode::RunModePlayer;
      LOGI("RemoteControl plugin started as player (server mode, using port: "s + std::to_string(portProp_Val) + ')');
      udpThread = std::thread(
         []()
         {
            StateMsg stateMsg;
            uint32_t lastSequence = 0;
            int consecutiveTimeouts = 0;
            UdpServerSocket server(portProp_Val, 500);
            while (runMode != RunMode::RunModeNone)
            {
               const int n = server.receiveData(&stateMsg, sizeof(stateMsg));
               if (n == sizeof(stateMsg))
               {
                  consecutiveTimeouts = 0;
                  if (stateMsg.version != RemoteControlProtocolVersion)
                  {
                     LOGE("RemoteControl plugin versions do not match"s);
                     runMode = RunMode::RunModeNone;
                     break;
                  }
                  if (static_cast<int32_t>(stateMsg.sequence - lastSequence) <= 0)
                     continue; // Drop reordered or duplicated messages
                  lastSequence = stateMsg.sequence;
                  {
                     std::lock_guard lock(stateMutex);
                     inputState[1 - activeInputState] = stateMsg;
                     activeInputState = 1 - activeInputState;
                     lastReceivedMsgId++;
                  }
                  if (connectionState == ConnectionState::Unconnected)
                     connectionState = ConnectionState::ConnectionMade;
                  int acq = 0;
                  server.sendData(&acq, 1);
               }
               else if (n >= 0)
               {
                  LOGW("RemoteControl received an incomplete input state message"s);
               }
               else if (server.hasTimedOut())
               {
                  if (connectionState == ConnectionState::Connected && ++consecutiveTimeouts > 4)
                     connectionState = ConnectionState::ConnectionLost;
               }
               else
               {
                  LOGE("RemoteControl socket error while waiting for controller state"s);
               }
            }
            server.closeConnection();
         });
      msgApi->SubscribeMsg(endpointId, onUpdatePhysicsId, onPlayerUpdatePhysics, nullptr);
      msgApi->SubscribeMsg(endpointId, onPrepareFrameId, onPrepareFrame, nullptr);
   }
   else
   {
      runMode = RunMode::RunModeNone;
   }
}

static void stopThread()
{
   if (runMode == RunMode::RunModePlayer)
   {
      msgApi->UnsubscribeMsg(onUpdatePhysicsId, onPlayerUpdatePhysics, nullptr);
      msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame, nullptr);
   }
   else if (runMode == RunMode::RunModeController)
   {
      msgApi->UnsubscribeMsg(onUpdatePhysicsId, onControllerUpdatePhysics, nullptr);
      msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame, nullptr);
      msgApi->UnsubscribeMsg(onActionEventId, onControllerActionEvent, nullptr);
   }
   runMode = RunMode::RunModeNone;
   if (udpThread.joinable())
   {
      msgReadySem.release();
      udpThread.join();
   }
}

static void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   stopThread();
}

LPI_IMPLEMENT_CPP // Implement shared log support

}

using namespace RemoteControl;

MSGPI_EXPORT void MSGPIAPI RemoteControlPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   runMode = RunMode::RunModeNone;
   LPISetup(endpointId, msgApi); // Request and setup shared login API
   msgApi->RegisterSetting(endpointId, &runModeProp);
   msgApi->RegisterSetting(endpointId, &portProp);
   msgApi->RegisterSetting(endpointId, &hostProp);
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   onUpdatePhysicsId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_UPDATE_PHYSICS);
   onPrepareFrameId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   onActionEventId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_ACTION_CHANGED);
}

MSGPI_EXPORT void MSGPIAPI RemoteControlPluginUnload()
{
   stopThread();
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart, nullptr);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd, nullptr);
   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onUpdatePhysicsId);
   msgApi->ReleaseMsgID(onPrepareFrameId);
   msgApi->ReleaseMsgID(onActionEventId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
