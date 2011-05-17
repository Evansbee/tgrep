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
// Name:    map_file.h
// Notes:   header for the map file
//
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************



#ifndef __TGREP_MAP_FILE_H__
#define __TGREP_MAP_FILE_H__

struct _map_item {
   
   //time as reference to 0 from starting day
   int time;
   
   //position of the first character of the first line
   off_t starting_offset;
   
   //position of the /n of the last line
   off_t ending_offset;
   
   //we should always try to keep track of whether or not we know that we've
   //found the start and end.  Otherwise we're just using guesses
   int starting_offset_confirmed;
   int ending_offset_confirmed;

   //used for the run time data structure so we can insert in the middle to
   //keep the list nice and ordered.
   struct _map_item *next;
};

typedef struct _map_item map_item;
   
   
//creating a new map item will create a new map item for time in the correct
//position and return a pointer to it.  If the time already exists it will
//return the existing map item.   
map_item *create_new_map_item(int time);   
   
//we're passing around real pointers to the structure so we should be able to 
//just modify this in place as I don't want the map items to have a lot of 
//programming logic in them, just scanning and ordering  
map_item *find_exact_map_item(int time);
map_item *find_prev_map_item(int time);
map_item *find_next_map_item(int time);   

//pre cooked files need to be stored so we can speed up the lookup
void create_map_file_directory(void);

//not sure where this needs to go, it's more related to the file, but the
//map structure actually HAS the information handy
int get_log_start_time(void);
int get_log_end_time(void);

//debugging tool used to verify the map file creation is going as planned
//shouldn't be used for real builds!
void print_map(void);

//now we're going to make the file functions in order to pre parse the data on
//run.
void load_map_file(char *file_name);
void save_map_file(char *file_name);
#endif
