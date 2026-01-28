

#include "threads.hpp"

namespace Threads
{

	Mutex::Mutex() {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&_mutex, &attr);
		pthread_mutexattr_destroy(&attr);
	}

	Mutex::~Mutex() {
		pthread_mutex_destroy(&_mutex);
	}

	void Mutex::Lock() {
		pthread_mutex_lock(&_mutex);
	}

	void Mutex::UnLock() {
		pthread_mutex_unlock(&_mutex);
	}

	bool Mutex::TryLock() {
		return (pthread_mutex_trylock(&_mutex) == 0);
	}

	AutoMutex::AutoMutex() :_local(true) {
		_mutex = new rc_mutex();
		pthread_mutex_init(_mutex, NULL);
		pthread_mutex_lock(_mutex);
	}

	AutoMutex::AutoMutex(Mutex& mut) :_local(false) {
		_mutex = &mut._mutex;
		pthread_mutex_lock(_mutex);
	}

	AutoMutex::~AutoMutex() {
		pthread_mutex_unlock(_mutex);
		if (_local)
			pthread_mutex_destroy(_mutex);
		if (_local)
			delete(_mutex);
	}

	Barrier::Barrier(int value) :_value(value) {
		_entered_barrier = 0;
		_exited_barrier = 0;
		pthread_mutex_init(&_mutex, NULL);
		pthread_cond_init(&_entered_cond, NULL);
		pthread_cond_init(&_exited_cond, NULL);
	}

	Barrier::~Barrier() {
		pthread_mutex_destroy(&_mutex);
		pthread_cond_destroy(&_entered_cond);
		pthread_cond_destroy(&_exited_cond);
	}

	void Barrier::WaitEnter() {
		pthread_mutex_lock(&_mutex);
		_entered_barrier++;
		if (_entered_barrier < _value)
			pthread_cond_wait(&_entered_cond, &_mutex);
		else {
			_exited_barrier = 0;
			pthread_cond_broadcast(&_entered_cond);
		}
		pthread_mutex_unlock(&_mutex);
	}

	void Barrier::WaitExit() {
		pthread_mutex_lock(&_mutex);
		_exited_barrier++;
		if (_exited_barrier < _value)
			pthread_cond_wait(&_exited_cond, &_mutex);
		else {
			_entered_barrier = 0;
			pthread_cond_broadcast(&_exited_cond);
		}
		pthread_mutex_unlock(&_mutex);
	}

	void Barrier::Wait() {
		pthread_mutex_lock(&_mutex);
		_exited_barrier++;
		if (_exited_barrier < _value)
			pthread_cond_wait(&_exited_cond, &_mutex);
		else {
			_exited_barrier = 0;
			pthread_cond_broadcast(&_exited_cond);
		}
		pthread_mutex_unlock(&_mutex);
	}


	CondVar::CondVar() {
		pthread_mutex_init(&_crtmutex, NULL);
		pthread_cond_init(&_cond, NULL);
	}

	CondVar::~CondVar() {
		pthread_mutex_destroy(&_crtmutex);
		pthread_cond_destroy(&_cond);
	}

	int CondVar::Wait(const double& time)
	{
		int ret = -1;
		// небольшая защита - если поток будет прерван во время pthread_cond_wait(),
		// он сразу же залочит мьютекс - мы должны разлочить его перед выходом
		pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*)&this->_crtmutex);
		// блокируем мьютекс доступа
		pthread_mutex_lock(&_crtmutex);
		if (time >= 0.0) {
			struct timespec tp;
			clock_gettime(CLOCK_REALTIME, &tp);
			tp.tv_sec  += (int)time;
			tp.tv_nsec += (int)((time - (int)time)*1e9);
			ret = pthread_cond_timedwait(&_cond, &_crtmutex, &tp);
		}
		else
			ret = pthread_cond_wait(&_cond, &_crtmutex);
		pthread_mutex_unlock(&_crtmutex);
		pthread_cleanup_pop(0);
		if (!ret)
			return THREAD_SUCCESS;
		if (ret == ETIMEDOUT)
			return THREAD_TIMEOUT;
		return THREAD_FAILURE;
	}

	int CondVar::Notify() {
		pthread_mutex_lock(&_crtmutex);
		int ret = pthread_cond_signal(&_cond);
		pthread_mutex_unlock(&_crtmutex);
		return ret;
	}

	int CondVar::NotifyAll() {
		pthread_mutex_lock(&_crtmutex);
		int ret = pthread_cond_broadcast(&_cond);
		pthread_mutex_unlock(&_crtmutex);
		return ret;
	}

	Event::Event(int type) :_evt_type(type) {}

	bool Event::IsUserEvent() {
		return _evt_type > 0;
	}

	int Event::Type() {
		return _evt_type;
	}

	Thread::TermEx::TermEx(int code) :exit_code(code) {}

	Thread::Thread() :_state(STATE_STOPPED), _barrier(2), _start_flag(FLAG_NOT_STARTED) {
		memset(&_thread, 0, sizeof(_thread));
	}

	Thread::~Thread() {
		// Остановим поток и подождем его завершения 1 секунду
		Stop();
		int ret = Join(1.0);
		// Убъем поток, если он не хочет тормозить )
		if (ret != THREAD_SUCCESS && ret != THREAD_WRONG_SEQ)
			Kill();
	}

	void Thread::Sleep(double timeout) {
		if (timeout <= 0.0)
			pause();
		else {
			struct timespec t;
			t.tv_sec = (int)timeout;
			t.tv_nsec = (int)((timeout - t.tv_sec)*1e9);
			nanosleep(&t, NULL);
		}
	}

	Thread::State Thread::ThreadState() {
		Thread::State state;
		_mutex.Lock();
		state = _state;
		_mutex.UnLock();
		return state;
	}

	int Thread::StartFlag() {
		_sync_mutex.Lock();
		int ret = _start_flag;
		_sync_mutex.UnLock();
		return ret;
	}

	int Thread::Start() {
		_mutex.Lock();
		// запуск возможен только из состояния STOP
		if (_state == STATE_STOPPED) {
			// Почистим за собой предыдущим
			CleanupThread();
			int ret = pthread_create(&_thread, NULL, &Thread::RealMain, this);
			if (ret) {
				_mutex.UnLock();
				return THREAD_FAILURE;
			}
			// Подождем старта потока
			_barrier.Wait();
			_state = STATE_RUNNING;
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		else if (_state == STATE_STOPPING) {
			_state = STATE_RESTARTING;
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		_mutex.UnLock();
		return THREAD_WRONG_SEQ;
	}

	int Thread::WaitStartup() {
		int ret = FLAG_NOT_STARTED;
		// Проверим, может мы и не стартовали
		State st = ThreadState();
		if (st == STATE_STOPPED || st == STATE_STOPPING)
			return ret;
		// Подождем старта
		for (; (ret = StartFlag()) == Thread::FLAG_NOT_STARTED; Sleep(0.05));
		return ret;
	}

	int Thread::Stop() {
		_mutex.Lock();
		if (_state == STATE_RUNNING) {
			pthread_cancel(_thread);
			_state = STATE_STOPPING;
			_mutex.UnLock();
			_sync_mutex.Lock();
			_events.clear();
			_events.push_back(Event(THREAD_STOP_SIG));
			_sync_mutex.UnLock();
			_condvar.NotifyAll();
			return THREAD_SUCCESS;
		}
		else if (_state == STATE_RESTARTING) {
			_state = STATE_STOPPING;
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		_mutex.UnLock();
		return THREAD_WRONG_SEQ;
	}

	int Thread::Join(const double& time) {
		_mutex.Lock();
		if (_state == STATE_STOPPED) {
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		_mutex.UnLock();
		struct timespec abstime;
		abstime.tv_sec  = (int)time;
		abstime.tv_nsec = (int)((time - abstime.tv_sec)*1e9);
		int ret = pthread_timedjoin_np(_thread, NULL, &abstime);
		if (!ret) {
			_mutex.Lock();
			CleanupThread();
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		return THREAD_FAILURE;
	}

	int Thread::Kill() {
		int ret = !(pthread_kill(_thread, SIGTERM));
		if (ret) {
			_mutex.Lock();
			_state = STATE_STOPPED;
			CleanupThread();
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		return THREAD_FAILURE;
	}

	void Thread::Notify(const Event& evt) {
		_sync_mutex.Lock();
		_events.push_back(evt);
		_sync_mutex.UnLock();
		_condvar.Notify();
	}

	int Thread::MainStart() { 
		return 0; 
	}

	void Thread::MainQuit() {}

	void Thread::CancelPoint() {
		int oldstate;
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
		pthread_testcancel();
		pthread_setcancelstate(oldstate, NULL);
	}

	Event Thread::Wait(const double& time, bool interruptable) {
		while (_events.empty()) {
			int rt = _condvar.Wait(time);
			if (rt == THREAD_TIMEOUT)
				return Event(THREAD_TIMEOUT);
			else if (rt != THREAD_SUCCESS)
				return Event(THREAD_FAILURE);
		}
		_sync_mutex.Lock();
		Event evt = _events.front();
		_events.pop_front();
		_sync_mutex.UnLock();
		if (evt.Type() == THREAD_STOP_SIG && interruptable)
			CancelPoint();
		return evt;
	}

	void* Thread::RealMain(void* thread_ptr) {
		Thread* thr = reinterpret_cast<Thread*>(thread_ptr);
		thr->_sync_mutex.Lock();
		thr->_start_flag = FLAG_NOT_STARTED;
		thr->_sync_mutex.UnLock();
		// Отключить прерывание потока
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		// синхронизация с потоком-родителем
		thr->_barrier.Wait();
		// установить функцию, срабатывающую по прерыванию
		pthread_cleanup_push(&RealMainQuit, thread_ptr);
		// запустим код пользователя
		int ret = thr->MainStart();
		// Запускаем Main()
		if (ret == 0) {
			thr->_sync_mutex.Lock();
			thr->_start_flag = FLAG_STARTED_SUCCESFULLY;
			thr->_sync_mutex.UnLock();
			thr->Main();
		}
		else {
			thr->_sync_mutex.Lock();
			thr->_start_flag = ret;
			thr->_sync_mutex.UnLock();
		}
		pthread_cleanup_pop(1);
		return NULL;
	}

	void Thread::RealMainQuit(void* thread_ptr) {
		Thread* thr = reinterpret_cast<Thread*>(thread_ptr);
		thr->_mutex.Lock();
		if (thr->_state == STATE_RESTARTING) {
			thr->_state = STATE_STOPPED;
			thr->Start();
		}
		else {
			thr->_state = STATE_STOPPED;
		}
		thr->_mutex.UnLock();
		thr->MainQuit();
		thr->_start_flag = FLAG_NOT_STARTED;
	}

	int Thread::CleanupThread() {
		_events.clear();
		_start_flag = FLAG_NOT_STARTED;
		// мьютекс должен быть залочен тут!
		rc_thread clr;
		memset(&clr, 0, sizeof(clr));
		if (!memcmp(&_thread, &clr, sizeof(rc_thread))) {
			pthread_detach(_thread);
			memset(&_thread, 0, sizeof(_thread));
			return THREAD_SUCCESS;
		}
		return THREAD_WRONG_SEQ;
	}

} 
