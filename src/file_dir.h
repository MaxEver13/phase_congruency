/*
 * @Descripttion: 
 * @version: 
 * @Author: Jiawen Ji
 * @Date: 2022-01-18 11:34:30
 * @LastEditors: Jiawen Ji
 * @LastEditTime: 2022-01-18 11:45:08
 */

#ifndef __FILEDIR_H__
#define __FILEDIR_H__

#include <string>
#include <fstream>
#include <vector>

int GetExecuteDirPath(char* dir, int len);			//执行文件目录



													//int GetConfigDirPath(char* dir, int len);			//获取配置文件目录
													//int GetDataDirPath(char* dir, int len);			//获取数据文件目录
													//int GetRecordFilePath(char* dir, int len);			//获取清扫记录的文件目录

bool FileDirectoryExists(std::string path);			//文件是否存在
int GetDirectoryFileCnt(const char * dir);
bool CreateFileDirectory(std::string file_directory);	//创建文件夹


int WriteStrToFile(std::string fileName, const char* buf, int len);//写数据到文件
int AppendStrToFile(std::string fileName, const char* buf, int len);//追加数据到文件
int ReadStrFromFile(std::string fileName, char* buf, int len);//读文件到本地

															  //std::string GetSettingFileAbsName(const char* settingFileName);
long long int GetDirectorySize(const char *dir);

int GetFileLen(std::ifstream& fin);
int GetFileSize(std::string fileName);
void RemoveFilesBesideFolder(std::string folder);
void RemoveAllFiles(std::string folder);

bool CreateEmptyFile(std::string name);

int GetRegFileList(std::string folder, std::vector<std::string> &files);
int GetRegFileListWaitoutDots(std::string folder, std::vector<std::string>& files);
std::string SplitNameFromFullPath(std::string fullPath);
std::string GetFileSuffix(const char *filename);

#endif
