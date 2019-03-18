#ifndef PLATFORM_UTIL_H
#define PLATFORM_UTIL_H

#include <time.h>
#include <vector>
#include <set>
#include <string>

time_t fileLastModifiedTime(const char* file);
time_t fileAge(const char* file);
off_t fileSize(const char* file);

void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories);
void getDirectoryListingR(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories);

std::vector<std::string> getLocalIPs();

double iclock();

bool md5_fromfile(const char* filename, unsigned char md5sum[16]);
void md5_frombuffer(const unsigned char *buffer,unsigned int size, unsigned char md5sum[16]);
void aes_encrypt(unsigned char *buffer,size_t size,const unsigned char *key,size_t klen,const unsigned char *iv);
void aes_decrypt(unsigned char *buffer,size_t size,const unsigned char *key,size_t klen,const unsigned char *iv);

#endif
