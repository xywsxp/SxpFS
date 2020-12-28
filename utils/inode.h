//
// Created by bytedance on 2020/12/5.
//

#ifndef SXPFS_INODE_H
#define SXPFS_INODE_H

#include "headers.h"
#include "disk.h"

struct inode{
    size_t id;
    size_t size;
    size_t blocks;
    size_t type;
    size_t location[28];
};

#endif //SXPFS_INODE_H
