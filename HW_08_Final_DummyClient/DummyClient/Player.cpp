#include "stdafx.h"
#include "Player.h"

Player::Player( ClientSession* session ): mSession( session )
{
	PlayerReset();
}

Player::~Player()
{
}

void Player::PlayerReset()
{
	memset( mPlayerName, 0, sizeof( mPlayerName ) );
	mPlayerId = -1;
	mIsValid = false;
	mPosX = mPosY = mPosZ = 0;
	mHP = 0;
}

void Player::RequestLogin( int pid )
{

}

void Player::ResponseLogin( int pid, float x, float y, float z, wchar_t* name, bool valid )
{

}

void Player::RequestUpdatePosition( float x, float y, float z )
{

}

void Player::ResponseUpdatePosition( float x, float y, float z )
{
	//////////////////////////////////////////////////////////////////////////
	// �̵��� �������� ä���� ��������
}

void Player::RequestChat( const char* comment )
{

}

void Player::ResponseChat( const char* name, const char* comment )
{
	//////////////////////////////////////////////////////////////////////////
	// ä���� �޾����� ī��Ʈ �ؼ� 100ȸ �ʰ� �Ǹ� ���� ����
	// �ƴϸ� �̵�
}
