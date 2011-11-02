#include "../include/my_funcs.h"

void trim_str(char* str) {
	int len = strlen(str); 
	int i = len - 1; 

	while (i >=0 && (str[i] == '\n' || str[i] == ' ' || str[i] == '\t')) {
		--i; 
	}

	str[i+1] = '\0'; 
}
