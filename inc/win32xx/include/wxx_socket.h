// Win32++   Version 9.0
// Release Date: 30th April 2022
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2022  David Nash
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


////////////////////////////////////////////////////////
// wxx_socket.h
//  Declaration of the CSocket class
//
// The CSocket class represents a network socket. It can be used to create
// two types of sockets namely:
// 1) An asycn socket.
// 2) An event socket.
//
// 1. Async Sockets.
// These sockets use StartAsync to monitor network events. Network events are
// passed to the specified window as a window message and processed in the
// window procedure. The network events can be one of:
// FD_READ; FD_WRITE; FD_OOB; FD_ACCEPT; FD_CONNECT; FD_CLOSE; FD_QOS;
// FD_GROUP_QOS; FD_ROUTINGINTERFACE_CHANGE; FD_ADDRESS_LIST_CHANGE;
// FD_ADDRESS_LIST_CHANGE.
// Refer to GetAsyncSelect in the Windows API documentation for more
// information on using async sockets.
// Refer to the NetClientAsync and NetServerAsync samples for an example of
// how to use this class to create async sockets for a TCP/UDP client & server.
//
// 2. Event Sockets
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

// To compile programs with CSocket, link with ws3_32.lib for Win32,
// and ws2.lib for Windows CE. This class uses Winsock version 2, and
// supports Windows 98 and above.

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
// * OnDisconnect can be used to detect when the client is disconnected from the server.

// Notes regarding IPv6 support
// * IPv6 is supported on Windows Vista and above. Windows XP with SP2 provides
//    "experimental" support, which can be enabled by entering "ipv6 install"
//    at a command prompt.
// * IPv6 is not supported by all compilers and development environments. In
//    particular, it is not supported by Dev-C++ or Borland 5.5. A modern
//    Platform SDK needs to be added to Visual Studio 6 for it to support IPv6.
// * IsIPV6Supported returns false if either the operating system or the
//    development environment fails to support IPv6.
//


#ifndef _WIN32XX_SOCKET_H_
#define _WIN32XX_SOCKET_H_

// CSocket requires features from the Win32++ framework.
#include "wxx_wincore.h"
#include "wxx_thread.h"
#include "wxx_mutex.h"

// Work around a bugs in older versions of Visual Studio
#if defined (_MSC_VER) && (_MSC_VER < 1500)  // < VS2008
  // Skip loading wspiapi.h
  #define _WSPIAPI_H_
#endif

#include <WS2tcpip.h>


namespace Win32xx
{
    const int THREAD_TIMEOUT = 100;

    typedef int  WINAPI GETADDRINFO(LPCSTR, LPCSTR, const struct addrinfo*, struct addrinfo**);
    typedef void WINAPI FREEADDRINFO(struct addrinfo*);

    /////////////////////////////////////////////////////////////
    // CSocket manages a network socket. It can be used to create
    // network connections, and pass data over those connections.
    class CSocket
    {
    public:
        CSocket();
        virtual ~CSocket();

        // Operations
        virtual void Accept(CSocket& rClientSock, struct sockaddr* addr, int* addrlen) const;
        virtual int  Bind(LPCTSTR addr, UINT port) const;
        virtual int  Bind(const struct sockaddr* name, int namelen) const;
        virtual int  Connect(LPCTSTR addr, UINT port) const;
        virtual int  Connect(const struct sockaddr* name, int namelen) const;
        virtual bool Create( int family, int type, int protocol = IPPROTO_IP);
        virtual void Disconnect();

#ifdef GetAddrInfo
        virtual void FreeAddrInfo( struct addrinfo* ai ) const;
        virtual int  GetAddrInfo( LPCTSTR nodename, LPCTSTR servname, const struct addrinfo* hints, struct addrinfo** res) const;
#endif

        virtual CString GetErrorString() const;
        virtual int  ioCtlSocket(long cmd, u_long* argp) const;
        virtual bool IsIPV6Supported() const;
        virtual int  Listen(int backlog = SOMAXCONN) const;
        virtual int  Receive(char* buf, int len, int flags) const;
        virtual int  ReceiveFrom(char* buf, int len, int flags, struct sockaddr* from, int* fromlen) const;
        virtual int  Send(const char* buf, int len, int flags) const;
        virtual int  SendTo(const char* send, int len, int flags, LPCTSTR addr, UINT port) const;
        virtual int  SendTo(const char* buf, int len, int flags, const struct sockaddr* to, int tolen) const;

        virtual int  StartAsync(HWND wnd, UINT message, long events);
        virtual void StartEvents();
        virtual void StopEvents();

        // Accessors and mutators
        virtual int  GetPeerName(struct sockaddr* name, int* namelen) const;
        virtual int  GetSockName(struct sockaddr* name, int* namelen) const;
        SOCKET& GetSocket() { return m_socket; }
        virtual int  GetSockOpt(int level, int optname, char* optval, int* optlen) const;
        virtual int  SetSockOpt(int level, int optname, const char* optval, int optlen) const;

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
        operator SOCKET() const {return m_socket;}

    private:
        CSocket(const CSocket&);                // Disable copy construction
        CSocket& operator = (const CSocket&);   // Disable assignment operator
        static UINT WINAPI EventThread(LPVOID pThis);

        SOCKET m_socket;
        HMODULE m_ws2_32;
        WorkThreadPtr m_threadPtr;          // Smart pointer to the worker thread for the events
        CEvent m_stopRequest;               // A manual reset event to signal the event thread should stop

        GETADDRINFO* m_pfnGetAddrInfo;      // pointer for the getaddrinfo function
        FREEADDRINFO* m_pfnFreeAddrInfo;    // pointer for the freeaddrinfo function
    };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    inline CSocket::CSocket() : m_socket(INVALID_SOCKET), m_stopRequest(FALSE, TRUE)
    {
        // Initialize the Windows Socket services
        WSADATA wsaData;

        if (::WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
            throw CNotSupportedException(GetApp()->MsgSocWSAStartup());

        m_ws2_32 = LoadLibrary(_T("WS2_32.dll"));
        if (m_ws2_32 == 0)
            throw CNotSupportedException(GetApp()->MsgSocWS2Dll());

        m_pfnGetAddrInfo = reinterpret_cast<GETADDRINFO*>( GetProcAddress(m_ws2_32, "getaddrinfo") );
        m_pfnFreeAddrInfo = reinterpret_cast<FREEADDRINFO*>( GetProcAddress(m_ws2_32, "freeaddrinfo") );

        WorkThreadPtr threadPtr(new CWorkThread(EventThread, this));
        m_threadPtr = threadPtr;
    }

    inline CSocket::~CSocket()
    {
        ::shutdown(m_socket, SD_BOTH);
        // Ask the event thread to stop
        m_stopRequest.SetEvent();

        // Wait for the event thread to stop.
        while (WAIT_TIMEOUT == ::WaitForSingleObject(*m_threadPtr, THREAD_TIMEOUT * 10))
        {
            // Waiting for the event thread to signal the m_stopped event.

            // Note: An excessive delay in processing any of the notification functions
            // can cause us to get here.
            TRACE("*** Error: Event Thread won't die ***\n");
        }

        m_stopRequest.ResetEvent();

        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;

        if (m_stopRequest.GetHandle())
            CloseHandle(m_stopRequest);

        // Terminate the  Windows Socket services
        ::WSACleanup();

        ::FreeLibrary(m_ws2_32);
    }

    // The accept function permits an incoming connection attempt on the socket.
    // Refer to accept in the Windows API documentation for additional information.
    inline void CSocket::Accept(CSocket& rClientSock, struct sockaddr* addr, int* addrlen) const
    {
        rClientSock.m_socket = ::accept(m_socket, addr, addrlen);
        if (INVALID_SOCKET == rClientSock.GetSocket())
            TRACE("Accept failed\n");
    }

    // The bind function associates a local address with the socket.
    // Refer to bind in the Windows API documentation for additional information.
    inline int CSocket::Bind(LPCTSTR addr, UINT port) const
    {
        int result = 0;

        if (IsIPV6Supported())
        {

#ifdef GetAddrInfo  // Skip the following code block for older development environments

            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
            ADDRINFO *AddrInfo;
            CString portName;
            portName.Format(_T("%u"), port);

            result = GetAddrInfo(addr, portName, &hints, &AddrInfo);
            if (result != 0)
            {
                TRACE("GetAddrInfo failed\n");
                return result;
            }

            // Bind the IP address to the listening socket
            result =  ::bind( m_socket, AddrInfo->ai_addr, static_cast<int>(AddrInfo->ai_addrlen) );
            if (result == SOCKET_ERROR )
            {
                TRACE("Bind failed\n");
                return result;
            }

            // Free the address information allocated by GetAddrInfo
            FreeAddrInfo(AddrInfo);

#endif

        }
        else
        {
            sockaddr_in clientService;
            clientService.sin_family = AF_INET;

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4996 )
#endif // _MSC_VER

                clientService.sin_addr.s_addr = inet_addr(TtoA(addr));

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER

            clientService.sin_port = htons( static_cast<u_short>(port) );

            result = ::bind( m_socket, reinterpret_cast<SOCKADDR*>( &clientService), sizeof(clientService) );
            if ( 0 != result)
                TRACE("Bind failed\n");
        }

        return result;
    }

    // The bind function associates a local address with the socket.
    // Refer to bind in the Windows API documentation for additional information.
    inline int CSocket::Bind(const struct sockaddr* name, int namelen) const
    {
        int result = ::bind (m_socket, name, namelen);
        if ( 0 != result)
            TRACE("Bind failed\n");
        return result;
    }

    // The Connect function establishes a connection to the socket.
    // Refer to connect in the Windows API documentation for additional information.
    inline int CSocket::Connect(LPCTSTR addr, UINT port) const
    {
        int result = 0;

        if (IsIPV6Supported())
        {

#ifdef GetAddrInfo  // Skip the following code block for older development environments

            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
            ADDRINFO *AddrInfo;

            CString portName;
            portName.Format(_T("%u"), port);
            result = GetAddrInfo(addr, portName, &hints, &AddrInfo);
            if (result != 0)
            {
                TRACE("getaddrinfo failed\n");
                return SOCKET_ERROR;
            }

            // Bind the IP address to the listening socket
            result = Connect( AddrInfo->ai_addr, static_cast<int>(AddrInfo->ai_addrlen) );
            if (result == SOCKET_ERROR )
            {
                TRACE("Connect failed\n");
                return result;
            }

            // Free the address information allocated by GetAddrInfo
            FreeAddrInfo(AddrInfo);

#endif

        }
        else
        {
            sockaddr_in clientService;
            clientService.sin_family = AF_INET;

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4996 )
#endif // _MSC_VER

            clientService.sin_addr.s_addr = inet_addr( TtoA(addr) );

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER

            clientService.sin_port = htons( static_cast<u_short>(port) );

            result = ::connect( m_socket, reinterpret_cast<SOCKADDR*>( &clientService ), sizeof(clientService) );
            if ( 0 != result)
                TRACE("Connect failed\n");
        }

        return result;
    }

    // The Connect function establishes a connection to the socket.
    // Refer to connect in the Windows API documentation for additional information.
    inline int CSocket::Connect(const struct sockaddr* name, int namelen) const
    {
        int result = ::connect( m_socket, name, namelen );
        if ( 0 != result)
            TRACE("Connect failed\n");

        return result;
    }

    // Creates the socket
    // Valid parameter values:
    //  family:     AF_INET or AF_INET6
    //  type:       SOCK_DGRAM, SOCK_SEQPACKET, SOCK_STREAM, SOCK_RAW
    //  protocol:   IPPROTO_IP, IPPROTO_TCP, IPPROTO_UDP, IPPROTO_RAW, IPPROTO_ICMP, IPPROTO_ICMPV6
    //
    // Refer to socket in the Windows API documentation for additional information.
    inline bool CSocket::Create( int family, int type, int protocol /*= IPPROTO_IP*/)
    {
        m_socket = socket(family, type, protocol);
        if (m_socket == INVALID_SOCKET)
        {
            TRACE("Failed to create socket\n");
            return FALSE;
        }

        return TRUE;
    }

    // Shuts down the socket.
    inline void CSocket::Disconnect()
    {
        ::shutdown(m_socket, SD_BOTH);
        StopEvents();

        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
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
    //  FD_ROUTING_INTERFACE_CHANGE Notification of routing interface changes for the specified destination.
    //  FD_ADDRESS_LIST_CHANGE      Notification of local address list changes for the address family of the socket.
    inline UINT WINAPI CSocket::EventThread(LPVOID pThis)
    {
        WSANETWORKEVENTS networkEvents;
        CSocket* pSocket = reinterpret_cast<CSocket*>(pThis);
        CEvent& stopRequestEvent = pSocket->m_stopRequest;
        SOCKET& clientSocket = pSocket->m_socket;

        WSAEVENT allEvents[2];
        allEvents[0] = ::WSACreateEvent();
        allEvents[1] = reinterpret_cast<WSAEVENT>(stopRequestEvent.GetHandle());  // cast supports Borland v5.5
        long events = FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE;
        if (GetWinVersion() != 1400) // Win Version != Win95
            events |= FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE;

        // Associate the network events with the client socket.
        if ( SOCKET_ERROR == WSAEventSelect(clientSocket, allEvents[0], events))
        {
            TRACE("Error in Event Select\n");
            ::WSACloseEvent(allEvents[0]);
            return 0;
        }

        // loop until the stop event is set
        for (;;) // infinite loop
        {
            // Wait for a network event, or a request to stop
            DWORD result = ::WSAWaitForMultipleEvents(2, allEvents, FALSE, WSA_INFINITE, FALSE);

            // Check event for stop thread
            if (result - WSA_WAIT_EVENT_0 == 1)
            {
                ::WSACloseEvent(allEvents[0]);
                return 0;
            }

            if (result == WSA_WAIT_FAILED)
            {
                TRACE("WSAWaitForMultipleEvents failed\n");
                ::WSACloseEvent(allEvents[0]);
                return 0;
            }

            // Proceed if a network event occurred
            if (result != WSA_WAIT_TIMEOUT)
            {

                if ( SOCKET_ERROR == ::WSAEnumNetworkEvents(clientSocket, allEvents[0], &networkEvents) )
                {
                    TRACE("WSAEnumNetworkEvents failed\n");
                    ::WSACloseEvent(allEvents[0]);
                    return 0;
                }

                if (networkEvents.lNetworkEvents & FD_ACCEPT)
                    pSocket->OnAccept();

                if (networkEvents.lNetworkEvents & FD_READ)
                    pSocket->OnReceive();

                if (networkEvents.lNetworkEvents & FD_WRITE)
                    pSocket->OnSend();

                if (networkEvents.lNetworkEvents & FD_OOB)
                    pSocket->OnOutOfBand();

                if (networkEvents.lNetworkEvents & FD_QOS)
                    pSocket->OnQualityOfService();

                if (networkEvents.lNetworkEvents & FD_CONNECT)
                    pSocket->OnConnect();

                if (networkEvents.lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE)
                    pSocket->OnRoutingChange();

                if (networkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE)
                    pSocket->OnAddresListChange();

                if (networkEvents.lNetworkEvents & FD_CLOSE)
                {
                    ::shutdown(clientSocket, SD_BOTH);
                    ::closesocket(clientSocket);
                    pSocket->OnDisconnect();
                    ::WSACloseEvent(allEvents[0]);
                    return 0;
                }
            }
        }
    }


#ifdef GetAddrInfo

    // Frees address resources allocated by the GetAddrInfo function.
    inline void CSocket::FreeAddrInfo(struct addrinfo* ai) const
    {
        m_pfnFreeAddrInfo(ai);
    }

    // Provides protocol-independent translation from host name to address.
    // Refer to getaddrinfo in the Windows API documentation for additional information.
    inline int CSocket::GetAddrInfo( LPCTSTR nodename, LPCTSTR servname, const struct addrinfo* hints, struct addrinfo** res) const
    {
        return m_pfnGetAddrInfo(TtoA(nodename), TtoA(servname), hints, res);
    }

#endif

    // Returns a string containing the most recent network error.
    // Refer to WSAGetLastError in the Windows API documentation for additional information.
    inline CString CSocket::GetErrorString() const
    {
        int errorCode = WSAGetLastError();
        LPTSTR message = NULL;
        CString errorMessage;

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      NULL, errorCode, 0, reinterpret_cast<LPTSTR>(&message), 1024, NULL);

        if (message)
        {
            errorMessage = message;
            ::LocalFree(message);
        }

        return errorMessage;
    }

    // Retrieves the name of the peer to which the socket is connected.
    // Refer to getpeername in the Windows API documentation for additional information.
    inline int  CSocket::GetPeerName(struct sockaddr* name, int* namelen) const
    {
        int result = ::getpeername(m_socket, name, namelen);
        if (result != 0)
            TRACE("GetPeerName failed\n");

        return result;
    }

    // Retrieves the local name for the socket.
    // Refer to getsockname in the Windows API documentation for additional information.
    inline int  CSocket::GetSockName(struct sockaddr* name, int* namelen) const
    {
        int result = ::getsockname(m_socket, name, namelen);
        if (result != 0)
            TRACE("GetSockName Failed\n");

        return result;
    }

    // Retrieves the socket option.
    // Refer to getsockopt in the Windows API documentation for additional information.
    inline int  CSocket::GetSockOpt(int level, int optname, char* optval, int* optlen) const
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
        bool isIPV6Supported = FALSE;

#ifdef GetAddrInfo

        if (m_pfnGetAddrInfo != 0 && m_pfnFreeAddrInfo != 0)
            isIPV6Supported = TRUE;

#endif

        return isIPV6Supported;
    }

    // Places the socket in a state in which it is listening for an incoming connection.
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
            TRACE(_T("Receive failed\n"));
        return result;
    }

    // Receives a datagram and stores the source address.
    // Refer to recvfrom in the Windows API documentation for additional information.
    inline int CSocket::ReceiveFrom(char* buf, int len, int flags, struct sockaddr* from, int* fromlen) const
    {
        int result = ::recvfrom(m_socket, buf, len, flags, from, fromlen);
        if (SOCKET_ERROR == result)
            TRACE(_T("ReceiveFrom failed\n"));
        return result;
    }

    // Sends data on the connected socket.
    // Refer to send in the Windows API documentation for additional information.
    inline int CSocket::Send(const char* buf, int len, int flags) const
    {
        int result = ::send(m_socket, buf, len, flags);
        if (SOCKET_ERROR == result)
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE(_T("Send failed\n"));
        return result;
    }

    // Sends data to a specific destination.
    // Refer to sendto in the Windows API documentation for additional information.
    inline int CSocket::SendTo(const char* buf, int len, int flags, const struct sockaddr* to, int tolen) const
    {
        int result =  ::sendto(m_socket, buf, len, flags, to, tolen);
        if (SOCKET_ERROR == result)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                TRACE(_T("SendTo failed\n"));
        }

        return result;
    }

    // Sends data to a specific destination.
    // Refer to sendto in the Windows API documentation for additional information.
    inline int CSocket::SendTo(const char* send, int len, int flags, LPCTSTR addr, UINT port) const
    {
        int result = 0;

        if (IsIPV6Supported())
        {

#ifdef GetAddrInfo  // Skip the following code block for older development environments

            ADDRINFO hints;
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
            ADDRINFO *addrInfo;
            CString portName;
            portName.Format(_T("%u"), port);

            result = GetAddrInfo(addr, portName, &hints, &addrInfo);
            if (result != 0)
            {
                TRACE("GetAddrInfo failed\n");
                return SOCKET_ERROR;
            }

            result = ::sendto(m_socket, send, len, flags, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen) );
            if (result == SOCKET_ERROR )
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    TRACE("SendTo failed\n");
                    return result;
                }
            }

            // Free the address information allocated by GetAddrInfo
            FreeAddrInfo(addrInfo);

#endif

        }
        else
        {
            sockaddr_in clientService;
            clientService.sin_family = AF_INET;

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4996 )
#endif // _MSC_VER

            clientService.sin_addr.s_addr = inet_addr(TtoA(addr));

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER


            clientService.sin_port = htons( static_cast<u_short>(port) );

            result = ::sendto( m_socket, send, len, flags, reinterpret_cast<SOCKADDR*>( &clientService ), sizeof(clientService) );
            if (SOCKET_ERROR != result)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    TRACE("SendTo failed\n");
            }

        }

        return result;
    }

    // Sets the socket option.
    // Refer to setsockopt in the Windows API documentation for additional information.
    inline int CSocket::SetSockOpt(int level, int optname, const char* optval, int optlen) const
    {
        int result = ::setsockopt(m_socket, level, optname, optval, optlen);
        if (result != 0)
            TRACE("SetSockOpt failed\n");

        return result;
    }

    // This function redirects socket events to a window in a message. It can
    // be used as an alternative to StartEvents. Several sockets can redirect
    // events to the same window.
    // Parameters:
    //  wnd      The window to receive the socket events.
    //  message  The message to be received when a network event occurs.
    //  events   A bitmask that has zero or more of the following:
    //  FD_READ     Notification of readiness for reading.
    //  FD_WRITE    Notification of readiness for writing.
    //  FD_OOB      Notification of the arrival of Out Of Band data.
    //  FD_ACCEPT   Notification of incoming connections.
    //  FD_CONNECT  Notification of completed connection or multipoint join operation.
    //  FD_CLOSE    Notification of socket closure.
    //  FD_QOS      Notification of socket Quality Of Service changes.
    //  FD_ROUTING_INTERFACE_CHANGE Notification of routing interface changes for the specified destination.
    //  FD_ADDRESS_LIST_CHANGE      Notification of local address list changes for the address family of the socket.
    // Refer to WSAAsyncSelect in the Windows API documentation for additional information.
    inline int CSocket::StartAsync(HWND wnd, UINT message, long events)
    {
        StopEvents();   // Ensure the event thread isn't running

#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4996 )
#endif // _MSC_VER

        return ::WSAAsyncSelect(*this, wnd, message, events);

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER

    }

    // This function starts the thread which monitors the socket for events.
    inline void CSocket::StartEvents()
    {
        StopEvents();   // Ensure the thread isn't already running

        m_threadPtr->CreateThread();
    }

    // Terminates the event thread gracefully (if possible)
    inline void CSocket::StopEvents()
    {
        // Ask the event thread to stop
        m_stopRequest.SetEvent();

        // Wait for the event thread to stop.
        while (WAIT_TIMEOUT == ::WaitForSingleObject(*m_threadPtr, THREAD_TIMEOUT * 10))
        {
            // Waiting for the event thread to signal the m_stopped event.

            // Note: An excessive delay in processing any of the notification functions
            // can cause us to get here.
            TRACE("*** Error: Event Thread won't die ***\n");
        }

        m_stopRequest.ResetEvent();
    }
}


#endif // #ifndef _WIN32XX_SOCKET_H_

