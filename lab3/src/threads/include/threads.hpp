#ifndef THREADS_HPP
#define THREADS_HPP

#ifdef WIN32
#	include <windows.h>  // HANDLE и CRITICAL_SECTION
typedef CRITICAL_SECTION rc_mutex; 
typedef HANDLE rc_thread;
typedef DWORD rc_thread_id;
#else
#	include <pthread.h>       // pthread_* functions
#	include <signal.h>        // pthread_kill()
#   include <unistd.h>        // pause()
#   include <errno.h>         // system error types
typedef pthread_t rc_thread;
typedef pthread_mutex_t rc_mutex;
typedef pthread_t rc_thread_id;
#endif

#include <string.h> // memset()
#include <deque>    // std::deque

namespace Threads
{
	// Коды возврата
	enum ThreadReturns
	{
		THREAD_SUCCESS = 0,      // Успех
		THREAD_FAILURE = -1,     // Общая, неспецифицированная ошибка
		THREAD_TIMEOUT = -2,     // Таймаут
		THREAD_WRONG_SEQ = -3,   // Неверный порядок вызова
		THREAD_INVALIDPAR = -4,  // Неверный параметр
		THREAD_STOP_SIG = -5     // Сигнал на остановку потока
	};

	// Простой, рекурсивный мьютекс
	class Mutex
	{
		friend class AutoMutex;
	public:
		Mutex();
		~Mutex();
		void Lock();
		void UnLock();
		bool TryLock();
	private:
		rc_mutex _mutex;
		// Защита от копирования
		Mutex(Mutex const&) {}
		Mutex& operator=(Mutex const&) { return *this; }
	};

	// Специальный класс мьютекса, который блокируется при создании
	// и разблокируется при уничтожении
	class AutoMutex
	{
	public:
		AutoMutex();
		AutoMutex(Mutex& mut);
		~AutoMutex();
	private:
		rc_mutex* _mutex;
		bool _local;
		AutoMutex(Mutex const&) {}
		AutoMutex& operator=(Mutex const&) { return *this; }
	};

	// Барьер
	class Barrier
	{
	public:
		// Параметр конструктора - число ожидающих потоков
		Barrier(int value = 2);
		~Barrier();
		// Подождать, пока все потоки войдут за барьер
		void WaitEnter();
		// Подождать, пока все потоки выйдут
		void WaitExit();
		// Ожидание в потоке
		void Wait();
	private:
		int _value;
#ifdef WIN32
		volatile LONG _entered_barrier;
		volatile LONG _exited_barrier;
		HANDLE _entrance_semaphore;
		HANDLE _exit_semaphore;
#else
		volatile int _entered_barrier;
		volatile int _exited_barrier;
		pthread_mutex_t _mutex;
		pthread_cond_t _entered_cond;
		pthread_cond_t _exited_cond;
#endif
		// Защита от копирования
		Barrier(Barrier const&) {}
		Barrier& operator=(Barrier const&) { return *this; }
	};

	// Условная переменная, которую могут подождать несколько потоков
	class CondVar
	{
	public:
		CondVar();
		~CondVar();
		// Ожидать сигнала time секунд. Вечно, если time < 0
		int Wait(const double& time = -1.0);
		// Предупредить один ожидающий поток
		int Notify();
		// Предупредить все ожидающие потоки
		int NotifyAll();
	private:
		// Мьютекс для синхронизации
		rc_mutex _crtmutex;
#ifdef WIN32
		// Список ожидающих потоков
		::std::deque<HANDLE> _waiting_list;
		// мьютекс для защиты листа ожидания
		rc_mutex _wlmutex;
#else
		// условная переменная
		pthread_cond_t _cond;
#endif
		// защита от копирования
		CondVar(CondVar const&) {}
		CondVar& operator=(CondVar const&) { return *this; }
	};

	// Класс события
	class Event
	{
	public:
		Event(int type);
		bool IsUserEvent();
		int Type();
	private:
		int _evt_type;
	};

	// Класс потока
	// Наследники перегружают функции Main(), MainStart() и MainStop()
	class Thread
	{
	public:
		// Машина состояний потока
		enum State
		{
			STATE_STOPPED,
			STATE_RUNNING,
			STATE_STOPPING,
			STATE_RESTARTING
		};
		// Результат запуска потока
		enum StartingFlag
		{
			FLAG_NOT_STARTED = -1,
			FLAG_STARTED_SUCCESFULLY = 0,
			FLAG_FAIL_TO_START = 1
		};
		// exception бросаемый при завершении
		struct TermEx
		{
			TermEx(int code);
			int exit_code;
		};

		Thread();
		virtual ~Thread();

	public:
		static void Sleep(double timeout);
		// Состояние потока 
		Thread::State ThreadState();
		// Результат запуска потока
		int StartFlag();
		// Запустить поток и продолжить исполнение родителя
		int Start();
		// Подождать запуска потока в родителе
		int WaitStartup();
		// Послать потоку команду на останов
		int Stop();
		// Подождать завершение исполнения потока time секунд
		int Join(const double& time = -1.0);
		// Убить поток принудительно
		int Kill();
		// Предупредить ожидающий поток
		void Notify(const Event& evt);

	protected:
		// Главная функция в потоке
		virtual void Main() = 0;
		// Функция, запускаемая до запуска основного потока
		virtual int MainStart();
		// Функция, запуская перед остановкой потока
		virtual void MainQuit();
		// Точка возможного прерывания потока
#ifdef WIN32
#	if defined (_MSC_VER)
		void CancelPoint() throw(...);
#   else
		void CancelPoint();
#	endif
#else
		void CancelPoint();
#endif
		// Подождать предупреждения (Notify())
		Event Wait(const double& time = -1.0, bool interruptable = true);

	private:
		// Основная функция потока
		static void* RealMain(void* thread_ptr);
		// Основная функция завершения потока
		static void RealMainQuit(void* thread_ptr);
		// Очистить поток
		int CleanupThread();

		// Состояние потока
		State _state;
		// Системный объект потока
		rc_thread _thread;
		// Условная переменная для ожидания
		CondVar _condvar;
		// Барьер для синхронизации потоков на старте
		Barrier _barrier;
		// Мьютексы для защиты данных потока
		Mutex _sync_mutex;
		Mutex _mutex;
		// Notify-сигнал
		::std::deque<Event> _events;
		// Результат старта
		int _start_flag;
#ifdef WIN32
		//Событие очистки
		HANDLE _cleanup_event;
#endif
		// Защита от копирования
		Thread(Thread const&) {}
		Thread& operator=(Thread const&) { return *this; }
	};
}

#endif // THREADS_HPP

