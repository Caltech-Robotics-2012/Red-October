#include <ControlledSharedMemory.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

ControlledSharedMemory::ControlledSharedMemory(const char* in_sem_name,
					       const char* in_shm_name,
					       int size):
  sem_name(in_sem_name), shm_name(in_shm_name){
    this->size = size;
    initialized = false;
}

ControlledSharedMemory::~ControlledSharedMemory(){
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
  // Initialze update thread mutex
  if(pthread_mutex_init(updateThreadMutex, NULL) != 0){
    perror("Attempting to init updateThreadMutex ");
    return -1;
  }

  // Initialize read thread mutex
  if(pthread_mutex_init(readThreadMutex, NULL) != 0){
    perror("attempting to init readThreadMutex ");
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
  readCallbacks.push_back(func);
  return 0;
}

int ControlledSharedMemory::registerUpdateCallback(Callback func){
  if(!func){
    fprintf(stderr, "Attempting to register a null callback function.");
    return -1;
  }
  updateCallbacks.push_back(func);
  return 0;
}

int ControlledSharedMemory::unlinkMemorySegment(void){
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

int ControlledSharedMemory::updateSharedMemory(const void* data){
  // Check if thread already exists
  int status = pthread_mutex_trylock(updateThreadMutex);

  if(status == 0){// Thread does not exist
    // Create a new thread

    // Copy data into local space
    void* copy_data = malloc(size);
    if(!copy_data){
      fprintf(stderr, "Unable to allocate memory for local_copy data.\n");
      return -1;
    }
    memcpy(copy_data, data, size);

    // Create associated mutex for protected data
    pthread_mutex_t* associated_mutex;
    if(pthread_mutex_init(associated_mutex, NULL) != 0){
      perror("Unable to initialize associated mutex for protected data ");
      free(copy_data);
      return -1;
    }

    // Create mutex protected data
    mutex_data_t* mutex_data = (mutex_data_t*)malloc(sizeof(mutex_data_t));

    if(!mutex_data){
      fprintf(stderr, "Unable to allocate memory for mutex protected data.\n");
      free(copy_data);
      pthread_mutex_
      return -1;
    }

    mutex_data->data = copy_data;
    mutex_data->mutex = mutex_data;

    this->updateThreadData = mutex_data;

    // Spawn new thread
    pthread_t thread_id;
    status = pthread_create(&thread_id, NULL, updateSegment, (void*)mutex_data);

    // Unlock mutex
    if(pthread_mutex_unlock(updateThreadMutex) != 0){
      perror("Attempting to unlock mutex ");
      return -1;
    }
  }else if(status == EBUSY){// Thread does exist
    // Attempt to lock on the data
  }else{// Error code
    perror("Attempting to determine if thread exists ");
    return -1;
  }
}
