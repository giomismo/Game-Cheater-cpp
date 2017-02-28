# Game-Cheater  
I created this software to cheat games by editing memory values from game processes in a easy way through command line. For more advanced memory scans/edits I use [CheatEngine](https://github.com/cheat-engine/cheat-engine), however this software enables me to use it remotely from command line (thanks [Ncat](https://nmap.org/ncat/)), with arguments that allow me to speed up and batch script cheats, and also it is not detected by modern games as a cheating software.  
  
##How this software works  
There are four phases involved. Each phase requires user input to specify the options (however they can be skipped if proper arguments are set):  
1. Specifying process to attach: you can enter PID, process name or windows title of the process to attach.  
2. Entering/finding the address to edit: you can specify the address to edit, or searching for the memory address through filtering memory addresses that contain specific values.  
3. Entering the value to set: at this moment only integers are supported.  
4. Specify the operation mode: set the value once and exit, or continuously checking the value and resetting it if changes.  
  
At this moment the code is in an early stage so there are a few limitations (no support for 64 bits and only 4 byte integers are supported to find/set) and things to be improved, but it works fine at this moment.  
  
##Command line arguments  
These are the arguments allowed at this moment, organized by phase:  
1. Specifying process to attach (only one is required):  
&nbsp;&nbsp;&nbsp;&nbsp;-pi \<pid\>: process id to modify  
&nbsp;&nbsp;&nbsp;&nbsp;-pn \<process name\>: name of the process to modify  
&nbsp;&nbsp;&nbsp;&nbsp;-pw \<window title\>: window title of the process to modify  
2. Entering/finding the address to edit (no flag will make the software to enter in address scan mode):  
&nbsp;&nbsp;&nbsp;&nbsp;-a \<address\>: decimal or hexadecimal address to edit, this will skip address scan   
&nbsp;&nbsp;&nbsp;&nbsp;-na \<number of addresses\>: in scan mode, stop when the given number of addresses or less have been found  
3. Entering the value to set:  
&nbsp;&nbsp;&nbsp;&nbsp;-s \<value\>: decimal value to set  
4. Specify the operation mode:  
&nbsp;&nbsp;&nbsp;&nbsp;-so: set the value once  
&nbsp;&nbsp;&nbsp;&nbsp;-sf: set the value forever  
  
There are more options pending to be added when I have time.  
  
##Examples  
There are a few examples:  
* game_cheater.exe <- This will guide you step by step to specify process, address, value and operation mode  
* game_cheater.exe -pn "bioshock.exe" -s 1234 -so <- Set the value 1234 to the process with name "bioshock.exe" once, address will be found through scan mode  
* game_cheater.exe -pn "bioshock.exe" -a 0xdeadbeef -s 1234 -f <- This will be setting constantly the value 1234 on 0xdeadbeef address of process "bioshock.exe"  





 
