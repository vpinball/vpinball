// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// wxx_socket.h
//  Declaration of the CSocket class
//
// The CSocket class represents a network socket. It can be used to create
// an event socket.
//
// Event Sockets
// These sockets use StartEvents to monitor network events. A separate thread
// is created for each event socket. After StartEvents is called, CSocket
// monitors the socket and responds automatically to network events.
// Inherit from CSocket and override the following functions to respond
// to network events: OnAccept; OnAddresListChange; OnDisconnect; OnConnect;
// OnOutOfBand; OnQualityOfService; OnReceive; OnRoutingChange; OnSend.

// When using event sockets, users of this class should be aware that functions
// like OnReceive, OnAccept, etc. are called on a different thread from the one
// CSocket is instantiated on. The thread for these functions needs to respond
// quickly to other network events, so it shouldn't be delayed. It also doesn't
// run a message loop, so it can't be used to create windows. For these reasons
// it might be best to use PostMessage in response to these functions in a
// windows environment.
// Refer to GetEventSelect in the Windows API documentation for more
// information on using event sockets.
// Refer to the NetClient and NetServer samples for an example of how to use
// this class to create event sockets for a TCP/UDP client & server.

// To compile programs with CSocket, link with ws2_32.lib. This class uses
// Winsock version 2, and supports Windows 98 and above. Windows 95 systems
// will need to install the "Windows Sockets 2.0 for Windows 95".

// For a TCP server using event sockets, inherit a class from CSocket
// and override OnAccept, OnDisconnect and OnReceive. Create one instance
// of this class and use it as a listening socket. The purpose of the
// listening socket is to detect connections from clients and accept them.
// For the listening socket, we do the following:
// 1) Create the socket.
// 2) Bind an IP address to the socket.
// 3) Listen on the socket for incoming connection requests.
// 4) Use StartEvents to receive notification of network events.
// 5) Override OnAccept to accept requests on a newly created data CSocket object.
// 6) Create a new data socket for each client connection accepted.
// 7) The server socket uses the 'accept' function to accept an incoming connection
//     from this new data socket.

// The purpose of the data socket is to send data to, and receive data from the
// client. There will be one data socket for each client accepted by the server.
// To use it we do the following:
// * To receive data from the client, override OnReceive and use Receive.
// * To send data to use Send.
// * OnDisconnect can be used to detect when the client is disconnected.

// For a TCP client, inherit from CSocket and override OnReceive and OnDisconnect.
// Create an instance of this inherited class, and  perform the following steps:
// 1) Create the socket.
// 2) Connect to the server.
// 3) Use StartEvents to receive notification of network events.
//    We are now ready to send and receive data from the server.
// * Use Send to send data to the server.
// * Override OnReceive and use Receive to receive data from the server
// * OnDisconnect can be used to detect when the client is disconnected from the
//    server.

// Notes regarding IPv6 support
// * IPv6 is supported on Windows Vista and above. Windows XP with SP2 provides
//    "experimental" support, which can be enabled by entering "ipv6 install"
//    at a command prompt.
// * IsIPV6Supported returns false if for Windows XP.
//


#ifndef WIN32XX_SOCKET_H_
#define WIN32XX_SOCKET_H_

// CSocket requires features from the Win32++ framework.
#include "wxx_wincore.h"
#include "wxx_thread.h"
#include "wxx_mutex.h"

#include <ws2tcpip.h>


namespace Win32xx
{
    const int THREAD_TIMEOUT = 1000;

    /////////////////////////////////////////////////////////////
    // CSocket manages a network socket. It can be used to create
    // network connections, and pass data over those connections.
    class CSocket
    {
    public:
        CSocket();
        virtual ~CSocket();

        // Operations
        void Accept(CSocket& rClientSock, struct sockaddr* addr, int* addrlen);
        int  Bind(LPCTSTR addr, UINT port) const;
        int  Bind(const struct sockaddr* name, int namelen) const;
        int  Connect(LPCTSTR addr, UINT port) const;
        int  Connect(const struct sockaddr* name, int namelen) const;
        bool Create(int family, int type, int protocol = IPPROTO_IP);
        void Disconnect();
        CString GetErrorString() const;
        int  ioCtlSocket(long cmd, u_long* argp) const;
        bool IsIPV6Supported() const;
        int  Listen(int backlog = SOMAXCONN) const;
        int  Receive(char* buf, int len, int flags) const;
        int  ReceiveFrom(char* buf, int len, int flags, struct sockaddr* from,
            int* fromlen) const;
        int  Send(const char* buf, int len, int flags) const;
        int  SendTo(const char* send, int len, int flags, LPCTSTR addr, UINT port) const;
        int  SendTo(const char* buf, int len, int flags, const struct sockaddr* to,
            int tolen) const;
        void StartEvents();
        void StopEvents();

        // Accessors and mutators
        int  GetPeerName(struct sockaddr* name, int* namelen) const;
        int  GetSockName(struct sockaddr* name, int* namelen) const;
        SOCKET GetSocket() const { return m_socket; }
        int  GetSockOpt(int level, int optname, char* optval, int* optlen) const;
        int  SetSockOpt(int level, int optname, const char* optval, int optlen) const;

        // Override these functions to monitor events
        virtual void OnAccept()     {}
        virtual void OnAddresListChange() {}
        virtual void OnDisconnect() {}
        virtual void OnConnect()    {}
        virtual void OnOutOfBand()  {}
        virtual void OnQualityOfService() {}
        virtual void OnReceive()    {}
        virtual void OnRoutingChange() {}
        virtual void OnSend()       {}

        // Allow CSocket to be used as a SOCKET
        operator SOCKET() const { return m_socket; }

    private:
        CSocket(const CSocket&) = delete;
        CSocket& operator=(const CSocket&) = delete;

        static UINT WINAPI EventThread(LPVOID pThis);
        inline static LONG m_socketCount = 0;

        CCriticalSection m_cs;
        SOCKET m_socket = INVALID_SOCKET;
        WorkThreadPtr m_threadPtr;          // Smart pointer to the worker thread for the events.
        CEvent m_stopRequest;               // A manual reset event to signal the event thread should stop.
    };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ////////////////////////////////////
    // Definitions of the CSocket class.
    //

    inline CSocket::CSocket() : m_stopRequest(FALSE, TRUE)
    {
        // Ensure instance initialization is thread-safe for this object.
        CThreadLock lock(m_cs);

        LONG newCount = ::InterlockedIncrement(&m_socketCount);
        if (newCount == 1)
        {
            // Initialize the Windows Socket Services version 2.2 for the
            // first socket.
            WSADATA wsaData;
            if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
                throw CNotSupportedException(GetApp()->MsgSocWSAStartup());
        }
    }

    inline CSocket::~CSocket()
    {
        CThreadLock lock(m_cs);
        Disconnect();

        // Terminate the  Windows Socket Services for the last socket.
        LONG newCount = ::InterlockedDecrement(&m_socketCount);
        if (newCount == 0)
            ::WSACleanup();
    }

    // The accept function permits an incoming connection attempt on the socket.
    // Refer to accept in the Windows API documentation for additional information.
    inline void CSocket::Accept(CSocket& rClientSock, struct sockaddr* addr,
        int* addrlen)
    {
        // Ensure the destination socket handle is clean before overwriting it
        if (rClientSock.m_socket != INVALID_SOCKET)
        {
            rClientSock.Disconnect();
        }

        rClientSock.m_socket = ::accept(m_socket, addr, addrlen);
        if (INVALID_SOCKET == rClientSock.GetSocket())
            TRACE("Accept failed\n");
    }

    // The bind function associates a local address with the socket.
    // Refer to bind in the Windows API documentation for additional information.
    inline int CSocket::Bind(LPCTSTR addr, UINT port) const
    {
        ADDRINFOT hints = {};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM; // Note: Lock to STREAM means this function won't work for UDP
        hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

        CString portName;
        portName.Format(_T("%u"), port);

        ADDRINFOT* pAddrInfo = nullptr;
        int result = ::GetAddrInfo(addr, portName, &hints, &pAddrInfo);
        if (result != 0)
        {
            TRACE("GetAddrInfo failed with error: " + ToCString(result) + "\n");
            return result;
        }

        result = ::bind(m_socket, pAddrInfo->ai_addr, static_cast<int>(pAddrInfo->ai_addrlen));
        if (result == SOCKET_ERROR)
        {
            TRACE("Bind failed with Winsock error: " + GetErrorString() + "\n");
        }

        ::FreeAddrInfo(pAddrInfo);
        return result;
    }

    // The bind function associates a local address with the socket.
    // Refer to bind in the Windows API documentation for additional information.
    inline int CSocket::Bind(const struct sockaddr* name, int namelen) const
    {
        int result = ::bind(m_socket, name, namelen);
        if (result == SOCKET_ERROR)
            TRACE("Bind failed\n");
        return result;
    }

    // The Connect function establishes a connection to the socket.
    // Refer to connect in the Windows API documentation for additional information.
    inline int CSocket::Connect(LPCTSTR addr, UINT port) const
    {
        ADDRINFOT hints = {};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICHOST;

        CString portName;
        portName.Format(_T("%u"), port);

        ADDRINFOT* pAddrInfo = nullptr;
        int result = ::GetAddrInfo(addr, portName, &hints, &pAddrInfo);
        if (result != 0)
        {
            TRACE("GetAddrInfo failed with error: " + ToCString(result) + "\n");
            return SOCKET_ERROR;
        }

        result = Connect(pAddrInfo->ai_addr, static_cast<int>(pAddrInfo->ai_addrlen));
        if (result == SOCKET_ERROR)
        {
            TRACE("Connect failed with Winsock error: " + GetErrorString() + "\n");
        }

        ::FreeAddrInfo(pAddrInfo);
        return result;
    }

    // The Connect function establishes a connection to the socket.
    // Refer to connect in the Windows API documentation for additional information.
    inline int CSocket::Connect(const struct sockaddr* name, int namelen) const
    {
        int result = ::connect(m_socket, name, namelen);
        if (result == SOCKET_ERROR)
            TRACE("Connect failed\n");

        return result;
    }

    // Creates the socket
    // Valid parameter values:
    //  family:     AF_INET or AF_INET6
    //  type:       SOCK_DGRAM, SOCK_SEQPACKET, SOCK_STREAM, SOCK_RAW
    //  protocol:   IPPROTO_IP, IPPROTO_TCP, IPPROTO_UDP, IPPROTO_RAW,
    //              IPPROTO_ICMP, IPPROTO_ICMPV6
    //
    // Refer to socket in the Windows API documentation for additional information.
    inline bool CSocket::Create(int family, int type, int protocol /*= IPPROTO_IP*/)
    {
        // Clean up if a socket is already active
        if (m_socket != INVALID_SOCKET)
        {
            Disconnect();
        }

        m_socket = ::socket(family, type, protocol);
        if (m_socket == INVALID_SOCKET)
        {
            TRACE("Failed to create socket\n");
            return false;
        }

        return true;
    }

    // Shuts down the socket.
    inline void CSocket::Disconnect()
    {
        StopEvents(); // Halt the network thread.

        if (m_socket != INVALID_SOCKET)
        {
            ::shutdown(m_socket, SD_BOTH);
            ::closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }

    // Responds to network events.
    // These are the possible network event notifications:
    //  FD_READ     Notification of readiness for reading.
    //  FD_WRITE    Notification of readiness for writing.
    //  FD_OOB      Notification of the arrival of Out Of Band data.
    //  FD_ACCEPT   Notification of incoming connections.
    //  FD_CONNECT  Notification of completed connection or multipoint join operation.
    //  FD_CLOSE    Notification of socket closure.
    //  FD_QOS      Notification of socket Quality Of Service changes
    //  FD_ROUTING_INTERFACE_CHANGE Notification of routing interface changes
    //                              for the specified destination.
    //  FD_ADDRESS_LIST_CHANGE      Notification of local address list changes
    //                              for the address family of the socket.
    inline UINT WINAPI CSocket::EventThread(LPVOID pThis)
    {
        CSocket* pSocket = reinterpret_cast<CSocket*>(pThis);
        if (!pSocket) return 0;

        // Cache the handle locally by value.
        SOCKET localSocket = pSocket->GetSocket();
        if (localSocket == INVALID_SOCKET) return 0;

        WSAEVENT allEvents[2] = {};
        allEvents[0] = ::WSACreateEvent();
        allEvents[1] = pSocket->m_stopRequest.GetHandle();

        long events = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE |
            FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE;

        // Associate the network events
        if (SOCKET_ERROR == ::WSAEventSelect(localSocket, allEvents[0], events))
        {
            TRACE("Error in Event Select\n");
            ::WSACloseEvent(allEvents[0]);
            return 0;
        }

        bool keepRunning = true;
        WSANETWORKEVENTS networkEvents;

        while (keepRunning)
        {
            DWORD result = ::WSAWaitForMultipleEvents(2, allEvents, FALSE, WSA_INFINITE, FALSE);

            switch (result)
            {
            case WSA_WAIT_EVENT_0 + 1: // Stop event signalled
                keepRunning = false;
                break;

            case WSA_WAIT_EVENT_0:     // Network event signalled
                if (SOCKET_ERROR == ::WSAEnumNetworkEvents(localSocket, allEvents[0], &networkEvents))
                {
                    TRACE("WSAEnumNetworkEvents failed\n");
                    keepRunning = false;
                    break;
                }

                // Dispatch callbacks safely
                if (networkEvents.lNetworkEvents & FD_ACCEPT)                  pSocket->OnAccept();
                if (networkEvents.lNetworkEvents & FD_READ)                    pSocket->OnReceive();
                if (networkEvents.lNetworkEvents & FD_WRITE)                   pSocket->OnSend();
                if (networkEvents.lNetworkEvents & FD_OOB)                     pSocket->OnOutOfBand();
                if (networkEvents.lNetworkEvents & FD_QOS)                     pSocket->OnQualityOfService();
                if (networkEvents.lNetworkEvents & FD_CONNECT)                 pSocket->OnConnect();
                if (networkEvents.lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE) pSocket->OnRoutingChange();
                if (networkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE)     pSocket->OnAddresListChange();

                if (networkEvents.lNetworkEvents & FD_CLOSE)
                {
                    // Notify the consumer.
                    pSocket->OnDisconnect();
                    keepRunning = false;
                }
                break;

            default: // WSA_WAIT_FAILED or unexpected index
                TRACE("WSAWaitForMultipleEvents error or failure\n");
                keepRunning = false;
                break;
            }
        }

        // Clean up Winsock events and detach from the socket
        ::WSAEventSelect(localSocket, nullptr, 0);
        ::WSACloseEvent(allEvents[0]);
        return 0;
    }

    // Returns a string containing the most recent network error.
    // Refer to WSAGetLastError in the Windows API documentation for additional information.
    inline CString CSocket::GetErrorString() const
    {
        DWORD errorCode = static_cast<DWORD>(::WSAGetLastError());
        LPTSTR message = nullptr;
        CString errorMessage;

        ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
            nullptr, errorCode, 0, reinterpret_cast<LPTSTR>(&message), 0, nullptr);

        if (message)
        {
            errorMessage = message;
            ::LocalFree(message);
        }

        return errorMessage;
    }

    // Retrieves the name of the peer to which the socket is connected.
    // Refer to getpeername in the Windows API documentation for additional information.
    inline int CSocket::GetPeerName(struct sockaddr* name, int* namelen) const
    {
        int result = ::getpeername(m_socket, name, namelen);
        if (result != 0)
            TRACE("GetPeerName failed\n");

        return result;
    }

    // Retrieves the local name for the socket.
    // Refer to getsockname in the Windows API documentation for additional information.
    inline int CSocket::GetSockName(struct sockaddr* name, int* namelen) const
    {
        int result = ::getsockname(m_socket, name, namelen);
        if (result != 0)
            TRACE("GetSockName Failed\n");

        return result;
    }

    // Retrieves the socket option.
    // Refer to getsockopt in the Windows API documentation for additional information.
    inline int CSocket::GetSockOpt(int level, int optname, char* optval, int* optlen) const
    {
        int result = ::getsockopt(m_socket, level, optname, optval, optlen);
        if (result != 0)
            TRACE("GetSockOpt Failed\n");

        return result;
    }

    // Controls the I/O mode of the socket.
    // Refer to ioctlsocket in the Windows API documentation for additional information.
    inline int CSocket::ioCtlSocket(long cmd, u_long* argp) const
    {
        int result = ::ioctlsocket(m_socket, cmd, argp);
        if (result != 0)
            TRACE("ioCtlSocket Failed\n");

        return result;
    }

    // Returns true if this system supports IP version 6.
    inline bool CSocket::IsIPV6Supported() const
    {
        SOCKET s = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (s != INVALID_SOCKET)
        {
            ::closesocket(s);
            return true;
        }
        return false;
    }

    // Places the socket in a state in which it is listening for an incoming
    // connection.
    // Refer to listen in the Windows API documentation for additional information.
    inline int CSocket::Listen(int backlog /*= SOMAXCONN*/) const
    {
        int result = ::listen(m_socket, backlog);
        if (result != 0)
            TRACE("Listen Failed\n");

        return result;
    }

    // Receives data from the connected or bound socket.
    // Refer to recv in the Windows API documentation for additional information.
    inline int CSocket::Receive(char* buf, int len, int flags) const
    {
        int result = ::recv(m_socket, buf, len, flags);
        if (SOCKET_ERROR == result)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE("Receive failed\n");
        }
        return result;
    }

    // Receives a datagram and stores the source address.
    // Refer to recvfrom in the Windows API documentation for additional information.
    inline int CSocket::ReceiveFrom(char* buf, int len, int flags,
        struct sockaddr* from, int* fromlen) const
    {
        int result = ::recvfrom(m_socket, buf, len, flags, from, fromlen);
        if (SOCKET_ERROR == result)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE("ReceiveFrom failed\n");
        }
        return result;
    }

    // Sends data on the connected socket.
    // Refer to send in the Windows API documentation for additional information.
    inline int CSocket::Send(const char* buf, int len, int flags) const
    {
        int result = ::send(m_socket, buf, len, flags);
        if (SOCKET_ERROR == result)
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE("Send failed\n");
        return result;
    }

    // Sends data to a specific destination.
    // Refer to sendto in the Windows API documentation for additional information.
    inline int CSocket::SendTo(const char* buf, int len, int flags,
        const struct sockaddr* to, int tolen) const
    {
        int result =  ::sendto(m_socket, buf, len, flags, to, tolen);
        if (SOCKET_ERROR == result)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE("SendTo failed\n");
        }

        return result;
    }

    // Sends data to a specific destination.
    // Refer to sendto in the Windows API documentation for additional information.
    inline int CSocket::SendTo(const char* send, int len, int flags,
        LPCTSTR addr, UINT port) const
    {
        ADDRINFOT hints = {};
        hints.ai_family = AF_UNSPEC;     // Automatically support both IPv4 and IPv6
        hints.ai_socktype = SOCK_DGRAM;  // Default for SendTo/UDP typologies
        hints.ai_flags = AI_NUMERICHOST;

        CString portName;
        portName.Format(_T("%u"), port);

        ADDRINFOT* pAddrInfo = nullptr;
        int result = ::GetAddrInfo(addr, portName, &hints, &pAddrInfo);
        if (result != 0)
        {
            TRACE("GetAddrInfo failed in SendTo\n");
            return SOCKET_ERROR;
        }

        result = SendTo(send, len, flags, pAddrInfo->ai_addr, static_cast<int>(pAddrInfo->ai_addrlen));
        ::FreeAddrInfo(pAddrInfo);

        return result;
    }

    // Sets the socket option.
    // Refer to setsockopt in the Windows API documentation for additional information.
    inline int CSocket::SetSockOpt(int level, int optname, const char* optval,
        int optlen) const
    {
        int result = ::setsockopt(m_socket, level, optname, optval, optlen);
        if (result != 0)
            TRACE("SetSockOpt failed\n");

        return result;
    }

    // This function starts the thread that monitors the socket for events.
    inline void CSocket::StartEvents()
    {
        StopEvents();   // Ensure the thread isn't already running.

        m_threadPtr = std::make_unique<CWorkThread>(EventThread, this);
        m_threadPtr->CreateThread();
    }

    // Terminates the event thread gracefully (if possible).
    inline void CSocket::StopEvents()
    {
        if (m_threadPtr)
        {
            m_stopRequest.SetEvent();
            DWORD waitResult = ::WaitForSingleObject(*m_threadPtr, 3 * THREAD_TIMEOUT);
            if (waitResult == WAIT_TIMEOUT)
            {
                TRACE("*** Critical Warning: Event Thread failed to exit within timeout! Waiting to avoid race... ***\n");
                if (m_socket != INVALID_SOCKET)
                {
                    ::WSAEventSelect(m_socket, nullptr, 0);
                }
                ::WaitForSingleObject(*m_threadPtr, INFINITE);
            }

            m_stopRequest.ResetEvent();
            m_threadPtr.reset();
        }
    }
}

#endif // WIN32XX_SOCKET_H_
