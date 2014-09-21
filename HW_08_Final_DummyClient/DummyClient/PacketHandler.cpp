#include "stdafx.h"
#include "Log.h"
#include "ClientSession.h"
#include "Player.h"

#define PKT_NONE	0
#define PKT_MAX		1024

typedef void(*HandlerFunc)(ClientSession* session);

static HandlerFunc HandlerTable[PKT_MAX];

static void DefaultHandler(ClientSession* session)
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
	static void Handler_##PKT_TYPE(ClientSession* session); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(ClientSession* session)

//@}


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
		if (mRecvBuffer.GetStoredSize() < header.size)
			return;


		if (header.type >= PKT_MAX || header.type <= PKT_NONE)
		{
			// LoggerUtil::EventLog("packet type error", mPlayer->GetPlayerId());

			// �������� ���� ��Ŷ�� �̻��ϴ�?!

			DisconnectRequest(DR_ACTIVE);
			return;
		}

		/// packet dispatch...
		HandlerTable[header.type](this);
	}
}

/////////////////////////////////////////////////////////
// REGISTER_HANDLER(PKT_CS_LOGIN)
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

