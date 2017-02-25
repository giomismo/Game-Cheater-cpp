#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;


HANDLE GetProcessHandleStr(wstring window_name){
	HANDLE pHandle = new HANDLE;
	DWORD pid;
	HWND hwnd;
    hwnd = FindWindow(NULL, "Jnes 1.0");
    
	if(!hwnd) {
		cout <<"Window not found!\n";
		cin.get();
		return pHandle;
	}
 	
	GetWindowThreadProcessId(hwnd,&pid);
	return OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,0,pid);
}

HANDLE GetProcessHandle(){
	int pid;
	cout << "Enter PID: ";
	std::cin >> pid;
	return OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,0,pid);
}

vector<long>* ScanMemmory(HANDLE pHandle, int value_to_find){
	vector<long>* addresses = new vector<long>;

    SYSTEM_INFO sysInfo = { 0 };
    GetSystemInfo(&sysInfo);
 
    LONG aStart = (long)sysInfo.lpMinimumApplicationAddress;
    LONG aEnd = (long)sysInfo.lpMaximumApplicationAddress;
    cout << "Start: " << aStart << ". End: " << aEnd << "\n";
    int found = 0;
 	bool runThread = true;
 	char addrHex[20];
    do{
 
        while (aStart < aEnd){
            MEMORY_BASIC_INFORMATION mbi = { 0 };
            if (!VirtualQueryEx(pHandle, (LPCVOID)aStart, &mbi, sizeof(mbi))){
                cout << "Cannot VirtualQueryEx. Error:" << GetLastError() << "\n";
                CloseHandle(pHandle);
                return addresses;         
            }
            
            if (mbi.State == MEM_COMMIT && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect == PAGE_NOACCESS) == 0)){
 
                BOOL isWritable = ((mbi.Protect & PAGE_READWRITE) != 0 || (mbi.Protect & PAGE_WRITECOPY) != 0 || (mbi.Protect & PAGE_EXECUTE_READWRITE) != 0 || (mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0);
                if (isWritable){
 
                    unsigned char *dump = new unsigned char[mbi.RegionSize + 1]();
                    memset(dump, 0x00, mbi.RegionSize + 1);
                    ReadProcessMemory(pHandle, mbi.BaseAddress, dump, mbi.RegionSize, NULL);             
                    for (int x = 0; x < mbi.RegionSize - 4; x += 1){
 						if (*(DWORD*)(dump + x) == value_to_find) {
 							itoa(aStart+x, addrHex, 16);
 							//cout << "Address: 0x"<< addrHex << "\n";
 							addresses->push_back((int)(aStart+x));
 						}                                   
                    }
                    delete[] dump;
                }
            }
            aStart += mbi.RegionSize;
        }
        runThread = false;
    } while (runThread);
 
    if (runThread){     
        CloseHandle(pHandle);         
    }  
    return addresses;
}

vector<long>* ReScanMemmory(int value_to_find, vector<long>* addresses, HANDLE pHandle){
	vector<long>::iterator it2;
	unsigned char *dump = new unsigned char[4]();
	char addrHex[20];
	
	for(it2 = addresses->begin(); it2 != addresses->end();) {
		ReadProcessMemory(pHandle, (LPCVOID)*it2, dump, 4, NULL);
		itoa(*it2, addrHex, 16);
		
   		if(*(DWORD*)dump != value_to_find) {
      		it2 = addresses->erase(it2); 
   		} else {
      		++it2;
		}
	}
}

void PrintVector(vector<long>* addresses, int searched_value){
	cout << "Number of matched addresses: " << addresses->size() << "\n";
	if ((addresses->size() > 0) && (addresses->size() <= 10)){
		vector<long>::iterator it2;
		char addrHex[20];
		for(it2 = addresses->begin(); it2 != addresses->end();) {
			itoa(*it2, addrHex, 16);
			cout << "Address: 0x" << addrHex << "(" << *it2 << "). Value: " << searched_value << "\n";
			++it2;
		}
	} 
}

void int_to_char(int value, char return_char[]){
	return_char[3] = (value & 0xFF000000) >> 24;
	return_char[2] = (value & 0x00FF0000) >> 16;
	return_char[1] = (value & 0x0000FF00) >> 8;
	return_char[0] = value & 0x000000FF;
}

int main() {
	HANDLE pHandle = GetProcessHandle();
	int val;
	char addrHex[20];
	    
	if(!pHandle) {
		cout <<"Could not get handle!\n";
		cin.get();
		return -1;
	}
	cout << "\nEnter next value or -1 to exit: ";
	std::cin >> val;
	if (val == -1) {
		return -1;
	}
		
	vector<long>* addresses = ScanMemmory(pHandle, val);	
	PrintVector(addresses, val);    
    while (true) {
		cout << "\nEnter next value or -1 to exit: ";
		std::cin >> val;
		if (val == -1) {
			break;
		}
    	ReScanMemmory(val, addresses, pHandle);
    	if (addresses->size() == 0){
    		cout << "No more items, exiting.";
    		return -2;
		} else if (addresses->size() == 1) {
			itoa(*addresses->begin(), addrHex, 16);
			cout << "Address found! 0x" << addrHex << "(" << *addresses->begin() << ")";
			break;
		} 
		PrintVector(addresses, val);
	}
	
	cout << "\nEnter value to freeze or -1 to exit: ";
	std::cin >> val;
	if (val == -1) {
		return -1;
	}
	char valBuff[4];
	int_to_char(val, valBuff);

	unsigned char *dump = new unsigned char[4]();
	while(1) {
		ReadProcessMemory(pHandle,(LPCVOID)*addresses->begin(),dump,4,0);
		if (*(DWORD*)dump != val) {
			WriteProcessMemory(pHandle, (LPVOID)*addresses->begin(), &valBuff, 4, NULL);
		}
		Sleep(500);
	}
}
