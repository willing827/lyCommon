#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakSleep.h"

namespace RakNet
{
	// Copied from Multiplayer.cpp
	// If the first byte is ID_TIMESTAMP, then we want the 5th byte
	// Otherwise we want the 1st byte
	unsigned char RAK_DLL_EXPORT GetPacketIdentifier(RakNet::Packet *p)
	{
		return (unsigned char) p->data[0];
	}


	RakNet::SystemAddress RAK_DLL_EXPORT ConnectBlocking(RakNet::RakPeerInterface *rakPeer, const char* ipAddr,int port,
		const char* szPass)
	{
		if (rakPeer->Connect(ipAddr, port , szPass, strlen(szPass))!=RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		}

		RakNet::Packet *packet;
		while (1)
		{
			RakSleep(1);

			for (packet=rakPeer->Receive(); 
				packet; 
				rakPeer->DeallocatePacket(packet), packet=rakPeer->Receive())
			{
				if (packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
				{
					return packet->systemAddress;
				}
				else
				{
					return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
				}
			}
		}
	}


}