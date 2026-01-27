

#include "threads.hpp"

namespace Threads
{
	// ==================== Mutex ====================
	Mutex::Mutex() {
		InitializeCriticalSection(&_mutex);
	}

	Mutex::~Mutex() {
		DeleteCriticalSection(&_mutex);
	}

	void Mutex::Lock() {
		EnterCriticalSection(&_mutex);
	}

	void Mutex::UnLock() {
		LeaveCriticalSection(&_mutex);
	}

	bool Mutex::TryLock() {
		return (TryEnterCriticalSection(&_mutex) != 0);
	}

	// ==================== AutoMutex ====================
	AutoMutex::AutoMutex() :_local(true) {
		_mutex = new rc_mutex();
		InitializeCriticalSection(_mutex);
		EnterCriticalSection(_mutex);
	}

	AutoMutex::AutoMutex(Mutex& mut) :_local(false) {
		_mutex = &mut._mutex;
		EnterCriticalSection(_mutex);
	}

	AutoMutex::~AutoMutex() {
		LeaveCriticalSection(_mutex);
		if (_local)
			DeleteCriticalSection(_mutex);
		if (_local)
			delete(_mutex);
	}

	// ==================== Barrier ====================
	Barrier::Barrier(int value) :_value(value) {
		_entered_barrier = 0;
		_exited_barrier = 0;
		_entrance_semaphore = CreateSemaphore(NULL, 0, 4096, NULL);
		_exit_semaphore = CreateSemaphore(NULL, 0, 4096, NULL);
	}

	Barrier::~Barrier() {
		CloseHandle(_entrance_semaphore);
		CloseHandle(_exit_semaphore);
	}

	void Barrier::WaitEnter() {
		if (InterlockedIncrement(&_entered_barrier) < _value)
			WaitForSingleObject(_entrance_semaphore, INFINITE);
		else {
			_exited_barrier = 0;
			ReleaseSemaphore(_entrance_semaphore, _value - 1, NULL);
		}
	}

	void Barrier::WaitExit() {
		if (InterlockedIncrement(&_exited_barrier) < _value)
			WaitForSingleObject(_exit_semaphore, INFINITE);
		else {
			_entered_barrier = 0;
			ReleaseSemaphore(_exit_semaphore, _value - 1, NULL);
		}
	}

	void Barrier::Wait() {
		if (InterlockedIncrement(&_exited_barrier) < _value)
			WaitForSingleObject(_exit_semaphore, INFINITE);
		else {
			_exited_barrier = 0;
			ReleaseSemaphore(_exit_semaphore, _value - 1, NULL);
		}
	}

	// ==================== CondVar ====================
	CondVar::CondVar() {
		InitializeCriticalSection(&_crtmutex);
		InitializeCriticalSection(&_wlmutex);
	}

	CondVar::~CondVar() {
		DeleteCriticalSection(&_crtmutex);
		DeleteCriticalSection(&_wlmutex);
	}

	int CondVar::Wait(const double& time)
	{
		// Заблокируем мьютьекс доступа
		EnterCriticalSection(&_crtmutex);
		// Создадим новое событие
		HANDLE wait_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (wait_event == NULL)
			return THREAD_FAILURE;
		// Поместим событие в deque
		EnterCriticalSection(&_wlmutex);
		_waiting_list.push_back(wait_event);
		LeaveCriticalSection(&_wlmutex);
		// Освободим мьютекс доступа
		LeaveCriticalSection(&_crtmutex);
		// ожидаем события
		DWORD millisecs = INFINITE;
		if (time >= 0.0)
			millisecs = (DWORD)(time * 1e3);
		DWORD wait_result = WaitForSingleObjectEx(wait_event, millisecs, FALSE);
		// Заблокируем мьютьекс доступа
		EnterCriticalSection(&_crtmutex);
		// Если мы вышли по таймауту - значит никто не достал событие из списка
		// значит сделаем это сами
		if (wait_result == WAIT_TIMEOUT) {
			EnterCriticalSection(&_wlmutex);
			_waiting_list.pop_front();
			LeaveCriticalSection(&_wlmutex);
		}
		// Закроем хэндл события
		if (!CloseHandle(wait_event)) {
			LeaveCriticalSection(&_crtmutex);
			return THREAD_FAILURE;
		}
		// Выходим из функции
		LeaveCriticalSection(&_crtmutex);
		if (wait_result == WAIT_TIMEOUT)
			return THREAD_TIMEOUT;
		if (wait_result == WAIT_OBJECT_0 || wait_result == WAIT_IO_COMPLETION)
			return THREAD_SUCCESS;
		return THREAD_FAILURE;
	}

	int CondVar::Notify() {
		// Достать первый поток из очереди (который ждет дольше всех)
		EnterCriticalSection(&_wlmutex);
		HANDLE wait_event = NULL;
		if (!_waiting_list.empty()) {
			wait_event = _waiting_list.front();
			_waiting_list.pop_front();
		}
		LeaveCriticalSection(&_wlmutex);
		// Если никто не ждет - выходим
		if (wait_event == NULL)
			return THREAD_SUCCESS;
		// Сигнализируем событие
		if (SetEvent(wait_event))
			return THREAD_SUCCESS;
		return THREAD_FAILURE;
	}

	int CondVar::NotifyAll() {
		int ret = THREAD_SUCCESS;
		EnterCriticalSection(&_wlmutex);
#if __cplusplus >= 201103L
		::std::deque<HANDLE>::const_iterator it = _waiting_list.begin();
		::std::deque<HANDLE>::const_iterator end = _waiting_list.end();
#else
		::std::deque<HANDLE>::iterator it = _waiting_list.begin();
		::std::deque<HANDLE>::iterator end = _waiting_list.end();
#endif
		for (; it < end; it++) {
			if (!SetEvent(*it)) {
				ret = THREAD_FAILURE;
				break;
			}
		}
		if (ret == THREAD_SUCCESS)
			_waiting_list.clear();
		else
			_waiting_list.erase(_waiting_list.begin(), it);
		LeaveCriticalSection(&_wlmutex);
		return ret;
	}

	// ==================== Event ====================
	Event::Event(int type) :_evt_type(type) {}

	bool Event::IsUserEvent() {
		return _evt_type > 0;
	}

	int Event::Type() {
		return _evt_type;
	}

	// ==================== Thread ====================
	Thread::TermEx::TermEx(int code) :exit_code(code) {}

	Thread::Thread() :_state(STATE_STOPPED), _barrier(2), _start_flag(FLAG_NOT_STARTED) {
		_cleanup_event = NULL;
		_thread = NULL;
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
			::Sleep(INFINITE);
		else
			::Sleep((DWORD)(timeout * 1e3));
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
			_cleanup_event = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (!_cleanup_event) {
				_mutex.UnLock();
				return THREAD_FAILURE;
			}
			_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(Thread::RealMain), this, 0, NULL);
			if (!_thread) {
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
			SetEvent(_cleanup_event);
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
		DWORD wtm = INFINITE;
		if (time > 0.0)
			wtm = (DWORD)(time * 1e3);
		int ret = WaitForSingleObject(_thread, wtm);
		if (ret == WAIT_TIMEOUT)
			return THREAD_TIMEOUT;
		if (ret == WAIT_OBJECT_0) {
			_mutex.Lock();
			CleanupThread();
			_mutex.UnLock();
			return THREAD_SUCCESS;
		}
		return THREAD_FAILURE;
	}

	int Thread::Kill() {
		int ret = TerminateThread(_thread, 0);
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

#if defined (_MSC_VER)
	void Thread::CancelPoint() throw(...) {
#else
	void Thread::CancelPoint() {
#endif
		int ret = WaitForSingleObject(_cleanup_event, 0);
		if (ret == WAIT_OBJECT_0) {
			throw TermEx(0);
		}
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
		// синхронизация с потоком-родителем
		thr->_barrier.Wait();
		// запустим код пользователя
		int ret = thr->MainStart();
		// Запускаем Main()
		if (ret == 0) {
			thr->_sync_mutex.Lock();
			thr->_start_flag = FLAG_STARTED_SUCCESFULLY;
			thr->_sync_mutex.UnLock();
			try { 
				thr->Main(); 
			}
			catch (TermEx)
			{
			}
		}
		else {
			thr->_sync_mutex.Lock();
			thr->_start_flag = ret;
			thr->_sync_mutex.UnLock();
		}
		RealMainQuit(thr);
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
		if (_thread) {
			CloseHandle(_thread);
			CloseHandle(_cleanup_event);
			_thread = NULL;
			_cleanup_event = NULL;
			return THREAD_SUCCESS;
		}
		return THREAD_WRONG_SEQ;
	}

} // namespace cplib
