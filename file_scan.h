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
// Name:    file_scan.h
// Notes:   header for file_scan module
//
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************


#ifndef __TGREP_FILE_SCAN_H__
#define __TGREP_FILE_SCAN_H__

//keep all file reading stuff in this module
int open_log_file(char *file_name);
void close_log_file();


//these will have a little bit of application knowledge in them so that
//they can start at the next highest point if a start time is missing and end at 
//the next prior time if the end is missing
off_t find_time_start_offset(int time);
off_t find_time_end_offset(int time);


//The start one is for verbosity, but the end one sets the internal limits of 
//the file that will be used for read calculations the side effect of these
//is to set the modules read limits.
off_t set_log_file_start_offset(void);
off_t set_log_file_end_offset(void);

//this will give us our unique identifier for this file based on it's first line
//so we can figure out if we have a pre-generated map for it  note, this will 
//malloc everytime, so the caller needs to be sure to free old values returned.
char *get_file_hash(void);


//provide a nice "dump" function to print everything out and keep track of all the 
//pointers
void dump_file_range(off_t dump_start_offset, off_t dump_end_offset); 
#endif
