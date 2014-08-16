#pragma once

#include "TypeTraits.h"
#include "FastSpinlock.h"
#include "Timer.h"


class SyncExecutable : public std::enable_shared_from_this<SyncExecutable>
{
public:
	SyncExecutable() : mLock(LO_BUSINESS_CLASS)
	{}
	virtual ~SyncExecutable() {}

	//ȣ�� �ڵ�
	//mPlayer->DoSync(&Player::PlayerReset);

	template <class R, class T, class... Args>
	R DoSync(R (T::*memfunc)(Args...), Args... args)
	{
		//���� ���� �򰡸� ������ Ÿ�ӿ��� Ȯ���� ���׸� ��� ������ ��
		//http://frompt.egloos.com/viewer/2742204
		//SyncExecutable�� ��� ���� �༮�� ��� �����ϵ��� ��
		static_assert(true == std::is_convertible<T, SyncExecutable>::value, "T should be derived from SyncExecutable");

		//TODO: mLock���� ��ȣ�� ���¿���, memfunc�� �����ϰ� ����� R�� ����
		FastSpinlockGuard criticalSection( mLock );
		
		//������ �ɸ��� ������...
		//auto temp = std::bind( memfunc, static_cast<T*>( this ), std::forward<Args>( args )... )( );
		//std::bind( memfunc, static_cast<T*>( this ), std::forward<Args>( args )... )( );

		return std::bind( memfunc, static_cast<T*>( this ), std::forward<Args>( args )... )( );
	}
	
	void EnterLock() { mLock.EnterWriteLock(); }
	void LeaveLock() { mLock.LeaveWriteLock(); }
	
	//ȣ���ڵ�
	//mPlayerId = GPlayerManager->RegisterPlayer(GetSharedFromThis<Player>());

 	template <class T>
	std::shared_ptr<T> GetSharedFromThis()
 	{
		static_assert(true == std::is_convertible<T, SyncExecutable>::value, "T should be derived from SyncExecutable");
 		
		//TODO: this �����͸� std::shared_ptr<T>���·� ��ȯ.
		//(HINT: �� Ŭ������ std::enable_shared_from_this���� ��ӹ޾Ҵ�. �׸��� static_pointer_cast ���)

		//return std::shared_ptr<T>((Player*)this); ///< �̷��� �ϸ� �ȵɰ�???
		//�ٷ� this�� ��ȯ�ϰ� �Ǹ� ������ 2�� ���� ���� �߻�
		//weak pointer�� �Ѱ���� �ϴµ� �� ����� enable_shared_from_this�� shared_from_this()�� ����ϴ� ��
		//�ڼ��� ������ ��ũ
		//https://www.evernote.com/shard/s335/sh/ff59ace2-9cea-42ae-8307-e881c1df5edc/f7e65e4901b33fe3a9cf26cd6dcc3244
		//return static_pointer_cast<T>( T::shared_from_this() );
		return std::static_pointer_cast<T>( shared_from_this() );
 	}

private:

	FastSpinlock mLock;
};

//ȣ�⹮ ���� DoSyncAfter(10, mPlayer, &Player::Start, 1000);

template <class T, class F, class... Args>
void DoSyncAfter(uint32_t after, T instance, F memfunc, Args&&... args)
{
	static_assert(true == is_shared_ptr<T>::value, "T should be shared_ptr");
	static_assert(true == std::is_convertible<T, std::shared_ptr<SyncExecutable>>::value, "T should be shared_ptr SyncExecutable");

	//TODO: instance�� memfunc�� bind�� ��� LTimer->PushTimerJob() ����
	//std::bind�� functor�� �����ִ� �Լ�������
	//http://la-stranger.tistory.com/32

	//std::forward()�� ���� �������� ���� ����������, ���� �������� ���� ���������� ĳ��Ų ���ִ� ����
	//http://frompt.egloos.com/viewer/2765424
	
	//�Լ� ȣ�⼺ ��ü�� �� ������ �ִ�
	//�츮�� ���� ����ϴ� ���� �Լ� ������ / ��� �Լ� ������
	//���⼭ ����ϴ� ���� ��� �Լ� �����ͱ� ������ 2��° ���ڷ� ��ü�� ����
	//http://en.cppreference.com/w/cpp/utility/functional/bind

	LTimer->PushTimerJob( instance, std::bind( memfunc, instance, std::forward<Args>( args )... ), after );
}