#include <ControlledSharedMemory.h>

using namespace std;

int main(int argc, char* argv[]){
  ControlledSharedMemory x("/semaphore", "/shared_segment", 10);
  x.initializeMemorySegment();
  x.unlinkMemorySegment();
}

