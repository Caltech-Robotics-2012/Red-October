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
    }else if(status == EBUSY){
        // Update thread is running, kill thread
        if(pthread_cancel(*(this->updateThreadIdentifer)) != 0){
            perror("Attempting to kill update thread ");
        }
    }else{
        // Error on try lock on update thread mutex
        perror("Error on try locking on update thread mutex ");
    }

    if(pthread_mutex_destroy(this->updateThreadMutex) != 0){
        perror("Attempting to destroy update thread mute ");
    }

    // Delete internal state associate with read threa
    status = pthread_mutex_trylock(this->readThreadMutex);
    if(status == 0){
        // No thread running
        if(pthread_mutex_unlock(this->readThreadMutex) != 0){
            perror("Attempting to unlock read thread mutex ");
        }
    }else if(status == EBUSY){
        // Read thread is running, ill thread
        if(pthread_cancel(*(this->readThreadIdentifer)) != 0){
            perror("Attempting to kill read thread ");
        }
    }else{
        // Error on locking on read thread mutex
        perror("Error on trying to lock on read thread mutex ");
    }

    if(pthread_mutex_destroy(this->readThreadMutex) != 0){
        perror("Atttempting to destory read thread mutex ");
    }

    // Delete the mutex
    delete this->updateThreadMutex;
    delete this->readThreadMutex;

    // Delete internal pointers
    delete this->updateThreadIdentifer;
    delete this->readThreadIdentifer;
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

    // Allocate for readThreadIdentifier
    this->readThreadIdentifer = new pthread_t();
    if(!this->readThreadIdentifer){
        fprintf(stderr, "Unable to create new read thread identifier.\n");
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
        }else{
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
        status = pthread_mutex_trylock(this->updateThreadMutex);

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
    ((ControlledSharedMemory*)in_ptr)->updateSegment();
    return (void*)NULL;
}

void* ControlledSharedMemory::launchReadThread(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->readSegment();
    return (void*)NULL;
}

void ControlledSharedMemory::launchCleanupUpdateThreadData(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->cleanupUpdateThreadData();
}

void ControlledSharedMemory::launchCleanupUpdateThreadMutex(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->cleanupUpdateThreadMutex();
}

void ControlledSharedMemory::launchCleanupUpdateThreadDataMutex(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->cleanupUpdateThreadDataMutex();
}

void ControlledSharedMemory::launchCleanupSem(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->cleanupSem();
}

void ControlledSharedMemory::launchCleanupReadThreadMutex(void* in_ptr){
    ((ControlledSharedMemory*)in_ptr)->cleanupReadThreadMutex();
}

void ControlledSharedMemory::readSegment(){
    bool success = true;
    // All this thread should do is block on the semaphore, the lock the semaphre.
    // Then read to a file block, unlock the semaphore, the notify

    if(pthread_mutex_lock(this->readThreadMutex) != 0){
        perror("Attempting to lock read thread signal mutex ");
        success = false;
    }else{
        pthread_cleanup_push(launchCleanupReadThreadMutex, (void*) this);

        if(success && sem_wait(this->sem) == 0){
            void* buffer;
            pthread_cleanup_push(launchCleanupSem, (void*) this);

            // Copy data to local
            buffer = operator new(this->size);
            memcpy(buffer, this->data, this->size);

            // Call sem cleanup now, with execution
            pthread_cleanup_pop(1);

            // Notify
            if(this->notifyReadCallbacks(buffer) != 0){
                fprintf(stderr, "Error calling read callbacks.\n");
                success = false;
            }
            operator delete(buffer);
        }else{
            perror("Error waiting on sem ");
            success = false;
        }
        // This is cleanup of read thread mutex
        pthread_cleanup_pop(1);
    }

    if(success){
//        pthread_exit((void*)&SUCCESS_READ_THREAD);
        pthread_exit(NULL);
    }else{
//        pthread_exit((void*)&ERROR_READ_THREAD);
        pthread_exit(NULL);
    }
}

void ControlledSharedMemory::updateSegment(void){
    bool success = true;
    pthread_cleanup_push(launchCleanupUpdateThreadData, (void*) this);

    // All this thread should do is block on the semaphore, then lock the semaphore.
    // Lock the mutex protected data, and write

    // Lock mutex to signal thread is running
    if(pthread_mutex_lock(this->updateThreadMutex) != 0){
        perror("Attempting to lock signal mutex ");
        success = false;
    }else{
        pthread_cleanup_push(launchCleanupUpdateThreadMutex, (void*) this);

        if(success && sem_wait(this->sem) == 0){
            pthread_cleanup_push(launchCleanupSem, (void*) this);

            // Now lock mutex protected data
            if(pthread_mutex_lock(this->updateThreadData->mutex) == 0){
                pthread_cleanup_push(launchCleanupUpdateThreadDataMutex, (void*) this);
                // Copy data
                if(this->updateThreadData->data){
                    memcpy(this->data, this->updateThreadData->data, this->size);
                }else{
                    fprintf(stderr, "Invalid data to copy over.\n");
                    success = false;
                }

                if(success && this->notifyUpdateCallbacks() != 0){
                    fprintf(stderr, "Error notifying update callbacks.\n");
                    success = false;
                }
                // This is the data mutex cleanup
                pthread_cleanup_pop(1);
            }else{
                perror("Attempting to lock mutex protected data in update thread ");
                success = false;
            }
            // This is the sem cleanup
            pthread_cleanup_pop(1);
        }else{
            perror("Attempting to wait on semaphore in update thread ");
            success = false;
        }
        // This is the mutex cleanup
        pthread_cleanup_pop(1);
    }

    // This is the data cleanup
    pthread_cleanup_pop(1);
    if(success){
        //      pthread_exit((void*)&SUCCESS_UPDATE_THREAD);
        pthread_exit(NULL);
    }else{
//        pthread_exit((void*)&ERROR_UPDATE_THREAD);
        pthread_exit(NULL);
    }
}

int ControlledSharedMemory::notifyReadCallbacks(void* in_data){
    pthread_t thread_id;
    for(size_t i = 0; i < this->readCallbacks->size(); ++i){
        void* copy = operator new(this->size);
        memcpy(copy, in_data, this->size);
        if(pthread_create(&thread_id, NULL,
                          this->readCallbacks->at(i), copy) != 0){
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
    // Check initialized
    if(!this->initialized){
        fprintf(stderr, "Shared memory is not initialized.\n");
        return -1;
    }

    // Attempt to lock on sempahore
    int status = sem_trywait(this->sem);
    if(status == 0){
        // Lock was successful, copy locally
        void* buffer = operator new(this->size);
        memcpy(buffer, this->data, this->size);

        // If read thread exists, kill
        status = pthread_mutex_trylock(this->readThreadMutex);

        if(status == EBUSY){
            if(pthread_cancel(*(this->readThreadIdentifer)) != 0){
                perror("Attempting to cancel read thread ");
                return -1;
            }
        }else{
            if(pthread_mutex_unlock(this->readThreadMutex) != 0){
                perror("Attempting to unlock read thread mutex ");
                return -1;
            }
        }

        // Unlock sem
        if(sem_post(this->sem) != 0){
            perror("Error unlocking semaphore ");
            return -1;
        }

        if(this->notifyReadCallbacks(buffer) != 0){
            fprintf(stderr, "Error calling read callbacks.\n");
            operator delete(buffer);
            return -1;
        }
        operator delete(buffer);

        return IMMEDIATE_READ;
    }else if(status == EAGAIN){
        // Busy,
        // Check if thread already exists
        status = pthread_mutex_trylock(this->readThreadMutex);

        if(status == 0){
            // Thread does not exist
            // Create a new thread

            // Unlock read mutex so thread can use
            if(pthread_mutex_unlock(this->readThreadMutex) != 0){
                perror("Attempting to unlock read thread mutex for thread use ");
                return -1;
            }

            if(pthread_create(this->readThreadIdentifer, NULL,
                              launchReadThread, (void*)this) != 0){
                perror("Attempting to create read thread ");
                return -1;
            }
            return SPAWNING_NEW_UPDATE_THREAD;
        }else if(status == EBUSY){// Thread exists
            return OLD_THREAD_RUNNING;
        }else{// Error reading read thread mutex
            perror("Attempting to determine if thread exists ");
            return -1;
        }
    }else{
        // Error on semaphore trywait
        perror("Attemtping to wait on semaphore ");
        return -1;
    }
}

void ControlledSharedMemory::cleanupUpdateThreadMutex(void){
    if(pthread_mutex_unlock(this->updateThreadMutex) != 0){
        perror("In cleanupUpdateThread, unable to unlock update thread mutex signal ");
    }
}

void ControlledSharedMemory::cleanupSem(void){
    if(sem_post(this->sem) != 0){
        perror("In cleanupUpdateThread, unable to post on sem ");
    }
}

void ControlledSharedMemory::cleanupReadThreadMutex(void){
    if(pthread_mutex_unlock(this->readThreadMutex) != 0){
        perror("In cleanupReadThreadMutex, unable to unock read thread mutex signal ");
    }
}

void ControlledSharedMemory::cleanupUpdateThreadDataMutex(void){
    if(pthread_mutex_unlock(this->updateThreadData->mutex) != 0){
        perror("In cleanupUpdateThread, unable to unlock mutex protected data ");
    }
}

void ControlledSharedMemory::cleanupUpdateThreadData(void){
    // Delete all associatedd data containers
    if(this->updateThreadData){
        operator delete(this->updateThreadData->data);

        if(pthread_mutex_destroy(this->updateThreadData->mutex) != 0){
            perror("In cleanupUpdateThread, unable to destroy protected data mutex ");
        }

        delete this->updateThreadData->mutex;
        delete this->updateThreadData;
    }
}

int unitTest(void){


}
