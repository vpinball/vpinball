// license:GPLv3+

///////////////////////////////////////////////////////////////////////////////
// Remote Control plugin
//
// This plugin allows to use one instance of VPX running on a computer as a 
// controller for another instance of VPX running on another computer, on the
// same local area network. The use case is to allow to play in VR on a cabinet,
// while the cabinet computer is not powerful enough to feed the VR headset.

#include "MsgPlugin.h"
#include "VPXPlugin.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <semaphore>

// Shared logging
#include "LoggingPlugin.h"
LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE


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
         #ifdef UNICODE
            WCHAR wsz[64];
            swprintf_s(wsz, L"%S", host);
            InetPton(AF_INET, wsz, &(saddr_in.sin_addr.s_addr));
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
   struct sockaddr_in _si_other { 0 };
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
   bool hasTimedOut()
   {
      #ifdef _WIN32
         return WSAGetLastError() == WSAETIMEDOUT;
      #else
         return false; // TODO implement
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

MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;

uint32_t endpointId;
unsigned int getVpxApiId, onGameStartId, onGameEndId, onUpdatePhysicsId, onPrepareFrameId, onActionEventId;

std::thread udpThread;
std::binary_semaphore msgReadySem { 0 };

struct StateMsg
{
   uint16_t version = 0;
   uint32_t timestamp = 0;
   uint64_t keyState = 0;
   float nudgeX = 0.f;
   float nudgeY = 0.f;
   float plunger = 0.f;
};

enum RunMode
{
   RunModeNone,
   RunModeController,
   RunModePlayer
};
RunMode runMode = RunMode::RunModeNone;

StateMsg lastState; // Last state acquired from VPX (controller mode) or received from network (player mode)
uint64_t actionState = 0;
uint32_t lastPlayerAppliedStateTimestamp = 0;
enum class ConnectionState
{
   Unconnected, ConnectionMade, Connected, ConnectionLost
};
ConnectionState connectionState = ConnectionState::Unconnected;


void onPrepareFrame(const unsigned int eventId, void* userData, void* eventData)
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
         vpxApi->PushNotification("Remote controller disconnected", 1000);
      break;
   }
}

void onControllerActionEvent(const unsigned int eventId, void* userData, void* eventData)
{
   if (connectionState == ConnectionState::Connected)
   {
      VPXActionEvent* event = static_cast<VPXActionEvent*>(eventData);
      uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(event->action);
      if (event->isPressed)
      {
         actionState |= mask;
         event->isPressed = false;
      }
      else
      {
         actionState &= ~mask;
      }
   }
}

void onControllerUpdatePhysics(const unsigned int eventId, void* userData, void* eventData)
{
   // Gather input state and broadcast it to the server
   uint64_t newKeyState;
   float newNudgeX, newNudgeY, newPlunger;
   vpxApi->GetInputState(&newKeyState, &newNudgeX, &newNudgeY, &newPlunger);
   if ((lastState.keyState != actionState) || (lastState.nudgeX != newNudgeX) || (lastState.nudgeY != newNudgeY) || (lastState.plunger != newPlunger))
   {
      // LOGI(">>> New InputState");
      lastState.keyState = actionState;
      lastState.nudgeX = newNudgeX;
      lastState.nudgeY = newNudgeY;
      lastState.plunger = newPlunger;
      msgReadySem.release();
   }
}

void onPlayerUpdatePhysics(const unsigned int eventId, void* userData, void* eventData)
{
   // Process any pending message from the controller
   if (lastState.timestamp != lastPlayerAppliedStateTimestamp)
   {
      // LOGI(">>> New InputState %08x", lastState.timestamp);
      lastPlayerAppliedStateTimestamp = lastState.timestamp;
      actionState = lastState.keyState & ~(1 << VPXACTION_Debugger);
      vpxApi->SetInputState(actionState, lastState.nudgeX, lastState.nudgeY, lastState.plunger);
   }
}

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   lastState.timestamp = 0;
   char buf[32];
   msgApi->GetSetting("RemoteControl", "RunMode", buf, sizeof(buf));
   int opt = atoi(buf);
   msgApi->GetSetting("RemoteControl", "Port", buf, sizeof(buf));
   int port = atoi(buf);
   msgApi->GetSetting("RemoteControl", "Host", buf, sizeof(buf));
   if (opt == 1)
   {
      runMode = RunMode::RunModeController;
      LOGI("RemoteControl plugin started as controller (client mode, server ip is %s:%d)", buf, port);
      udpThread = std::thread([buf, port]()
         {
            using namespace std::literals;
            StateMsg stateMsg;
            UdpClientSocket client(buf, port, 500);
            auto start = std::chrono::high_resolution_clock::now();
            while (runMode != RunMode::RunModeNone)
            {
               msgReadySem.acquire();
               stateMsg = lastState;
               stateMsg.timestamp = static_cast<uint32_t>((std::chrono::high_resolution_clock::now() - start) / 1us);
               int sent = 0;
               while (sent < sizeof(stateMsg))
               {
                  int n = client.sendData(reinterpret_cast<char*>(&stateMsg) + sent, sizeof(stateMsg) - sent);
                  if (n < 0)
                  {
                     if (client.hasTimedOut())
                     {
                        LOGE("RemoteControl failed to send input state over network (timed out), retrying");
                     }
                     else
                     {
                        LOGE("RemoteControl failed to send input state over network, stopping");
                        runMode = RunMode::RunModeNone;
                     }
                     break;
                  }
                  sent += n;
               }
               // LOGD("Sent %08x", stateMsg.timestamp);
               int n = client.receiveData(reinterpret_cast<char*>(&stateMsg), 1);
               if (n == 1) // Acked, therefore connected
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
      float newNudgeX, newNudgeY, newPlunger;
      vpxApi->GetInputState(&actionState, &newNudgeX, &newNudgeY, &newPlunger);
   }
   else if (opt == 2)
   {
      runMode = RunMode::RunModePlayer;
      LOGI("RemoteControl plugin started as player (server mode, using port: %d)", port);
      udpThread = std::thread([port]()
         {
            StateMsg stateMsg;
            UdpServerSocket server(port, 500);
            while (runMode != RunMode::RunModeNone)
            {
               int rcv = 0;
               while (rcv < sizeof(stateMsg))
               {
                  int n = server.receiveData(reinterpret_cast<char*>(&stateMsg) + rcv, sizeof(stateMsg) - rcv);
                  if (n < 0)
                  {
                     // Just ignore failed request and continue to wait for messages
                     /* if (server.hasTimedOut())
                     {
                        LOGE("RemoteControl failed to receive controller state (timed out), retrying");
                     }
                     else
                     {
                        LOGE("RemoteControl failed to receive controller state, stopping");
                        runMode = RunMode::RunModeNone;
                     } */
                     break;
                  }
                  rcv += n;
               }
               if (rcv == sizeof(stateMsg))
               {
                  if (stateMsg.version != 0)
                  {
                     LOGE("RemoteControl plugin versions do not match");
                     runMode = RunMode::RunModeNone;
                     break;
                  }
                  //LOGD("Rcv ok %08x", stateMsg.timestamp);
                  lastState = stateMsg;
                  server.sendData(&stateMsg, 1);
               }
            }
            server.closeConnection();
         });
      actionState = 0;
      msgApi->SubscribeMsg(endpointId, onUpdatePhysicsId, onPlayerUpdatePhysics, nullptr);
      msgApi->SubscribeMsg(endpointId, onPrepareFrameId, onPrepareFrame, nullptr);
   }
   else
   {
      runMode = RunMode::RunModeNone;
   }
}

void stopThread()
{
   if (runMode == RunMode::RunModePlayer)
   {
      msgApi->UnsubscribeMsg(onUpdatePhysicsId, onPlayerUpdatePhysics);
      msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame);
   }
   else if (runMode == RunMode::RunModeController)
   {
      msgApi->UnsubscribeMsg(onUpdatePhysicsId, onControllerUpdatePhysics);
      msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame);
      msgApi->UnsubscribeMsg(onActionEventId, onControllerActionEvent);
   }
   runMode = RunMode::RunModeNone;
   if (udpThread.joinable())
   {
      msgReadySem.release();
      udpThread.join();
   }
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   stopThread();
}

LPI_IMPLEMENT // Implement shared login support

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   runMode = RunMode::RunModeNone;
   LPISetup(endpointId, msgApi); // Request and setup shared login API
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   onUpdatePhysicsId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_UPDATE_PHYSICS);
   onPrepareFrameId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   onActionEventId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_ACTION_CHANGED);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   stopThread();
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onUpdatePhysicsId);
   msgApi->ReleaseMsgID(onPrepareFrameId);
   msgApi->ReleaseMsgID(onActionEventId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
