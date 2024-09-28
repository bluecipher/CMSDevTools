#define _WIN32_WINNT 0x0501  // Windows xp
#define MAX_BUFFER_SIZE 1024
#include <SDKDDKVer.h>
#include <afxwin.h> // ���� MFC ͷ�ļ�
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
//#include <windows.h>

// �ļ���Ϣ�ṹ��
struct FileInfo {
	DWORD fileSizeLow;
	DWORD fileSizeHigh;
	ULONGLONG fileSize;
	FILETIME lastWriteTime;
	FILETIME lastAccessTime;
	FILETIME lastCreateTime;
	bool isFile = 0 ;
	std::string strCreateAccTime;
	std::string strLastWriteTime;
	std::string strLastAccTime;
	FileInfo() {}
	FileInfo(FileInfo& fileInfo)
	{
		fileSize = fileInfo.fileSize;
		fileSizeLow = fileInfo.fileSizeLow;
		fileSizeHigh = fileInfo.fileSizeHigh;
		lastWriteTime = fileInfo.lastWriteTime;
		lastAccessTime = fileInfo.lastAccessTime;
		lastCreateTime = fileInfo.lastCreateTime;
		strCreateAccTime = fileInfo.strCreateAccTime;
		strLastWriteTime = fileInfo.strLastWriteTime;
		strLastAccTime = fileInfo.strLastAccTime;
	}
	// �������������
	friend std::ostream& operator<<(std::ostream& os, const FileInfo& info) {
		// ����ļ���С
		os << "�ļ���С: " << (info.fileSizeHigh<<16 ) + info.fileSizeLow << " �ֽ�\n";
		os << "�ļ���СfileSize: " << info.fileSize  << " �ֽ�\n";

		// ���ʱ����Ϣ
		os << "���д��ʱ��: " << info.strLastWriteTime << "\n";
		os << "������ʱ��: " << info.strLastAccTime << "\n";
		os << "����ʱ��: " << info.strCreateAccTime << "\n";

		return os;
	}
};
// �� FILETIME ת��Ϊ�ɶ���ʽ
std::string ConvertFileTimeToString(const FILETIME& ft) {
	SYSTEMTIME st;
	FILETIME localFt;
	FileTimeToLocalFileTime(&ft, &localFt);// ע��ʱ�������ʱ����
	FileTimeToSystemTime(&localFt, &st);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return buffer;
}

// ��ȡ�ļ���Ϣ
bool GetFileInfo(const std::string& path, FileInfo& fileInfo) {
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes;

	if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileAttributes)) {
		// �����ļ���С
		ULARGE_INTEGER fileSize;
		fileSize.HighPart = fileAttributes.nFileSizeHigh;
		fileSize.LowPart = fileAttributes.nFileSizeLow;
		fileInfo.fileSize = fileSize.QuadPart; //�Դ�СС��4G��׼ȷ((fileSize.HighPart * (MAXDWORD + 1)) + fileSize.LowPart);
		fileInfo.fileSizeLow = fileSize.LowPart; 
		fileInfo.fileSizeHigh = fileSize.HighPart;
 
		//��ȡʱ����Ϣ
		fileInfo.lastWriteTime = fileAttributes.ftLastWriteTime;
		fileInfo.lastAccessTime = fileAttributes.ftLastAccessTime;
		fileInfo.lastCreateTime = fileAttributes.ftCreationTime;

		//ת��ʱ��Ϊ�ַ�����ʽ
		fileInfo.strLastWriteTime = ConvertFileTimeToString(fileInfo.lastWriteTime);
		fileInfo.strLastAccTime = ConvertFileTimeToString(fileInfo.lastAccessTime);
		fileInfo.strCreateAccTime = ConvertFileTimeToString(fileInfo.lastCreateTime);
		return true;  
	}

	return false;  
}

// ���ڴ洢·�����ļ���Ϣ��ӳ��
std::map<std::string, FileInfo> localFileMap;
std::map<std::string, FileInfo> remoteFileMap;

std::wstring StringToWString_ACP(const std::string& str)
{
	int acp = GetACP(); // ��ȡ��ǰϵͳ�Ļ����ҳ
	int wStrLength = MultiByteToWideChar(acp, 0, str.c_str(), -1, NULL, 0);
	std::wstring wStr(wStrLength, L'\0');
	MultiByteToWideChar(acp, 0, str.c_str(), -1, &wStr[0], wStrLength);
	return wStr;
}

std::string  WStringToString_ACP(const std::wstring& wstr)
{
	int acp = GetACP(); // ��ȡ��ǰϵͳ�Ļ����ҳ
	int mbsLength = WideCharToMultiByte(acp, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string str(mbsLength, '\0');
	WideCharToMultiByte(acp, 0, wstr.c_str(), -1, &str[0], mbsLength, NULL, NULL);
	return str;
}

bool StringReplace(std::string& in, const std::string& search, const std::string& replacement)
{
	bool bRet = false;
	std::string::size_type pos = 0;
	while (pos < in.length() && (pos = in.find(search, pos)) != std::string::npos)
	{
		bRet = true;
		in.replace(pos, search.length(), replacement);
		pos += replacement.length();
	}
	return bRet;
};

// ���ڴ洢INI�ļ������ݣ�section -> (key -> value)
std::map<std::string, std::map<std::string, std::string>> iniData;


boolean GetIniFile(const std::string& iniFilePath, std::string& remotePath, std::string& localPath) {
	char buffer[MAX_PATH];
	GetPrivateProfileString("Paths", "RemotePath", "", buffer, sizeof(buffer), iniFilePath.c_str());
	remotePath = buffer;
	GetPrivateProfileString("Paths", "LocalPath", "", buffer, sizeof(buffer), iniFilePath.c_str());
	localPath = buffer;
	return !remotePath.empty() && !localPath.empty();
}

// ��ȡINI�ļ�
bool ReadIniFile(const std::string& iniFilePath, std::string& remotePath, std::string& localPath) {
	char buffer[MAX_PATH];
	GetPrivateProfileString("Paths", "RemotePath", "", buffer, sizeof(buffer), iniFilePath.c_str());
	remotePath = buffer;
	GetPrivateProfileString("Paths", "LocalPath", "", buffer, sizeof(buffer), iniFilePath.c_str());
	localPath = buffer;
	return !remotePath.empty() && !localPath.empty();
}

// ��������ȡ���� section
std::vector<std::string> GetSections(const std::string& filePath) {
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	std::vector<std::string> sections;

	// ʹ�� GetPrivateProfileSectionNames ��ȡ���е� section ����
	DWORD charsRead = GetPrivateProfileSectionNames(buffer, MAX_BUFFER_SIZE, filePath.c_str());
	if (charsRead > 0) {
		char* section = buffer;
		while (*section) {
			sections.push_back(section);
			section += strlen(section) + 1;
		}
	}
	return sections;
}

// ��������ȡĳ�� section �е����м�ֵ��
std::map<std::string, std::string> GetSectionData(const std::string& section, const std::string& filePath) {
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	std::map<std::string, std::string> keyValues;

	// ʹ�� GetPrivateProfileSection ��ȡָ�� section �����м�ֵ��
	DWORD charsRead = GetPrivateProfileSection(section.c_str(), buffer, MAX_BUFFER_SIZE, filePath.c_str());
	if (charsRead > 0) {
		char* keyValue = buffer;
		while (*keyValue) {
			std::string pair = keyValue;
			size_t equalPos = pair.find('=');
			if (equalPos != std::string::npos) {
				std::string key = pair.substr(0, equalPos);
				std::string value = pair.substr(equalPos + 1);
				keyValues[key] = value;
			}
			keyValue += strlen(keyValue) + 1;
		}
	}
	return keyValues;
}

// ��������INI�ļ��е�����section�ͼ�ֵ�Դ洢��map��
void LoadIniFile(const std::string& filePath) {
	std::vector<std::string> sections = GetSections(filePath);
	for (size_t i = 0; i < sections.size(); ++i) {
		iniData[sections[i]] = GetSectionData(sections[i], filePath);
	}
}

// ���Ժ�������ӡ������section������ֵ
void PrintIniData() {
	for (std::map<std::string, std::map<std::string, std::string> >::const_iterator it = iniData.begin(); it != iniData.end(); ++it) {
		std::cout << "[" << it->first << "]" << std::endl;
		const std::map<std::string, std::string>& sectionData = it->second;
		for (std::map<std::string, std::string>::const_iterator keyValIt = sectionData.begin(); keyValIt != sectionData.end(); ++keyValIt) {
			std::cout << keyValIt->first << "=" << keyValIt->second << std::endl;
		}
		std::cout << std::endl;
	}
}
std::wstring GetCurrentWorkingDirectory() {
#ifdef UNICODE
	TCHAR currentPath[MAX_PATH]; // ���ڴ洢��ǰ·��
	// ��ȡ��ǰ����Ŀ¼
	DWORD length = GetCurrentDirectoryW(MAX_PATH, currentPath);
	if (length > 0 && length < MAX_PATH) {
		return std::wstring(currentPath); // ���ص�ǰ·��
	}
	else {
		return _T(""); // ʧ��ʱ���ؿ��ַ���
	}
#else
	char currentPath[MAX_PATH]; // ���ڴ洢��ǰ·��
// ��ȡ��ǰ����Ŀ¼
	DWORD length = GetCurrentDirectory(MAX_PATH, currentPath);
	if (length > 0 && length < MAX_PATH) {
		return StringToWString_ACP(currentPath); // ���ص�ǰ·��
	}
	else {
		return L""; // ʧ��ʱ���ؿ��ַ���
	}
#endif
}

bool IsFile(const std::wstring& path) {
	DWORD attributes = GetFileAttributesW(path.c_str());  
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		return false; // ·����Ч
	}
	return !(attributes & FILE_ATTRIBUTE_DIRECTORY); // �������Ŀ¼����Ϊ�ļ�
}

bool IsDirectoryExist(const std::string& absPathDir) {
	DWORD attributes = GetFileAttributes(absPathDir.c_str());
	return (attributes != INVALID_FILE_ATTRIBUTES &&
		(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool IsAbsPathFoDExists(const std::string& absPath) {
	DWORD dwFileAttr = GetFileAttributes(absPath.c_str());
	if (dwFileAttr != INVALID_FILE_ATTRIBUTES) {
		return true;  
	}
	else {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND ||
			lastError == ERROR_PATH_NOT_FOUND ||
			lastError == ERROR_INVALID_DRIVE) 
		{
			return false;  
		}
		else 
		{
			std::cerr << "GetFileAttributes error: " << lastError << std::endl;
			return false;  
		}
	}
	return  true;
}

bool CheckAndCreateDirectory(const std::string& strAbsPath) {
	if (IsDirectoryExist(strAbsPath))
	{
		return true;
	}
	else
	{
		if (CreateDirectoryA(strAbsPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
			std::cout << "Ŀ¼�Ѵ������Ѵ���: " << strAbsPath << std::endl;
			return true;
		}
		else {
			std::cerr << "�޷�����Ŀ¼: " << strAbsPath << " �������: " << GetLastError() << std::endl;
			return false;
		}
	}
}



bool CreateDirectoryRecursive(const std::string& strAbsPath) {
	if (IsDirectoryExist(strAbsPath))
	{
		return true;
	}
	else
	{
		// �ҵ����һ����б�ܵ�λ��
		size_t pos = strAbsPath.find_last_of("\\/");
		if (pos != std::string::npos) {
			std::string strAbsParentPath = strAbsPath.substr(0, pos); 
			if (!CreateDirectoryRecursive(strAbsParentPath)) {
				return false;  
			}
			return CheckAndCreateDirectory(strAbsPath);
		}
	}
	return true; 
}


// �ݹ����Ŀ¼���洢�ļ���Ϣ
void ListDirAndFiles(const std::string& strRootDir, const std::string& strRelPath, std::map<std::string, FileInfo>& fileMap) {
	std::string strAbsPath = strRootDir;

	if (strAbsPath.rfind("\\") == std::string::npos)
	{
	strAbsPath = strAbsPath + "\\";
	}
	std::string strParentPath = strAbsPath;
	strAbsPath = strAbsPath + strRelPath;

	if (IsFile(StringToWString_ACP(strAbsPath)))
	{
		FileInfo fileInfo;
		fileInfo.isFile = 1;
		GetFileInfo(strAbsPath, fileInfo);
		fileMap[strAbsPath] = fileInfo;
		return;
	}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((strAbsPath + "\\*").c_str(), &findFileData);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			const std::string fileOrDir = findFileData.cFileName;
			if (fileOrDir != "." && fileOrDir != "..") {
				const std::string strNewRelPath = strRelPath + "\\" + fileOrDir;
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

					ListDirAndFiles(strRootDir, strNewRelPath, fileMap);  //recursion 
				}
				else
				{
					FileInfo fileInfo;
					fileInfo.isFile = 1;
					std::string strNewAbsPath = strRootDir + strNewRelPath;
					GetFileInfo(strNewAbsPath, fileInfo);
					fileMap[strNewAbsPath] = fileInfo;  // <���·��,�ļ���Ϣ>
				}
			}
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
	}
}

bool CopyFileFromAbsPath(std::string absPathFrom , std::string absPathTo)
{
	return true;
}
std::string GetParentDirectory(const std::string& path) {
	char buffer[MAX_PATH] = { 0 };
	strncpy_s(buffer, path.c_str(), MAX_PATH); 
	PathRemoveFileSpec(buffer);
	return std::string(buffer);
}
bool SynDirFilesByList(const std::string& strRootDir1, const std::map<std::string, FileInfo>& mpFiles1, const std::string& strRootDir2, const std::map<std::string, FileInfo>& mpFiles2)
{
	std::string strAbsPath1 = strRootDir1;
	if (strAbsPath1.rfind("\\") == std::string::npos)
	{
		strAbsPath1 = strAbsPath1 + "\\";
	}

	std::string strAbsPath2 = strRootDir2;
	if (strAbsPath1.rfind("\\") == std::string::npos)
	{
		strAbsPath2 = strAbsPath2 + "\\";
	}
	if (!CreateDirectoryRecursive(strAbsPath2))
	{
		return false;
	}
	std::string strKeyAbsPath = "";
	FileInfo fiOneValue;
	std::string strKeyAbsPath2 = "";
	FileInfo fiOneValue2;
	for (std::map<std::string, FileInfo>::const_iterator itA = mpFiles1.begin(); itA != mpFiles1.end(); itA++)
	{
		strKeyAbsPath = itA->first;
		fiOneValue = itA->second;

		std::string strAbsPathx(strKeyAbsPath);
		StringReplace(strAbsPathx, strAbsPath1, "");
		strAbsPathx = strAbsPath2 + strAbsPathx;

		if (fiOneValue.isFile)
		{
			std::map<std::string, FileInfo>::const_iterator itB = mpFiles2.find(strAbsPathx);
			if (itB == mpFiles2.end() 
				|| CompareFileTime(&fiOneValue.lastWriteTime, &itB->second.lastWriteTime) > 0 //�޸�ʱ��仯 
				|| fiOneValue.fileSize != itB->second.fileSize  //��С�仯
				
			) 
			{
				std::string strAbsPathParent = GetParentDirectory(strAbsPathx); //transformed absPath
				if (!IsAbsPathFoDExists(strAbsPathParent))
				{
					CreateDirectoryRecursive(strAbsPathParent);
				}
				
				CopyFile(strKeyAbsPath.c_str(), strAbsPathx.c_str(), FALSE);
			}
		}
	}
	return true;
}

int main(int argc, char* argv[]) {
	// ��ʼ�� MFC
	//AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);
	//theApp.InitInstance();
	//return theApp.Run(); // ��������Ϣѭ��

	std::string strPwd = WStringToString_ACP(GetCurrentWorkingDirectory());
	std::cout << strPwd << std::endl;
	//std::cout << "mark--->" << std::endl;
	//std::string strTmp = strPwd;

	std::string iniFilePath = strPwd.substr(0, strlen(strPwd.c_str())) + _T("\\Config.ini"); // ָ��INI�ļ�·��
	iniFilePath = "D:\\F_�����������\\ϵͳ��װ���\\Win10SSD_1EFI(ESP)_2MSR_3System_4RecoveryBak.GHO";
	iniFilePath = "D:\\F_�����������\\ϵͳ��װ���\\С����Ϸ�� 15.6 (��8��CPUƽ̨����)_�������ϰ�.zip";

	std::string absPath("D:\\E_RunDB\\08222.txt.rtsrc.cyc.de");
	std::cout << iniFilePath << std::endl;
	//LoadIniFile(iniFilePath);
	//PrintIniData();
	FileInfo fileInfo;
	if (GetFileInfo(absPath, fileInfo))
	{
		std::cout << fileInfo << std::endl;
	}
	iniFilePath = strPwd.substr(0, strlen(strPwd.c_str())) + _T("\\Config.ini");
	std::string strRootDir1 = "D:\\E_RunDB\\08222.txt.rtsrc.cyc.de";
	std::map<std::string, FileInfo>  mpFiles1;
	std::string strRootDir2 = "D:\\E_RunDB\\0929";
	std::map<std::string, FileInfo>  mpFiles2;
    ListDirAndFiles(absPath,"", mpFiles1);
	ListDirAndFiles(strRootDir2, "", mpFiles2);

	SynDirFilesByList(strRootDir1, mpFiles1, strRootDir2, mpFiles2);
	/* 
	//bool bFlag = CreateDirectoryRecursive("D:\\E_RunDB\\0923\\�½��ļ���\\result\\aa\\bb\\cc\\������"); 
	bool bFlag = IsAbsPathFoDExists("D:\\E_RunDB\\0923\\�½��ļ���\\result\\aa\\bb\\cc\\������");
	bool bFlag2 = IsAbsPathFoDExists("D:\\666");
	bool bFlag3 = IsAbsPathFoDExists("D:\\D_����ͼƬ\\380.jpg");
	bool bFlag4 = IsAbsPathFoDExists("D:\\E_RunDB\\0928");
	bool bFlag5 = IsAbsPathFoDExists("D:\\E_RunDB\\0928\\abc.txt");
	bool bFlag6 = IsAbsPathFoDExists("D:\\7777.bbb");
	if (bFlag)
	{
		std::cout << "IsFileExists--->OK " << std::endl;
	}
	else {
		std::cout << "IsFileExists--->failed " << std::endl;
	}
	*/

   
	//std::string strIn(_T("caomingshi"));
	 
	//StringReplace(strIn , "ming", "");
	//StringReplace(strIn , "ming", "xx");
	//std::cout << "--->" << strIn << "<---" << std::endl;
	
	return 0;
}



class CMyApp2 : public CWinApp {
public:
	BOOL InitInstance();
};

class CFrameWnd3 :public CFrameWnd
{
public:
	CFrameWnd3() {
		Create(NULL, _T("My MFC SimpleShortMain")); // ��������
	}
};
BOOL CMyApp2::InitInstance() {
	// ����������
	CFrameWnd3* pFrame = new CFrameWnd3;
	m_pMainWnd = pFrame;
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE; // ���� TRUE����ʾ��ʼ���ɹ�

}
CMyApp2 theApp; // һ��Ҫ����ȫ��





