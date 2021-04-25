#pragma once
#include <bits/stdc++.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

template<class T>
T* shmmap(const char * filename) {
    int fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
    if(fd == -1) {
        std::cerr << "shm_open failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    /*
    struct stat st;
    bool need_construct = false;
    if(fstat(fd, &st) == -1) {
        std::cerr << "fstat failed: " << strerror(errno) << std::endl;
        close(fd);
        return nullptr;
    }
    if(st.st_size != sizeof(T)) {
        */
    if(ftruncate(fd, sizeof(T))) {
        std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
        close(fd);
        return nullptr;
    }
    /*
    need_construct = true;
}
*/
    T* ret = (T*)mmap(0, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if(ret == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    // T's constuctor must be thread safe
    new(ret) T;
    return ret;
}
