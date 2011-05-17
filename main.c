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
// Name:    main.c
// Notes:   houses the application itself
//
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************



//******************************************************************************
// Library includes
//******************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>



//******************************************************************************
// Project includes
//******************************************************************************
#include "parse_time.h"
#include "map_file.h"
#include "file_scan.h"
#include "console_output.h"



//******************************************************************************
// Module Specific #defines
//******************************************************************************
#define DEFAULT_LOG_FILE "/logs/haproxy.log"



//******************************************************************************
//******************************************************************************
// Implimentation
//******************************************************************************
//******************************************************************************

//******************************************************************************
// Name:    main
// Notes:   basically the coolest. function. ever.
//
//******************************************************************************
int main(int argc, char **argv)
{
    //first just create the map file directory to make sure the application
    //can save it's stored searches
    create_map_file_directory();

    //process the options that we have.
    int opt;
    while((opt = getopt(argc,argv,"vdh")) != -1)
    {
        switch(opt)
        {
        case 'v':
            console_enable_info();
            break;
        case 'd':
            console_enable_debug();
            break;
        case 'h':
            console_print_help();
            return 0;
            break;
        case '?':
            //i know that getopt dumps its own little message but i want to
            //have it in RED!
            console_print_error("Unknown option -%c.\n",optopt);
            console_print_error("Try %s -h for help.\n",argv[0]);
            return 0;
            break;
        }
    }

    //now process the times and filename (if present)
    int i;
    int found_file_name = 0;

    //so I double scan the options because I want to make sure that the file
    //gets opened before I start working on the search string, that way I
    //can just process the search and end times in the loop
    for(i = optind; i < argc; i++)
    {
        if(is_valid_search_time(argv[i]))
        {
            console_print_debug("Ignoring found search string for now.\n");
        }
        else
        {
            if(open_log_file(argv[i]) >0)
            {
                found_file_name =1;
                break;
            }
            else
            {
                console_print_error("Invalid log file specification.\n");
                return 0;
            }
        }
    }


    //we didn't find a filename within our list of args, so we need to just
    //use the default.  This is sort of interesting, i'm up in the air on
    //whether to default to this if you get a BAD name in the arg list...
    //I'll probably just return then so that you only get this if you haven't
    //added an arg.
    if(found_file_name == 0)
    {
        if(open_log_file(DEFAULT_LOG_FILE) > 0)
        {
            //good!
        }
        else
        {
            console_print_error("Could not open: %s\n",DEFAULT_LOG_FILE);
            return 0;
        }
    }

    //at this point we know a bit about the file, so let's grab some info so
    //we can cook our data a bit

    int file_start_time = get_log_start_time();
    if(file_start_time < 0)
    {
        console_print_error("Invalid log file.\n");
        return 0;
    }
    //the end time already has it's day added, if it's needed.  all handled
    //inline
    int file_end_time = get_log_end_time();
    if(file_end_time < 0 || file_end_time < file_start_time)
    {
        //wtf
        console_print_error("Invalid log file.\n");
        return 0;
    }

    char *file_hash = get_file_hash();
    load_map_file(file_hash);


    //SECOND LOOP
    //figure out the search times.
    int search_start_time = 0;
    int search_end_time = 0;
    int search_duration = 0;

    //keep track of whether we need to run a second search due to some
    //goofy ambiguity inthe log files.
    int second_search_start_time = 0;
    int second_search_end_time = 0;
    int found_search_string = 0;
    for(i = optind; i < argc; i++)
    {
        if(is_valid_search_time(argv[i]))
        {
            found_search_string = 1;
            console_print_info("Found search time: \"%s\"\n",argv[i]);

            //so, we've got our search times, finally! let's get them in and
            //process them a bit so that they make sense within the context of our
            //application and the current log file.
            //i.e if there's more than 24 hours
            //example, log file covers 6:00 AM DAY 1 - 8:00 AM DAY 2 and the search
            //is for something like 6:30-7:00, this should be broken in 2, i'll add a
            //command line for no breaking of searches if I decide that this is actually
            //a bad idea.

            //so this is really a baseline
            parse_search_string(argv[i], &search_start_time, &search_end_time);

            //start with the end time larger than the start time to make life easy.
            search_duration = search_end_time - search_start_time;
            while(search_duration < 0)
            {
                search_duration += SECONDS_PER_DAY;
            }

            second_search_start_time = search_start_time + SECONDS_PER_DAY;

            search_end_time = search_start_time + search_duration;
            second_search_start_time = search_start_time + SECONDS_PER_DAY;
            second_search_end_time = second_search_start_time + search_duration;

            if(search_start_time < file_start_time)
            {
                search_start_time = file_start_time;
            }

            if(search_end_time > file_end_time)
            {
                search_end_time = file_end_time;
            }
            console_print_debug("Start: %d End: %d.\n",search_start_time, search_end_time);
            if(search_start_time > search_end_time)
            {
                console_print_debug("Search 1 is INvalid.\n");
            }
            else
            {
                console_print_debug("Search 1 is valid.\n");
            }

            if(second_search_start_time < file_start_time)
            {
                second_search_start_time = file_start_time;
            }

            if(second_search_end_time > file_end_time)
            {
                second_search_end_time = file_end_time;
            }
            console_print_debug("Start: %d End: %d.\n",second_search_start_time, second_search_end_time);
            if(second_search_start_time > second_search_end_time)
            {
                console_print_debug("Search 2 is INvalid.\n");
            }
            else
            {
                console_print_debug("Search 2 is valid.\n");
            }

            //donezo.
            break;
        }

    }


    //if we didn't find a search string, do nothing
    if(found_search_string == 0)
    {
        console_print_error("No search string found.\n");
        return 0;
    }

    //perform the searches if we need to.
    if(search_start_time <= search_end_time)
    {
        console_print_info("Scanning for times %d - %d.\n",search_start_time, search_end_time);
        dump_file_range(find_time_start_offset(search_start_time),find_time_end_offset(search_end_time));
    }

    if(second_search_start_time <= second_search_end_time)
    {
        console_print_info("Scanning for times %d - %d.\n",second_search_start_time, second_search_end_time);
        dump_file_range(find_time_start_offset(second_search_start_time),find_time_end_offset(second_search_end_time));
    }

    //store the map file
    save_map_file(file_hash);
    return 1;
}





























