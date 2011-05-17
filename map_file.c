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
// Name:    map_file.c
// Notes:   This houses the metadata about the log file.  It keeps track of
//          the first and last offset position for given times as well as 
//          whether or not those offsets are confirmed to be the start/end or
//          if they're just our current best guess.
//
//          The mapping list is internally stored as an ordered linked list.
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
#include <errno.h>



//******************************************************************************
// Project includes
//******************************************************************************
#include "map_file.h"
#include "console_output.h"



//******************************************************************************
// Module Specific Global Variables
//******************************************************************************
static map_item *map_head;



//******************************************************************************
//******************************************************************************
// Implimentation
//******************************************************************************
//******************************************************************************



//******************************************************************************
// Name:    create_new_map_item
// Notes:   Does one of 2 things, it will create a new map item if there doesn't
//          already exist one within the list.  Otherwise it will just return
//          the one that already exists.
//
//******************************************************************************  
map_item *create_new_map_item(int time)
{
   //start by just seeing if we already have an entry in our data structure
   map_item *temp = find_exact_map_item(time);
   if(temp!=NULL)
   {
      //easy button
      return temp;
   }
   //we don't have one already, let's do some adding
   
   //create a new map item;
   temp = malloc(sizeof(map_item));
   
   //default all elements
   temp->time = time;

   //64-bit offsets
   temp->starting_offset = -1;
   temp->ending_offset = (off_t) -1;
   temp->starting_offset_confirmed = 0;
   temp->ending_offset_confirmed = 0;
   temp->next = NULL;
   
   //add it into the list
   if(map_head == NULL)
   {
      map_head = temp;
   }
   else
   {
      //keep track of previous item so that we can update it's pointer
      //when iterator is the next largest time
      map_item *prev = map_head;
      map_item *iterator;
      
      for(iterator = map_head; iterator != NULL ; iterator = iterator->next)
      {
         if(iterator->time > temp->time)
         {
            break;
         }
         
         prev = iterator;
      }
      
      //take some extra care to make sure we do this right if we're trying
      //to update the head item
      if(prev == map_head && map_head->time > temp->time)
      {
         temp->next = map_head;
         map_head = temp;
      }
      else
      {
         prev->next = temp;
         temp->next = iterator;
      }      
   }
   console_print_debug("Map entry created for time <%d>.\n",temp->time);
   return temp;
}
   
   

//******************************************************************************
// Name:    find_exact_map_item
// Notes:   Returns a pointer to the map item that has teh EXACT time as asked
//          for.  The application is responsible for modifying the returned
//          value.  This way, the map file module doesn't need to have much
//          application logic in it.
//
//******************************************************************************  
map_item *find_exact_map_item(int time)
{
   map_item *iterator = NULL;
   for(iterator = map_head; iterator != NULL; iterator = iterator->next)
   {
      //break if it's early
      if(iterator->time == time)
      {
         break;
      }
   }
   return iterator;
}



//******************************************************************************
// Name:    find_prev_map_item
// Notes:   finds a map item with a lower time than the one specified, if one 
//          doesn't exist, we just return a null pointer.
//
//******************************************************************************
map_item *find_prev_map_item(int time)
{
   //handle the special case first
   if(time <= map_head->time)
   {
      return NULL;
   }
   
   //keep track of our iterator and the previous item
   map_item *iterator = NULL;
   map_item *prev = map_head;
   
   for(iterator = map_head; iterator != NULL; iterator = iterator->next)
   {
      if(iterator->time >= time)
      {
         break;
      }
      prev = iterator;
   }
   
   
   return prev;
}



//******************************************************************************
// Name:    find_next_map_item
// Notes:   finds a map item with the next highest time than the one specified, 
//          if one doesn't exist, we just return a null pointer.
//
//******************************************************************************
map_item *find_next_map_item(int time)
{
   map_item *iterator;
   
   for(iterator = map_head; iterator != NULL; iterator = iterator->next)
   {
      if(iterator->time > time)
      {
         break;
      }
   }
   
   //this will naturally be null when we search for the last item in the list
   return iterator;
}



//******************************************************************************
// Name:    create_map_file_directory
// Notes:   creates a directory for map files. Permissions allow for anyone
//          to use this directory.
//
//******************************************************************************
void create_map_file_directory(void)
{
   char* short_pre_search_folder = "/.tgrepmapfiles";
   char* full_pre_search_folder = NULL;

   char *home_path = getenv("HOME");
   int home_path_len = strlen(home_path);
   
   int short_pre_search_folder_len = strlen(short_pre_search_folder);
   int full_pre_search_len = home_path_len + short_pre_search_folder_len + 1;
   full_pre_search_folder = malloc(full_pre_search_len);
   strncpy(full_pre_search_folder,home_path,home_path_len);
   strncat(full_pre_search_folder,short_pre_search_folder,short_pre_search_folder_len);
   
   if(mkdir(full_pre_search_folder,0777))
   {
      console_print_info("Existing map file directory found.\n");
   }
   else
   {
      console_print_info("Created map file directory at <%s>.\n",full_pre_search_folder);
   }
}



//******************************************************************************
// Name:    print_map
// Notes:   Really just a debug function that we use to dump out our map file.
//
//******************************************************************************
void print_map(void)
{
   map_item *iterator;
   console_print_debug("Printing map file...\n");
   for(iterator = map_head; iterator != NULL; iterator = iterator->next)
   {
     console_print_debug("<t:%d, s:%lld(%d), e:%lld(%d)>\n",iterator->time,iterator->starting_offset,iterator->starting_offset_confirmed,iterator->ending_offset,iterator->ending_offset_confirmed);
   }
}




//******************************************************************************
// Name:    get_log_start_time
// Notes:   returns the lowest time that we have.  
//
//******************************************************************************
int get_log_start_time(void)
{
   if(map_head == NULL)
   {
      return -1;
   }
   return map_head->time;   
}



//******************************************************************************
// Name:    get_log_end_time
// Notes:   Returns the time of the highest entry we have
//
//******************************************************************************
int get_log_end_time(void)
{
   //this is a shitty way to get the end, but we only all this once at the 
   //start
   map_item *iterator;
   for(iterator = map_head; iterator->next != NULL; iterator = iterator->next)
   {
      //do something super funny and awesome...perhaps related to underpants
      //gnomes.  
   }
   //iterator->next == NULL which means it's the last entry in the ordered
   //list
   return iterator->time;
}



//******************************************************************************
// Name:    load_map_file
// Notes:   tries to pull in the pre-existing map file that has been made for a
//          file.
//
//******************************************************************************
void load_map_file(char *file_name)
{
   int read_count = 0;

   char *folder = "/.tgrepmapfiles/";
   char *home = getenv("HOME");
   char *full_path = NULL;
   
   int home_len = strlen(home);
   int file_len = strlen(file_name);
   int folder_len = strlen(folder);
   
   full_path = malloc(home_len + file_len + folder_len + 1);
   strcpy(full_path, home);
   strcat(full_path, folder);
   strcat(full_path,file_name);
   console_print_info("Using map file: %s\n",full_path);
   
   //I'm cheesing this a bit because i want the simplicity of a nice
   //formatted get line interface
   FILE *fd = fopen(full_path, "r" );
   if(fd!=NULL)
   {
      int t;
      off_t so;
      int so_c;
      off_t eo;
      int eo_c;
      long long int cs;
      console_print_info("Loading map file: %s\n",full_path);
      while(6==fscanf(fd, "%d %lld %d %lld %d %lld\n", &t, &so, &so_c, &eo, &eo_c, &cs))
      {
         if((t+so+so_c+eo+eo_c)==cs)
         {
            console_print_debug("Found map item for time: %d\n",t);   
            map_item *nm = create_new_map_item(t);
            if(nm != NULL)
            {
               read_count++;
               nm->starting_offset = so;
               nm->starting_offset_confirmed = so_c;
               nm->ending_offset = eo;
               nm->ending_offset_confirmed = eo_c;
            }
         }
      }
      console_print_info("Read in %d entries from map file.\n",read_count);
      fclose(fd);
   }
   else
   {
      console_print_info("Map file not found: %s\n",full_path);
   }
}



//******************************************************************************
// Name:    save_map_file
// Notes:   This dumps the map file to the specified file within the map file
//          directory
//
//          This actually keeps a simple additive checksum for everything in the
//          line to prevent someone from modifying the file by hand (it's
//          trivial to get around, but stops stupidity).
//
//******************************************************************************
void save_map_file(char *file_name)
{
   char output_buffer[1024];
   
   char *folder = "/.tgrepmapfiles/";
   char *home = getenv("HOME");
   char *full_path = NULL;
   
   int home_len = strlen(home);
   int file_len = strlen(file_name);
   int folder_len = strlen(folder);
   
   full_path = malloc(home_len + file_len + folder_len + 1);
   strcpy(full_path, home);
   strcat(full_path, folder);
   strcat(full_path,file_name);
   
   int fd = open(full_path, O_RDWR | O_TRUNC | O_CREAT, 0666);
   if(fd>0)
   {
      console_print_info("Saving map file: %s\n",full_path);
      map_item *iterator = map_head;
      for(;iterator!=NULL;iterator=iterator->next)
      {
         int t = iterator->time;
         off_t so = iterator->starting_offset;
         int so_c = iterator->starting_offset_confirmed;
         off_t eo = iterator->ending_offset;
         int eo_c = iterator->ending_offset_confirmed;
         long long int cs = t + so + so_c + eo + eo_c;
         sprintf(output_buffer,"%d %lld %d %lld %d %lld\n",t,so,so_c,eo,eo_c,cs);
         int resp = write(fd,output_buffer,strlen(output_buffer));
         
         //we're not going to do anything but output some debug messages here
         //if we're at all short on our write, the checksums will fail the
         //line/lines.
         if(resp < strlen(output_buffer))
         {
            console_print_debug("Only wrote %d of %d requested to map file\n.",resp,strlen(output_buffer));
         }
         
      }
      close(fd);
   }
   else
   {
      console_print_info("%s\n",strerror( errno ));
   }      
}

