#include "file_dir.h"
// #include "utils.h"

#ifdef _WINDOWS
#include<windows.h>
#include <io.h>
#else
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#endif

#include <stdlib.h>
#include <fstream>

// #include "utils/string_utils.h"
using namespace std;

bool  CreateFileDirectory(std::string dir_name)
{
#if _WINDOWS
    ::CreateDirectory(dir_name.c_str(), NULL);
#else
    mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return true;
}

bool CreateEmptyFile(std::string name)
{
#ifdef _WINDOWS
    //TODO: 
#else
    std::string cmd = "touch " + name;
    system(cmd.c_str());
#endif
    return true;
}


int WriteStrToFile(std::string fileName, const char * buf, int len)
{
    std::ofstream ofs;
    ofs.open(fileName.c_str(), std::ios_base::out | ios::binary);
    if (ofs.is_open()) {
        ofs.write(buf, len);
        ofs.flush();
        ofs.close();
        return len;
    } else {
        return -1;
    }
}

int AppendStrToFile(std::string fileName, const char * buf, int len)
{
    std::ofstream ofs;
    ofs.open(fileName.c_str(), std::ios_base::out | ios::binary | ios::app);
    if (ofs.is_open()) {
        ofs.write(buf, len);
        ofs.flush();
        ofs.close();
        return len;
    } else {
        return -1;
    }
}
int ReadStrFromFile(std::string fileName, char * buf, int len)
{
    ifstream fin;
    fin.open(fileName.c_str(), ios_base::in | ios_base::binary);
    if (fin.fail()) {
        printf("open %s fail !\n", fileName.c_str());
        return -1;
    }

    fin.read(buf, len - 1);
    int size = fin.gcount();
    if (size > 0) {
        buf[size] = 0;
    }
    fin.close();
    //printf("read size=%d\n",size);
    return size;
}

long long int GetDirectorySize(const char * dir)
#ifndef _WINDOWS
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    long long int totalSize = 0;

    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "Cannot open dir: %s\n", dir);
        return -1;
    }

    lstat(dir, &statbuf);
    totalSize += statbuf.st_size;

    while ((entry = readdir(dp)) != NULL) {
        char subdir[256];
        sprintf(subdir, "%s/%s", dir, entry->d_name);
        lstat(subdir, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0) {
                continue;
            }

            long long int subDirSize = GetDirectorySize(subdir);
            totalSize += subDirSize;
        } else {
            totalSize += statbuf.st_size;
        }
    }

    closedir(dp);
    return totalSize;
}
#else
{
    WIN32_FIND_DATA data;
    long long int size = 0;
    string fname = dir;
    fname += "\\*.*";
    HANDLE h = FindFirstFile(fname.c_str(), &data);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                LARGE_INTEGER sz;
                sz.LowPart = data.nFileSizeLow;
                sz.HighPart = data.nFileSizeHigh;
                size += sz.QuadPart;
            }
        } while (FindNextFile(h, &data) != 0);
        FindClose(h);
    }
    return size;
}
#endif

bool FileDirectoryExists(std::string path)
{
#if _WINDOWS
    return 0 == _access(path.c_str(), 0x00);
#else
    return 0 == access(path.c_str(), 0x00);
#endif
}

int GetDirectoryFileCnt(const char * dir)
{
    int cnt = 0;
#ifndef _WINDOWS
    DIR *dp;
    struct dirent *entry;

    if ((dp = opendir(dir)) == NULL) {
        printf("Cannot open dir: %s", dir);
        return -1;
    }
    while ((entry = readdir(dp)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        cnt++;
    }
    closedir(dp);

#endif
    return cnt;
}

#define TMP_SAVE
int GetExecuteDirPath(char * dir, int len)
{
#if _WINDOWS
    int n = GetModuleFileName(NULL, dir, len);
    while (dir[n] != '\\')
        n--;
#else
    int n = 0;
#ifndef TMP_SAVE
    n = readlink("/proc/self/exe", dir, len);
    while (dir[n] != '/')
        n--;
#else
    strcpy(dir, "/tmp/");
    n = 4;
#endif

#endif

    dir[n + 1] = 0;
    return n + 1;
}

int GetFileLen(std::ifstream& fin)
{
    streampos pos = fin.tellg();
    fin.seekg(0, ios::end);
    int fileLen = fin.tellg();
    fin.seekg(pos);
    return fileLen;
}

int GetFileSize(std::string fileName)
{
    ifstream fin;
    fin.open(fileName.c_str(), ios_base::in | ios_base::binary);
    if (fin.fail()) {
        printf("open %s fail !\n", fileName.c_str());
        return -1;
    }

    int size = GetFileLen(fin);
    fin.close();
    return size;
}

void RemoveFilesBesideFolder(std::string folder)
{
#if !defined(_WINDOWS)
    DIR* dir;
    struct dirent* ptr;
    char filepath[256];

    if ((dir = opendir(folder.c_str())) == nullptr) {
        return;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr->d_type == DT_DIR) {
            continue;
        } else {
            // build the path for each file in the folder
            sprintf(filepath, "%s/%s", folder.c_str(), ptr->d_name);
            remove(filepath);
        }
    }

    closedir(dir);
#endif
}

void RemoveAllFiles(std::string folder)
{
#if !defined(_WINDOWS)
    DIR* dir;
    struct dirent* ptr;
    char filepath[256];

    if ((dir = opendir(folder.c_str())) == nullptr) {
        return;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else {
            // build the path for each file in the folder
            sprintf(filepath, "%s/%s", folder.c_str(), ptr->d_name);
            remove(filepath);
        }
    }
    
    closedir(dir);
#endif
}

int GetRegFileList(std::string folder, std::vector<std::string>& files)
{
#if !defined(_WINDOWS)
    DIR *dir;
    struct dirent *ptr;
    
    if ((dir=opendir(folder.c_str())) == nullptr) {
        printf("Open folder %s failed!!", folder.c_str());
        return -1;
    }

    // clear file list first
    files.clear();
    
    // get all reg files
    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr->d_type == DT_REG) {
            files.push_back(ptr->d_name);
        }
    }

    closedir(dir);
#endif
    return files.size();
}

int GetRegFileListWaitoutDots(std::string folder, std::vector<std::string>& files)
{
#if !defined(_WINDOWS)
    DIR *dir;
    struct dirent *ptr;
    
    if ((dir=opendir(folder.c_str())) == nullptr) {
        printf("Open folder %s failed!!", folder.c_str());
        return -1;
    }

    // clear file list first
    files.clear();
    
    // get all reg files
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0  && strcmp(ptr->d_name, "..") != 0) {
            printf("get file %s when scan %s", ptr->d_name, folder.c_str());
            files.push_back(ptr->d_name);
        }
    }

    closedir(dir);
#endif
    return files.size();
}

std::string SplitNameFromFullPath(std::string fullPath)
{
#if _WINDOWS
    int pos = fullPath.find_last_of("\\");
#else
    int pos = fullPath.find_last_of("/");
#endif
    return fullPath.substr(pos+1);
}

std::string GetFileSuffix(const char *filename)
{
    std::string filenameStr(filename);

    return filenameStr.substr(filenameStr.find_last_of("."));
}
