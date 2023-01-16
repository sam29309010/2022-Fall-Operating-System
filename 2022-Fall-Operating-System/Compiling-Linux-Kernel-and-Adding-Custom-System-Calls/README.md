Compiling Linux Kernel and Adding Custom System Calls
===

###### tags: `OS`

* [Assignment 1: Compiling Linux Kernel and Adding Custom System Calls](https://hackmd.io/yJfXqJKnSy2_liuIOiD4Hg)


### Kernel Compilation
The creation of the kernel mainly follows [this tutorial](https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8).

There are three modifications for kernel sources. First, the variable `core-y` within the build system `Makefile` is modified to search for newly-added system calls' path. Second, the prototype of the system call is added under `include/linux/syscalls.h`. Third, the system call table is updated by changing `arch/x86/entry/syscalls/syscall_64.tbl`.


### System call `sys_hello` and `sys_revstr`
`sys_hello` is implemented using macro `SYSCALL_DEFINE0` as follows. It calls `printk` to print messages to the kernel log. The prototype of macro and kernel function is declared in `<linux/kernel.h>` and `<linux/syscalls.h>` respectively.

```cpp
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello)
{
    printk("Hello world.\n");
    printk("310551145.\n");
    return 0;
}

```

`sys_revstr` is implemented using macro `SYSCALL_DEFINE2`, whose suffix `2` specifies the number of arguments. It uses a `char` array with a length of 16 to store a given string argument. Function `copy_from_user` under path `<linux/uaccess.h>` is called to copy the data from user space to kernel space for reverse operation. The implementation here allocates a fixed-size array since the maximum string length is known. If the string size is unknown, allocating memory dynamically is required using a function such as `kmalloc`.

```cpp
#include <linux/kernel.h>   // For printk
#include <linux/string.h>   // For strlen
#include <linux/uaccess.h>  // For copy_from_user
#include <linux/syscalls.h> // For SYSCALL_DEFINE1

// Definition of the system call
SYSCALL_DEFINE2(reverse_string, const unsigned int, str_len, const char __user *, str)
{
    int i;
    char message[16], temp;

    copy_from_user(message, str, str_len);
    message[str_len] = '\0';
    printk("%s\n", message);
    for (i=0; i<str_len/2; i++)  
    {  
        temp = message[i];  
        message[i] = message[str_len-i-1];  
        message[str_len-i-1] = temp;
    }
    printk("%s\n", message);
  return 0;
}
```

### System Call Output

#### Makefile
```
CC = g++
test_sys_calls = test_sys_hello.o test_sys_revstr.o
all: test_sys_hello.o test_sys_revstr.o
%.o: %.c
	$(CC) $< -o $@
clean:
	rm -rf *.o
```

#### Kernel compilation
```
(uname -a && cat /etc/os-release) > kernel_info
cat kernel_info
Linux sam-B550M-AORUS-ELITE 5.19.12-os-310551145 #4 SMP PREEMPT_DYNAMIC Sun Oct 16 15:44:41 CST 2022 x86_64 x86_64 x86_64 GNU/Linux
PRETTY_NAME="Ubuntu 22.04.1 LTS"
NAME="Ubuntu"
VERSION_ID="22.04"
VERSION="22.04.1 LTS (Jammy Jellyfish)"
VERSION_CODENAME=jammy
ID=ubuntu
ID_LIKE=debian
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
UBUNTU_CODENAME=jammy
```
#### `sys_hello`
```
./test_sys_revstr.o && sudo dmesg > sys_revstr_output
cat sys_revstr_output
[    0.000000] Linux version 5.19.12-os-310551145 (root@sam-B550M-AORUS-ELITE) (gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #4 SMP PREEMPT_DYNAMIC Sun Oct 16 15:44:41 CST 2022
[    0.000000] Command line: BOOT_IMAGE=/boot/vmlinuz-5.19.12-os-310551145 root=UUID=ad781e3a-8af2-48bd-986f-4fbe3375ef88 ro quiet splash vt.handoff=7
...
[  147.608745] Hello world.
[  147.608747] 310551145.
```

#### `sys_revstr`
```
./test_sys_hello.o && sudo dmesg > sys_hello_output
cat sys_hello_output
[    0.000000] Linux version 5.19.12-os-310551145 (root@sam-B550M-AORUS-ELITE) (gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #4 SMP PREEMPT_DYNAMIC Sun Oct 16 15:44:41 CST 2022
[    0.000000] Command line: BOOT_IMAGE=/boot/vmlinuz-5.19.12-os-310551145 root=UUID=ad781e3a-8af2-48bd-986f-4fbe3375ef88 ro quiet splash vt.handoff=7
...
[  302.085686] hello
[  302.085689] olleh
[  302.085690] 5Y573M C411
[  302.085691] 114C M375Y5
```


### Others
Since Linux cannot automatically switch to the new kernel after rebooting, adding the flag `GRUB_DISABLE_OS_PROBER=false` to the grub settings could help to detect it.

```
Memtest86+ needs a 16-bit boot, that is not available on EFI, exiting
Warning: os-prober will not be executed to detect other bootable partitions.
Systems on them will not be added to the GRUB boot configuration.
Check GRUB_DISABLE_OS_PROBER documentation entry.
Adding boot menu entry for UEFI Firmware Settings …
```

### Reference
1. [Adding a system call to the linux kernel in ubuntu](https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8)
2. [Kernel config error](https://blog.csdn.net/qq_36393978/article/details/118157426)
3. [Reboot with new kernel](https://askubuntu.com/questions/1414245/why-do-i-get-warning-os-prober-will-not-be-executed-to-detect-other-bootable-p)
4. [Adding a new system call](https://www.kernel.org/doc/html/latest/process/adding-syscalls.html)
5. [Implement a linux system call](https://github.com/shikshan/syscall/blob/master/code/greet_o/sys_greet_o.c)