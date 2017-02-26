#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;

/*
* Constants
*/

const short HANDLE_MODE_ASK = 0;
const int HANDLE_MODE_PROCESS_ID = 1;
const int HANDLE_MODE_PROCESS_NAME = 2;
const int HANDLE_MODE_WINDOW_TITLE = 3;
const int OPERATION_MODE_ASK = 0;
const int OPERATION_MODE_SET_ONCE = 1;
const int OPERATION_MODE_SET_FREEZE = 2;

/*
* Structs
*/

struct arguments {
	int handle_mode = HANDLE_MODE_ASK;
	string process_name;
	string window_title;
	unsigned int process_id;
	vector<long>* addresses = new vector<long>;
	unsigned int number_of_addresses_to_find = 1;
	boolean value_set = false;
	int value_to_set;
	int operation_mode = OPERATION_MODE_ASK;
	int sleep_time_ms = 500;  
} parsed_arguments;

/*
* Type conversions
*/

void int_to_char(int value, char return_char[]){
	return_char[3] = (value & 0xFF000000) >> 24;
	return_char[2] = (value & 0x00FF0000) >> 16;
	return_char[1] = (value & 0x0000FF00) >> 8;
	return_char[0] = value & 0x000000FF;
}

unsigned long strnumber_to_ulong(char* strnumber) {
	if (strlen(strnumber) <= 2) {
		return strtoul(strnumber, NULL, 10);
	}
	string first_characters = string(strnumber).substr(0,2);
	if (first_characters.compare(string("0x")) == 0 || first_characters.compare(string("0X")) == 0) {
			return strtoul(strnumber, NULL, 16);	
	}
	return strtoul(strnumber, NULL, 10);
}

unsigned int strnumber_to_uint(char* strnumber) {
	unsigned long long_number = strnumber_to_ulong(strnumber);
	if (long_number > UINT_MAX){
		return INT_MAX;
	} else if (long_number < 0){
		return 0;
	}
	return (int) long_number;
}

long strnumber_to_long(char* strnumber) {
	if (strlen(strnumber) <= 2) {
		return strtol(strnumber, NULL, 10);
	}
	string first_characters = string(strnumber).substr(0,2);
	if (first_characters.compare(string("0x")) == 0 || first_characters.compare(string("0X")) == 0) {
			return strtol(strnumber, NULL, 16);	
	}
	return strtol(strnumber, NULL, 10);
}

int strnumber_to_int(char* strnumber) {
	long long_number = strnumber_to_long(strnumber);
	if (long_number > INT_MAX){
		return INT_MAX;
	} else if (long_number < INT_MIN){
		return 0;
	}
	return (int) long_number;
}

string long_to_hex_string(long input){
	char buff[20];
	itoa(input, buff, 16);
	return string("0x").append(string(buff));
}

/*
* User input functions
*/

int get_stdin_int(string message){
	int val;
	cout << "\n" << message;
	std::cin >> val;
	return val;
}

/*
* Handle retrieving functions
*/

HANDLE get_handle_by_process_id(unsigned int pid){
	cout << "Attaching to PID: " << pid << "\n";
	return OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,0,pid);
}

int get_pid_by_process_name(const char* process_name){
	int pid = -1;
	PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (stricmp(entry.szExeFile, process_name) == 0) {  
                pid = entry.th32ProcessID;
				break;
            }
        }
    }
    CloseHandle(snapshot);
    return pid;
}

HANDLE get_handle_by_process_name(const char* process_name){
	HANDLE pHandle = NULL;
	int pid = get_pid_by_process_name(process_name);
    if (pid >0){
    	pHandle = get_handle_by_process_id(pid);
	}
    return pHandle;
}

HANDLE get_handle_by_window_title (LPCSTR window_name){
	HANDLE pHandle = NULL;
	DWORD pid;
	HWND hwnd;
    hwnd = FindWindow(NULL, window_name);
    
	if(!hwnd) {
		cout <<"Window not found!\n";
		return pHandle;
	}
	GetWindowThreadProcessId(hwnd,&pid);
	
	return get_handle_by_process_id((unsigned int)pid);
}

HANDLE get_handle_from_user_input(){
	HANDLE pHandle;
	int buff_size = 32;
	char buff[buff_size] = {0};
	cout << "Enter process identifientification [i (process [i]d) / n (process [n]ame) / w ([w]indow title)]: ";
	cin.getline(buff, buff_size, '\n');
	
	if (strcmp(buff, "i") == 0){
		pHandle = get_handle_by_process_id(get_stdin_int("Enter process id: "));
	} else if (strcmp(buff, "n") == 0){
		cout << "\nEnter process name: ";
		cin.getline(buff, buff_size, '\n');
		pHandle = get_handle_by_process_name(buff);
	} else if (strcmp(buff, "w") == 0){
		cout << "\nEnter window title: ";
		cin.getline(buff, buff_size, '\n');
		pHandle = get_handle_by_window_title(buff);
	} else {
		cout << "\n[ERROR] Incorrect option: " << buff;
	}
	return pHandle;
}

HANDLE get_handle(){
	HANDLE pHandle;
	if (parsed_arguments.handle_mode == HANDLE_MODE_PROCESS_ID) {
		pHandle = get_handle_by_process_id(parsed_arguments.process_id);
	} else if (parsed_arguments.handle_mode == HANDLE_MODE_PROCESS_NAME) {
		pHandle = get_handle_by_process_name(parsed_arguments.process_name.c_str());
	} else if (parsed_arguments.handle_mode == HANDLE_MODE_WINDOW_TITLE) {
		pHandle = get_handle_by_window_title(parsed_arguments.window_title.c_str());
	} else {
		pHandle = get_handle_from_user_input(); 
	}
	return pHandle;
}

/*
* Memory access/display funcitons
*/

vector<long>* ScanMemmory(HANDLE pHandle){
	vector<long>* addresses = new vector<long>;
	int found = 0;
 	bool runThread = true;
 	char addrHex[20];
 	SYSTEM_INFO sysInfo = { 0 };
    
    int value_to_find = get_stdin_int("Enter value to find: ");
    
	GetSystemInfo(&sysInfo);
    LONG aStart = (long)sysInfo.lpMinimumApplicationAddress;
    LONG aEnd = (long)sysInfo.lpMaximumApplicationAddress;
    cout << "Scanning memory from " << aStart << " to " << aEnd << "\n";

    do{
        while (aStart < aEnd){
            MEMORY_BASIC_INFORMATION mbi = { 0 };
            if (!VirtualQueryEx(pHandle, (LPCVOID)aStart, &mbi, sizeof(mbi))){
                cout << "Cannot VirtualQueryEx. Error:" << GetLastError() << "\n";
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

void PrintVector(vector<long>* addresses) {
	if ((addresses->size() > 0) && (addresses->size() <= 10)){
		vector<long>::iterator it2;
		for(it2 = addresses->begin(); it2 != addresses->end();) {
			cout << long_to_hex_string(*it2) << "(" << *it2 << ").\n";
			++it2;
		}
	} 
}

vector<long>* get_addresses(HANDLE pHandle){
	if (parsed_arguments.addresses->size() > 0){
		return parsed_arguments.addresses;
	}
	vector<long>* addresses = ScanMemmory(pHandle);
	cout << "Matched addresses: " << addresses->size() << "\n";
	PrintVector(addresses);    
    
    int val;
	while (true) {
		val = get_stdin_int("Enter next value to find: ");
    	ReScanMemmory(val, addresses, pHandle);
    	if (addresses->size() == 0){
    		cout << "No more items, exiting.";
    		addresses = new vector<long>;
    		break;
		} else if (addresses->size() <= parsed_arguments.number_of_addresses_to_find) {
			cout << "[WARNING] Value will be set to a total of " << addresses->size() << " addresses (but expecting to find " << parsed_arguments.number_of_addresses_to_find << "):\n";
			PrintVector(addresses);  
			break;
		} else if (addresses->size() <= parsed_arguments.number_of_addresses_to_find) {
			cout << "Address " << addresses->size() << " number of addresses have been found:\n";
			PrintVector(addresses);
			break;
		} 
	cout << "Matched addresses: " << addresses->size() << "\n";
	PrintVector(addresses);    
	}
	return addresses;
}

/* 
* Value setting functions
*/

void fix_value_to_address(HANDLE pHandle,vector<long>* addresses, int value_to_set, int sleep_time_ms){
	//TODO: Detect Ctrl+c and add CloseHandle
	char value_to_set_buff[4];
	int_to_char(value_to_set, value_to_set_buff);
	unsigned char *value_in_memory = new unsigned char[4]();
	while(1) {
		ReadProcessMemory(pHandle, (LPCVOID)*addresses->begin(), value_in_memory, 4, 0);
		if (*(DWORD*)value_in_memory != value_to_set) {
			WriteProcessMemory(pHandle, (LPVOID)*addresses->begin(), &value_to_set_buff, 4, NULL);
		}
		Sleep(sleep_time_ms);
	}	
}

int get_operation_mode(){
	if (parsed_arguments.operation_mode == OPERATION_MODE_SET_ONCE){
		cout << "Setting value once.";
	} else if (parsed_arguments.operation_mode == OPERATION_MODE_SET_FREEZE){
		cout << "Setting freezing value.";
	} else {
		char set_once_mode_buff[4];
		itoa(OPERATION_MODE_SET_ONCE, set_once_mode_buff, 10);
		char freeze_mode_buff[4];
		itoa(OPERATION_MODE_SET_FREEZE, freeze_mode_buff, 10);
		string operation_setting_mode_string = string("Enter value setting mode [").append(string(set_once_mode_buff)).append(" (to set value once) or ").append(string(freeze_mode_buff)).append(" (to freeze value constantly)]: ");
		return get_stdin_int(operation_setting_mode_string);
	}
	return parsed_arguments.operation_mode;
}

int get_value_to_set(){
	if (parsed_arguments.value_set){
		return parsed_arguments.value_to_set;
	}
	return get_stdin_int("Enter value to set: ");
}

int edit_memory(HANDLE pHandle, vector<long>* addresses){
	int operation_mode = get_operation_mode();
	
	if (operation_mode == OPERATION_MODE_SET_ONCE){
		char value_to_set_buff[4];
		int_to_char(get_value_to_set(), value_to_set_buff);
		WriteProcessMemory(pHandle, (LPVOID)*addresses->begin(), &value_to_set_buff, 4, NULL);
	} else if (operation_mode == OPERATION_MODE_SET_FREEZE) {
		fix_value_to_address(pHandle, addresses, get_value_to_set(), 500);
	} else {
		cout << "Invalid operation mode: " << operation_mode;
		return -3;
	}
	return 0;
}

/*
* Arguments
*/

void parse_arguments(int argc, char* argv[]){
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-pi") == 0 && i+1 < argc) {
			parsed_arguments.handle_mode = HANDLE_MODE_PROCESS_ID;
			parsed_arguments.process_id = strnumber_to_uint(argv[i+1]);
			i++;
		} else if (strcmp(argv[i], "-pn") == 0 && i+1 < argc) {
			parsed_arguments.handle_mode = 2;
			parsed_arguments.process_name = argv[i+1];
			i++;
		} else if (strcmp(argv[i], "-pw") == 0 && i+1 < argc) {
			parsed_arguments.handle_mode = HANDLE_MODE_WINDOW_TITLE;
			parsed_arguments.window_title = argv[i+1];
			i++;
		} else if (strcmp(argv[i], "-a") == 0 && i+1 < argc) {
			parsed_arguments.addresses->push_back(strnumber_to_long(argv[i+1]));
			i++;
		} else if (strcmp(argv[i], "-na") == 0 && i+1 < argc) {
			parsed_arguments.number_of_addresses_to_find = strnumber_to_uint(argv[i+1]);
			i++;
		} else if (strcmp(argv[i], "-s") == 0 && i+1 < argc) {
			parsed_arguments.value_set = true;
			parsed_arguments.value_to_set = strnumber_to_int(argv[i+1]);
			i++;
		} else if (strcmp(argv[i], "-so") == 0) {
			parsed_arguments.operation_mode = OPERATION_MODE_SET_ONCE;
		} else if (strcmp(argv[i], "-sf") == 0) {
			parsed_arguments.operation_mode = OPERATION_MODE_SET_FREEZE;
		} else {
			cout << "[ERROR] Invalid argument: " << argv[i] << "\n";
		}
	}
	if (parsed_arguments.addresses->size() > 0 && parsed_arguments.addresses->size() != parsed_arguments.number_of_addresses_to_find){
		parsed_arguments.number_of_addresses_to_find = parsed_arguments.addresses->size();
	}
}

/*
* Main
*/

int main(int argc, char* argv[]) {
	parse_arguments(argc, argv);
	HANDLE pHandle = get_handle();
	if(!pHandle) {
		cout <<"Could not get handle!\n";
		return -1;
	}

	vector<long>* addresses = get_addresses(pHandle);
	if (addresses->size() == 0){
		cout << "Cannot find addresses to edit!\n";
		CloseHandle(pHandle);
		return -2;
	}
	cout << "\nAddresses to edit:\n";
	PrintVector(addresses);
	
	int exit_status = edit_memory(pHandle, addresses);
	CloseHandle(pHandle);
	return exit_status;
}
