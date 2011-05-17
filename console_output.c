//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//******************************************************************************
//******************************************************************************
// Name:    console_output.c
// Notes:   This is a front-end to all console debug/info/error printing.
//          The 3 print types will be indidually selectable via a flag passed in
//          at run-time.  They will print in color on vt-100 style terminals
//          which is probably the vast majority of the things that this program
//          will be run on.
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************



//******************************************************************************
// Library includes
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



//******************************************************************************
// Project includes
//******************************************************************************
#include "console_output.h"



//******************************************************************************
// Module Specific Global Variables
//******************************************************************************

static int debug_enabled = 0;
static int info_enabled = 0;
static int error_enabled = 1;



//******************************************************************************
//******************************************************************************
// Implimentation
//******************************************************************************
//******************************************************************************

//******************************************************************************
// Name:    console_enable_debug
// Notes:   Enables the debug printing (-d)
//
//******************************************************************************
void console_enable_debug()
{
    debug_enabled = 1;
}



//******************************************************************************
// Name:    console_enable_info
// Notes:   Enables info printing (-v)
//
//******************************************************************************
void console_enable_info()
{
    info_enabled = 1;
}



//******************************************************************************
// Name:    console_enable_error
// Notes:   Enables error printing.  This will be enabled by default due to the
//          global variable up above, but I wanted to have a way to supress
//          errors even though they don't get redirected.
//
//******************************************************************************
void console_enable_error()
{
    error_enabled = 1;
}



//******************************************************************************
// Name:    console_disable_debug
// Notes:   disables debug printing (default)
//
//******************************************************************************
void console_disable_debug()
{
    debug_enabled = 0;
}



//******************************************************************************
// Name:    console_disable_info
// Notes:   disables info printing (default
//
//******************************************************************************
void console_disable_info()
{
    info_enabled = 0;
}



//******************************************************************************
// Name:    console_disable_error
// Notes:   disables error printing, not sure why you'd want this since things
//          would fail silently, but meh
//
//******************************************************************************
void console_disable_error()
{
    error_enabled = 0;
}



//******************************************************************************
// Name:    console_print_debug
// Notes:   prints a formatted debug message in GREEN/BOLD
//
//******************************************************************************
void console_print_debug(const char *format, ...)
{
    //check if we're supposed to print this stuff
    if(debug_enabled)
    {
        va_list arg;
        va_start(arg,format);
        fprintf(stderr, "\033[01;32mDEBUG: ");
        vfprintf(stderr, format, arg);
        va_end(arg);
        fprintf(stderr,"\033[0m");
    }
}



//******************************************************************************
// Name:    console_print_info
// Notes:   prints a formatted info message in BLUE/BOLD
//
//******************************************************************************
void console_print_info(const char *format, ...)
{
    //check if we're supposed to print this stuff
    if(info_enabled)
    {
        va_list arg;
        va_start(arg,format);
        fprintf(stderr, "\033[01;36mINFO: ");
        vfprintf(stderr, format, arg);
        va_end(arg);
        fprintf(stderr,"\033[0m");
    }
}



//******************************************************************************
// Name:    console_print_info
// Notes:   prints a formatted error message in RED/BOLD
//
//******************************************************************************
void console_print_error(const char *format, ...)
{
    //check if we're supposed to print this stuff
    if(error_enabled)
    {
        va_list arg;
        va_start(arg,format);
        fprintf(stderr, "\033[01;31mERROR: ");
        vfprintf(stderr, format, arg);
        va_end(arg);
        fprintf(stderr,"\033[0m");
    }
}



//******************************************************************************
// Name:    console_print_help
// Notes:   prints the help information if -h is passed to the program
//          it would be a good idea to actually pass the program name as a
//          variable, but it's not really important.
//
//******************************************************************************
void console_print_help()
{
    fprintf(stderr,"Usage: tgrep [OPTION]... SEARCH_PATTERN [FILE]\n");
    fprintf(stderr,"Search for SEARCH_PATTERN in FILE or /logs/haproxy.log\n");
    fprintf(stderr,"SEARCH_PATTERN is, by default, a basic hh:mm:ss timestamp\n");
    fprintf(stderr,"Example: tgrep 12:13:16-14:32:44 ~/logs/super_awesome_log.log\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"  -v,                       Print more verbose output to standard error\n");
    fprintf(stderr,"  -d,                       Print debug output to standard error. DEBUG ONLY\n");
    fprintf(stderr,"  -h                        Prints this helpful help message!\n");
}




