#include <ControlledSharedMemory.h>

#include <pthread.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>

#include <stdio.h>

ControlledSharedMemory::ControlledSharedMemory(const char* in_sem_name,
					       const char* in_shm_name,
					       int size):
  sem_name(in_sem_name), shm_name(in_shm_name){
    this->size = size;
    this->readCallbacks = new std::vector<Callback>();
    this->updateCallbacks = new std::vector<Callback>();
    this->initialized = false;
}

ControlledSharedMemory::~ControlledSharedMemory(){
  // Delete vectors
  delete this->readCallbacks;
  delete this->updateCallbacks;

  // If I'm not initialzed I should just return
  if(!initialized){
    return;
  }

  // Closes POSIX semaphore
  if(sem_close(sem) < 0){
    perror("Attempting to close semaphore ");
  }
  
  // Unmap local address
  if(munmap(data, size) < 0){
    perror("Attempting to unmap local address ");
  }

  // Closes POSIX shared memory
  if(close(shm) < 0){
    perror("Attempting to close shared memory ");
  }

  // Delete internal state associate with update thread
  int status = pthread_mutex_trylock(this->updateThreadMutex);
  if(status == 0){
    // No thread running
    if(pthread_mutex_unlock(this->updateThreadMutex) != 0){
      perror("Attempting to unlock update thread mutex ");
    }

    if(pthread_mutex_destroy(this->updateThreadMutex) != 0){
      perror("Attempting to destroy update thread mute ");
    }
  }else if(status == EBUSY){
    // Update thread is running, kill thread
    if(pthread_cancel(*(this->updateThreadIdentifer)) != 0){
      perror("Attempting to kill update thread ");
    }
    operator delete(this->updateThreadData->data);
    delete this->updateThreadData;
  }else{
    // Error on try lock on update thread mutex
    perror("Error on try locking on update thread mutex ");
  }

  // Delete the mutex
  // Notice: if update thread is running, will not destroy mutex
  // before deletion.
  delete this->updateThreadMutex;

  // Delete internal pointers
  delete this->updateThreadIdentifer;

}

int ControlledSharedMemory::initialize(void){
  if(initialized){
    fprintf(stderr, "Already intilized memory segment.\n");
    return -1;
  }
  if(initializeMutexes() < 0){
    fprintf(stderr, "Unable to initialize mutexes.\n");
    return -1;
  }
  if(initializeMemorySegment() < 0){
    fprintf(stderr, "Unable to initialize memory segments.\n");
    return -1;
  }
  initialized = true;
  return 0;
}

int ControlledSharedMemory::initializeMutexes(void){
  // Allocate for updateThreadIdentifer
  this->updateThreadIdentifer = new pthread_t();
  if(!this->updateThreadIdentifer){
    fprintf(stderr, "Unable to creat new update thread identifier.\n");
    return -1;
  }

  // Allocate update thread mutex
  this->updateThreadMutex = new pthread_mutex_t();
  if(!this->updateThreadMutex){
    fprintf(stderr, "Unable to create new update thread mutex.\n");
    return -1;
  }

  // Initialze update thread mutex
  if(pthread_mutex_init(updateThreadMutex, NULL) != 0){
    perror("Attempting to init updateThreadMutex ");
    return -1;
  }

  // Allocate read threa mutex
  this->readThreadMutex = new pthread_mutex_t();
  if(!this->readThreadMutex){
    fprintf(stderr, "Unable to creat new read thread mutex.\n");
    return -1;
  }

  // Initialize read thread mutex
  if(pthread_mutex_init(readThreadMutex, NULL) != 0){
    perror("Attempting to init readThreadMutex ");
    return -1;
  }
  return 0;
}

int ControlledSharedMemory::initializeMemorySegment(void){
  mode_t mode_0766 = S_IRWXU | 
    S_IRGRP | S_IWGRP |
    S_IROTH | S_IWOTH;

  // Create POSIX semaphore object
  if(!sem_name) return -1;
  sem = sem_open(sem_name, O_RDWR | O_CREAT, 
		 mode_0766, 1);

  if(sem == SEM_FAILED){
    perror("Attempting to intialize memory semaphore ");
    return -1;
  }

  // Create POSIX shared memory object
  if(!shm_name) return -1;
  shm = shm_open(shm_name, O_RDWR | O_CREAT, mode_0766);
  
  if(shm < 0){
    perror("Attempting to intilize shared memory segment ");
    return -1;
  }

  // Truncate the file descriptor to the correct length
  if(ftruncate(shm, size) < 0){
    perror("Attemtping to truncate shared memory segment ");
    return -1;
  }

  // Map the fd into memory
  data = mmap(0, size, PROT_WRITE | PROT_READ,
		  MAP_SHARED, shm, 0);
  if(data < 0){
    perror("Attemtping to make shared memory into local memory ");
    return -1;
  }

  return 0;
}

int ControlledSharedMemory::registerReadCallback(Callback func){
  if(!func){
    fprintf(stderr, "Attempting to register a null callback function.");
    return -1;
  }
  readCallbacks->push_back(func);
  return 0;
}

int ControlledSharedMemory::registerUpdateCallback(Callback func){
  if(!func){
    fprintf(stderr, "Attempting to register a null callback function.");
    return -1;
  }
  updateCallbacks->push_back(func);
  return 0;
}

int ControlledSharedMemory::unlinkMemorySegment(void){
  // Test if initialized
  if(!initialized){
    fprintf(stderr, "Attempting to unlink unitialized memory segment.\n");
    return -1;
  }

  // Unlink shared mem
  if(shm_unlink(shm_name) < 0){
    perror("Attempting to unlink shared memory segment ");
    return -1;
  }

  // Unlink semaphore
  if(sem_unlink(sem_name) < 0){
    perror("Attempting to unlinke semaphore ");
    return -1;
  }
  return 0;
}

int ControlledSharedMemory::updateSharedMemory(const void* in_data){
  // Check initialized
  if(!this->initialized){
    fprintf(stderr, "Shared memory is not initialized.\n");
    return -1;
  }

  // Argument checking
  if(!in_data){
    fprintf(stderr, "Null in_data pointer.\n");
    return -1;
  }

  // Attempt to lock on sempahore
  int status = sem_trywait(this->sem);
  if(status == 0){
    // Lock was successful
    // Copy to shared memory
    memcpy(this->data, in_data, this->size);

    // If update thread exist, kill
    status = pthread_mutex_trylock(this->updateThreadMutex);

    if(status == EBUSY){
      if(pthread_cancel(*(this->updateThreadIdentifer)) != 0){
	perror("Attempting to cancel update thread ");
	return -1;
      }

      if(pthread_mutex_unlock(this->updateThreadMutex) != 0){
	perror("Attempting to unlcok update thread mutex ");
	return -1;
      }
    }

    // Unlock sem
    if(sem_post(this->sem) != 0){
      perror("Error unlocking semaphore ");
      return -1;
    }
    
    if(this->notifyUpdateCallbacks() != 0){
      fprintf(stderr, "Error calling update callbacks.\n");
      return -1;
    }

    return IMMEDIATE_UPDATE;
  }else if(status == EAGAIN){
  
    // Check if thread already exists
    status = pthread_mutex_trylock(updateThreadMutex);

    if(status == 0){// Thread does not exist
      // Create a new thread
      
      // Copy data into local space
      //      void* copy_data = malloc(size);
      void* copy_data = operator new(size);
      if(!copy_data){
	fprintf(stderr, "Unable to allocate memory for local_copy data.\n");
	return -1;
      }
      memcpy(copy_data, in_data, size);

      // Create associated mutex for protected data
      pthread_mutex_t* associated_mutex = new pthread_mutex_t();
      if(pthread_mutex_init(associated_mutex, NULL) != 0){
	perror("Unable to initialize associated mutex for protected data ");
	operator delete(copy_data);
	return -1;
      }
      
      // Create mutex protected data
      //      mutex_data_t* mutex_data = (mutex_data_t*)malloc(sizeof(mutex_data_t));
      mutex_data_t* mutex_data = new mutex_data_t();
      
      if(!mutex_data){
	fprintf(stderr, "Unable to allocate memory for mutex protected data.\n");
	operator delete(copy_data);
	pthread_mutex_destroy(associated_mutex);
	delete associated_mutex;
	return -1;
      }
      
      mutex_data->data = copy_data;
      mutex_data->mutex = associated_mutex;
      
      this->updateThreadData = mutex_data;

      // Unlock mutex so thread can use
      if(pthread_mutex_unlock(this->updateThreadMutex) != 0){
	perror("Attempting to unlock update thread mutex for thread use ");
	operator delete(copy_data);
	pthread_mutex_destroy(associated_mutex);
	delete associated_mutex;
	delete mutex_data;
	return -1;
      }
      
      // Spawn new thread
      if(pthread_create(this->updateThreadIdentifer, NULL, 
			launchUpdateThread, (void*)this) != 0){
	perror("Attempting to create update thread ");
	operator delete(copy_data);
	pthread_mutex_destroy(associated_mutex);
	delete associated_mutex;
	delete mutex_data;
	return -1;
      }

      return SPAWNING_NEW_UPDATE_THREAD;
    }else if(status == EBUSY){// Thread does exist
      // Attempt to lock on the data
      mutex_data_t* mutex_data = this->updateThreadData;
      
      status = pthread_mutex_trylock(mutex_data->mutex);
      
      if(status == 0){
	// Modify mutex protected data
	if(mutex_data->data){
	  memcpy(mutex_data->data, in_data, this->size);
	}else{
	  // This occurs if a call happens right when the update thread
	  // is releasing the mutex and about to delete it
	  fprintf(stderr, "Data unavaible, try again.\n");
	  return -1;
	}
	
	// Unlock mutex protected data
	if(pthread_mutex_unlock(mutex_data->mutex) != 0){
	  perror("Attempting to unlcok mutex protected data ");
	  return -1;
	}
	return UPDATED_OLD_THREAD;
      }else if(status == EBUSY){
	// Internal data lock
	return INTERNAL_DATA_LOCK;
      }else{
	// Error Code
	perror("Attempting to lock on internal data ");
	return -1;
      }
    }else{// Error code
      perror("Attempting to determine if thread exists ");
      return -1;
    }
  }else{
    // Error on semaphore trywait
    perror("Attempting to wait on semaphore ");
    return -1;
  }
}

void* ControlledSharedMemory::launchUpdateThread(void* in_ptr){
  return ((ControlledSharedMemory*)in_ptr)->updateSegment();
}

void* ControlledSharedMemory::updateSegment(void){
  // All this thread should do is block on the semaphore, then lock the semaphore.
  // Lock the mutex protected data, and write

  // Lock mutex to signal thread is running
  if(pthread_mutex_lock(this->updateThreadMutex) != 0){
    perror("Attempting to lock signal mutex ");
    pthread_exit((void*)&ERROR_UPDATE_THREAD);
  }

  if(sem_wait(this->sem) == 0){
    // Now lock mutex protected data
    if(pthread_mutex_lock(this->updateThreadData->mutex) == 0){
      // Copy data
      if(this->updateThreadData->data){
	memcpy(this->data, this->updateThreadData->data, this->size);
      }else{
	fprintf(stderr, "Invalid data to copy over.\n");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }

      // Unlock and delete all associated data containers
      operator delete(this->updateThreadData->data);

      if(pthread_mutex_unlock(this->updateThreadData->mutex) != 0){
	perror("Attemtping to unlock mutex protected data ");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }
      
      if(pthread_mutex_destroy(this->updateThreadData->mutex) != 0){
	perror("Attempting to destroy mutex protected data ");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }
      delete this->updateThreadData->mutex;
      delete this->updateThreadData;

      if(pthread_mutex_unlock(this->updateThreadMutex) != 0){
	perror("Attempting to unlock update thread mutex signal ");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }

      if(sem_post(this->sem) != 0){
	perror("Attempting to post shared memory associate semaphore ");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }

      if(this->notifyUpdateCallbacks() != 0){
	fprintf(stderr, "Error notifying update callbacks.\n");
	pthread_exit((void*)&ERROR_UPDATE_THREAD);
      }

      pthread_exit((void*)&SUCCESS_UPDATE_THREAD);
    }else{
      sem_post(this->sem);
      perror("Attempting to lock mutex protected data in update thread ");
      pthread_exit((void*)&ERROR_UPDATE_THREAD);
    }
  }else{
    perror("Attempting to wait on semaphore in update thread ");;
    pthread_exit((void*)&ERROR_UPDATE_THREAD);
  }
}

int ControlledSharedMemory::notifyReadCallbacks(void){
  pthread_t thread_id;
  for(size_t i = 0; i < this->readCallbacks->size(); ++i){
    if(pthread_create(&thread_id, NULL,
		      this->readCallbacks->at(it), (void*)NULL) != 0){
      perror("Error creating notify read callback ");
      return -1;
    }
  }
  return 0;
}

int ControlledSharedMemory::notifyUpdateCallbacks(void){
  pthread_t thread_id;
  for(size_t i = 0; i < this->updateCallbacks->size(); ++i){
    if(pthread_create(&thread_id, NULL, 
		      this->updateCallbacks->at(i), (void*)NULL) != 0){
      perror("Error creating notify update callback ");
      return -1;
    }
  }
  return 0;
}

int ControlledSharedMemory::requestRead(void){

}
