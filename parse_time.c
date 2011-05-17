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
// Name:    parse_time.c
// Notes:   This file has the helper functions to parse the log and search
//          times.  Times are represented as number of seconds past 00:00:00 on
//          the first log file day
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************



//******************************************************************************
// Library includes
//******************************************************************************
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



//******************************************************************************
// Project includes
//******************************************************************************
#include "parse_time.h"



//******************************************************************************
// Module Specific #defines
//******************************************************************************

//Little tools to make the code easier to read and more consise
#define IS_CHAR(x) (((x | 0x20) >= 'a') && ((x | 0x20) <= 'z'))
#define IS_NUM(x) (x >='0' && x <='9')
#define IS_SPACE(x) (x == ' ')
#define IS_COLON(x) (x == ':')



//******************************************************************************
// Module Specific Global Variables
//******************************************************************************

//this keeps track of the first and second log days (or one log day)
static int log_time_start_day = -1;

//internal helpers
static int parse_search_time(char *time_string, int pad);
static int parse_log_day(char *time_string);



//******************************************************************************
//******************************************************************************
// Implimentation
//******************************************************************************
//******************************************************************************



//******************************************************************************
// Name:    is_valid_search_time
// Notes:   This file just checks to make sure that the entry is a valid search
//          time, it's pretty forgiving and it's used to determine which cmd
//          line arg is the search time.
//
//******************************************************************************
int is_valid_search_time(char *time_string)
{
    if(time_string == NULL)
    {
        return 0;
    }

    int i;

    int digit_count = 0;
    int colon_count = 0;
    int hyphen_count = 0;

    //simple parse of the string making sure there's nothing super out of the
    //ordinary about it
    for(i=0; time_string[i]!= '\0'; i++)
    {
        if(time_string[i] >= '0' && time_string[i] <= '9')
        {
            digit_count++;
        }
        else if(time_string[i] == ':')
        {
            colon_count++;
        }
        else if(time_string[i] == '-')
        {
            hyphen_count++;
        }
        else
        {
            return 0;
        }
    }

    //again, make sure no one does anything stupid.  This really only prevents
    //someone from doing `tgrep -----:::::0000000000000000000000:::::------' and
    //hoping it does something logical
    if(hyphen_count > 1 || colon_count > 4 || digit_count > 12)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}



//******************************************************************************
// Name:    is_valid_log_time
// Notes:   The start of a log file line is very rigidly defined, this makes
//          sure that the line we're looking at matches the log file format
//
//******************************************************************************
int is_valid_log_time(char *time_string)
{
    if(time_string == NULL)
    {
        //fail.
        return 0;
    }

    int i = 0;

    //note: there's probably a more clever way to do this, but whatever...
    //constantly try to fail the string.

    //check the month portion
    if(!IS_CHAR(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_CHAR(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_CHAR(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_SPACE(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_SPACE(time_string[i])&&!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_SPACE(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_COLON(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_COLON(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }
    i++;
    if(!IS_NUM(time_string[i]))
    {
        return 0;
    }

    //WIN!
    return 1;
}



//******************************************************************************
// Name:    parse_search_string
// Notes:   Takes in the string that's provided on the command line and searches
//          it for the start and stop times of the search.  The times returned
//          are non-descript (they don't line up with a day), so the application
//          has to do something with them that makes sense.
//
//******************************************************************************
int parse_search_string(char *time_string, int *start_time, int *end_time)
{
    if(time_string == NULL)
    {
        return 0;
    }

    //need to keep 2 points because strtok won't nest right with our parse
    //time function -- stupid side-effects.
    char *start_string;
    char *end_string;

    //see if we're given a range or if we have to take an implied range


    if(strchr(time_string,'-') != NULL)
    {

        //real range given, delimiter doesn't really matter on the second call
        start_string = strtok(time_string,"-");
        end_string   = strtok(NULL, "-");

        if(start_string == NULL || end_string == NULL)
        {
            return 0;
        }

        *start_time = parse_search_time(start_string, 0);
        *end_time   = parse_search_time(end_string, 59);
    }
    else
    {

        //this is going to blow apart the time_string so we need to copy it first
        end_string = malloc(strlen(time_string)+1);
        if(end_string == NULL)
        {
            return 0;
        }
        //duplicate the string before : changes to \0 everywhere.

        strcpy(end_string, time_string);

        *start_time = parse_search_time(time_string, 0);

        *end_time = parse_search_time(end_string, 59);


        //free up the mem we created
        free(end_string);
    }
    return 1;
}



//******************************************************************************
// Name:    parse_log_time
// Notes:   Grab the time from the log entry.  This is offset by 24 hours if the
//          day on the log entry doesn't match the first day on the log entry.
//          Note, this falls apart the second the logs rotate with a frequency
//          that causes more than one midnight crossing (for this example we
//          were told to ignore that.
//
//******************************************************************************
int parse_log_time(char *time_string, int *log_time)
{
    if(time_string == NULL)
    {
        return 0;
    }

    int log_day = parse_log_day(time_string);
    if(log_day == -1)
    {
        return 0;
    }

    if(log_day != log_time_start_day)
    {
        *log_time = SECONDS_PER_DAY;
    }
    else
    {
        *log_time = 0;
    }

    //hackey, but I don't want to write a non-padding version of the parse
    //time function, feels wasteful.
    *log_time += parse_search_time(time_string + 7, 0);
    return 1;
}



//******************************************************************************
// Name:    set_log_time_start_day
// Notes:   sets up the log start day.  It parses the string itself to make it
//          more straightforward to callers.
//
//******************************************************************************
int set_log_time_start_day(char *time_string)
{
    if(time_string == NULL)
    {
        return 0;
    }

    log_time_start_day = parse_log_day(time_string);
    return 1;
}



//******************************************************************************
// Name:    parse_search_time
// Notes:   parses the search time with a padding.  For example if you enter
//          the string "6" it will actualy treat it like 06:PAD:PAD
//
//******************************************************************************
static int parse_search_time(char *time_string, int pad)
{
    int h=0, m=pad, s=pad;
    char *working;
    char *offset;
    if(time_string == NULL)
    {
        return 0;
    }
    working = malloc(strlen(time_string)+1);
    strcpy(working,time_string);
    offset = working;

    offset = strtok(working, ":");
    if(offset!=NULL)
    {
        h = atoi(offset);
    }
    offset = strtok(NULL, ":");
    if(offset!=NULL)
    {
        m = atoi(offset);
    }
    offset = strtok(NULL, ":");
    if(offset!=NULL)
    {
        s = atoi(offset);
    }
    return (h * 3600) + (m * 60) + s;
}



//******************************************************************************
// Name:    parse_log_day
// Notes:   returns a decimal representation of the log day
//
//******************************************************************************
static int parse_log_day(char *time_string)
{
    int log_day = 0;
    //i really don't want this to stuff \0s into the line we're dealing with
    //so i'll just copy the digits.
    if(time_string == NULL)
    {
        //yuck
        return -1;
    }

    if(IS_NUM(time_string[4]))
    {
        log_day = (time_string[4] - '0') * 10;
    }

    if(IS_NUM(time_string[5]))
    {
        log_day+=time_string[5] - '0';
        return log_day;
    }
    else
    {
        //yuck
        return -1;
    }

}

