#include "pch.h"

#include "semaphore.h"

#ifdef KORE_WINDOWS

#include <Windows.h>

Semaphore::Semaphore(int count) {
	semaphore = CreateSemaphore(nullptr, count, 0x7fffffff, nullptr);
}

Semaphore::~Semaphore() {
	CloseHandle(semaphore);
}

void Semaphore::wait() {
	WaitForSingleObject(semaphore, INFINITE);
}

void Semaphore::signal() {
	ReleaseSemaphore(semaphore, 1, nullptr);
}

#endif
