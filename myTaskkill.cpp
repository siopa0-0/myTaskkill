#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wtsapi32.h>
#pragma comment(lib, "Advapi32.lib")

using namespace std;

void pidKill(DWORD pid, bool forceKill, bool childKill);
void childprocKill(DWORD pid);
void imgKill(const string& imageName, bool forceKill);
void userKill(const string& userName);
bool getProcessUser(HANDLE hProcess, string& userName);

int main(int argc, char* argv[])
{
	//process command line arguments
	if (argc < 3)
	{
		printf("\nUsage: myTaskkill [option] <process> || myTaskkill [option] <process> [option: /f or /t]\nOptions:\n");
		printf("\t%15s", "/PID or /pid");
		printf("\t\tSpecifies the PID of the process to be terminated.\n");
		printf("\t%15s", "/IM or /im");
		printf("\t\tSpecifies the image name of the process to be terminated.\n");
		printf("\t%15s", "/F or /f");
		printf("\t\tSpecifies to forcefully terminate the process.\n");
		printf("\t%15s", "/U or /u");
		printf("\t\tSpecifies the user context under which the command should execute.\n");
		printf("\t%15s", "/T or /t");
		printf("\t\tTerminates the specified process and any child processes which were started by it.\n");
		return 1;
	}

	string flag = argv[1];
	bool forceKill = false;
	bool childKill = false;

	if (flag == "/PID" || flag == "/pid" || flag == "/IM" || flag == "/im")
	{
		string ftFlag = (argc > 3) ? argv[3] : "";
		if (flag == "/PID" || flag == "/pid")
		{
			DWORD pid = atoi(argv[2]);
			if (ftFlag == "/F" || ftFlag == "/f")
			{
				forceKill = true;
			}
			else if (ftFlag == "/T" || ftFlag == "/t")
			{
				childKill = true;
			}
			else if (ftFlag.empty())
			{
			}
			else
			{
				printf("Invalid fourth flag. Please choose between /F or /T\n");
				return 1;
			}

			pidKill(pid, forceKill, childKill);
		}
		else if (flag == "/IM" || flag == "/im")
		{
			string imageName = argv[2];
			if (ftFlag == "/F" || ftFlag == "/f")
			{
				forceKill = true;
			}
			else if (ftFlag.empty())
			{
			}
			else
			{
				printf("Invalid fourth flag. Only /F flag can be used with /IM \n");
				return 1;
			}

			imgKill(imageName, forceKill);
		}
	}
	else if (flag == "/U" || flag == "/u")
	{
		string userName = argv[2];
		userKill(userName);
	}
	else if (flag == "/?")
	{
		printf("\nUsage: myTaskkill [option] <process> || myTaskkill [option] <process> [option: /f or /t]\nOptions:\n");
		printf("\t%15s", "/PID or /pid");
		printf("\t\tSpecifies the PID of the process to be terminated.\n");
		printf("\t%15s", "/IM or /im");
		printf("\t\tSpecifies the image name of the process to be terminated.\n");
		printf("\t%15s", "/F or /f");
		printf("\t\tSpecifies to forcefully terminate the process.\n");
		printf("\t%15s", "/U or /u");
		printf("\t\tSpecifies the user context under which the command should execute.\n");
		printf("\t%15s", "/T or /t");
		printf("\t\tTerminates the specified process and any child processes which were started by it.\n");
		return 1;
	}
	else
	{
		printf("\nInvalid argument.");
		return 1;
	}
	return 0;
}

// Function to terminate processes by PID
void pidKill(DWORD pid, bool forceKill, bool childKill)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (hProcess == NULL)
	{
		printf("Failed to open processes. Error: %lu\n", GetLastError());
		return;
	}
	if (forceKill)
	{
		if (!TerminateProcess(hProcess, 1))
		{
			printf("Forced termination of process unsuccessful. %d\n", GetLastError());
		}
		else
			printf("Force termination of process %lu successful. \n", pid);
	}
	else if (childKill)
	{
		childprocKill(pid);
	}
	else
	{
		if (!TerminateProcess(hProcess, 0))
		{
			printf("Failed to terminate process. Error: %d\n", GetLastError());
		}
		else
		{
			printf("Successfully terminated process %lu.\n", pid);
		}
	}

	CloseHandle(hProcess);
}

// Function to terminate processes and their children processes
void childprocKill(DWORD pid)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create snapshot. Error: %lu\n", GetLastError());
		return;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			if (pe32.th32ParentProcessID == pid)
			{
				pidKill(pe32.th32ProcessID, false, false);
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
	pidKill(pid, false, false);
}

// Function to terminate processes by image name
void imgKill(const string& imageName, bool forceKill)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create snapshot. Error: %lu\n", GetLastError());
		return;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			if (_stricmp(pe32.szExeFile, imageName.c_str()) == 0)
			{
				if (forceKill)
				{
					pidKill(pe32.th32ProcessID, true, false);
				}
				else
				{
					pidKill(pe32.th32ProcessID, false, false);
				}
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);
}

// Function to get process owner (username)
bool getProcessUser(HANDLE hProcess, string& userName)
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
	{
		return false;
	}

	DWORD tokenInfoLength = 0;
	GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenInfoLength);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		CloseHandle(hToken);
		return false;
	}

	PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenInfoLength);
	if (!pTokenUser)
	{
		CloseHandle(hToken);
		return false;
	}

	if (!GetTokenInformation(hToken, TokenUser, pTokenUser, tokenInfoLength, &tokenInfoLength))
	{
		free(pTokenUser);
		CloseHandle(hToken);
		return false;
	}

	SID_NAME_USE sidType;
	char user[256], domain[256];
	DWORD userSize = sizeof(user);
	DWORD domainSize = sizeof(domain);

	if (!LookupAccountSid(NULL, pTokenUser->User.Sid, user, &userSize, domain, &domainSize, &sidType))
	{
		free(pTokenUser);
		CloseHandle(hToken);
		return false;
	}

	userName = string(domain) + "\\" + string(user);

	free(pTokenUser);
	CloseHandle(hToken);
	return true;
}

// Function to terminate processes belonging to a specific user
void userKill(const string& userName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create snapshot. Error: %lu\n", GetLastError());
		return;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
			if (hProcess != NULL)
			{
				string procUser;
				if (getProcessUser(hProcess, procUser) && _stricmp(procUser.c_str(), userName.c_str()) == 0)
				{
					printf("Successfully terminated process %lu belonging to %s\n", pe32.th32ProcessID, userName.c_str());
					TerminateProcess(hProcess, 0);
				}
				CloseHandle(hProcess);
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);

}

//.\PsExec -s -i -d cmd.exe