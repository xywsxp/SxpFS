//
// Created by bytedance on 2020/12/5.
//

#ifndef SXPFS_HEADERS_H
#define SXPFS_HEADERS_H

#include <cstdio>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <bits/stdc++.h>
const int FILE_NUM=1024;
const int BLOCK_SIZE=1024;
const int BLOCK_NUM=FILE_NUM*10;
const int DIR_SIZE=128;
const int DIRECTORY_TYPE = 1;
const int FILE_TYPE = 2;
#endif //SXPFS_HEADERS_H
