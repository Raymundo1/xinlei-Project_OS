/*
 # ------------------------------------------------------------
 #  Compilation: gcc -o mrefgen mrefgen.c -lm
 #
 #  Usage: mrefgen -m memsize -l [0-9] -f [0-9] -w <0-1> -n numref
 #
 #  Remark: To see the generated 32-bit binary words as 4 hex digits
 #	    per line using 'od', use
 #          "od -Ad -w4 -x -v filename".
 #          (Note, however, the byte order when interpreting the output.)
 #
 #	    [Nov. 2017] This line displays a 32-bit integer/line:
 #	    "od -Ad -w4 --format=x4 -v filename"
 #
 #
 #  Disclaimer: This program is very naive. It does not purport to be
 #  a realistic model of anything.
 #
 #  mrefgen.c:  EE, 2015
 #  mrefgen.c:  EE, 2010 
 #  mrsgen.c:   PG, 2009
 # ------------------------------------------------------------
*/

#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define BUFSIZE 16384
#define	NBASE 1024

typedef	unsigned int   address;		// 4 bytes
typedef	unsigned long  counter;		// 8 bytes


extern char *optarg;                    // Required by getopt
extern int optind, opterr, optopt;

address	maxaddr = 0;
int	locality = -1,
	focus = -1;

counter	nref = 0,
	refCount= 0,
	writeCount= 0;			// number of write operations

double	writper = -1.0;

address obuf [BUFSIZE];
int 	obptr = 0;

address	dist_focus,
	cent_focus,
	dist_local,
	cent_local;

counter numb_local;

double	prob_focus;

// Accumulator related variables
//
enum {OP_INC= 0x00, OP_DEC= 0x40, OP_WR= 0x80, OP_RD= 0xc0 };
#define MAX_INCVAL  63
int     acc_netVal= 0,
	acc_incVal;

address ulrandom (address start, address end) {

	return start + (address) ((end - start) * drand48 ());

}

address newref (address center, address dist) {

	double d;
	address href;

	d = drand48 () - 0.5;
	href = (address) (dist * pow (fabs (d) * 2.0, (double) locality));
	if (d < 0.0) {
		if (href > center)
			return 0;
		return center - href;
	}
	href += center;
	if (href < center || href >= maxaddr)
		return maxaddr - 1;
	return href;
}

void initgen () {

	srand48 (time (NULL));
	dist_focus = (address) (maxaddr / ((double) (1 << (focus + 3))));
	if (dist_focus < 128)
		dist_focus = 128;
	cent_focus = ulrandom (dist_focus, maxaddr - dist_focus);
	if (cent_focus >= maxaddr)
		cent_focus = maxaddr - 1;
	prob_focus = (double) (focus) / (9.0 + 1.0);
	numb_local = 0;
	// fprintf (stderr, "%f %u %u\n", prob_focus, cent_focus, dist_focus);
}

void resetlocal () {

	dist_local = (address)
		((pow (drand48(), (double) locality) * (double) maxaddr) / 2.0);
	if (dist_local < 1024)
		dist_local = 1024;
	cent_local = ulrandom (dist_local, maxaddr - dist_local);
	if (cent_local >= maxaddr)
		cent_local = maxaddr - 1;
	numb_local = 16 +
		(counter) ((double)NBASE * (1 << locality) * drand48 ());
	// fprintf (stderr, "%lu %u %u\n", numb_local, cent_local, dist_local);
}

address genref () {

	address dref;

	if (drand48 () < prob_focus) {
		dref = newref (cent_focus, dist_focus);
	} else {
		if (numb_local == 0)
			resetlocal ();
		dref = newref (cent_local, dist_local);
		numb_local--;
	}

	// insert the op_code and op_value in the last byte
	//
	dref &= ~(0xff);				// clear last byte
	if (drand48 () < writper) { dref |= OP_WR; writeCount++; }
	else {
	     if (drand48 () < 0.2) dref |= OP_RD;	// op_code= OP_RD
	     else {
		acc_incVal = ulrandom (0, MAX_INCVAL);
		assert (acc_incVal < MAX_INCVAL);
		dref |= acc_incVal;    // set the incVal (and op_code= OP_INC)
		if (drand48() < 0.5) acc_netVal += acc_incVal;	
                else {
		   dref |= OP_DEC;	// op_code= OP_DEC
		   acc_netVal -= acc_incVal;
		}
	     }
        }
	refCount++;		
	return dref;
}

void flushbuf () {

	write (1, (char*)obuf, obptr * sizeof (address));
	obptr = 0;
}

void generate () {

	while (nref--) {
		if (obptr == BUFSIZE)
			flushbuf ();
		obuf [obptr++] = genref ();
	}

	if (obptr)
		flushbuf ();
}

void badusage () {

    fprintf (stderr,
        "Usage:\t mrefgen -m memsize -l [0-9] -f [0-9] -w <0-1> -n nref\n");
    fprintf (stderr,"\t (maxaddr= %u, locality= %d, focus= %d, wratio= %f, nref= %lu)\n", maxaddr, locality, focus, writper, nref);	

    exit (99);
}

address getint32 () {

	address v;
	int s;

	if (sscanf (optarg, "%u%n", &v, &s) < 1 || s < strlen (optarg))
		badusage ();

	return v;
}

counter getint64 () {

	counter v;
	int s;

	if (sscanf (optarg, "%lu%n", &v, &s) < 1 || s < strlen (optarg))
		badusage ();

	return v;
}

int getdigit () {

	int dig;

	dig = *optarg - '0';
	if (dig < 0 || dig > 9 || strlen (optarg) > 1)
		badusage ();

	return dig;
}

double getper () {

	double v;
	int s;

	if (sscanf (optarg, "%lf%n", &v, &s) < 1 || s < strlen (optarg))
		badusage ();

	if (v < 0.0 || v > 1.0)
		badusage ();

	return v;
}

int main (int argc, char *argv []) {

	int ec;

	opterr = 0;
	while ((ec = getopt (argc, argv, "m:l:f:w:n:")) >= 0 ) {
		switch (ec) {
			case 'm':
				maxaddr = getint32 ();
				break;
			case 'l':
				locality = getdigit ();
				break;
			case 'f':
				focus = getdigit ();
				break;
			case 'w':
				writper = getper ();
				break;
			case 'n':
				nref = getint64 ();
				break;
			default:
				badusage ();
		}
	}
	if (maxaddr == 0 || locality < 0 || focus < 0 || nref == 0 ||
		writper < 0.0) 
			badusage ();
	initgen ();
	generate ();

	fprintf (stderr, "[%s] %lu references generated, write count= %lu\n",
			  argv[0], refCount, writeCount);
	fprintf (stderr, "[%s] Accumulator final value= %d \n",
			  argv[0], acc_netVal);
}
