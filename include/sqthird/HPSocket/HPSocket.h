/*
 * Copyright: JessMA Open Source (ldcsaa@gmail.com)
 *
 * Version	: 3.3.1
 * Author	: Bruce Liang
 * Website	: http://www.jessma.org
 * Project	: https://github.com/ldcsaa
 * Blog		: http://www.cnblogs.com/ldcsaa
 * Wiki		: http://www.oschina.net/p/hp-socket
 * QQ Group	: 75375912
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
Module:  HPSocket DLL

Usage:
		����һ��
		--------------------------------------------------------------------------------------
		0. Ӧ�ó������ SocketInterface.h �� HPSocket.h ͷ�ļ�
		1. ���� Ye_Create_Xxx() �������� HPSocket ����
		2. ʹ����Ϻ���� Ye_Destroy_Xxx() �������� HPSocket ����

		��������
		--------------------------------------------------------------------------------------
		0. Ӧ�ó������ SocketInterface.h �� HPSocket.h ͷ�ļ�
		1. ���� CXxxPtr ����ָ�룬ͨ������ָ��ʹ�� HPSocket ����

Release:
		1. x86/HPSocket.dll		- (32λ/MBCS/Release)
		2. x86/HPSocket_D.dll	- (32λ/MBCS/DeBug)
		3. x86/HPSocket_U.dll	- (32λ/UNICODE/Release)
		4. x86/HPSocket_UD.dll	- (32λ/UNICODE/DeBug)
		5. x64/HPSocket.dll		- (64λ/MBCS/Release)
		6. x64/HPSocket_D.dll	- (64λ/MBCS/DeBug)
		7. x64/HPSocket_U.dll	- (64λ/UNICODE/Release)
		8. x64/HPSocket_UD.dll	- (64λ/UNICODE/DeBug)

******************************************************************************/

#pragma once

/**************************************************/
/********* imports / exports HPSocket.dll *********/

#ifdef HPSOCKET_EXPORTS
	#define HPSOCKET_API EXTERN_C __declspec(dllexport)
#else
	#define HPSOCKET_API EXTERN_C __declspec(dllimport)
#endif

#include "SocketInterface.h"

/**************************************************/
/************** HPSocket.dll �������� **************/

// ���� ITcpServer ����
HPSOCKET_API ITcpServer* Ye_Create_TcpServer(ITcpServerListener* pListener);
// ���� ITcpAgent ����
HPSOCKET_API ITcpAgent* Ye_Create_TcpAgent(ITcpAgentListener* pListener);
// ���� ITcpClient ����
HPSOCKET_API ITcpClient* Ye_Create_TcpClient(ITcpClientListener* pListener);
// ���� ITcpPullServer ����
HPSOCKET_API ITcpPullServer* Ye_Create_TcpPullServer(ITcpServerListener* pListener);
// ���� ITcpPullAgent ����
HPSOCKET_API ITcpPullAgent* Ye_Create_TcpPullAgent(ITcpAgentListener* pListener);
// ���� ITcpPullClient ����
HPSOCKET_API ITcpPullClient* Ye_Create_TcpPullClient(ITcpClientListener* pListener);
// ���� IUdpServer ����
HPSOCKET_API IUdpServer* Ye_Create_UdpServer(IUdpServerListener* pListener);
// ���� IUdpClient ����
HPSOCKET_API IUdpClient* Ye_Create_UdpClient(IUdpClientListener* pListener);
// ���� IUdpCast ����
HPSOCKET_API IUdpCast* Ye_Create_UdpCast(IUdpCastListener* pListener);

// ���� ITcpServer ����
HPSOCKET_API void Ye_Destroy_TcpServer(ITcpServer* pServer);
// ���� ITcpAgent ����
HPSOCKET_API void Ye_Destroy_TcpAgent(ITcpAgent* pAgent);
// ���� ITcpClient ����
HPSOCKET_API void Ye_Destroy_TcpClient(ITcpClient* pClient);
// ���� ITcpPullServer ����
HPSOCKET_API void Ye_Destroy_TcpPullServer(ITcpPullServer* pServer);
// ���� ITcpPullAgent ����
HPSOCKET_API void Ye_Destroy_TcpPullAgent(ITcpPullAgent* pAgent);
// ���� ITcpPullClient ����
HPSOCKET_API void Ye_Destroy_TcpPullClient(ITcpPullClient* pClient);
// ���� IUdpServer ����
HPSOCKET_API void Ye_Destroy_UdpServer(IUdpServer* pServer);
// ���� IUdpClient ����
HPSOCKET_API void Ye_Destroy_UdpClient(IUdpClient* pClient);
// ���� IUdpCast ����
HPSOCKET_API void Ye_Destroy_UdpCast(IUdpCast* pCast);

// ��ȡ���������ı�
HPSOCKET_API LPCTSTR Ye_GetSocketErrorDesc(EnSocketError enCode);
// ����ϵͳ�� ::GetLastError() ������ȡϵͳ�������
HPSOCKET_API DWORD SYS_GetLastError	();
// ����ϵͳ�� ::WSAGetLastError() ������ȡͨ�Ŵ������
HPSOCKET_API int SYS_WSAGetLastError();
// ����ϵͳ�� setsockopt()
HPSOCKET_API int SYS_SetSocketOption(SOCKET sock, int level, int name, LPVOID val, int len);
// ����ϵͳ�� getsockopt()
HPSOCKET_API int SYS_GetSocketOption(SOCKET sock, int level, int name, LPVOID val, int* len);
// ����ϵͳ�� ioctlsocket()
HPSOCKET_API int SYS_IoctlSocket(SOCKET sock, long cmd, u_long* arg);
// ����ϵͳ�� ::WSAIoctl()
HPSOCKET_API int SYS_WSAIoctl(SOCKET sock, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned);

// ITcpServer ���󴴽���
struct TcpServer_Creator
{
	static ITcpServer* Create(ITcpServerListener* pListener)
	{
		return Ye_Create_TcpServer(pListener);
	}

	static void Destroy(ITcpServer* pServer)
	{
		Ye_Destroy_TcpServer(pServer);
	}
};

// ITcpAgent ���󴴽���
struct TcpAgent_Creator
{
	static ITcpAgent* Create(ITcpAgentListener* pListener)
	{
		return Ye_Create_TcpAgent(pListener);
	}

	static void Destroy(ITcpAgent* pAgent)
	{
		Ye_Destroy_TcpAgent(pAgent);
	}
};

// ITcpClient ���󴴽���
struct TcpClient_Creator
{
	static ITcpClient* Create(ITcpClientListener* pListener)
	{
		return Ye_Create_TcpClient(pListener);
	}

	static void Destroy(ITcpClient* pClient)
	{
		Ye_Destroy_TcpClient(pClient);
	}
};

// ITcpPullServer ���󴴽���
struct TcpPullServer_Creator
{
	static ITcpPullServer* Create(ITcpServerListener* pListener)
	{
		return Ye_Create_TcpPullServer(pListener);
	}

	static void Destroy(ITcpPullServer* pServer)
	{
		Ye_Destroy_TcpPullServer(pServer);
	}
};

// ITcpPullAgent ���󴴽���
struct TcpPullAgent_Creator
{
	static ITcpPullAgent* Create(ITcpAgentListener* pListener)
	{
		return Ye_Create_TcpPullAgent(pListener);
	}

	static void Destroy(ITcpPullAgent* pAgent)
	{
		Ye_Destroy_TcpPullAgent(pAgent);
	}
};

// ITcpPullClient ���󴴽���
struct TcpPullClient_Creator
{
	static ITcpPullClient* Create(ITcpClientListener* pListener)
	{
		return Ye_Create_TcpPullClient(pListener);
	}

	static void Destroy(ITcpPullClient* pClient)
	{
		Ye_Destroy_TcpPullClient(pClient);
	}
};

// IUdpServer ���󴴽���
struct UdpServer_Creator
{
	static IUdpServer* Create(IUdpServerListener* pListener)
	{
		return Ye_Create_UdpServer(pListener);
	}

	static void Destroy(IUdpServer* pServer)
	{
		Ye_Destroy_UdpServer(pServer);
	}
};

// IUdpClient ���󴴽���
struct UdpClient_Creator
{
	static IUdpClient* Create(IUdpClientListener* pListener)
	{
		return Ye_Create_UdpClient(pListener);
	}

	static void Destroy(IUdpClient* pClient)
	{
		Ye_Destroy_UdpClient(pClient);
	}
};

// IUdpClient ���󴴽���
struct UdpCast_Creator
{
	static IUdpCast* Create(IUdpCastListener* pListener)
	{
		return Ye_Create_UdpCast(pListener);
	}

	static void Destroy(IUdpCast* pCast)
	{
		Ye_Destroy_UdpCast(pCast);
	}
};

/**************************************************/
/************** HPSocket ��������ָ�� **************/

template<class T, class _Listener, class _Creator> class CHPSocketPtr
{
public:
	CHPSocketPtr(_Listener* pListener = nullptr)
	{
		if(pListener)
			m_pObj = _Creator::Create(pListener);
		else
			m_pObj = nullptr;
	}

	~CHPSocketPtr()
	{
		Reset();
	}

public:
	CHPSocketPtr&  Reset(T* pObj = nullptr)
	{
		if(pObj != m_pObj)
		{
			if(m_pObj)
				_Creator::Destroy(m_pObj);

			m_pObj = pObj;
		}

		return *this;
	}

	CHPSocketPtr& Attach(T* pObj)
	{
		return Reset(pObj);
	}

	T* Detach()
	{
		T* pObj	= m_pObj;
		m_pObj	= nullptr;

		return pObj;
	}

	BOOL IsValid	()	const	{return m_pObj != nullptr	;}
	T* Get			()	const	{return m_pObj				;}
	T* operator ->	()	const	{return m_pObj				;}
	operator T*		()	const	{return m_pObj				;}

	CHPSocketPtr& operator = (T* pObj)	{return Reset(pObj)	;}

private:
	CHPSocketPtr(const CHPSocketPtr&);
	CHPSocketPtr& operator = (const CHPSocketPtr&);

private:
	T* m_pObj;
};

// ITcpServer ��������ָ��
typedef CHPSocketPtr<ITcpServer, ITcpServerListener, TcpServer_Creator>			CTcpServerPtr;
// ITcpAgent ��������ָ��
typedef CHPSocketPtr<ITcpAgent, ITcpAgentListener, TcpAgent_Creator>			CTcpAgentPtr;
// ITcpClient ��������ָ��
typedef CHPSocketPtr<ITcpClient, ITcpClientListener, TcpClient_Creator>			CTcpClientPtr;
// ITcpPullServer ��������ָ��
typedef CHPSocketPtr<ITcpPullServer, ITcpServerListener, TcpPullServer_Creator>	CTcpPullServerPtr;
// ITcpPullAgent ��������ָ��
typedef CHPSocketPtr<ITcpPullAgent, ITcpAgentListener, TcpPullAgent_Creator>	CTcpPullAgentPtr;
// ITcpPullClient ��������ָ��
typedef CHPSocketPtr<ITcpPullClient, ITcpClientListener, TcpPullClient_Creator>	CTcpPullClientPtr;
// IUdpServer ��������ָ��
typedef CHPSocketPtr<IUdpServer, IUdpServerListener, UdpServer_Creator>			CUdpServerPtr;
// IUdpClient ��������ָ��
typedef CHPSocketPtr<IUdpClient, IUdpClientListener, UdpClient_Creator>			CUdpClientPtr;
// IUdpCast ��������ָ��
typedef CHPSocketPtr<IUdpCast, IUdpCastListener, UdpCast_Creator>				CUdpCastPtr;
