// DiffieHellmanKeyExchange.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "DHKeyExchanger.h"


int _tmain(int argc, _TCHAR* argv[])
{
	DHKeyExchanger bob;
	bob.CreatePrivateKey();

	getchar();
	return 0;
}

