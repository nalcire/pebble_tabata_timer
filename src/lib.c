#include "lib.h"
#include <pebble.h>

char * itoa(int num, int size) {
    int temp = num;
    int length = 0;
    do {
        temp /= 10;
        length++;
    } while( temp > 0 );

    static char buff[20] = {'0'};

    for(int i=0; i < size; i++) {
	if( i >= length ) {
	    buff[size-1-i] = '0';
	}
	else {
	    buff[size-1-i] = '0' + num % 10;		
	    num /= 10;	    
	}
    }
    buff[size] = 0;

    return buff;
}

int get_ticks_now_in_seconds() {
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}
