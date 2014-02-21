#include <stdarg.h>
#include "String.h"
#include "syscall.h"

void _print(const char *str, int fdout){
    
    if(fdout == 0)
        fdout = mq_open("/tmp/mqueue/out", 0);//default message queue

    write(fdout, str, strlen(str) + 1);
}


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
    char *str_out;

    while( format[curr_ch] != '\0' ){
        
        if(format[curr_ch] == '%'){

            switch(format[curr_ch + 1]){
                case 's':
                    str = va_arg(ap, char *);
                    str_out = str;
                    break;
                case 'd':
                    itoa(va_arg(ap, int), str_num);
                    str_out = str_num;
                    break;
                case 'c':                  
                    out_ch[0] = (char)va_arg(ap, int);
                    str_out = out_ch;
                    break;
                case 'x':
                    xtoa(va_arg(ap, int), str_num);
                    str_out = str_num;
                    break;
                case '%':
                    str_out = percentage;
                    break;
                default:;
            }//End of switch(format[curr_ch + 1])

            curr_ch++;

        }else if(format[curr_ch] == '\n'){
            
            str_out = newLine;
        
        }else{
            
            out_ch[0] = format[curr_ch];
            str_out = out_ch;
        }
        curr_ch++;
        _print(str_out, fdout); //print on screen by syscall write()
    }//End of while( format[curr_ch] != '\0' )
    va_end(ap);
}//End of void fdprintf(int fdout, const char *format, ...)


//put string into default pipe defined in _print().
void printf(const char *format, ...){
    va_list ap;
    va_start(ap, format);
    int curr_ch = 0;
    char out_ch[2] = {'\0', '\0'};
    char newLine[3] = {'\n' , '\r', '\0'};
    char percentage[] = "%";
    char *str;
    char str_num[10];
    int out_int;
    char *str_out;

    while( format[curr_ch] != '\0' ){
        
        if(format[curr_ch] == '%'){

            switch(format[curr_ch + 1]){
                case 's':
                    str = va_arg(ap, char *);
                    str_out = str;
                    break;
                case 'd':
                    itoa(va_arg(ap, int), str_num);
                    str_out = str_num;
                    break;
                case 'c':                  
                    out_ch[0] = (char)va_arg(ap, int);
                    str_out = out_ch;
                    break;
                case 'x':
                    xtoa(va_arg(ap, int), str_num);
                    str_out = str_num;
                    break;
                case '%':
                    str_out = percentage;
                    break;
                default:;
            }//End of switch(format[curr_ch + 1])

            curr_ch++;

        }else if(format[curr_ch] == '\n'){
            
            str_out = newLine;
        
        }else{
            
            out_ch[0] = format[curr_ch];
            str_out = out_ch;
        }
        curr_ch++;
        _print(str_out, 0); //print on screen by syscall write()
    }//End of while( format[curr_ch] != '\0' )
    va_end(ap);
}//End of void printf(const char *format, ...)