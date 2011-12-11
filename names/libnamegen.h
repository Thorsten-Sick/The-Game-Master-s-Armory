/* Libnamegen Library to generate realistic names for people and places
 * Copyright (C) 2005-2007  Thorsten Sick
 *
 * libnamegen.h Library to generate names for people and places.
 *
 * Copyright 2007 Thorsten Sick
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef _libnamegen_h_
#define _libnamegen_h_


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>



#define BASEDIR DATA_DIR G_DIR_SEPARATOR_S
#define LIBNAMEGEN_API_VERSION 4	///< This is the version of the API. changed when major changes where
#define LIBNAMEGEN_DB_VERSION 1	///< This is the version of the DB, Changed with major versions

#define LIBNAMEGEN_MALE 1	///< Set if you are looking for a male
#define LIBNAMEGEN_FEMALE 2	///< Set if you are looking for a female
#define LIBNAMEGEN_VILLAGE 3	///< Set if you are looking for a village
#define LIBNAMEGEN_CITY 4	///< Set if you are looking for a city
#define LIBNAMEGEN_MOUNTAIN 5	///< Set if you are looking for a mountain
#define LIBNAMEGEN_RIVER 6	///< Set if you are looking for a river
#define LIBNAMEGEN_LAKE 7	///< Set if you are looking for a lake
#define LIBNAMEGEN_TAVERN 8	///< Set for taverns, hotels, inns...
#define LIBNAMEGEN_SEX_WILDCARD 9	///< This matches on every sex


#define LIBNAMEGEN_MAX_NAMELENGTH 1000	  // Maximum length of name
#define LIBNAMEGEN_MAX_RULE_LENGTH 1000	  // How long can a name rule be
#define LIBNAMEGEN_MAX_INSERT_LENGTH 800	// Max name of a file called by rules

#define LIBNAMEGEN_RULEID_FEMALE "f:"	///< starter of a female rule
#define LIBNAMEGEN_RULEID_MALE "m:"	///< starter of a male rule
#define LIBNAMEGEN_RULEID_VILLAGE "v:"	///< starter of a village rule
#define LIBNAMEGEN_RULEID_MOUNTAIN "h:"	///< starter of a mountain rule
#define LIBNAMEGEN_RULEID_CITY "c:"	///< starter of a city rule
#define LIBNAMEGEN_RULEID_RIVER "r:"	///< starter of a river rule
#define LIBNAMEGEN_RULEID_LAKE "l:"	///< starter of a lake rule
#define LIBNAMEGEN_RULEID_TAVERN "t:"	///< starter of a hotel/tavern



// Error
#define LIBNAMEGEN_ERROR  g_quark_from_static_string("libnamegen_Error_quark") /**< Error domain*/
enum libnamgen_error
{
  LIBNAMEGEN_ERROR_NONE = 0,	///< No error
  LIBNAMEGEN_ERROR_NONE_CONTINUE,	///< No error. It is possible to continue this command in a loop
  LIBNAMEGEN_ERROR_STARTS_HERE,	///< Every value >0 this one is an error
  LIBNAMEGEN_ERROR_DB,		///< Some error with the database
  LIBNAMEGEN_ERROR_NOT_SUPPORTED,	///< Some feature or parameter is not supported
  LIBNAMEGEN_ERROR_FILE_OPEN,	///< Error while opening file
  LIBNAMEGEN_ERROR_RULE_PARSING,	///< Error while parsing a specific rule
  LIBNAMEGEN_ERROR_NO_RULE_FOUND,	///< No (matching) rule was found in the file
  LIBNAMEGEN_ERROR_NO_MEMORY,	///< Not enough space in menory
  LIBNAMEGEN_ERROR_WRONG_API,	///< The API is wrong, this means: Mismatch between proggy and library
  LIBNAMEGEN_ERROR_WRONG_DB,	///< Wrong database format
  LIBNAMEGEN_ERROR_NAME_TO_LONG,	///< The name we wanted to create would be to long
  LIBNAMEGEN_ERROR_PARAMS	///< Wrong parameters when calling a function
};


// Internal settings
#define LIBNAMEGEN_LEFT_OF_FILENAME "{"	/**< In a rule file, this marks the left part of a filename*/
#define LIBNAMEGEN_RIGHT_OF_FILENAME "}" /**< In a rule file, this marks the right part of a file name*/




/** This structure is passed when initialising the libnamegen*/
struct libnamegen_initstruct
{
  char *db_base_path;	     /**< The base path of the database*/
  int expected_api;	  /**< The Application expects this API version*/
  int expected_db;	 /**< The application expects this database version*/
};


/** For internal use. Produced by init*/
typedef struct libnamegen_init_int
{
  char *db_base_path;	     /**< The path to look for the db in*/
  void *previous_names;	       /**< The list of previous generated names*/
} LIBNAMEGEN_INIT_INT;


/** The linked string list is to return countries and worlds*/
typedef struct libnamegen_linked_string_list
{
  char *string;			///< the string stored in here
  struct libnamegen_linked_string_list *next;	///< Next item in linked string list or NULL
} LIBNAMEGEN_LINKED_STRING_LIST;

// Official API:
int libnamegen_getname (struct libnamegen_init_int *init, char *world,
			char *country, int sex, char *special_tags,
			char *name);
int libnamegen_init (struct libnamegen_initstruct init,
		     struct libnamegen_init_int **preset_stuff);
int libnamegen_done (struct libnamegen_init_int **preset_stuff);
int libnamegen_getver (int *major, int *minor);
int libnamegen_get_description (struct libnamegen_init_int *init, char *world,
				char *country, char **description);
int libnamegen_get_logo (struct libnamegen_init_int *init, char *world,
			 char *country, char **logo);


int libnamegen_remove_unicode (char *name);
int libnamegen_remove_html_tags (char *name);
int libnamegen_keep_unicode (char *name);
int libnamegen_get_db_stats (struct libnamegen_init_int *init, char *world,
			     char *country, int sex,
			     unsigned long *possible_names);
int libnamegen_extract_tags (struct libnamegen_init_int *init, char *world,
			     char *country, int sex,
			     LIBNAMEGEN_LINKED_STRING_LIST ** list);

int libnamegen_get_worlds (struct libnamegen_init_int *preset_stuff,
			   struct libnamegen_linked_string_list **list);
int libnamegen_get_countries (struct libnamegen_init_int *preset_stuff,
			      char *world,
			      struct libnamegen_linked_string_list **list);

void libnamegen_free_linked_string_list (struct libnamegen_linked_string_list
					 *anchor);



// For test purpose. Do not use:
int libnamegen_get_unstripped_command_by_number (char *command,
						 struct libnamegen_init_int
						 *init, char *world,
						 char *country, int sex,
						 char *special_tags,
						 int number);
int libnamegen_strip_command (char *command);
#endif
