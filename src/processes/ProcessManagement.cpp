#include <iostream>
#include "ProcessManagement.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include "../encryptDecrypt/Cryption.hpp"
#include <sys/mman.h>
#include <atomic>
#include <sys/fcntl.h>
#include <semaphore.h>
#include <thread>

using namespace std;

ProcessManagement::ProcessManagement()
{
    itemsSemaphore = sem_open("/items_semaphore", O_CREAT, 0666, 0);
    emptySlotsSemaphore = sem_open("/empty_slots_semaphore", O_CREAT, 0666, 1000);

    if (itemsSemaphore == SEM_FAILED || emptySlotsSemaphore == SEM_FAILED)
    {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE); // Exit if semaphores aren't initialized
    }

    shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shmFd == -1)
    {
        perror("Shared memory initialization failed");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmFd, sizeof(SharedMemory)) == -1)
    {
        perror("Shared memory resizing failed");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory
    sharedMem = static_cast<SharedMemory *>(mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
    if (sharedMem == MAP_FAILED)
    {
        perror("Memory mapping failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the shared memory structure
    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
}

bool ProcessManagement::submitToQueue(unique_ptr<Task> task)
{
    sem_wait(emptySlotsSemaphore);
    {
        unique_lock<mutex> lock(queueLock);

        if (sharedMem->size.load() >= 1000)
        { 
            lock.unlock();
            sem_post(emptySlotsSemaphore); // Release empty slot if full
            return false;
        }

        strncpy(sharedMem->tasks[sharedMem->rear], task->toString().c_str(), 256);
        sharedMem->rear = (sharedMem->rear + 1) % 1000;
        sharedMem->size.fetch_add(1);
    }

    sem_post(itemsSemaphore); // Signal that a new item is available

    thread thread_1(&ProcessManagement::executeTasks,this);
    thread_1.detach();

    // int pid = fork();
    // if (pid < 0)
    //     return false;
    // else if (pid > 0)
    //     cout << "Entering the parent process" << endl;
    // else
    // {
    //     cout << "entering the child process" << endl;
    //     executeTasks();
    //     cout << "Exiting the child process" << endl;
    //     exit(0);
    // }
    return true;
}

void ProcessManagement::executeTasks()
{
    sem_wait(itemsSemaphore);

    char taskStr[256];
    {
        unique_lock<mutex> lock(queueLock);

        strncpy(taskStr, sharedMem->tasks[sharedMem->front], 256);
        sharedMem->front = (sharedMem->front + 1) % 1000;
        sharedMem->size.fetch_sub(1);
    }

    sem_post(emptySlotsSemaphore); 
    executeCryption(taskStr);
}

ProcessManagement::~ProcessManagement()
{
    munmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);
    sem_close(itemsSemaphore);
    sem_close(emptySlotsSemaphore);
    sem_unlink("/items_semaphore");
    sem_unlink("/empty_slots_semaphore");
}