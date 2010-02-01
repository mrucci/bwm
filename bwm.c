/* Copyright 2010 Marco Rucci <marco.rucci@gmail.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <time.h>
#include <sys/time.h>

#define SEC  1000000
#define MSEC 1000
#define USEC 1

struct option longopts[] = {
   { "interface",   required_argument, 	NULL,         'i' },
   { "download",   	no_argument, 		NULL,         'd' },
   { "upload", 		no_argument, 		NULL,         'u' },
   { 0, 			0, 					0, 			  0}
};

struct arguments
{
	long interval;
	int nsamples;

	char iface[8];
	int direction;

	char fname[128];
	FILE* f;

	char foutname[128];
	FILE* outf;
};

void argumentsParse( struct arguments* args, int argc, char** argv )
{
	int c;
	int option_index = 0;
	while( (c = getopt_long (argc, argv, "i:", longopts, &option_index)) != -1 )
	{
		switch (c)
		{
			case 'i':
				strcpy( args->iface, optarg );
				break;
			case 'd':
				args->direction = 0;
				break;
			case 'u':
				args->direction = 1;
				break;
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (longopts[option_index].flag != 0)
					break;
				printf ("option %s", longopts[option_index].name);
				if( optarg )
					printf(" with arg %s", optarg);
				printf ("\n");
				break;
			case ':':   /* missing option argument */
				fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:    /* invalid option */
				fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
				abort();
		}
	}
}

void argumentsCheck( struct arguments* args )
{
	if( args->interval > SEC )
	{
		fprintf( stderr, "interval must be less than 1 sec" ); //because of usleep definition
		exit( EXIT_FAILURE );
	}
	if( args->nsamples < 1 )
	{
		fprintf( stderr, "nsamples must be at least one" );
		exit( EXIT_FAILURE );
	}


	sprintf( args->fname, "/sys/class/net/%s/statistics/%s_bytes", args->iface, args->direction ? "tx" : "rx" );
	/*strcpy( args->fname, "/sys/class/net/eth0/statistics/rx_bytes" ); */
/*	strcpy( args->fname, "/sys/class/net/" ); */
/*	strcat( args->fname, args->iface ); */
/*	strcat( args->fname, "/statistics/" ); */
/*	strcat( args->fname, args->iface ); */


	args->f = fopen( args->fname, "r" );
	if( args->f == NULL )
	{
		fprintf( stderr, "Cannot open input files: \n[rx] %s \n", args->fname );
		exit( EXIT_FAILURE );
	}

	/* args->outf = fopen( foutname, "w" ); */
	if( args->outf == NULL )
	{
		fprintf( stderr, "Cannot open output files: \n[rx] %s \n", args->foutname );
		exit( EXIT_FAILURE );
	}



}

void argumentsSetDefault( struct arguments* args )
{
	/* default values: 2 seconds window average with 20 samples every 0.1 sec */
	args->interval = 100 * MSEC;
	args->nsamples = 20;

	strcpy( args->iface, "eth0" );
	args->direction = 0; //received

	//strcpy( args->foutname, "/tmp/rx-kBps" );
	args->outf = stdout;

}

long timevalSubtract(struct timeval* x, struct timeval* y )
{
	return (x->tv_sec - y->tv_sec)*1000000 + (x->tv_usec - y->tv_usec);
}

long readBytes( char* fname, FILE* fp )
{
	//TODO: see setbuf, setvbuf se possono migliorare le performances.
	static int nb;
	//fseek ( fp , 0 , SEEK_SET ); //rewind
	freopen( fname, "r", fp );
	fscanf( fp, "%d", &nb );
	return nb;
}


int main( int argc, char** argv )
{
	float rxBps, rxkBps;
	long* rxb;
	int length;
	int curr, first;
	long rxb1, rxb2;
	float rxaverage;
	long realInterval;
	int count;
	int j;
	struct timeval t1, t2;

	struct arguments* args = malloc( sizeof( *args ) );

	argumentsSetDefault( args );
	argumentsParse( args, argc, argv );
	argumentsCheck( args );

	//fprintf( stderr, "[monitoring interface %s in %s]\n", args->iface, args->direction ? "upload" : "download" );

  	rxb = malloc( args->nsamples * sizeof(long) );


	//==============================================
	// Do the dirty job
	//==============================================

	curr = -1;
	length = 0;

	rxb1 = readBytes( args->fname, args->f );
	gettimeofday( &t1, NULL );


	while( 1 )
	{
		usleep( args->interval );

		curr = (curr+1) % args->nsamples;
		rxb2 = readBytes( args->fname, args->f );
		rxb[curr] = rxb2 - rxb1;
		rxb1 = rxb2;

		if( length < args->nsamples )
			length++;

		if( args->nsamples > 1 )
		{
			first = (curr + (args->nsamples - (length-1))) % args->nsamples;
			rxaverage = 0.0;
			for( j=first, count = 0 ; count < length; j = (j+1)%args->nsamples )
			{
				//printf( "b[%d] = %ld  -  count=%d  length=%d\n", j, b[j], count, length );
				rxaverage += rxb[j];
				count++;
			}
			rxaverage /= (float)length;
		}
		else
		{
			rxaverage = rxb[curr];
		}

		//printf( "%2.3f %5ld\n", average, b[curr] );

		gettimeofday( &t2, NULL );
		realInterval = timevalSubtract( &t2, &t1 );
		t1.tv_sec  = t2.tv_sec;
		t1.tv_usec = t2.tv_usec;

		//printf( " args->interval: %ld usec\n", realInterval );

		rxBps = rxaverage * ((float)SEC / (float)realInterval);
		rxkBps = rxBps / 1024.0f;

		//printf( "U: %5.2fkBps D: %5.2fkBps\n", txkBps, rxkBps );
		fseek( args->outf, 0, SEEK_SET );
		fprintf( args->outf, "%3.0f\n", rxkBps );
	}

	printf( "\n" );

	fclose( args->f );
	fclose( args->outf );
	free( rxb );

	return 0;
}
