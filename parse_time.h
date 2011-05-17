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
// Name:    parse_time.h
// Notes:   header for the parse_time module
//
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//******************************************************************************



#ifndef __TGREP_PARSE_TIME_H__
#define __TGREP_PARSE_TIME_H__

//seconds per day is useful when we're adding the 1 day offsets
//to our search times and whatnot.
#define SECONDS_PER_DAY (24*60*60)

//this is allowed to have dashes and is more forgiving
//on the leading zeros, which will allow for things like
//tgrep 4 to be interpreted as tgrep 04:00:00
int is_valid_search_time(char *time_string);

//this is a bit more strict, must be the start of the line,
//thus it expects ccc_[d/_]d_dd:dd:dd.  This really only protects against
//some screwed up log file and I may not use it since the 2 options are
//(1)pass the bad value to the output and hope it gets picked off by a follow
//up grep/cut/whatever or (2) fail and give NO results.
int is_valid_log_time(char *time_string);

//parse the search string into a start and end time.  the times are passed in
//by reference so we can return times and a pass/fail
int parse_search_string(char *time_string, int *start_time, int *end_time);

//this will return the time that a log file line refers to.
int parse_log_time(char *time_string, int *log_time);

//this is goofy, but I want log times to be naturally returned with the correct
//offset rather than doing it in application for abstraction reasons.  no 
//less-than-48-hour period has 2 of the same DAY so we'll keep that around and
//use it to offset results to parse_log_time

//I chose doing it from the string so that no one else would have to do any
//string parsing.
int set_log_time_start_day(char *time_string);


#endif
