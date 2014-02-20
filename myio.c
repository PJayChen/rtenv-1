#include <stdarg.h>
#include "String.h"
#include "syscall.h"

void fdprintf(int fdout, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    int curr_ch = 0;
    char out_ch[2] = {'\0', '\0'};
    char newLine[3] = {'\n' , '\r', '\0'};
    char percentage[] = "%";
    char *str;
    char str_num[10];
    int out_int;

    while( format[curr_ch] != '\0' ){
        
        if(format[curr_ch] == '%'){
            if(format[curr_ch + 1] == 's'){
                str = va_arg(ap, char *);
                write(fdout, str, strlen(str) + 1);
            }else if(format[curr_ch + 1] == 'd'){
                itoa(va_arg(ap, int), str_num);
                write(fdout, str_num, strlen(str_num) + 1);
            }else if(format[curr_ch + 1] == 'c'){
                out_ch[0] = (char)va_arg(ap, int);
                write(fdout, out_ch, strlen(out_ch) + 1);
           }else if(format[curr_ch + 1] == 'x'){
                xtoa(va_arg(ap, int), str_num);
                write(fdout, str_num, strlen(str_num) + 1);
            }else if(format[curr_ch + 1] == '%'){
                write(fdout, percentage, strlen(percentage) + 1);
            }
            curr_ch++;
        }else if(format[curr_ch] == '\n'){
            write(fdout, newLine, strlen(newLine) + 1);
        }else{
            out_ch[0] = format[curr_ch];
            write(fdout, out_ch, strlen(out_ch) + 1);
        }
        curr_ch++;
    }//End of while
    va_end(ap);
}