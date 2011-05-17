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
// Name:    console_output.h
// Notes:   Header for console_output module
//
//
// Rev:     17-Feb-2011 Initial Rev
//
//******************************************************************************
//****************************************************************************** 

#ifndef __TGREP_CONSOLE_OUTPUT_H__
#define __TGREP_CONSOLE_OUTPUT_H__

//first we have our super fantastic printers
//debug stuff will only be used for debugging
//info is everything that would be printed out when
//-v is on the cmd line. Error will always get printed.
void console_print_debug(const char *format, ...);
void console_print_info(const char *format, ...);
void console_print_error(const char *format, ...);

//little different on this one
void console_print_help();

//now the enablers
void console_enable_debug();
void console_enable_info();
void console_enable_error();
void console_disable_debug();
void console_disable_info();
void console_disable_error();
#endif
