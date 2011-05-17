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
// Name:    file_scan.c
// Notes:   This module is responsible for all log file interation and some
//          meta data about the file, like it's start and end times.  This
//          module can give back a hash of the selected file to check for a
//          preexisting map if it's information from the map module.
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>



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

//4096, sure...  This could probably be optimized by aligning our reads to
//sector boundries to keep the HD from seeking around too much, oh well.
#define READ_BUFFER_SIZE   (4096)
#define READ_OFFSET        (READ_BUFFER_SIZE/2)



//******************************************************************************
// Module Specific Global Variables
//******************************************************************************
//stores the handle of our logfile
static int log_file = -1;

//Some data about the file itself
static struct stat log_file_stat;
static off_t file_end_offset;


//declare our read buffer
static char read_buffer[READ_BUFFER_SIZE];

//this is going to store our current buffer offsets
static off_t read_start_offset = (off_t)0;
static off_t read_end_offset = (off_t)0;



//******************************************************************************
// Module Specific Functions
//******************************************************************************
static char *read_from_offset_start(off_t read_offset);
static char *read_from_offset_center(off_t read_offset);
static void parse_times_around_offset(off_t offset);
static off_t _get_confirmed_start_offset(int time);
static int _start_offset_found(int time);
static off_t _get_confirmed_end_offset(int time);



//******************************************************************************
//******************************************************************************
// Implimentation
//******************************************************************************
//******************************************************************************

//******************************************************************************
// Name:    open_log_file
// Notes:   This needs to be the first thing that happens to the log file, as it
//          involves all the initialization.  Once you leave this function we
//          will have information about the files start and end times
//
//******************************************************************************
int open_log_file(char *file_name)
{
    if(file_name == NULL || log_file != -1)
    {
        console_print_debug("Could not open logfile.\n");
        return -1;
    }

    log_file = open(file_name, O_RDONLY);

    if(log_file <= 0)
    {
        return -1;
    }

    //The set log time start day will help us later with parsing the log entries
    if(set_log_time_start_day(read_from_offset_start(0)))
    {
        //now just compile some data about the file so we can start searching it
        file_end_offset = lseek(log_file, 0, SEEK_END);
        set_log_file_start_offset();
        set_log_file_end_offset();
        stat(file_name,&log_file_stat);
        console_print_info("Opened %s\n",file_name);
        return log_file;
    }
    else
    {
        console_print_error("Logfile of improper format.\n");
        close(log_file);
        return -1;
    }
}



//******************************************************************************
// Name:    close_log_file
// Notes:   Closes the log file.
//
//******************************************************************************
void close_log_file()
{
    if(log_file == -1)
    {
        console_print_error("Tried to close() a non-opened logfile.\n");
        return;
    }
    close(log_file);
}



//******************************************************************************
// Name:    _start_offset_found
// Notes:   This is really just a wrapper function that makes the while loop
//          below a bit more self documenting.  Originally it was 2 seperate
//          functions, but that was not needed
//
//******************************************************************************
static int _start_offset_found(int time)
{
    return (_get_confirmed_start_offset(time) != -1);
}



//******************************************************************************
// Name:    find_time_start_offset
// Notes:   This function is part of the meat and potatoes of the log search.
//          this will attempt to find the first entry of the given time, OR any
//          start time after that if an exact match can't be found.  This
//          function gets called for both the END and START times, but when
//          called to find the end of a range, it searches for range_end+1's
//          start time, which based on the map_file architecture, gives the same
//          amount of information
//
//******************************************************************************
off_t find_time_start_offset(int time)
{
    off_t target_offset;
    map_item *a, *b;

    while(!_start_offset_found(time))
    {
        b = find_exact_map_item(time);
        if(b == NULL)
        {
            b=find_next_map_item(time);
            if(b==NULL)
            {
                return -1;
            }
        }
        a = find_prev_map_item(time);
        if(a==NULL)
        {
            a = find_next_map_item(time);
            if(a->starting_offset == 0 && a->starting_offset_confirmed == 1)
            {
                return (off_t)0;
            }
            else
            {
                return -1;
            }
        }
        console_print_debug("Searching For %d With Bound Times: %d - %d\n",time,a->time,b->time);
        //this is sort of a special case since doing a straight interpolation
        //if our time is at the bounds would result in a problem (we would still
        //find the value since we center our read call around the target, thus we
        //would walk backwards through the file, but this would be O(n) for local
        //searching, that sucks, let's degrade to the O(log(n)) binary search
        if(time == b->time)
        {
            target_offset = a->ending_offset + ((b->starting_offset - a->ending_offset)/2);
        }
        else
        {
            //this should be a bit speedier due to the interpolative search
            //O(log(log(n)) for really flat data...which isn't going to happen, but
            //we're closer to that than we are the O(n) section of the interpolaive
            //search

            target_offset = a->ending_offset;

            double search_percent = (double)(time - a->time) / (double)(b->time - a->time);

            target_offset += search_percent * (b->starting_offset - a->ending_offset);

        }
        //now go do the work!
        parse_times_around_offset(target_offset);

    }
    return _get_confirmed_start_offset(time);
}



//******************************************************************************
// Name:    find_time_end_offset
// Notes:   As mentioned above, this is just a wrapper with some modifications
//          around the start time function, using hte same function for both
//          makes the code more simple.
//
//******************************************************************************
off_t find_time_end_offset(int time)
{
    //this is bodgey, but it's correct given the algorithm
    find_time_start_offset(time+1);

    //now just dump out th eend of file
    return _get_confirmed_end_offset(time);
}



//******************************************************************************
// Name:    get_file_hash
// Notes:   This will give us a mostly-unique identifier based on a few
//          different pieces of information. First we compact all the numbers
//          in the first line then strap on the file's modification date (and
//          a short file extension).  Note: this allocates memory so the caller
//          needs to be careful about freeing it.
//
//******************************************************************************
char *get_file_hash(void)
{
    if(log_file == -1)
    {
        return NULL;
    }
    char *working = read_from_offset_start(0);
    if(working==NULL)
    {
        return NULL;
    }
    ssize_t line_length = strcspn(working,"\n");
    char *hash_name = malloc(64);
    int all_numbers=0;

    if(line_length > 0)
    {
        int read_ptr=0;
        for(; read_ptr < line_length; read_ptr++)
        {
            if(working[read_ptr] >= '0' && working[read_ptr] <= '9')
            {
                all_numbers = (all_numbers * 10) + (working[read_ptr] - '0');
            }
        }

        sprintf(hash_name,".%d%lld.map",all_numbers,(long long int)log_file_stat.st_mtime);
        console_print_debug("Logfile hashes to %s.\n",hash_name);
        return hash_name;
    }
    return NULL;
}



//******************************************************************************
// Name:    read_from_offset_start
// Notes:   This is the read function that is (eventually) used to get info
//          out of the log file.
//
//******************************************************************************
static char *read_from_offset_start(off_t read_offset)
{
    ssize_t read_size;

    read_offset = lseek(log_file,read_offset,SEEK_SET);
    read_size = read(log_file,read_buffer,READ_BUFFER_SIZE);

    if(read_size != (ssize_t)-1)
    {
        read_start_offset = read_offset;
        read_end_offset = read_start_offset + (off_t)read_size;
        return read_buffer;
    }
    else
    {
        return NULL;
    }
}



//******************************************************************************
// Name:    read_from_offset_center
// Notes:   This function is pretty important.  Because of the way we're
//          finding our predicted offsets, we need the prediction to be the
//          center of the read data, that way we can get information from
//          AROUND the target to find starts/ends.  This just does some basic
//          bounds checking to make sure the whole buffer is in the file.
//
//******************************************************************************
static char *read_from_offset_center(off_t read_offset)
{
    if(read_offset >= READ_OFFSET)
    {
        read_offset -= (off_t)READ_OFFSET;
    }
    else
    {
        read_offset = (off_t) 0;
    }

    return read_from_offset_start(read_offset);
}



//******************************************************************************
// Name:    parse_times_around_offset
// Notes:   This fuction sucks out all the information from a dumped readbuffer
//          and puts it into a usable format in the map files.  It's responsible
//          for pushing around start and end offsets for specific log times
//          while confirming their start/end points.
//
//******************************************************************************
static void parse_times_around_offset(off_t read_offset)
{
    char *working = read_from_offset_center(read_offset);
    char *start = working;
    map_item *current_map_item = NULL;
    map_item *last_map_item = NULL;

    int current_time;
    off_t read_ptr = 0;
    //blarg
    if(working==NULL)
    {
        return;
    }

    //we want to squeeze every drop of blood we can from this buffer so we'll
    //run her right up to the end if we can.
    while(read_ptr < (read_end_offset - read_start_offset))
    {

        //verify that our log line is a real time
        if(is_valid_log_time(working))
        {
            //grab the time on that log line
            if(parse_log_time(working,&current_time))
            {

                //hunt down the current item
                //This looks goofy since I have a "find_exact" function
                //available.  However, create new map item returns an existing one
                //and creates a new one automagically if it doens't have it.
                //perhaps a new name is in order.
                current_map_item = create_new_map_item(current_time);

                //try to update the start by moving it LOWER
                if((current_map_item->starting_offset > read_ptr + read_start_offset || current_map_item->starting_offset == -1) && read_ptr != -1)
                {
                    current_map_item->starting_offset = read_ptr+read_start_offset;
                    if(current_map_item->starting_offset == 0)
                    {
                        current_map_item->starting_offset_confirmed = 1;
                    }
                }

                //find the end of line
                read_ptr += strcspn(working,"\n");

                //clamp the end of line if it's larger than our buffer
                if(read_ptr > (read_end_offset - read_start_offset))
                {
                    read_ptr = read_end_offset - read_start_offset;
                }

                //update the current end of line.
                if(current_map_item->ending_offset < read_ptr + read_start_offset)
                {
                    current_map_item->ending_offset = read_ptr + read_start_offset;

                    //-1 to account for the eof char
                    if(current_map_item->ending_offset == (file_end_offset - 1))
                    {
                        current_map_item->ending_offset_confirmed = 1;
                    }
                }

                //did we process more than one already?
                if(last_map_item != NULL)
                {
                    //if the time between 2 consecutive map entries are different
                    //we've found a boundry, update our confirmed limits.
                    if(last_map_item->time != current_map_item->time)
                    {
                        last_map_item->ending_offset_confirmed = 1;
                        current_map_item->starting_offset_confirmed = 1;
                    }
                }

                //update the last item
                last_map_item = current_map_item;

            }
        }
        else
        {
            //find the end of line
            read_ptr += strcspn(working,"\n");

            //clamp the end of line if it's larger than our buffer
            if(read_ptr > (read_end_offset - read_start_offset))
            {
                read_ptr = read_end_offset - read_start_offset;
            }
        }
        read_ptr++;
        working = start + read_ptr;
    }
}



//******************************************************************************
// Name:    _get_confirmed_start_offset
// Notes:   this returns a certain start address for the given time, or -1 if
//          we don't have it.  This does some nice things for us like
//          confirming a start address even if the entry is missing.  If this
//          this function and I were in prison together, I'd shiv someone for it
//
//******************************************************************************
static off_t _get_confirmed_start_offset(int time)
{
    map_item *target = find_exact_map_item(time);

    //target != null means we've got an exact entry for our time, let's see if
    //we can confirm our starting time
    if(target!=NULL)
    {
        if(target->starting_offset_confirmed)
        {
            return target->starting_offset;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        //this case is a bit trickier we need to verify that we've found the
        //end of the previous element before our start time and the starting
        //element of the next time and that they're next to each other, otherwise
        //we could just be missing our time.
        map_item *prev = find_prev_map_item(time);
        map_item *post = find_next_map_item(time);

        if(prev != NULL && post != NULL)
        {
            if(   prev->ending_offset_confirmed &&
                    post->starting_offset_confirmed &&
                    prev->ending_offset == post->starting_offset -1)

            {
                return post->starting_offset;
            }
            else
            {
                return -1;
            }
        }
        else if(prev==NULL)
        {
            if(post->starting_offset == 0 && post->starting_offset_confirmed)
            {
                return 0;
            }
        }
        else if(prev==NULL)
        {
            return -1;
        }
        return -1;
    }
}



//******************************************************************************
// Name:    _get_confirmed_end_offset
// Notes:   Same as above, but for end offsets, the prison position still
//          applies
//
//******************************************************************************
static off_t _get_confirmed_end_offset(int time)
{
    map_item *target = find_exact_map_item(time);

    //target != null means we've got an exact entry for our time, let's see if
    //we can confirm our starting time
    if(target!=NULL)
    {
        if(target->ending_offset_confirmed)
        {
            return target->ending_offset;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        //this case is a bit trickier we need to verify that we've found the
        //end of the previous element before our start time and the starting
        //element of the next time and that they're next to each other, otherwise
        //we could just be missing our time.
        map_item *prev = find_prev_map_item(time);
        map_item *post = find_next_map_item(time);
        if(prev != NULL && post != NULL)
        {
            if(   prev->ending_offset_confirmed &&
                    post->starting_offset_confirmed &&
                    prev->ending_offset == post->starting_offset -1)

            {
                return prev->ending_offset;
            }
            else
            {
                return -1;
            }
        }
        else if(prev==NULL)
        {
            return -1;
        }
        else if(post==NULL)
        {
            if(prev->ending_offset == file_end_offset-1 && prev->ending_offset_confirmed)
            {
                return file_end_offset -1;
            }
        }
        return -1;

    }
}




//******************************************************************************
// Name:    set_log_file_start_offset
// Notes:   The start offset is always going to be 0 for these cases, but this
//          utility function has a nice side effect of parsing a few log entries
//          around the zero point.
//
//******************************************************************************
off_t set_log_file_start_offset(void)
{
    parse_times_around_offset(0);
    return 0;
}




//******************************************************************************
// Name:    set_log_file_end_offset
// Notes:   Same as the start offset finder, but (you guessed it!) for the end
//          of the file.
//
//******************************************************************************
off_t set_log_file_end_offset(void)
{
    off_t end = lseek(log_file,0,SEEK_END);
    parse_times_around_offset(end);
    return 0;
}



//******************************************************************************
// Name:    dump_file_range
// Notes:   this actually dumps the desired range to stdout.
//
//******************************************************************************
void dump_file_range(off_t dump_start_offset, off_t dump_end_offset)
{
    off_t temp;
    size_t dump_size; //heh

    //protect us from goofy cases
    if(dump_start_offset == -1 || dump_end_offset == -1 || dump_end_offset < dump_start_offset)
    {
        return ;
    }

    //git'r'done
    temp = lseek(log_file,dump_start_offset,SEEK_SET);
    console_print_info("Printing output %lld - %lld.\n",dump_start_offset, dump_end_offset);
    while(temp < dump_end_offset)
    {
        //make sure we only read as far as we need to.
        if(dump_end_offset - temp > READ_BUFFER_SIZE)
        {
            dump_size = (size_t)READ_BUFFER_SIZE;
        }
        else
        {
            dump_size = (size_t)(dump_end_offset - temp);
        }
        dump_size = read(log_file, read_buffer, dump_size);

        fwrite(read_buffer, 1, dump_size, stdout);
        temp += (off_t)dump_size;
    }
    fwrite("\n",1,1,stdout);
    fflush(stdout);

}








