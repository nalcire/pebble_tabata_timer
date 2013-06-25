#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

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
    int ticks = 0;
    PblTm now;
    get_time(&now);
    ticks += now.tm_sec;
    ticks += now.tm_min * 60;
    ticks += now.tm_hour * 60 * 60;
    ticks += now.tm_yday * 60 * 60 * 24;
    // forget leap years, as long as we're consistently wrong
    ticks += now.tm_year * 60 * 60 * 24 * 365;     
    return ticks;
}
