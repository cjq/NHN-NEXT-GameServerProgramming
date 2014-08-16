#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "SyncExecutable.h"
#include "Timer.h"



Timer::Timer()
{
	LTickCount = GetTickCount64();
}

//ȣ���ڵ�: LTimer->PushTimerJob( instance, std::bind( memfunc, std::forward<Args>( args )... ), after );

void Timer::PushTimerJob(SyncExecutablePtr owner, const TimerTask& task, uint32_t after)
{
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);

	//TODO: mTimerJobQueue�� TimerJobElement�� push..

	TimerJobElement temp(owner, task, after);
	mTimerJobQueue.push( temp );
}


void Timer::DoTimerJob()
{
	/// thread tick update
	LTickCount = GetTickCount64();

	while (!mTimerJobQueue.empty())
	{
		//���� �켱 �Ǵ� ���� ���������� ���� ���� �ƴϴ�
		const TimerJobElement& timerJobElem = mTimerJobQueue.top(); 

		if (LTickCount < timerJobElem.mExecutionTick)
			break;

		timerJobElem.mOwner->EnterLock();
		
		timerJobElem.mTask();

		timerJobElem.mOwner->LeaveLock();

		//ó�� �����ϱ� ���� �켱 �Ǵ� ���� ����
		mTimerJobQueue.pop();
	}


}

