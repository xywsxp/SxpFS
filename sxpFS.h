//
// Created by bytedance on 2020/12/5.
//

#ifndef SXPFS_SXPFS_H
#define SXPFS_SXPFS_H

#include "utils/headers.h"
#include "utils/disk.h"
#include "utils/inodeEngine.h"

static inodeEngine inodeManager;
static Disk diskManager;

static char buffer[BLOCK_SIZE];
static std::string curPath;
static size_t curDir;

static int tot = 0;
std::map<size_t,std::string> pathMap;
std::map<size_t,size_t> curPos;
std::map<size_t,size_t> opened;
std::map<size_t,size_t> pool;
std::map<std::string,size_t> curMap;

void pull(size_t id,int pos) {
  inode info = inodeManager.getNode(id);
  int num = pos/BLOCK_SIZE;
  diskManager.Read(info.location[num],buffer);
}

void push(size_t id,int pos) {
  inode &info = inodeManager.getNode(id);
  int num = pos/BLOCK_SIZE;
  while(num >= info.blocks) {
    info.location[info.blocks++] = diskManager.alloc();
  }
  diskManager.Write(info.location[num],buffer);
}

void clear(size_t id) {
  memset(buffer,0,BLOCK_SIZE);
  inode &info = inodeManager.getNode(id);
  for(int i = 0 ; i < info.size ; ++i)
    diskManager.remove(info.location[i]);
  memset(info.location,0,sizeof(info.location));
  info.blocks = 0;
  info.size = 0;
}

void modify(size_t id,size_t pos,char *metaData,int nbyte) {
  inode &info = inodeManager.getNode(id);
  while(1) {
    pull(id,pos);
    int curpos = pos % BLOCK_SIZE;
    int res = BLOCK_SIZE - curpos;
    if(res >= nbyte) {
      memcpy(buffer+curpos,metaData,nbyte);
      push(id,pos);
      pos += nbyte;
      break;
    }
    else {
      memcpy(buffer+curpos,metaData,res);
      push(id,pos);
      nbyte -= res;
      pos += res;
    }
  }
  info.size = std::max(pos,info.size);
}

void output(size_t id,int &pos,int nbyte) {
  inode info = inodeManager.getNode(id);
  while(1) {
    pull(id,pos);
    int curpos = pos % BLOCK_SIZE;
    int res = BLOCK_SIZE - curpos;
    if(res >= nbyte) {
      for(int i = 0 ; i < nbyte ; ++i)
        putchar(buffer[curpos+i]);
      pos += nbyte;
      break;
    }
    else {
      for(int i = 0 ; i < res ; ++i)
        putchar(buffer[curpos+i]);
      nbyte -= res;
      pos += res;
    }
  }
}

void append(size_t id,char *metaData,int nbyte) {
  inode &info = inodeManager.getNode(id);
  modify(id,info.size,metaData,nbyte);
}

void addToDir(char *metadata,size_t id) {
  append(id,metadata,DIR_SIZE);
}

void my_mkdir() {
  static char name[100];
  size_t pid = curDir;

  scanf("%s",name);

  size_t id = inodeManager.allocNode();
  inode &info = inodeManager.getNode(id);
  info.type = DIRECTORY_TYPE;

  static char metaData[128];
  memcpy(metaData,name,124);
  memcpy(metaData+124,&id,4);
  addToDir(metaData,pid);

  memset(metaData,0,sizeof(metaData));
  metaData[0] = '.';
  memcpy(metaData+124,&id,4);
  addToDir(metaData,id);

  memset(metaData,0,sizeof(metaData));
  metaData[0] = '.';metaData[1] = '.';
  memcpy(metaData+124,&curDir,4);
  addToDir(metaData,id);
  curMap[std::string(name)] = id;
  pathMap[id] = curPath+"/"+std::string(name);
}

void my_create() {
  static char name[100];
  size_t pid = curDir;

  scanf("%s",name);

  size_t id = inodeManager.allocNode();
  inode &info = inodeManager.getNode(id);
  info.type = FILE_TYPE;

  static char metaData[128];
  memcpy(metaData,name,124);
  memcpy(metaData+124,&id,4);
  addToDir(metaData,pid);
  curMap[std::string(name)] = id;
  pathMap[id] = curPath+std::string(name);
}

void my_ls() {
  size_t id = curDir;

  static char metaData[128];
  static char name[128];
  inode info = inodeManager.getNode(id);
  if(info.type != DIRECTORY_TYPE) {
    std::cerr<<"it's not a directory"<<std::endl;
    return ;
  }
  for(int i = 0 ; i < info.size ; i += BLOCK_SIZE) {
    pull(id,i);
    for(int j = 0 ; j < BLOCK_SIZE ; j += DIR_SIZE) {
      memcpy(metaData,buffer+j,DIR_SIZE);
      memcpy(name,metaData,124);
      int sid;
      memcpy(&sid,metaData+124,4);
      if(sid && !inodeManager.isEmpty(sid))
        printf("%s\n",name);
    }
  }
}

void load_Dir(size_t id) {
  curMap.clear();
  static char metaData[128];
  static char name[128];
  inode info = inodeManager.getNode(id);
  if(info.type != DIRECTORY_TYPE) {
    std::cerr<<"load failed"<<std::endl;
    return ;
  }
  for(int i = 0 ; i < info.size ; i += BLOCK_SIZE) {
    pull(id,i);
    for(int j = 0 ; j < BLOCK_SIZE ; j += DIR_SIZE) {
      memcpy(metaData,buffer+j,DIR_SIZE);
      memcpy(name,metaData,124);
      int sid;
      memcpy(&sid,metaData+124,4);
      if(sid && !inodeManager.isEmpty(sid)) {
        curMap[name] = sid;
        if(name[0] != '.') pathMap[sid] = curPath+"/"+name;
      }
    }
  }
}

void dir_rm(size_t id,char *rawName) {
  static char metaData[128];
  static char name[128];
  inode info = inodeManager.getNode(id);

  for(int i = 0 ; i < info.size ; i += BLOCK_SIZE) {
    pull(id,i);
    int flag = 0;
    for(int j = 0 ; j < BLOCK_SIZE ; j += DIR_SIZE) {
      memcpy(metaData,buffer+j,DIR_SIZE);
      memcpy(name,metaData,124);
      if(name[0] == '.') continue;
      int sid;
      memcpy(&sid,metaData+124,4);
      if(!sid) continue;
      int res = strcmp(name,rawName);
      if(res == 0) {
        memset(buffer+j,0,128);
        flag = 1;
      }
    }
    if(flag == 1) {
      push(id,i);
      break;
    }
  }
}

void my_cd() {
  char rawName[100];
  scanf("%s",rawName);

  std::string name = rawName;
  if(!curMap.count(name)) {
    std::cerr<< "cannot find this file!" << std::endl;
    return ;
  }
  size_t id = curMap[name];
  inode info = inodeManager.getNode(id);
  if(info.type != DIRECTORY_TYPE) {
    std::cerr<< "it is not a directory" << std::endl;
    return ;
  }
  curPath = pathMap[id];
  if(name != ".")
    load_Dir(id);
  curDir = id;
}

void my_rm(){
  char rawName[100];
  scanf("%s",rawName);

  std::string name = rawName;
  if(!curMap.count(name)) {
    std::cerr<< "cannot find this file!" << std::endl;
    return ;
  }
  size_t id = curMap[name];
  inode info = inodeManager.getNode(id);
  if(info.type == DIRECTORY_TYPE) {
    std::cerr<< "it is a directory!" << std::endl;
    return ;
  }
  dir_rm(curDir,rawName);
  inodeManager.deleteNode(id);
}

void my_rmdir() {
  char rawName[100];
  scanf("%s",rawName);

  std::string name = rawName;
  if(!curMap.count(name)) {
    std::cerr<< "cannot find this file!" << std::endl;
    return ;
  }
  size_t id = curMap[name];
  inode info = inodeManager.getNode(id);
  if(info.type != DIRECTORY_TYPE) {
    std::cerr<< "it is not a directory!" << std::endl;
    return ;
  }
  dir_rm(curDir,rawName);
  inodeManager.deleteNode(id);
}

void my_open() {
  char rawName[100];
  scanf("%s",rawName);

  std::string name = rawName;
  if(!curMap.count(name)) {
    std::cerr<< "cannot find this file" << std::endl;
    return ;
  }
  size_t id = curMap[name];
  inode info = inodeManager.getNode(id);
  if(info.type == DIRECTORY_TYPE) {
    std::cerr<< "it is a directory!" << std::endl;
    return ;
  }
  if(opened.count(id)) {
    std::cerr<< "this file is already opened. fd = " << opened[id] << std::endl;
    return ;
  }
  else {
    ++tot;
    opened[id] = tot;
    pool[tot] = id;
    curPos[tot] = 0;
    std::cout << "fd = " << tot << std::endl;
  }
}

void my_close(){
  size_t fd;
  std::cin >> fd;
  if(!pool.count(fd)) {
    std::cout << "fd not exist!" << std::endl;
    return;
  }


  curPos.erase(fd);
  opened.erase(pool[fd]);
  pool.erase(fd);
};

void my_write() {
  size_t fd;
  std::cin >> fd;

  if(!pool.count(fd)) {
    std::cout << "can not find this fd!" << std::endl;
    return ;
  }
  std::cout << "which type do you want to choose? 1. rewrite 2.modify 3.append" << std::endl;
  static char info[1000];
  int type;
  scanf("%d",&type);
  scanf("%s",info);
  int size = strlen(info);
  size_t id = pool[fd];
  if(type == 1) {
    clear(id);
    append(id,info,size);
  }
  else if(type == 2) {
    int pos;
    scanf("%d",&pos);
    modify(id,pos,info,size);
  }
  else {
    append(id,info,size);
  }
}
void my_read() {
  size_t fd,nbyte;
  std::cin >> fd;
  std::cin >> nbyte;

  if(!pool.count(fd)) {
    std::cout << "can not find this fd!" << std::endl;
    return ;
  }
  size_t id = pool[fd];
  int pos = curPos[fd];
  output(id,pos,nbyte);
  curPos[fd] = pos;
}

void my_format() {
  size_t id = inodeManager.allocNode();
  inode &info = inodeManager.getNode(id);
  info.type = DIRECTORY_TYPE;

  static char metaData[128];

  memset(metaData,0,sizeof(metaData));
  metaData[0] = '.';
  memcpy(metaData+124,&id,4);
  addToDir(metaData,id);

  memset(metaData,0,sizeof(metaData));
  metaData[0] = '.';metaData[1] = '.';
  memcpy(metaData+124,&id,4);
  addToDir(metaData,id);

  curPath = "";
  curDir = id;
  pathMap[id] = "";
}

void my_initialize() {
  int flag = 1;
  flag &= inodeManager.Init();
  flag &= diskManager.Init();
  if(!flag) my_format();
  else {
    curPath = "";
    curDir = 1;
    pathMap[1] = "";
    load_Dir(curDir);
  }
}

void my_exit(){
  inodeManager.exit();
  diskManager.exit();
}

#endif //SXPFS_SXPFS_H
