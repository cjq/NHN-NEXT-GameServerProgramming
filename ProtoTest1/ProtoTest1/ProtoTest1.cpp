// ProtoTest1.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "mypacket.pb.h"
#include "google\protobuf\io\coded_stream.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include "KeyChanger.h"

#define MAX_BUFFER_SIZE 4096

KeyChanger GKeyChanger;
KeyPrivateSets GBobPrivateKeySets;
KeyPrivateSets GAlicePrivateKeySets;

struct MessageHeader
{
	google::protobuf::uint32 mSize;
	MyPacket::MessageType mType;
};

const int MessageHeaderSize = sizeof( MessageHeader );

void WriteMessageToStream(
	MyPacket::MessageType msgType,
	const google::protobuf::MessageLite& message,
	google::protobuf::io::CodedOutputStream& stream )
{
	MessageHeader messageHeader;
	messageHeader.mSize = message.ByteSize();
	messageHeader.mType = msgType;
	
	stream.WriteRaw( &messageHeader, sizeof( MessageHeader ) );
	message.SerializeToCodedStream( &stream );
}

int _tmain(int argc, _TCHAR* argv[])
{
	//////////////////////////////////////////////////////////////////////////
	// �� = ����1
	// ������ = ����2
	KeySendingSets bobSendingKeySets;
	KeySendingSets aliceSendingKeySets;

	GKeyChanger.GenerateKey( &GBobPrivateKeySets, &bobSendingKeySets );
	GKeyChanger.GenerateKey( &GAlicePrivateKeySets, &aliceSendingKeySets );
	
	//////////////////////////////////////////////////////////////////////////
	// ���� �׽�Ʈ
	
	bool flag = false;

	// �� Ű�� ��ȿ�� ������(255�� ���� ������) ��� �̾��ش�.
	// Why -> �� ���� ������ ����� �� ����. �׷��� �ؿ��� �ļ��� +1 ����
	// ������ unsigned char 255�� �༮(signed char -1)�� �����÷ο� �Ǹ鼭 0�� �ǹǷ�... risk!
	while ( !flag )
	{
		for ( DWORD i = 0; i < bobSendingKeySets.dwDataLen; ++i )
		{
			if ( bobSendingKeySets.pbKeyBlob[i] == (UCHAR)255 )
			{
				printf_s( "Ű �����! \n" );
				GKeyChanger.GenerateKey( &GBobPrivateKeySets, &bobSendingKeySets );
				break;
			}

			flag = true;
		}
	}

	MyPacket::CryptRequest cryptRequest;
	cryptRequest.mutable_sendkey()->set_datalen( bobSendingKeySets.dwDataLen );
	
	char* key = new char[bobSendingKeySets.dwDataLen];
	memcpy( key, bobSendingKeySets.pbKeyBlob, bobSendingKeySets.dwDataLen );

	// �ι��� ������ ����� �� ���Ƿ� +1�� �����ش�. ���� �� -1 ������
	printf_s( "Send \n" );
	for ( size_t i = 0; i < bobSendingKeySets.dwDataLen; ++i )
		printf_s( "%d ", (UCHAR)key[i]++ );
	printf_s( " \n" );
	
	cryptRequest.mutable_sendkey()->set_keyblob( key );
	delete key;

	google::protobuf::uint8 buferA[MAX_BUFFER_SIZE];
	google::protobuf::io::ArrayOutputStream* aos = new google::protobuf::io::ArrayOutputStream( buferA, MAX_BUFFER_SIZE );
	google::protobuf::io::CodedOutputStream* cos = new google::protobuf::io::CodedOutputStream( aos );

	WriteMessageToStream( MyPacket::MessageType::PKT_SC_CRYPT, cryptRequest, *cos );
	
	google::protobuf::io::ArrayInputStream ais( buferA, sizeof( buferA ) );
	google::protobuf::io::CodedInputStream cis( &ais );

	MessageHeader header;
	cis.ReadRaw( &header, MessageHeaderSize );

	if ( header.mType != MyPacket::MessageType::PKT_SC_CRYPT )
	{
		printf_s( "Packet Type Error! \n" );
	}

	const void* payloadPos = nullptr;
	int payloadSize = 0;

	cis.GetDirectBufferPointer( &payloadPos, &payloadSize );

	// payload �б�
	google::protobuf::io::ArrayInputStream payloadArrayStream( payloadPos, header.mSize );
	google::protobuf::io::CodedInputStream payloadInputStream( &payloadArrayStream );

	MyPacket::CryptRequest message;
	if ( false == message.ParseFromCodedStream( &payloadInputStream ) )
	{
		printf_s( "Packet Parse Error! \n" );
	}

	KeySendingSets copyKeySets;
	copyKeySets.pbKeyBlob = new BYTE[message.sendkey().datalen()];

	// Ű ���� ��
	copyKeySets.dwDataLen = message.sendkey().datalen();
	memcpy( copyKeySets.pbKeyBlob, message.sendkey().keyblob().data(),
			copyKeySets.dwDataLen );

	// �ι��� ������ +1 ������ �� -1
	printf_s( "Recv \n" );
	for ( size_t i = 0; i < copyKeySets.dwDataLen; ++i )
		printf_s( "%d ", --copyKeySets.pbKeyBlob[i] );
	printf_s( " \n" );

	if ( !strcmp( (char*)bobSendingKeySets.pbKeyBlob, (char*)copyKeySets.pbKeyBlob ) )
		printf_s( "Save \n" );
	else
		printf_s( "Out! \n" );

	GKeyChanger.GetSessionKey( &GBobPrivateKeySets, &aliceSendingKeySets );
	GKeyChanger.GetSessionKey( &GAlicePrivateKeySets, &copyKeySets );
		
	getchar();

	//////////////////////////////////////////////////////////////////////////

	google::protobuf::uint8 buferB[MAX_BUFFER_SIZE];
	google::protobuf::io::ArrayOutputStream* aos2 = new google::protobuf::io::ArrayOutputStream( buferB, MAX_BUFFER_SIZE );
	google::protobuf::io::CodedOutputStream* cos2 = new google::protobuf::io::CodedOutputStream( aos2 );

	MyPacket::LoginResult loginResult;

	loginResult.set_playerid( 1234 );


	char* name = "ProfessorNTR";
	loginResult.set_playername( name );

	MyPacket::Position position;
	position.set_x( 1.0f );
	position.set_y( 2.0f );
	position.set_z( 3.0f );


	loginResult.mutable_playerpos()->set_x( 1.0f );
	loginResult.mutable_playerpos()->set_y( 2.0f );
	loginResult.mutable_playerpos()->set_z( 3.0f );

	WriteMessageToStream( MyPacket::MessageType::PKT_SC_LOGIN, loginResult, *cos2 );
	
	GKeyChanger.EncryptData( GBobPrivateKeySets.hSessionKey, buferB, sizeof( buferB ), buferB );
	GKeyChanger.DecryptData( GAlicePrivateKeySets.hSessionKey, buferB, sizeof( buferB ) );

	
	//////////////////////////////////////////////////////////////////////////

	GKeyChanger.EncryptData( GAlicePrivateKeySets.hSessionKey, buferB, sizeof( buferB ), buferB );
	GKeyChanger.DecryptData( GBobPrivateKeySets.hSessionKey, buferB, sizeof( buferB ) );

	google::protobuf::io::ArrayInputStream ais2( buferB, sizeof( buferB ) );
	google::protobuf::io::CodedInputStream cis2( &ais2 );

	MessageHeader messageHeader;
	cis2.ReadRaw( &messageHeader, MessageHeaderSize );

	switch ( messageHeader.mType )
	{
		case MyPacket::MessageType::PKT_CS_LOGIN:
			printf_s( "PKT_CS_LOGIN \n" );
			break;
		case MyPacket::MessageType::PKT_CS_CHAT:
			printf_s( "PKT_CS_CHAT \n" );
			break;
		case MyPacket::MessageType::PKT_CS_MOVE:
			printf_s( "PKT_CS_MOVE \n" );
			break;


		// test
		case MyPacket::MessageType::PKT_SC_LOGIN:
		{
			printf_s( "PKT_SC_LOGIN \n" );
			
			const void* payloadPos = nullptr;
			int payloadSize = 0;

			cis2.GetDirectBufferPointer( &payloadPos, &payloadSize );

			// payload �б�
			google::protobuf::io::ArrayInputStream payloadArrayStream( payloadPos, messageHeader.mSize );
			google::protobuf::io::CodedInputStream payloadInputStream( &payloadArrayStream );

			MyPacket::LoginResult message;
			if ( false == message.ParseFromCodedStream( &payloadInputStream ) )
			{
				break;
			}

			int id = message.playerid();
			std::string name = message.playername();

			float x = message.playerpos().x();
			float y = message.playerpos().y();
			float z = message.playerpos().z();

			printf_s( "ID:%d Name:%s [%f][%f][%f] \n", id, name.c_str(), x, y, z );
			break;
		}
			
			
		case MyPacket::MessageType::PKT_SC_CHAT:
			printf_s( "PKT_SC_CHAT \n" );
			break;
		case MyPacket::MessageType::PKT_SC_MOVE:
			printf_s( "PKT_SC_MOVE \n" );
			break;
		default:
			printf_s( "default \n" );
			break;
	}


	getchar();
	return 0;
}

