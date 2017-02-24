#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;


HANDLE GetProcessHandle(){
	int pid;
	cout << "Enter PID: ";
	std::cin >> pid;
	return OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,0,pid);

}

int main() {
	HANDLE pHandle = GetProcessHandle();
	int val;
	int addr;
	char addrHex[20];
	    
	if(!pHandle) {
		cout <<"Could not get handle!\n";
		cin.get();
		return -1;
	}
	cout << "\nEnter address: ";
	std::cin >> addr;
	if (val == -1) {
		return -1;
	
	}
	unsigned char *dump = new unsigned char[4]();
	char addrBuff[4];
	char valBuff[4] =  {0x16, 0x0, 0x0, 0x00};
	itoa (addr,addrBuff,4); 
	ReadProcessMemory(pHandle, (LPCVOID)addr, dump, 4, NULL);
	cout << "\nCurrent value: " << *(DWORD*)(dump);
	
	WriteProcessMemory(pHandle, (LPVOID)addr, &valBuff, 4, NULL);

	ReadProcessMemory(pHandle, (LPCVOID)addr, dump, 4, NULL);
	cout << "\nNew  value: " << *(DWORD*)(dump);
}
