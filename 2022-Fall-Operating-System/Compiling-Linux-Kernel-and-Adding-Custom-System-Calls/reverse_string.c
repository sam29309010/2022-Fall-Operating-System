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
