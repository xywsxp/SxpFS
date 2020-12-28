//
// Created by bytedance on 2020/12/5.
//

#ifndef SXPFS_INODEWRAPPER_H
#define SXPFS_INODEWRAPPER_H

#include "headers.h"
#include "inode.h"

struct inodeEngine{
    std::list< size_t > freeNode;
    inode nodeList[FILE_NUM];

    size_t allocNode() {
      size_t nodeId = *freeNode.begin();
      freeNode.erase(freeNode.begin());
      return nodeId;
    }

    void deleteNode(size_t nodeID) {
      freeNode.push_back(nodeID);
      nodeList[nodeID].type = 0;
      nodeList[nodeID].size = 0;
      nodeList[nodeID].blocks = 0;
      memset(nodeList[nodeID].location,0,sizeof(nodeList[nodeID]));
    }

    int Init() {
      int fd = open("inode.dat",O_RDONLY);
      read(fd,nodeList,FILE_NUM * sizeof(inode));
      for(int i = 1 ; i < FILE_NUM ; ++i)
        if(nodeList[i].type == 0)
          freeNode.push_back(i);
      if(fd == -1) return 0;
      close(fd);
      return 1;
    }

    void exit() {
      int fd = open("inode.dat",O_WRONLY|O_CREAT,0777);
      write(fd,nodeList,FILE_NUM * sizeof(inode));
      close(fd);
    }

    inode &getNode(int ID) {
      return nodeList[ID];
    }

    bool isEmpty(int ID) {
      return nodeList[ID].type == 0;
    }
};

#endif //SXPFS_INODEWRAPPER_H
