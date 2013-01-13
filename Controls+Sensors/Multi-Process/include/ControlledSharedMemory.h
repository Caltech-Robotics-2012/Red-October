#ifndef CONTROLLED_SHARED_MEMORY_H
#define CONTROLLED_SHARED_MEMORY_H

#include <semaphore.h>

#include <pthread.h>

#include <vector>
#include <stdio.h>

typedef void (*Callback)(void*);

/**
 * This class is designed for controlling the access of multiple applications to an area of 
 * shared memory through semaphores.
 */
class ControlledSharedMemory{
 public:
  /**
   * Constructor.  Both of the below strings should be prefixed with a / and should not
   * have any other embedded slashes.
   * @param sem_name, Name of the named semaphore
   * @param shm_name, Name of the named shared memory segment
   */
  ControlledSharedMemory(const char* in_sem_name, const char* in_shm_name, int size);

  /**
   * Deconstructor.
   */
  ~ControlledSharedMemory(void);

  /**
   * Unlinks shared memory permanently deleting it from the systeml, this means that following 
   * references will create a new shared memory segment.
   * @return result, Integer result corresponding to one of the following conditions
   *	-1, Error
   *	 0, No Error
   */
  int unlinkMemorySegment(void);
  
  /**
   * Initialize the shared memory class library.
   * @return result, Integer result corresponding to one of the following conditions
   *	-1, Error
   *     0, No Error
   */
  int initialize(void);

  /**
   * Update the shared memory segment.  Spawns a thread to perform the update if unable to first
   * lock on the semaphore.  If an update is called again without the first thread performing the
   * update, the process will attempt to lock on the data being updated.  If it is locked, it will
   * spawn a new thread to update, if not, it will lock, then modify the data.
   * @param data, Pointer to data
   * @return result, Integer result corresponding to one of the following conditions
   *   -1, Error
   *	0, Immediate update
   *	1, Spawning new thread
   *	2, Updated old thread spawned
   */
  int updateSharedMemory(const void* data);

  /**
   * Requests a read from the shared memory segment.  If the process is able to lock on the data,
   * the read is performed immediately calling all registered callbacks. If the process is unable to
   * do so, spawns a thread that will call all the registered callbacks when the read it performed.
   * If an read is called again without the first thread performing the read, a new thread is not
   * spawned.
   * @return result, Integer result corresponding to one of the following conditions
   *   -1, Error
   *	0, Immediate read
   *    1, Spawning new thread
   *    2, Old thread not finished
   */
  int requestRead(void);

  /**
   * Registers a callback for when reads are performed.  A callback accepts a pointer to the local
   * address of the internal storage of the memory segment. Parent caller is responsible for race
   * conditions on this local storage.  Each callback is spawned in a new thread.
   * @param func, Function to be registered as a callback
   * @return result, Integer result correpsonding to one of the following conditions
   *   -1, Error
   *    0, Successful addition
   */
  int registerReadCallback(Callback func);

  /**
   * Registers a callback for when update is performed.  A callback accepts a pointer to the local 
   * address of the internal storage of the memory segment.  Parent caller is responsible for race
   * conditions on this local storage.  Each callback is spawned in a new thread.
   * @param func, Function to be registered as a callback
   * @return result, Integer result corresponding to one of the following conditions
   *   -1, Error
   *	0, Successful addition
   */
  int registerUpdateCallback(Callback func);

  /**
   * Performs unit tests.
   * @return result, Integer result corresponding to one of the following conditions
   *   -1, Error
   *    0, Successful tests.
   */
  static int unitTest(void);

 protected:
  struct mutex_data_t{
    void* data;
    pthread_mutex_t* mutex;
  }

  /**
   * State if initialized.
   */
  bool initialized;

  /**
   * Internal semaphore name storage.
   */
  const char* sem_name;

  /**
   * Internal shared memory name storage.
   */
  const char* shm_name;

  /**
   * Internal vector of read callback functions.
   */
  std::vector<Callback> readCallbacks;

  /**
   * Internal vector of update callback functions.
   */
  std::vector<Callback> updateCallbacks;

  /**
   * Internal address of the semaphore.
   */
  sem_t* sem;

  /**
   * Internal file descriptor of the shared memory.
   */
  int shm;

  /**
   * Internal pointer to the data location in mapped memory.
   */
  void* data;

  /**
   * Internal storage of the size of the memory segment.
   */
  int size;

  /**
   * Update thread mutex if thread is active.
   */
  pthread_mutex_t* updateThreadMutex;

  /**
   * Update thread data.
   */
  mutex_data_t* updateThreadData;

  /**
   * Read thread mutex if thread is active.
   */
  pthread_mutex_t* readThreadMutex;

  /**
   * Function for update thread entry point.
   * @param, Pointer to data to be done.
   */
  void* updateSegment(const mutex_data_t* data);

  /**
   * Initialize the mutexes.
   * @return result, Integer result corresponding to one of the following conditions
   *	-1, Error
   *     0, No Error
   */
  int initializeMutexes(void);

  /**
   * Initialize the memory segments, the semaphores and shm segments.
   * @return result, Integer result corresponding to one of the following conditions
   *	-1, Error
   *     0, No Error
   */
  int initializeMemorySegment(void);
};

#endif /* CONTROLLED_SHARED_MEMORY_H */
