﻿#include "stdafx.h"
#include "ThreadLocal.h"
#include "EduServer_IOCP.h"
#include "Log.h"
#include "DBContext.h"
#include "DBThread.h"
#include "DBManager.h"
#include "DBHelper.h"

DBManager* GDatabaseManager = nullptr;

DBManager::DBManager() : mDbCompletionPort(NULL)
{
	memset(mDbWorkerThread, 0, sizeof(mDbWorkerThread));
}

DBManager::~DBManager()
{

}


bool DBManager::Initialize()
{
	/// Create I/O Completion Port
	mDbCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mDbCompletionPort == NULL)
		return false;

	if (false == DBHelper::Initialize(SQL_SERVER_CONN_STR, MAX_DB_THREAD))
		return false;

	return true;
}

void DBManager::Finalize()
{
	DBHelper::Finalize();

	for (int i = 0; i < MAX_DB_THREAD; ++i)
	{
		CloseHandle(mDbWorkerThread[i]->GetHandle());
	}

	CloseHandle(mDbCompletionPort);
}

bool DBManager::StartDatabaseThreads()
{
	/// create DB Thread
	for (int i = 0; i < MAX_DB_THREAD; ++i)
	{
		DWORD dwThreadId;
		//create 에서 suspended하게 만들었기때문에 아래에서 ResumeThread를 해줘야 함
		//그런데 왜 그렇게 했을까?
		//DB Thread에서 필요로 하는 인자가 completionPort가 있는데 그게 없기 때문에
		//thread를 만들고 클래스를 채워준 다음에 정상 진행하도록 하기 위해서?
		//맞겠네... run이 dbWorkerThread에 있으니깐
		//http://blog.naver.com/lhk0523/140168956254
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, DbWorkerThread, (LPVOID)i, CREATE_SUSPENDED, (unsigned int*)&dwThreadId);
		if (hThread == NULL)
			return false;

		mDbWorkerThread[i] = new DBThread(hThread, mDbCompletionPort);
	}

	/// start!
	for (int i = 0; i < MAX_DB_THREAD; ++i)
	{
		ResumeThread(mDbWorkerThread[i]->GetHandle());
	}

	return true;
}


unsigned int WINAPI DBManager::DbWorkerThread(LPVOID lpParam)
{
	LThreadType = THREAD_DB_WORKER;
	LWorkerThreadId = reinterpret_cast<int>(lpParam);
	GThreadCallHistory[LWorkerThreadId] = LThreadCallHistory = new ThreadCallHistory(LWorkerThreadId);
	GThreadCallElapsedRecord[LWorkerThreadId] = LThreadCallElapsedRecord = new ThreadCallElapsedRecord(LWorkerThreadId);

	/// 반드시 DB 쓰레드가 먼저 띄우도록..
	CRASH_ASSERT(LWorkerThreadId < MAX_DB_THREAD);

	return GDatabaseManager->mDbWorkerThread[LWorkerThreadId]->Run();
}

void DBManager::PostDatabsaseRequest(DatabaseJobContext* dbContext)
{
	//todo: PQCS를 이용하여 dbContext를 mDbCompletionPort에 보내기
	if ( FALSE == PostQueuedCompletionStatus( mDbCompletionPort, 0, (ULONG_PTR)CK_DB_REQUEST, (LPOVERLAPPED)dbContext ) )
	{
		printf_s( "DBManager::PostDatabaseResult PostQueuedCompletionStatus Error: %d\n", GetLastError() );

		CRASH_ASSERT( false );
	}
}
