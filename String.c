#include <stddef.h>

int strcmp(const char *a, const char *b) __attribute__ ((naked));
int strcmp(const char *a, const char *b)
{
	asm(
        "strcmp_lop:                \n"
        "   ldrb    r2, [r0],#1     \n"
        "   ldrb    r3, [r1],#1     \n"
        "   cmp     r2, #1          \n"
        "   it      hi              \n"
        "   cmphi   r2, r3          \n"
        "   beq     strcmp_lop      \n"
		"	sub     r0, r2, r3  	\n"
        "   bx      lr              \n"
		:::
	);
}

int strncmp(const char *a, const char *b, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++)
		if (a[i] != b[i])
			return a[i] - b[i];

	return 0;
}

/*
size_t strlen(const char *string)
{
    size_t chars = 0;

    while(*string++) {
        chars++;
    }
    return chars;
}
*/
size_t strlen(const char *s) __attribute__ ((naked));
size_t strlen(const char *s)
{
	asm(
		"	sub  r3, r0, #1			\n"
        "strlen_loop:               \n"
		"	ldrb r2, [r3, #1]!		\n"
		"	cmp  r2, #0				\n"
        "   bne  strlen_loop        \n"
		"	sub  r0, r3, r0			\n"
		"	bx   lr					\n"
		:::
	);
}



#define MaxDigit 6
/*
* Main part of itoa and xtoa
* Utilize the concept of long division to implement
*/
void _toa(int in_num, char *out_str, int base, int digit){

    int Mdigit = digit;
    int neg = 0;
    out_str[digit--] = '\0';
    
    if(in_num == 0) out_str[digit--] = '0';
    else if(in_num < 0){
        in_num = -in_num;
        neg = 1;
    }

    while(in_num > 0){

        if(base == 16 && in_num % base >= 10)
            out_str[digit--] = (in_num % base) + 'A' - 10;
        else
            out_str[digit--] = (in_num % base) + '0';
        
        in_num /= base;
    }//End of while(in_num > 0)
    
    if(base == 16){
        out_str[digit--] = 'x';
        out_str[digit--] = '0';
    }

    if(neg) out_str[digit--] = '-'; //negative number

digit++;
    //reorder
    int j = 0;
    while(digit < Mdigit + 1){
        out_str[j++] = out_str[digit++];
    }
}

void xtoa(int in_num, char *out_str){
    
    _toa(in_num, out_str, 16, MaxDigit + 4);//MaxDigit + 4 that can contain address
}


void itoa(int in_num, char *out_str){
   
    _toa(in_num, out_str, 10, MaxDigit);
}