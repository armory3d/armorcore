#pragma once

class Semaphore {
public:
	Semaphore(int count);
	~Semaphore();
	void wait();
	void signal();
private:
#ifdef KORE_WINDOWS
	void* semaphore;
#endif
};
