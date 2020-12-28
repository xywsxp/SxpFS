//
// Created by bytedance on 2020/12/5.
//

#ifndef SXPFS_DISK_H
#define SXPFS_DISK_H

#include "headers.h"

struct Block{
  char rawData[BLOCK_SIZE];
  void read(char *metaData) {
    memcpy(metaData,rawData,sizeof(char) * BLOCK_SIZE);
  }
  void write(char *metaData) {
    memcpy(rawData,metaData,sizeof(char) * BLOCK_SIZE);
  }
  void clean() {
    memset(rawData,0,sizeof(char) * BLOCK_SIZE);
  }
  bool isempty() {
    for(int i = 0 ; i < BLOCK_SIZE ; ++i)
      if(rawData[i] != 0)
        return 0;
    return 1;
  }
};

struct Disk{
  Block data[BLOCK_NUM];
  std::list< size_t > freeBlock;
  int alloc() {
    size_t bid = *freeBlock.begin();
    freeBlock.pop_front();
    return bid;
  }

  void remove(int bid) {
    data[bid].clean();
    freeBlock.push_back(bid);
  }

  void Read(int bid,char *metaData) {
    data[bid].read(metaData);
  }
  void Write(int bid,char *metaData) {
    data[bid].write(metaData);
  }
  void Clear(int bid) {
    data[bid].clean();
  }

  int Init() {
    int fd = open("data.dat",O_RDONLY);
    read(fd,data,sizeof(Block)*BLOCK_NUM);
    for(int i = 1 ; i < BLOCK_NUM ; ++i)
      if(data[i].isempty())
        freeBlock.push_back(i);
    if(fd == -1) return 0;
    close(fd);
    return 1;
  }

  void exit() {
    int fd = open("data.dat",O_WRONLY|O_CREAT,0777);
    write(fd,data,sizeof(Block)*BLOCK_NUM);
    close(fd);
  }
};

#endif //SXPFS_DISK_H
