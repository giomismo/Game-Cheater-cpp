#include <windows.h>
#include <iostream>

using namespace std;

int main()
{
	DWORD address = 0x46eabc;
	int value = 0;
	DWORD pid;
	HWND hwnd;
        hwnd = FindWindow(NULL,"Jnes 1.0");
	if(!hwnd) {
		cout <<"Window not found!\n";
		cin.get();
		return 1;
	}
	
	GetWindowThreadProcessId(hwnd,&pid);
	HANDLE phandle = OpenProcess(PROCESS_VM_READ,0,pid);
	if(!phandle) {
		cout <<"Could not get handle!\n";
		cin.get();
		return 2;
	}	
		
	while(1) {
		ReadProcessMemory(phandle,(void*)address,&value,sizeof(value),0);
		cout << value << "\n";
		Sleep(1000);
	}
	
	return 0;
}
