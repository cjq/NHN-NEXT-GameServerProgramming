﻿#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "DBContext.h"
#include "DBThread.h"
#include "IocpManager.h"

DBThread::DBThread(HANDLE hThread, HANDLE hCompletionPort)
: mDbThreadHandle(hThread), mDbCompletionPort(hCompletionPort)
{}

DBThread::~DBThread()
{
	CloseHandle(mDbThreadHandle);
}

DWORD DBThread::Run()
{
	while (true)
	{
		DoDatabaseJob();
	}

	return 1;
}

void DBThread::DoDatabaseJob()
{
	DWORD dwTransferred = 0;
	LPOVERLAPPED overlapped = nullptr;
	ULONG_PTR completionKey = 0;

	int ret = GetQueuedCompletionStatus(mDbCompletionPort, &dwTransferred, (PULONG_PTR)&completionKey, &overlapped, INFINITE);

	if (CK_DB_REQUEST != completionKey)
	{
		CRASH_ASSERT(false);
		return;
	}

	
	DatabaseJobContext* dbContext = reinterpret_cast<DatabaseJobContext*>(overlapped);
	
	//todo: dbContext의 SQL을 실행하고 그 결과를 IO thread풀로 보내기
	//db 실행 정도로 뻑은 내는게 맞는가! ///# db 쿼리 하나 fail했다고 뻑내면 안되지 ㅎㅎ
	//너무 강력크한 db job인데...

	if ( dbContext->SQLExecute() )
	{
		dbContext->mSuccess = true;
	}
	
	GIocpManager->PostDatabaseResult( dbContext );
}

