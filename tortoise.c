#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
/* This is the guile interlink header! */
#include <libguile.h>

static const int WIDTH = 10;
static const int HEIGHT = 10;

static void plot_init( FILE *output ) {

    fprintf( output, "set multiplot\n" );
    fprintf( output, "set parametric\n" );
    fprintf( output, "set xrange [-%d:%d]\n", WIDTH, WIDTH );
    fprintf( output, "set yrange [-%d:%d]\n", HEIGHT, HEIGHT );
    fprintf( output, "set size ratio -1\n" );
    fprintf( output, "unset xtics\n" );
    fprintf( output, "unset ytics\n" );
}

static FILE *start_gnuplot() {

    FILE *output = NULL;
    if( (output = popen( "gnuplot","w" )) == NULL ) {
        fprintf( stderr, "error starting gnuplot!\n" );
        return NULL;
    }

    plot_init( output );

    return output;
}

static void gnuplot_close( FILE *stream ) {

    if( pclose( stream ) == -1 ) {
        fprintf( stderr,"Failed to close gnuplot!\n" );
    }
}
static FILE *global_output;

static void draw_line( 
        FILE *output, double x1, double y1, double x2, double y2 ) {
    fprintf( output, "plot [0:1] %f + %f * t, %f + %f * t notitle\n",
        x1, x2 - x1, y1, y2 - y1 );
    fflush(output);
}

static double x, y;
static double direction;
static int pendown;

static SCM tortoise_reset() {
    x = y = 0.0;
    direction = 0.0;
    pendown = 1;

    fprintf( global_output, "clear\n" );
    fflush( global_output );
    
    return SCM_UNSPECIFIED;
}

static SCM tortoise_pendown() { 
    SCM result = scm_from_bool( pendown );
    pendown = 1; 
    return result;
}
static SCM tortoise_penup() { 
    SCM result = scm_from_bool( pendown );
    pendown = 0;
    return result;
}

static SCM tortoise_turn( SCM degrees ) {
    const double value = scm_to_double( degrees );
    //stored internally in radians
    direction += M_PI / 180.0 * value;
    //Convert to degrees
    return scm_from_double( direction * 180.0 / M_PI );
}

static SCM tortoise_move( SCM length ) {
    double newX, newY;
    const double value = scm_to_double( length );
    newX = x + value * cos( direction );
    newY = y + value * sin( direction );
    
    if(pendown) {
        draw_line( global_output, x, y, newX, newY );
    }

    x = newX;
    y = newY;
    
    return scm_list_2( scm_from_double( x ), scm_from_double( y ) );
}

static void *register_functions( void* data ) {
    /* Add functions to guile enviornment */
    scm_c_define_gsubr("tortoise-reset", 0, 0, 0, &tortoise_reset);
    scm_c_define_gsubr("tortoise-penup", 0, 0, 0, &tortoise_penup);
    scm_c_define_gsubr("tortoise-pendown", 0, 0, 0, &tortoise_pendown);
    scm_c_define_gsubr("tortoise-turn", 1, 0, 0, &tortoise_turn);
    scm_c_define_gsubr("tortoise-move", 1, 0, 0, &tortoise_move);
    return NULL;
}

typedef struct {
    int argc;
    char **argv;
} passed_args;

void *run_scm_shell( void *const args ) {


    passed_args *a = (passed_args *) args;

    scm_with_guile( &register_functions, NULL );
    scm_shell( a->argc, a->argv );

    pthread_exit(NULL);
}


int main( int argc, char *argv[] ) {

    global_output = start_gnuplot();
    tortoise_reset();

    //Decouple the terminal REPL window from the gnuplot window
    //Without the threading anytime gnuplot draws it blocks the scheme terminal
    //so that no new commands can run
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE ); 

    passed_args args;
    args.argc = argc;
    args.argv = argv;

    pthread_create( &thread, &attr, run_scm_shell, &args );

    pthread_join( thread, NULL );
    pthread_attr_destroy( &attr );

    gnuplot_close( global_output );
    return EXIT_SUCCESS;
}

