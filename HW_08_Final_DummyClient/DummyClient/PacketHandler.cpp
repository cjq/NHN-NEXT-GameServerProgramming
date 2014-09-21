#include "stdafx.h"
#include "Log.h"
#include "ClientSession.h"
#include "Player.h"
#include "ProtoHeader.h"

#define PKT_NONE	0
#define PKT_MAX		1024

#define PKT_SC_LOGIN	MyPacket::MessageType::PKT_SC_LOGIN
#define PKT_SC_CHAT		MyPacket::MessageType::PKT_SC_CHAT
#define PKT_SC_MOVE		MyPacket::MessageType::PKT_SC_MOVE


typedef void(*HandlerFunc)(ClientSession* session, int size);

static HandlerFunc HandlerTable[PKT_MAX];

static void DefaultHandler(ClientSession* session, int size)
{
	LoggerUtil::EventLog("invalid packet handler", session->mPlayer->GetPlayerId());
	session->DisconnectRequest(DR_ACTIVE);
}

struct InitializeHandlers
{
	InitializeHandlers()
	{
		for (int i = 0; i < PKT_MAX; ++i)
			HandlerTable[i] = DefaultHandler;
	}
} _init_handlers_;

struct RegisterHandler
{
	RegisterHandler(int pktType, HandlerFunc handler)
	{
		HandlerTable[pktType] = handler;
	}
};

#define REGISTER_HANDLER(PKT_TYPE)	\
	static void Handler_##PKT_TYPE(ClientSession* session, int size); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(ClientSession* session, int size)


void ClientSession::OnRead(size_t len)
{
	/// ��Ŷ �Ľ��ϰ� ó��
	while (true)
	{
		/// ��Ŷ ��� ũ�� ��ŭ �о�ͺ���
		MessageHeader header;
		
		if (false == mRecvBuffer.Peek((char*)&header, MessageHeaderSize))
			return;

		/// ��Ŷ �ϼ��� �Ǵ°�? 

		// �� ������ �ڵ�ʹ� �ٸ���!
		// Easy Server������ ����� ��� ������ = ��Ŷ ��ü ������ ������
		// ���⼭�� ����������� ���� ������(���̷ε�)�� ��� ����
		if ( mRecvBuffer.GetStoredSize() < header.size - MessageHeaderSize )
			return;
		

		if (header.type >= PKT_MAX || header.type <= PKT_NONE)
		{
			// �������� ���� ��Ŷ�� �̻��ϴ�?!
			LoggerUtil::EventLog("packet type error", mPlayer->GetPlayerId());
			
			DisconnectRequest(DR_ACTIVE);
			return;
		}

		/// packet dispatch...
		HandlerTable[header.type](this, header.size);
	}
}

REGISTER_HANDLER( PKT_SC_LOGIN )
{

}

REGISTER_HANDLER( PKT_SC_MOVE )
{
	//////////////////////////////////////////////////////////////////////////
	// �����κ��� �̵� �㰡�� ������ ���̴�.
	char* packetTemp = new char[MessageHeaderSize + size];

	if ( false == session->ParsePacket( packetTemp, MessageHeaderSize + size ) )
	{
		LoggerUtil::EventLog( "packet parsing error", PKT_SC_MOVE );
		return;
	}

	// ��ũ��Ʈ


	// ��ø��������
	google::protobuf::io::ArrayInputStream is( packetTemp, MessageHeaderSize + size );
	is.Skip( MessageHeaderSize );

	MyPacket::MoveResult inPacket;
	inPacket.ParseFromZeroCopyStream( &is );

	delete packetTemp;

	MyPacket::Position pos = inPacket.playerpos();
	session->mPlayer->ResponseUpdatePosition( pos.x(), pos.y(), pos.z() );
}

REGISTER_HANDLER( PKT_SC_CHAT )
{
	//////////////////////////////////////////////////////////////////////////
	// ������ ���� ä���� �ߴ�
	char* packetTemp = new char[MessageHeaderSize + size];
		
	if ( false == session->ParsePacket( packetTemp, MessageHeaderSize + size ) )
	{
		LoggerUtil::EventLog( "packet parsing error", PKT_SC_CHAT );
		return;
	}
	
	// ��ũ��Ʈ


	// ��ø��������
	google::protobuf::io::ArrayInputStream is( packetTemp, MessageHeaderSize + size );
	is.Skip( MessageHeaderSize );

	MyPacket::ChatResult inPacket;
	inPacket.ParseFromZeroCopyStream( &is );

	delete packetTemp;

	session->mPlayer->ResponseChat( inPacket.playername().c_str() , inPacket.playermessage().c_str() );
}

/////////////////////////////////////////////////////////
// REGISTER_HANDLER(PKT_SC_LOGIN)
// {
// 	LoginRequest inPacket;
// 	if (false == session->ParsePacket(inPacket))
// 	{
// 		LoggerUtil::EventLog("packet parsing error", inPacket.mType);
// 		return;
// 	}
// 	
// 	/// �׽�Ʈ�� ���� 10ms�Ŀ� �ε��ϵ��� ����
// 	DoSyncAfter(10, session->mPlayer, &Player::RequestLoad, inPacket.mPlayerId);
// 
// }

// REGISTER_HANDLER(PKT_CS_MOVE)
// {
// 	MoveRequest inPacket;
// 	if (false == session->ParsePacket(inPacket))
// 	{
// 		LoggerUtil::EventLog("packet parsing error", inPacket.mType);
// 		return;
// 	}
// 
// 	if (inPacket.mPlayerId != session->mPlayer->GetPlayerId())
// 	{
// 		LoggerUtil::EventLog("PKT_CS_MOVE: invalid player ID", session->mPlayer->GetPlayerId());
// 		return;
// 	}
// 
// 	/// ������ ���� �׽�Ʈ�� ���� DB�� ������Ʈ�ϰ� �뺸�ϵ��� ����.
// 	session->mPlayer->DoSync(&Player::RequestUpdatePosition, inPacket.mPosX, inPacket.mPosY, inPacket.mPosZ);
// }

// REGISTER_HANDLER(PKT_CS_CHAT)
// {
// 	ChatBroadcastRequest inPacket;
// 	if (false == session->ParsePacket(inPacket))
// 	{
// 		LoggerUtil::EventLog("packet parsing error", inPacket.mType);
// 		return;
// 	}
// 
// 	if (inPacket.mPlayerId != session->mPlayer->GetPlayerId())
// 	{
// 		LoggerUtil::EventLog("PKT_CS_CHAT: invalid player ID", session->mPlayer->GetPlayerId());
// 		return;
// 	}
// 
// 	/// chatting�� ��� ���⼭ �ٷ� ���
// 	ChatBroadcastResult outPacket;
// 		
// 	outPacket.mPlayerId = inPacket.mPlayerId;
// 	wcscpy_s(outPacket.mChat, inPacket.mChat);
// 	GBroadcastManager->BroadcastPacket(&outPacket);
// 	
// }

