
#include "bitmap.h"
#include<pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include<time.h>

struct thread_args {
	struct bitmap * bm;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double itermax;
	int threadNum;
	int threadTotal;
} ;



int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int, int);
void * compute_image_thread(void *);

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("-n <threads>  number of threads to utilize in image generation (default=1)\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
	int threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h:n:"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				if (( threads = atoi(optarg)) == 0){
					show_help();
					exit(1);
				}
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);


	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));


	pthread_t threadArray [threads];
	struct thread_args allTargs[threads];
	
	for (int i = 0; i < threads; ++i){
	//	struct thread_args targs;
	//	pthread_t t = *threadArray[i];
		allTargs[i].bm = bm;
		allTargs[i].xmin = xcenter-scale;
		allTargs[i].xmax = xcenter+scale;
		allTargs[i].ymin = ycenter-scale;
		allTargs[i].ymax = ycenter+scale;
		allTargs[i].itermax = max;
		allTargs[i].threadNum = i; // which iteration are we on?
		allTargs[i].threadTotal = threads; // total number of threads

		if ((pthread_create(&(threadArray[i]), NULL, (void *) compute_image_thread, &(allTargs[i]))) != 0){
			printf("Unpexcted error in pthread_create(). Exiting mandel.c\n");
			exit(1);
		}
	} 

	for (int i =0; i < threads; ++i){
		if ( (pthread_join((threadArray[i]), NULL)) != 0 ){
			printf("Unexpected error in pthread_join(). Exiting mandel.c\n");
			exit(1);
		}
	}
	
	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}


	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/
void * compute_image_thread(void * args){
	// cast the void args structure to appropriate type
	struct thread_args targs =  * ((struct thread_args *) args);
	// call compute_image function with arguments
	compute_image(targs.bm, targs.xmin, targs.xmax, targs.ymin, targs.ymax, targs.itermax, targs.threadNum, targs.threadTotal);
	return 0;
}

void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int threadNum, int threadTotal)
{
	int i,j;

	int width = bitmap_width(bm);
	int height = bitmap_height(bm);

	// Calculate slice
	int sliceSize = height / threadTotal;

	// Set y pixel to begin image slice at
	int startVal = sliceSize*threadNum;
	// Set where to stop the image slice
	int maxy = startVal + sliceSize ;
	// If its the final thread, make sure it goes to the end
	if (threadNum == (threadTotal - 1)){ maxy = height;}

	for(j = startVal;j<maxy;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			bitmap_set(bm,i,j,iters);
		}
	}

}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}




