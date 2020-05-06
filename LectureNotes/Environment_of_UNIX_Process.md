---
###### tags: `SP`
---
# Environment of a Unix Process
## Process
* A process is an instance of a running program
* Process provides each program with two key abstractions:
  * ==Logical control flow==
    * Each program seems to have exclusive use of the CPU.
  * ==Private address space==
    * Each program seems to have exclusive use of main memory.
* ==virtual memory的size和physical memory無關，可是和硬體有關(排線bus)==
* How are these illusions maintained?
  * ==Process executions interleaved (multitasking)==
  * ==Address spaces managed by virtual memory system==
## main Function 
* The starting point of a C program. Programs written in different languages have a different name.
* A special ==start-up routine== is called to set things up first ==before call main()==
  * ==Set up by the link editor (invoked by the compiler)==
  * ![](https://i.imgur.com/Nm977wO.png)
  * ![](https://i.imgur.com/io39MYC.png)
  * ==nm -g program:list symbols from object files, list object file的externel symbols==
    * ![](https://i.imgur.com/U0TIHIs.png)
  * ==readelf:display info. about ELF files把執行檔的header讀出來==
    * elf的header一開始會有magic code
    * ![](https://i.imgur.com/XHpUkpe.png)
  * ==objdump -d program: display info. about obj. files==
    * -d:反組譯
## File Format/Layout of a Program
* ==ELF(Executable and Linkable Format)==
  * ==執行檔或者object file的format==
* ![](https://i.imgur.com/W0oiXkB.png)
* preprocessing -> 組語 -> object code
* ELF Object File Format
  * ELF header
    * ==Magic number(\177ELF), type (.o, exec, .so), machine, byte ordering, entry point, size, etc.==
  * ==Program header table: 執行檔 / Section header table: object file，在header可以知道type是誰==
  * ![](https://i.imgur.com/gS8mAJp.png)
  * ==.text section: code==
    * 指令，之後會compile成object code
  * ==.data section: initialized (static) data (有給初始值)(global variable)==，會紀錄global variable哪個位置的值是什麼
  * ==.bss section: Uninitialized (static) data(global variable)==
  * ==.symtab section==: Symbol table / Procedures and static variable names / Section names and ==locations==
    * ==處理程式裡面所有的symbol:包含變數、function，安排symbol的位置==
  * ==.rel.text section: Relocation info for .text section / Addresses of instructions that will need to be modified in the executable / Instructions for modifying==，要把未決定的都填完程式才會執行
  * .rel.data section: Relocation info for .data section / Addresses of pointer data that will need to be modified in the merged executable. 
* symbol
  * ==Code consists of symbol definitions and references==
  * ==definition: 不論是否initialized，只要有佔空間==
  * ==Each symbol definition has memory address References can be either local or external==
    * extern:等下會用到它，但在別的地方
    * 如果不是定義，代表memory不是自己allocate的
  * ![](https://i.imgur.com/kLKSpRT.png)
  * externel reference: compile後不知道address在哪 -> relocation
  * 在virtual memory中，通常不會放絕對位置，會放的是相對於data section一開頭所在的
## Typical Memory Arrangement
* ![](https://i.imgur.com/qTozsjr.png)
* 通常text底下會空一小段，也就是不會從0開始放
* virtual memory: ==kernel space(open file description table) + user space==
* ==text, initialized data, un-initialized是程式一執行起來size就固定的==
* ==stack呼叫function傳參數，heap是malloc用的==
* command-line也是main function的參數之一，command-line和environment variables會放在stack上面
* 將disk的檔案mapping上來(在syack和heap那個肚子內)
* 不論是哪一個程式，一定會從固定的位置開始放
* stack上面那一塊是main function argument 開始放的位置：==command line arguments and environment variable==，but這塊是固定的！這樣environment會造成問題，所以當要增加的時候，會把資料先搬到==heap==中，but指標仍指向這塊，所以可能會錯
* stack和heap之間的空間很大
* 肚子：
  * ==動態連結函式庫==
  * ==share memory(memory linking)==
## Memory Layout of a Process
* Pieces of a process
  * Text Segment
    * Read-only usually, sharable
  * Initialized Data Segment
  * Uninitialized Data Segment – bss (Block Started by Symbol)
  * Stack – return addr, automatic var, etc.
  * Heap – dynamic memory allocation (malloc)
  * Stack Heap這兩個segment在執行檔沒有
## Stack Frame
* ==每次call function都會push一個stack frame==
* When one function is called, one area of memory, called frame or stack frame, is set aside for the function. Each frame contains:
  * ==Return address==
  * ==Passed parameter(s)==
  * ==Saved registers==:紀錄pointer，因為之後pop掉的時候才知道範圍在哪，pop function的時候不會清掉，只是改動pointer，例如main function call function1，之前要先記錄main function的rbp, rsp(呼叫者main function自己存自己的rbp, rsp, return address)
  * ==Local variable(s)==
    * main function先push自己的register, return address, variables -> call first function -> first function push自己的variable(data是誰own的就是誰push的)，即使是有給初始值的variable，也不會放到initialize data區
* ==如果function內有呼叫static的變數，代表return回來之後要保留之前的值，那就不能放在stack，會依據是否initialize看放在initialized data或uninitialized data，variable的life cycle不會依據function的結束而結束，可是其他function還是不能動，因為compile會不給過==
## Virtual Memory vs Physical Memory
* ==每一個process都有自己的virtual memory，可是只有一個physical memory==
* ==每一個process都有自己的page table==，OS管的
* ![](https://i.imgur.com/3sxdIlF.png)
* 會把左右兩邊都切成大小一樣的page，對應每一個page到physical memory的page
* ==Address translation: Hardware converts virtual addresses to physical addresses via OS-managed lookup table (page table)==
* 因為==physical memory是有限的==，因此會用==硬碟==來幫忙physical memory的不足：==把physical memory的page move到disk上(page replacement algorithm)==
* 程式看到的都是virtual address的值，但是如果想要知道值，就會很快的把virtual address轉換到physical address(用硬體換，==硬體做translation，軟體管table==)
## Virtual Address Spaces
* ==Processes share physical pages by mapping in page table==
* ==Separate Virtual Address Spaces (separate page table)==
  * Virtual and physical address spaces divided into equal-sized blocks
    * Blocks are called “pages” (both virtual and physical)
  * Each process has its own virtual address space  
    * OS controls how virtual pages as assigned to physical memory
## Address Translation via Page Table
* ![](https://i.imgur.com/keex5EB.png)
  * 先看page size是多少，例如是4K，那就需要12個bit，所以後面page offset就要留0~11，假設virtual address有32bit，那前面現在剩下20bit，總共有2^20種變化，這樣page table就有2^20個entry，(VPN很像是page table的index)，physical的值就是page number裡面的值
  * ![](https://i.imgur.com/gzdvGYe.png)
* ![](https://i.imgur.com/t1gQ0eH.png)
## Virtual Memory
* The translation from virtual memory address to physical address has to be LIGHT fast.
  * Not possible to be done by operating systems.
  * ==Done with hardware support: Memory Management Unit (MMU)==
## Static Library v.s. Shared Library
* 只要不是自己寫的程式，就要link過來
* A program usually calls ==standard C/C++ library, POSIX functions=, etc.
* ==The binary codes of the pre-compiled function call have to be linked with the program.==
* ==Static library(static linking)==:
  * ==the library and program are complied into one file.==
  * ==In Linux, static library ends with .a.==
    * ==.a:很多.o merge成一個file==
    * ==libc.a: include stdio.h的library==
    * libm.a: 數學函式庫
    * 把object code搬到執行檔
  * Problem:
    * When the library is updated, the program has to be re-linked
    * The file size will be large. ==Potential for duplicating lots of common code in the executable files on a filesystem.==
* ==Shared library(dynamic linking)==:
  * ==Shared libraries (dynamic link libraries, DLLs) whose members are dynamically loaded into memory and linked into an application at run-time.==
  * ==執行檔裡面沒有binary的object code==
  * run的時候，會去動態連結的函式庫裡面去找有用到需要的instruction的object code，然後放到肚子，之後也不會清掉，所以如果再call一次就不用再link一次
  * 適合開發過程中的軟體
  * Why
    * Remove common routines from executable files
    * Have an easy way for upgrading
  * Problems
    * More overheads: dynamic linking
  * gcc
    * default: shared library
    * Static linking can be forced with the ==-static option to gcc to avoid the use of shared libraries==.
* ![](https://i.imgur.com/AubgmcC.png)
## Memory Allocation
* Three ANSI C Functions:
  * ==malloc – allocate a specified number of bytes of memory. Initial values are indeterminate.==
  * ==calloc – allocate a specified number of objects of a specified size. The space is initialized to all 0 bits.==
  * ==realloc – change the size of a previously allocated area. The initial value of increased space is indeterminate.== 
    * 不夠用的時候，所以假設要改成1000，原來有100，就會copy前面的100，剩下的900不確定，回傳新的address，如果後面的900沒人用就接著，如果有就會copy100到一塊地方再接900
  * ![](https://i.imgur.com/gRhAYq0.jpg)
* Remark:
  * ==realloc() could trigger moving of data -> avoid pointers to that area!==
  * ==sbrk() is used to expand or contract the heap of a process – a malloc pool==
    * 假設和系統要100個byte，因為sbrk是system call，cost很高，所以會直接和系統要4K，C library有管理機制
  * ==alloca() allocates space from the stack frame of the current function!==
    * alloc在呼叫function的stack上，好處是==不用free==
## Record-keeping Info
* ![](https://i.imgur.com/9h3lWZM.jpg)
* 系統會做link list管理(把白的地方串起來)
## Nonlocal Jumps: setjmp & longjmp
* ![](https://i.imgur.com/dMMp5F3.png)
* setjmp後可不可以restore
* ![](https://i.imgur.com/YxhOGqX.png)
  * autovariable把optimization打開，compiler會盡量放到register，變數放在register才會restore
  * ![](https://i.imgur.com/QHxasmB.jpg)
## Type Qualifiers in C
* Control over optimization
* ==const: no writes through this variable==
* ==volatile: variable may be modified externally; no cache through this variable (cache is not guaranteed to contain any previous value)(compiler會放到memory)==
* ==restrict: the pointer must be the variable used to access the object it points to(如果一個pointer用restrict，他所指到的object，一定要透過該pointer去access)==
  * ![](https://i.imgur.com/yfP3SmA.png)
## Memory-mapped I/O
* ==mmap() maps a file on disk into a buffer in memory==
* ![](https://i.imgur.com/TqW7Y9G.png)
* 這樣可以改動virtual memory，就可以改動到file裡面的
* ==When we fetch bytes from the buffer, the corresponding bytes of the file are read.==
* ==When we store data in the buffer, the corresponding bytes are written==
* ==This lets us perform I/O without using read/write or fscanf/fprintf.==
* ![](https://i.imgur.com/39hS12P.png)
  * ==prot: access permission==
  * ![](https://i.imgur.com/h6Ah3zK.png)
  * ==flag: private or share==，這邊改的話，如果別人也map，可不可以可得到 
* ![](https://i.imgur.com/HtDkIAa.png)
