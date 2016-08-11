/*
 * show/set CPU speed of z80sim
 * compile with Hitech C compiler
 * 
 * usage:
 *	speed		- show current CPU speed
 *	speed 4		- set CPU speed to 4 MHz
 *	speed 0		- set CPU speed to unlimited
 *
 * April 2008, Udo Munk
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys.h>

/* I/O ports for programmable clock generator */
#define SPEED_LOW	30
#define SPEED_HIGH	31

int main(int argc, char *argv[])
{
    register int speed;

    if (argc == 1) {
	speed = inp(SPEED_LOW);
	speed += inp(SPEED_HIGH) << 8;
	if (speed == 0)
	    puts("CPU speed is unlimited");
	else
	    printf("CPU speed is %d MHz\n", speed);
    } else {
	speed = atoi(argv[1]);
	outp(SPEED_LOW, (speed & 0xff));
	outp(SPEED_HIGH, (speed >> 8));
    }
    return(0);
}
