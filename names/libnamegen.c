/** \file libnamegen.c
*
* This is the library file of libnamegen. Libnamegen is a library to generate REALISTIC names.
* It does NOT work with sylabels which are attached to each other to create "elbish" names.
* It works wit a rule file which is interpreted and some database files for each language.
*
* This way it is possible to create real names.
*
*
*
* Licence: It is licenced under the LGPL 2.1 or later
* Author: Thorsten Sick
* thorstensick@users.sourceforge.net
*/

/*
 * Libnamegen Library to generate realistic names for people and places
 * Copyright (C) 2005-2007 Thorsten Sick libnamegen.c Library to generate 
 * names for persons Copyright 2007 Thorsten Sick This library is free
 * software; you can redistribute it and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. This library is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. You should have
 * received a copy of the GNU Lesser General Public License along with
 * this library; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */


#include "libnamegen.h"
#include "config.h"
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>

// Remove this as soon as we are ready for release
#define DEBUG 1


#define DIR_SEPARATOR "/"	// /< Linux version.

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// ////////////////////////////////// Helper and wrapper functions

/** \brief Get the string out of the list. 0 is the string of the anchor.
*
* \param anchor the list to browse
* \param id the number of the string to get
* \return the string. NULL on error (list is to short...)
* \author Thorsten Sick
* \date 25.4.2007
* \callgraph
*/
char *
libnamegen_get_linked_string_by_number (struct libnamegen_linked_string_list
					*anchor, unsigned int id)
{
  unsigned int counter = 0;
  struct libnamegen_linked_string_list *item_runner = NULL;

  if (anchor == NULL)
    return NULL;

  item_runner = anchor;
  while (anchor->next && counter < id)
    {
      item_runner = item_runner->next;
      counter++;
    }

  if ((counter == id) && (item_runner))
    return item_runner->string;

  return NULL;
}


/** \brief adds a string to the string list
*
* \param anchor The string list as it is
* \param add_this the string to add
* \return the new anchor NULL on error
* \author Thorsten Sick
* \date 25.4.2007
* \callgraph
*/
struct libnamegen_linked_string_list *
libnamegen_add_to_linked_string_list (struct libnamegen_linked_string_list
				      *anchor, char *add_this)
{
  struct libnamegen_linked_string_list *new_item = NULL;
  struct libnamegen_linked_string_list *item_runner = NULL;

  if (add_this == NULL)
    return NULL;

  new_item = malloc (sizeof (struct libnamegen_linked_string_list));
  if (new_item == NULL)
    return NULL;

  memset (new_item, 0, sizeof (struct libnamegen_linked_string_list));
  new_item->string = malloc (strlen ((char *) add_this) + 1);
  if (new_item->string == NULL)
    {
      free (new_item);
      return NULL;
    }
  strcpy ((char *) new_item->string, (char *) add_this);

  if (anchor == NULL)
    {
      return new_item;
    }

  item_runner = anchor;
  while (item_runner->next)
    item_runner = item_runner->next;
  item_runner->next = new_item;

  return anchor;
}

/*
 * \brief free a linked string list * * * \param anchor the anchor to free
 * * \author Thorsten Sick * \date 25.4.2007 * \callgraph 
 */
void
libnamegen_free_linked_string_list (struct libnamegen_linked_string_list
				    *anchor)
{
  struct libnamegen_linked_string_list *item_runner = NULL;
  struct libnamegen_linked_string_list *next_item = NULL;


  if (anchor == NULL)
    return;


  item_runner = anchor;
  while (item_runner->next)
    {
      free (item_runner->string);
      next_item = item_runner->next;
      free (item_runner);
      item_runner = next_item;
    }
  if (item_runner)
    {
      free (item_runner->string);
      free (item_runner);
    }
  return;
}

/** \brief tests if the passed char is in the delimiter list
*
* \param character the character to test. Just a string, where the first char is used (for ease-of-use in my function)
* \param delimiter the delimiter list
* \return 1 if it is in the delimiter list, 0 else
* \author Thorsten Sick
* \date 25.4.2007
* \callgraph
*/
int
libnamegen_is_delimiter (char *character, char *delimiter)
{
  if ((delimiter == NULL) || (character == NULL))
    return 0;

  if (strlen ((char *) character) < 1)
    return 0;

  if (strchr ((char *) delimiter, (int) character[0]) != NULL)
    return 1;

  return 0;
}


/** \brief splits a string into several sub strings. The delimiter marks the points where to cut. 
*
* A Delimiter is a char. You can pass a string containing several Delimiters.
*
* \param string The string to delimit
* \param delimiter An other string containing all the delimiters
* \return returns a linked string list or NULL
* \author Thorsten Sick
* \date 25.4.2007
* \callgraph
*/
struct libnamegen_linked_string_list *
libnamegen_delimit (char *string, char *delimiters)
{
  struct libnamegen_linked_string_list *list = NULL;
  int start_offset = -1;
  int end_offset = -1;
  int length = 0;
  int i;

  char *workbuff = NULL;	// Working buffer

  if ((string == NULL) || (delimiters == NULL))
    {
      return NULL;
    }
  // Prepare workbuff
  length = strlen ((char *) string);

  workbuff = malloc (length + 1);
  if (workbuff == NULL)
    {
      return NULL;
    }



  for (i = 0; i < length; i++)
    {
      if (((libnamegen_is_delimiter (string + i, delimiters) == 0))
	  && (start_offset == -1))
	{
	  // printf("Delimiter start set\n");
	  // A new start offset was found
	  start_offset = i;
	}
      else if ((libnamegen_is_delimiter (string + i, delimiters))
	       && (start_offset == -1))
	{
	  // Case where the string starts with a delimiter
	  // printf("Delimiter start -1\n");
	}
      else if ((libnamegen_is_delimiter (string + i, delimiters)))
	{
	  // printf("Delimiter end\n");

	  // Set end offset and create string for the list
	  end_offset = i;

	  memset (workbuff, 0, length);
	  strncpy ((char *) workbuff, (char *) string + start_offset,
		   end_offset - start_offset);
	  memset (workbuff + end_offset - start_offset, 0, 1);
	  // printf("Delimited 1|%s|\n", workbuff);

	  list =
	    libnamegen_add_to_linked_string_list (list, (char *) workbuff);

	  start_offset = -1;
	  end_offset = -1;
	}

    }
  // At the end, there is maybe some rest-string. Put it there
  if (start_offset != -1)
    {
      end_offset = i;

      memset (workbuff, 0, length);
      strncpy ((char *) workbuff, (char *) string + start_offset,
	       end_offset - start_offset);
      memset (workbuff + end_offset - start_offset, 0, 1);
      // printf("Delimited 2|%s|\n", workbuff);
      list = libnamegen_add_to_linked_string_list (list, (char *) workbuff);
      start_offset = -1;
      end_offset = -1;
    }


  return list;
}

/** \brief Converts an integer ID to the string id to search in rule files
*
* \param rule_id will be set with the string id for the rule file
* \param sex the id of the item to name
* \return The error value
* \author Thorsten Sick
* \date 25.4.2007
*/
int
get_ruleid (char rule_id[4], int sex)
{
  memset (rule_id, 0, 4);
  // Select rule
  if (sex == LIBNAMEGEN_MALE)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_MALE);
    }
  else if (sex == LIBNAMEGEN_FEMALE)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_FEMALE);
    }
  else if (sex == LIBNAMEGEN_VILLAGE)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_VILLAGE);
    }
  else if (sex == LIBNAMEGEN_CITY)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_CITY);
    }
  else if (sex == LIBNAMEGEN_MOUNTAIN)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_MOUNTAIN);
    }
  else if (sex == LIBNAMEGEN_RIVER)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_RIVER);
    }
  else if (sex == LIBNAMEGEN_LAKE)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_LAKE);
    }
  else if (sex == LIBNAMEGEN_TAVERN)
    {
      strcpy (rule_id, LIBNAMEGEN_RULEID_TAVERN);
    }
  else if (sex == LIBNAMEGEN_SEX_WILDCARD)
    {
      // Every sex is taken

    }
  else
    return LIBNAMEGEN_ERROR_NOT_SUPPORTED;

  return LIBNAMEGEN_ERROR_NONE;
}


/** \brief returns a filename for path/file requested
*
* The user can stop at any position to support parameters (to just request the path for the world, as an example)
* In this case, the path will end with a DIR_SEPARATOR (sys specific)
* 
* Free this afterwards !!!!
*
* \param base The base of the database must be set
* \param world The world to request can be NULL
* \param country The country to request can be NULL
* \param file the file in this country can be NULL
* \return newly allocated file name. Or NULL on error !
* \author Thorsten Sick
* \date 24.4.2007
* \callgraph
*/
char *
create_filename (char *base, char *world, char *country, char *file)
{
  unsigned int size = 0;
  char *result = NULL;

  if (base == NULL)
    return NULL;
  size += strlen (base) + strlen (DIR_SEPARATOR) + 1;	// the +1 is the
  // '\0'

  if (world)
    {
      size += strlen (world) + strlen (DIR_SEPARATOR);

      if (country)
	{
	  size += strlen (country) + strlen (DIR_SEPARATOR);
	  if (file)
	    size += strlen (file);
	}
    }

  result = malloc (size);
  if (result == NULL)
    {
      return NULL;
    }
  memset (result, 0, size);

  strcat (result, base);
  strcat (result, DIR_SEPARATOR);

  if (world)
    {
      strcat (result, world);
      strcat (result, DIR_SEPARATOR);

      if (country)
	{
	  strcat (result, country);
	  strcat (result, DIR_SEPARATOR);
	  if (file)
	    strcat (result, file);
	}
    }

  return result;

}

/** \brief my random init function (as a wrapper for different systems)
*
* Inits random using time
*
* \author Thorsten Sick
* \callgraph
* \date 21.4.2007
*/
void
set_random ()
{
  srand (time (0));
}

/** \brief my random function (as a wrapper for different systems)
*
* Good enough. from must be smaller than to
*
* \param from Lowest possible number
* \param to Highest possible number
* \author Thorsten Sick
* \callgraph
* \date 21.4.2007
*/
unsigned int
random_int_range (unsigned int from, unsigned int to)
{
  unsigned int j;

  j = from + (int) (to * (rand () / (RAND_MAX + 1.0)));
  return j;
}


/** \brief my stringcopy can copy overlapping strings.
*
* Please be aware: Not so many error checks
*
* \param dest
* \param src
* \return 0 on success. >0 on error
* \author Thorsten Sick
* \date 16.4.2007
* \callgraph 
*/
unsigned char
lng_strcopy (char *dest, char *src)
{
  unsigned int i;
  unsigned int length;

  if ((dest == NULL) || (src == NULL))
    return 1;
  if (strlen ((char *) src) > strlen ((char *) dest))
    return 1;

  length = strlen ((char *) src)+1;
  for (i = 0; i < length; i++)
    {
      dest[i] = src[i];
    }
	
  dest[i + 1] = '\0';

  return 0;
}


/** \brief removes leading and trailing whitespaces (' ' and '\t')
*
* \param string the string to work with
* \return 0 on success, >0 else 
* \author Thorsten Sick
* \date 16.4.2007
* \callgraph
*/
unsigned char
lng_strstrip (char *string)
{
  unsigned long length;
  unsigned long start, stop;


  if (string == NULL)
    return 1;

  length = strlen ((char *) string);

  start = 0;
  while ((start < length) &&
	 ((string[start] == '\t') || (string[start] == ' ')))
    {
      start++;
    }

  stop = length;
  while ((stop > 0) && ((string[stop] == '\t') || (string[stop] == ' ')))
    {
      stop--;
    }

  string[stop] = '\0';
  lng_strcopy ((char *) string, (char *) string + start);

  return 0;
}


// /////////////////////// End of helper and wrapper functions



/** \brief gets a command out of the rule-file. Commands are numbered from 0 to X for each sex/tag combination.
*
* To directly access a rule. This rule is returned completely
*
*
* \param command The command to return. Is NULL, if there are no commands left
* \param namegen_init_int Internal init structure with extra data
* \param world The World-name 
* \param country The country name
* \param sex The sex, can be also LIBNAMEGEN_LAKE or so.
* \param special_tags These are text tags specific for this country
* \param number the number of the command to return. 0 to x. If this command does not exist, the returned command parameter will be NULL
* \param error The error value. A lot of stuff can happen
* \return LIBNAMEGEN_ERROR_NONE_CONTINUE if there are more rules, Error or ...ERROR_NONE else.
* \author Thorsten Sick
* \date 7/16/2006
* \callgraph
*/
int
libnamegen_get_unstripped_command_by_number (char *command,
					     struct libnamegen_init_int *init,
					     char *world, char *country,
					     int sex, char *special_tags,
					     int number)
{
  char *filename1 = NULL;
  char *command_rand = NULL;	/* Random command */
  FILE *file = NULL;
  char rule_id[4];		/* Identification of the rule in the file. 
				 */
  char *c_temp = NULL;
  char command_plucked[LIBNAMEGEN_MAX_RULE_LENGTH];	/* To 
							 * read 
							 * from 
							 * file 
							 */
  struct libnamegen_linked_string_list *command_array;	/* Put all 
							 * the
							 * commands 
							 * which
							 * could
							 * work in 
							 * here. */
  long ca_length;		/* Length of this array */
  char *start = NULL;
  long i;


  char *tags_pure = NULL;
  char *tag_start = NULL;
  char *tag_end = NULL;

  unsigned int tag_length;

  unsigned char found;
  unsigned char all_tags_there;

  struct libnamegen_linked_string_list *tag_set_available = NULL;
  struct libnamegen_linked_string_list *tag_set_needed = NULL;

  struct libnamegen_linked_string_list *tag_needed_runner = NULL;
  struct libnamegen_linked_string_list *tag_available_runner = NULL;

  int go_on = 1;
	int error=LIBNAMEGEN_ERROR_NONE;

  /*
   * init vars 
   */
  filename1 = NULL;
  command_rand = NULL;
  file = NULL;
  c_temp = NULL;
  command_array = NULL;
  start = NULL;
  i = 0;
  tags_pure = NULL;
  tag_start = NULL;
  tag_end = NULL;
  tag_length = 0;
  ca_length = 0;


  filename1 = create_filename (init->db_base_path, world, country, "rules");

  file = fopen (filename1, "rt");
  if (file == NULL)
    {
			error=LIBNAMEGEN_ERROR_FILE_OPEN;
      go_on = 0;
      goto clean_up_the_mess;
    }

  if (get_ruleid (rule_id, sex) != LIBNAMEGEN_ERROR_NONE)
    {
      go_on = 0;
      goto clean_up_the_mess;
    }



  memset (command, 0, LIBNAMEGEN_MAX_RULE_LENGTH);
  memset (command_plucked, 0, LIBNAMEGEN_MAX_RULE_LENGTH);
  while (fgets (command_plucked, LIBNAMEGEN_MAX_RULE_LENGTH, file))
    {

      // Check if it is a rule:
      if ((strncmp (command_plucked, "#", 1) == 0) ||
	  (strlen (command_plucked) < 2))
	{
	  // This is no rule, but a comment, we don't process this.  

	}
      else
	// Match Sex
      if ((strstr (command_plucked, rule_id) == command_plucked) ||
	    (sex == LIBNAMEGEN_SEX_WILDCARD))
	{

	  // Check if there are any special_tags
	  if (strstr (command_plucked, "[?:"))	// Flags ID
	    {
	      // Check tags here first
	      tag_start = strstr (command_plucked, "[?:") + 3;
	      tag_end = strstr (tag_start, "]");
	      if (tag_end == NULL)
		{
		  goto clean_up_the_mess;
		}
	      if ((special_tags) && (strncmp (special_tags, "*", 1) == 0))
		{		// If 
		  // tag 
		  // "*" 
		  // is 
		  // passed, 
		  // we 
		  // take 
		  // all 
		  // tags

		  all_tags_there = TRUE;
		}
	      else
		{
		  // Test all tags in the rule, if they are passed
		  tag_length = tag_end - tag_start;
		  tags_pure = malloc (tag_length + 1);
		  if (tags_pure == NULL)
		    {
		      go_on = 0;
		      goto clean_up_the_mess;
		    }
		  memset (tags_pure, 0, tag_length + 1);
		  strncpy (tags_pure, tag_start, tag_length);

		  tag_set_needed =
		    libnamegen_delimit ((char *) tags_pure, (char *) ",]");

		  tag_set_available =
		    libnamegen_delimit ((char *) special_tags, (char *) ",");

		  tag_needed_runner = tag_set_needed;


		  // tag_set_pointer = tag_set_needed;
		  all_tags_there = TRUE;
		  while (tag_needed_runner != NULL)
		    {

		      // Check if this tag was passed:
		      tag_available_runner = tag_set_available;
		      found = FALSE;
		      while ((tag_available_runner != NULL) && (!found))
			{
			  if (strcasecmp
			      ((char *) tag_available_runner->string,
			       (char *) tag_needed_runner->string) == 0)
			    {
			      found = TRUE;
			    }
			  else
			    {
			      tag_available_runner =
				tag_available_runner->next;
			    }
			}
		      if (!found)
			{
			  all_tags_there = FALSE;
			}

		      tag_needed_runner = tag_needed_runner->next;
		    }
		}

	      libnamegen_free_linked_string_list (tag_set_available);
	      libnamegen_free_linked_string_list (tag_set_needed);

	      if (all_tags_there)
		{		// Now we can take this ones.

		  c_temp = malloc (strlen (command_plucked) + 1);
		  if (c_temp == NULL)
		    {
		      go_on = 0;
		      goto clean_up_the_mess;
		    }
		  memset (c_temp, 0, strlen (command_plucked) + 1);
		  strcpy (c_temp, command_plucked);
		  assert (c_temp);

		  command_array =
		    libnamegen_add_to_linked_string_list (command_array,
							  (char *) c_temp);
		  ca_length++;
		}
	    }
	  else
	    {			// No tag problem, taking it
	      c_temp = malloc (strlen (command_plucked) + 1);
	      if (c_temp == NULL)
		{
		  go_on = 0;
		  goto clean_up_the_mess;
		}
	      memset (c_temp, 0, strlen (command_plucked) + 1);
	      strcpy (c_temp, command_plucked);
	      assert (c_temp);
	      command_array =
		libnamegen_add_to_linked_string_list (command_array,
						      (char *) c_temp);
	      ca_length++;
	    }
	}
      memset (command_plucked, 0, LIBNAMEGEN_MAX_RULE_LENGTH);
	 if (c_temp)
    {
      free (c_temp);
      c_temp = NULL;
    }
    }


  if (ca_length == 0)
    {
      goto clean_up_the_mess;
    }
  

  if (file)
    {
      fclose (file);
      file = NULL;
    }

  if (ca_length <= number)
    {
      go_on = 0;
      goto clean_up_the_mess;
    }
  else
    {
      command_rand =
	libnamegen_get_linked_string_by_number (command_array, number);
      go_on = 1;
      strncpy (command, (char *) command_rand, LIBNAMEGEN_MAX_RULE_LENGTH);
    }


  // Delete all pointers in the array
  libnamegen_free_linked_string_list (command_array);
  command_array = NULL;

  if (filename1)
    {
      free (filename1);
      filename1 = NULL;
    }

  if (file)
    {
      fclose (file);
      file = NULL;
    }

  if (go_on)
    return LIBNAMEGEN_ERROR_NONE_CONTINUE;
  else
    return LIBNAMEGEN_ERROR_NONE;


clean_up_the_mess:

  // Delete all pointers in the array
  libnamegen_free_linked_string_list (command_array);
  command_array = NULL;

  if (c_temp)
    {
      free (c_temp);
      c_temp = NULL;
    }

  if (filename1)
    {
      free (filename1);
      filename1 = NULL;
    }

  if (file)
    {
      fclose (file);
      file = NULL;
    }


  return error;

}

/** This one removes all tags and the m: (or whatever) from the beginning
*
*
*
* \param command The command string to strip
* \return The error value
* \author Thorsten Sick
* \date 7/16/2006
* \callgraph
*/
int
libnamegen_strip_command (char *command)
{
  char *start;

  if (command == NULL)
    {
#ifdef DEBUG
      printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
      return LIBNAMEGEN_ERROR_RULE_PARSING;
    }

  start = strchr (command, ':') + 1;
  if (strstr (start, "[?:"))
    {
      if (strstr (start, "]"))
	{
	  start = strstr (start, "]") + 1;
	}
      else
	{
#ifdef DEBUG
	  printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
	  return LIBNAMEGEN_ERROR_RULE_PARSING;
	}
    }


  lng_strcopy ((char *) command, (char *) start);

  lng_strstrip ((char *) command);	// removes leading and trailing
  // whitespaces from a string

  return LIBNAMEGEN_ERROR_NONE;
}



/** This one searches for tags in a country/world and returns them
*
*	It is checked to have each tag ONCE in this list.
* Free the pointer array afterwards !
*
* \param init the initialized data
* \param world The world it takes place in
* \param country The country to request tags from
* \param sex the sex to get tags from (normally you want LIBNAMEGEN_SEX_WILDCARD)
* \param list returns a linked string list with the tags.
* \param error An error variable
* \return 0 or the error value
* \author Thorsten Sick
* \date 7/16/2006
* \callgraph
*/
int
libnamegen_extract_tags (struct libnamegen_init_int *init,
			 char *world,
			 char *country,
			 int sex, LIBNAMEGEN_LINKED_STRING_LIST ** list)
{
  char command[LIBNAMEGEN_MAX_RULE_LENGTH];			  /** The command-line read from the file*/
  unsigned int a = 0;
  char *tag_start = NULL;
  char *tag_end = NULL;
  unsigned int tag_length;
  char *tags_pure = NULL;
  struct libnamegen_linked_string_list *tag_set = NULL;
  struct libnamegen_linked_string_list *tag_runner = NULL;
  struct libnamegen_linked_string_list *result_list = NULL;
  struct libnamegen_linked_string_list *list_runner = NULL;

  unsigned char already_there = 0;



  int error = 0;

  error =
    libnamegen_get_unstripped_command_by_number (command, init, world,
						 country, sex, "*", a);
  if (error >= LIBNAMEGEN_ERROR_STARTS_HERE)
    {
      goto clean_up_the_mess;
    }
  while (error == LIBNAMEGEN_ERROR_NONE_CONTINUE)
    {
      // printf("Command: %s", command);
      tag_start = strstr (command, "[?:");
      if (tag_start)
	{
	  tag_start += 3;
	  tag_end = strstr (tag_start, "]");
	  if (tag_end)
	    {
	      tag_length = tag_end - tag_start;
	      tags_pure = malloc (tag_length + 1);
	      if (tags_pure == NULL)
		{
		  error = LIBNAMEGEN_ERROR_NO_MEMORY;
		  goto clean_up_the_mess;
		}
	      memset (tags_pure, 0, tag_length + 1);
	      strncpy (tags_pure, tag_start, tag_length);



	      tag_set =
		libnamegen_delimit ((char *) tags_pure, (char *) ",]");
	      free (tags_pure);

	      if (tag_set)
		{
		  tag_runner = tag_set;
		  while (tag_runner)
		    {
		      already_there = 0;
		      // printf("Tag: %s\n", tag_runner->string); 
		      // check if the string is already in the list
		      list_runner = result_list;
		      while ((list_runner) && (already_there == 0))
			{
			  if (strcmp
			      ((char *) tag_runner->string,
			       (char *) list_runner->string) == 0)
			    already_there = 1;
			  list_runner = list_runner->next;
			}

		      if (!already_there)
			{
			  result_list =
			    libnamegen_add_to_linked_string_list
			    (result_list, (char *) tag_runner->string);
			}
		      tag_runner = tag_runner->next;
		    }
		}
	    }
	}

      a++;
      error =
	libnamegen_get_unstripped_command_by_number (command, init,
						     world, country, sex, "*",
						     a);
      if (error >= LIBNAMEGEN_ERROR_STARTS_HERE)
	{		
	  goto clean_up_the_mess;
	}
    }


clean_up_the_mess:

  libnamegen_free_linked_string_list (tag_set);
  *list = result_list;
  if (error < LIBNAMEGEN_ERROR_STARTS_HERE)
    {
      error = LIBNAMEGEN_ERROR_NONE;
    }
  return error;
}


/** Returns the number of lines (no comments, only with useable content) in a db-file
*
*
*
* \param filename The file to count the lines in
* \param count The number of lines
* \return The error
* \author Thorsten Sick
* \date 6/18/2006
* \callgraph
*/
int
libnamegen_count_lines_in (char *filename, unsigned long *count)
{
  unsigned long line_count = 0;
  FILE *infile;
  char name[LIBNAMEGEN_MAX_NAMELENGTH];

  /*
   * sanity checks 
   */
  assert (filename != NULL);

  *count = 0;

  infile = fopen (filename, "r");
  if (infile == NULL)
  {
			#ifdef DEBUG
				printf("Error opening %s\n", filename);
			#endif
      return LIBNAMEGEN_ERROR_FILE_OPEN;
  }

  while (feof (infile) == 0)
    {
      fgets (name, LIBNAMEGEN_MAX_NAMELENGTH, infile);
      if ((strlen (name) > 1) && (name[0] != '#'))
	{
	  line_count++;
	}

    }

  fclose (infile);

  if (line_count == 0)
    {
      *count = 0;
      return LIBNAMEGEN_ERROR_NONE;
    }

  *count = line_count - 1;
  return LIBNAMEGEN_ERROR_NONE;


}



/** Returns a random line out of a file
*
*
*
* \param filename The file to open
* \param random_line The string contains the random line
* \return The error code
* \author Thorsten Sick
* \date 5/25/2006
* \callgraph
*/
int
libnamegen_return_random_line (char *filename, char **random_line)
{
  long i;
  FILE *infile = NULL;
  char name[LIBNAMEGEN_MAX_NAMELENGTH], *returnstring;
  unsigned long line_count;
  char *tmp;

  /*
   * initialize vars 
   */
  i = 0;
  infile = NULL;
  line_count = 0;
  returnstring = NULL;

  /*
   * sanity checks 
   */
  assert (filename != NULL);

  // Get number of useable lines in file:
  libnamegen_count_lines_in (filename, &line_count);

  // printf("Lines : %d", line_count);

  infile = fopen (filename, "r");
	if (infile==NULL)
	{
		#ifdef DEBUG
			printf("Error opening %s\n", filename);
		#endif
		return LIBNAMEGEN_ERROR_FILE_OPEN;
	}	
  i = random_int_range (1, line_count);

  // printf("i: %d",i);

  while ((i > 0) && (feof (infile) == 0))
    {
      fgets (name, LIBNAMEGEN_MAX_NAMELENGTH, infile);
      if ((strlen (name) > 1) && (name[0] != '#'))
	{
	  i--;
	}

    }


  if (infile)
    {
      fclose (infile);
      infile = NULL;
    }

  if (name)
    {
      returnstring = malloc (strlen (name) + 1);
      if (returnstring == NULL)
	{
	  return LIBNAMEGEN_ERROR_NO_MEMORY;
	}
      memset (returnstring, 0, strlen (name) + 1);
      strcpy (returnstring, name);
    }
  else
    {
      returnstring = NULL;
    }
  // Remove "\n"
  tmp = strchr (returnstring, '\n');
  if (tmp != NULL)
    {
      *tmp = 0;
    }


  assert (returnstring);
  *random_line = returnstring;
  return LIBNAMEGEN_ERROR_NONE;
}




/** \brief Calculates statistics for world/country/sex
*
*
*
* \param init Internal init structure with extra data
* \param world The World-name (or the Role Playing Game you play in)
* \param country The country name
* \param sex The sex, can be also LIBNAMEGEN_LAKE or so.
* \param possible_names The total number of possible names
* \return Error
* \author Thorsten Sick
* \date 7/18/2006
* \callgraph
*/

int
libnamegen_get_db_stats (struct libnamegen_init_int *init,
			 char *world,
			 char *country,
			 int sex, unsigned long *possible_names)
{

  unsigned long total = 0;
  unsigned long this_rule = 1;

  char command[LIBNAMEGEN_MAX_RULE_LENGTH];			  /** The command-line read from the file*/
  char *filename2;


  char *fspointer = NULL;	// Points to start of filename
  char *fepointer = NULL;	// Points to end of filename
  char *start_pointer = NULL;	// Start of useable
  // command
  int length = 0;		// lenght of filename
  char rule_part[LIBNAMEGEN_MAX_INSERT_LENGTH];	// Sub-file 
  // length
  int command_length;		// Length of command string
  char *tmp;
  long a;			// Rule counter


  int result = LIBNAMEGEN_ERROR_NONE;	// Error returned

  unsigned long lines;		// /< Lines counted in the file



  a = 0;
  while (libnamegen_get_unstripped_command_by_number
	 (command, init, world, country, sex, "*",
	  a) == LIBNAMEGEN_ERROR_NONE_CONTINUE)
    {

      libnamegen_strip_command (command);
      // Go through all rules in the country
      // calc he possible numbers for each rule, add them together.

      // Go through command
      start_pointer = command;	// Start after :


      command_length = strlen (command);

      while ((command + command_length - 1 > start_pointer)
	     && (start_pointer != NULL))
	{
	  // Doing Direct String
	  fspointer = start_pointer;
	  fepointer = strstr (start_pointer, LIBNAMEGEN_LEFT_OF_FILENAME);
	  if (fepointer != NULL)
	    {
	      length = fepointer - fspointer;
	      if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
		{
#ifdef DEBUG
		  printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
		  result = LIBNAMEGEN_ERROR_RULE_PARSING;
		  goto clean_up_the_mess;
		}
	      memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
	      strncpy (rule_part, fspointer, length);
	    }
	  else
	    {
	      length = strlen (fspointer);
	      if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
		{
#ifdef DEBUG
		  printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
		  result = LIBNAMEGEN_ERROR_RULE_PARSING;
		  goto clean_up_the_mess;
		}
	      memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
	      strncpy (rule_part, fspointer, length);
	    }
	  tmp = strstr (rule_part, "\n");
	  if (tmp != NULL)
	    {
	      *tmp = 0;
	    }
	  // Doing file name
	  if (fepointer != NULL)
	    {
	      fspointer = fepointer + 1;
	      // fspointer++;
	      fepointer = strstr (fspointer, LIBNAMEGEN_RIGHT_OF_FILENAME);
	      if (fepointer != NULL)
		{
		  length = fepointer - fspointer;
		  // printf("%s|%s|%d\n", fspointer, fepointer, length);

		  if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
		    {
#ifdef DEBUG
		      printf
			("Error in line %d of %s with fspointer=%s %d\n",
			 __LINE__, __FILE__, fspointer, strlen (fspointer));
#endif
		      result = LIBNAMEGEN_ERROR_RULE_PARSING;
		      goto clean_up_the_mess;
		    }
		  memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
		  strncpy (rule_part, fspointer, length);

		  filename2 =
		    create_filename (init->db_base_path, world, country,
				     rule_part);

		  if (libnamegen_count_lines_in (filename2, &lines) >
		      LIBNAMEGEN_ERROR_NONE)
		    {
		      lines = 0;
		    }

		  this_rule *= lines;


		  free (filename2);


		  // Return to dissecting here.
		  start_pointer = fepointer + 1;
		}
	      else
		start_pointer = NULL;
	    }
	  else
	    {
	      start_pointer = NULL;
	    }
	}


      total += this_rule;
      this_rule = 1;
      a++;
    }

clean_up_the_mess:


  *possible_names = total;

  return result;



}


/** \brief gets a random command out of the rule-file. Matching special_tags and the sex
*
*
* \param command The command to return
* \param namegen_init_int Internal init structure with extra data
* \param world The World-name (or the Role Playing Game you play in)
* \param country The country name
* \param sex The sex, can be also LIBNAMEGEN_LAKE or so.
* \param special_tags These are text tags specific for this country
* \param error The error value. A lot of stuff can happen
* \return The error var.
* \author Thorsten Sick
* \date 5/25/2006
* \callgraph
*/
int
libnamegen_get_random_command (char *command,
			       struct libnamegen_init_int *init,
			       char *world,
			       char *country, int sex, char *special_tags)
{
  int number_of_commands;
				/**< The number of commands in this rule file matching*/
	int chosen=0; ///< The command chosen randomly
	char * pos1;

  // calc commands
  number_of_commands = 0;
  while (libnamegen_get_unstripped_command_by_number (command,
						      init,
						      world,
						      country,
						      sex,
						      special_tags,
						      number_of_commands)
	 == LIBNAMEGEN_ERROR_NONE_CONTINUE)
    {
      number_of_commands++;
    }

	#ifdef DEBUG
		printf("Number of Commands in %s %s with %s %d\n", world, country, special_tags, number_of_commands);
	#endif
  if (number_of_commands == 0)
    {
      return LIBNAMEGEN_ERROR_NO_RULE_FOUND;
    }
  // get one random command
	chosen=random_int_range (0, number_of_commands);	
	#ifdef DEBUG
		printf("Chosen Command %d\n", chosen);
	#endif
  libnamegen_get_unstripped_command_by_number (command, init, world,
					       country, sex, special_tags,
					       chosen);
  // strip it.
  libnamegen_strip_command (command);

	if (strchr(command,'\n'))
	{
		pos1= strchr(command,'\n');
		pos1[0]=0;
	}

  return LIBNAMEGEN_ERROR_NONE;
}


/** \brief This one removes the <uname> Tag and everything contained
*
* The tags have to be lower case
* <uname> for the start, </uname> for the end.
*
*
* \param name The name to process
* \return The error
* \author Thorsten Sick
* \date 25.5.2006
* \callgraph
*/
int
libnamegen_remove_unicode (char *name)
{
  char *pos1;
  char *pos2;
  unsigned char still_left = 1;

  while (still_left)
    {
      // printf("Recurse\n");
      pos1 = strstr (name, "<uname>");
      if (pos1)
	{
	  pos2 = strstr (name, "</uname>");
	  if (pos2)
	    {
				if (strlen(pos2)>8)
				{
	      	// Move upper part to the front.
	      	lng_strcopy (pos1, pos2 + 8);
				}
				else
				{
					memset(pos2,0,8);
				}

	    }
	  else
	    {
	      still_left = 0;
	    }
	}
      else
	{
	  still_left = 0;
	}
    }

  return 0;
}


/** \brief This one keeps only the content of the <uname> Tag
*
* The tags have to be lower case
* <uname> for the start, </uname> for the end.
*
*
* \param name The name to process
* \return The error
* \author Thorsten Sick
* \date 25.5.2006
* \callgraph
*/
int
libnamegen_keep_unicode (char *name)
{
  char *pos1;
  char *pos2;
  unsigned char still_left = 1;
  char wbuff[LIBNAMEGEN_MAX_NAMELENGTH];



  pos1 = name;
  pos2 = name;
  memset (wbuff, 0, LIBNAMEGEN_MAX_NAMELENGTH);

  while (still_left)
    {
      pos1 = strstr (pos2, "<uname>");
      if (pos1)
	{
	  pos2 = strstr (pos1, "</uname>");
	  if (pos2)
	    {
	      // Move upper part to the front.  
	      strncat (wbuff, pos1 + 7, pos2 - (pos1 + 7));
	    }
	  else
	    {
	      strncat (wbuff, pos1 + 7, strlen (pos1 + 7));
	      still_left = 0;
	    }
	}
      else
	{
	  still_left = 0;
	}
    }

  memcpy (name, wbuff, LIBNAMEGEN_MAX_NAMELENGTH);

  return 0;
}




/** \brief remove tags
*
* This removes html tags a simple way. Everything between < and > is gone afterwards
*
*
* \param name The name to process
* \return The error
* \author Thorsten Sick
* \date 25.5.2006
* \callgraph
*/
int
libnamegen_remove_html_tags (char *name)
{
  char *pos1;
  char *pos2;
  unsigned char still_left = 1;

  while (still_left)
    {

      pos1 = strstr (name, "<");
      if (pos1)
	{

	  pos2 = strstr (name, ">");
	  if (pos2)
	    {

	      // Move upper part to the front.
	      strcpy (pos1, pos2 + 1);

	    }
	  else
	    {
	      still_left = 0;
	    }
	}
      else
	{
	  still_left = 0;
	}
    }

  return 0;
}

/** \brief This one produces names
*
*
* this is the fixed, cleaner and more stable version
*
*
* \param init The data from the init function
* \param world The world to show
* \param country The country
* \param sex sex of the name-owner or the object you want (now with citys and so on) (LIBNAMEGEN_MAX_NAMELENGTH)
* \param special_tags These tags allow further and more detailed selection of rules. Depending on the country. It is a string with a comma separated list of the tags
* \param name the name to create. Pass a buffer of the size LIBNAMEGEN_MAX_NAMELENGTH
* \return 0 if everything is ok, the error else
* \author Thorsten Sick
* \date 5.8.2007
* \callgraph
*/
int
libnamegen_getname (struct libnamegen_init_int *init,
		    char *world,
		    char *country, int sex, char *special_tags, char *name)
{
  char command[LIBNAMEGEN_MAX_RULE_LENGTH];		  ///< The command-line read from the file
	char rule_part[LIBNAMEGEN_MAX_RULE_LENGTH];   ///< To cut and paste smallerparts of the rule for later use
	char *name_part;                              ///< A small piece of the name for later use
	int  error = 0;                               ///< The normal error var.
	char * command_pointer=NULL;                  ///< The pointer into the command. From here there has to be copied data
	char * next_mark=NULL;                        ///< The next step to take pointing at { or }
	unsigned long name_space_left=0;                ///< Number of bytes, which can be still fitted into the name.
	char *filename=NULL;                          ///< When reading data from a file we need a filename
	
	
  if ((world == NULL) || (country == NULL) || (name==NULL))
    return LIBNAMEGEN_ERROR_PARAMS;

	memset(command,0,LIBNAMEGEN_MAX_RULE_LENGTH);
	memset(name, 0, LIBNAMEGEN_MAX_NAMELENGTH);
	name_space_left=LIBNAMEGEN_MAX_NAMELENGTH-1; // Make sure there is still a \0 at the end.
	
  error =
    libnamegen_get_random_command (command, init, world, country, sex,
				   special_tags);
  if (error != LIBNAMEGEN_ERROR_NONE)
    {
     goto clean_up_the_mess;
    }

	#ifdef DEBUG
		printf("Command: %s|\n", command);
	#endif		
		
	command_pointer=command;	
	// While there is still some command left
	while (strlen(command_pointer))
	{
		#ifdef DEBUG
		printf("Command left: %s NSL %d\n", command_pointer,name_space_left);
		#endif		
		// Direct part of name
		// Copy everything from command_pointer to the first left of Filename into result	
		next_mark=strstr (command_pointer, LIBNAMEGEN_LEFT_OF_FILENAME);
		if (next_mark)
		{
			if (next_mark-command_pointer>name_space_left)
			{
				#ifdef DEBUG
				printf("Error 1\n");
				#endif
				error=LIBNAMEGEN_ERROR_NAME_TO_LONG;
				goto clean_up_the_mess;
			}
			else
			{
				strncat (name,command_pointer,next_mark-command_pointer); 
				name_space_left -= next_mark-command_pointer;
			}
		}
		else
		{
			// Copy till eol
			if (strlen(command_pointer)>name_space_left)
			{
				#ifdef DEBUG
				printf("Error 2\n");
				#endif
				error=LIBNAMEGEN_ERROR_NAME_TO_LONG;
				goto clean_up_the_mess;
			}
			else
			{				
				next_mark=command_pointer+strlen(command_pointer); // Go to the end
				if (command_pointer[0] != '}')
				{
					strcat(name, command_pointer);
					name_space_left -= strlen(command_pointer);
					#ifdef DEBUG
						printf("After constant part %s, NSL: %d",name, name_space_left);
					#endif
				}
			}
			
		}
		
		// indirect part of name. Get this from a file.
		// Go to start of filename, extract it.
		if ((next_mark) && (strlen(next_mark)))
		{
			memset (rule_part, 0, LIBNAMEGEN_MAX_RULE_LENGTH);
			
			command_pointer=next_mark;
		
			next_mark=strstr(command_pointer,LIBNAMEGEN_RIGHT_OF_FILENAME);
			if (next_mark==NULL)
			{
				#ifdef DEBUG
				printf("Error 3\n");
				#endif
				// No closing } Error
				error=LIBNAMEGEN_ERROR_RULE_PARSING;
				goto clean_up_the_mess;
			}
			
		
		
		  strncpy (rule_part, command_pointer+1, next_mark-command_pointer-1);

	  	filename =
	    	create_filename (init->db_base_path, world, country, rule_part);
	  

			#ifdef DEBUG
	  		printf ("Filename:%s|\n", filename);
			#endif
			
	  	error = libnamegen_return_random_line (filename, &name_part);
	  	if ((error) || (name_part==NULL))
	    {
				#ifdef DEBUG
	      printf ("Error: 5 %d", error);
				#endif
	      goto clean_up_the_mess;
	    }
			
			if (strlen(name_part)<name_space_left)
			{
				strcat(name,name_part);
				name_space_left -= strlen(name_part);
			}
			else
			{
				#ifdef DEBUG
				printf("Error 4 Left: %d needing %d\n", name_space_left, strlen(name_part));
				#endif
				error=LIBNAMEGEN_ERROR_NAME_TO_LONG;
				goto clean_up_the_mess;
			}
			#ifdef DEBUG
				printf("Random part |%s|%s|NSL: %d\n", name_part, name, name_space_left);
			#endif
	  	free (filename);
			filename=NULL;
			free(name_part);
			name_part=NULL;
			
			// Create random name and add it to result.
		}
		command_pointer=next_mark+1;		
	}
		

	#ifdef DEBUG
		printf("Result: %s\n", name);
	#endif
	
	clean_up_the_mess:
	
		free (filename);
		filename=NULL;
		free(name_part);
		name_part=NULL;
		return error;
	
}




/** \brief This one produces names
*
*
*
*
*
* \param init The data from the init function
* \param world The world to show
* \param country The country
* \param sex sex of the name-owner or the object you want (now with citys and so on) (LIBNAMEGEN_MAX_NAMELENGTH)
* \param special_tags These tags allow further and more detailed selection of rules. Depending on the country. It is a string with a comma separated list of the tags
* \param name_spoken The name how it is spoken. Please pass a buffer (LIBNAMEGEN_MAX_NAMELENGTH)
* \param name_written The name as it is written. Pass a buffer again
* \return 0 if everything is ok, the error else
* \author Thorsten Sick
* \date 2.10.05
* \todo This direly needs some cleanup. Idea: Burn and rebuild
* \callgraph
*/
/*int
libnamegen_getname (struct libnamegen_init_int *init,
		    char *world,
		    char *country, int sex, char *special_tags, char *name)
{
  int result;
  char *filename2;
  FILE *file;
  char command[LIBNAMEGEN_MAX_RULE_LENGTH];		  /// The command-line read from the file
  char *fspointer = NULL;	// Points to start of filename
  char *fepointer = NULL;	// Points to end of filename
  char *start_pointer = NULL;	// Start of useable command
  int length = 0;		// lenght of filename
  char rule_part[LIBNAMEGEN_MAX_INSERT_LENGTH];	// Sub-file 
  // length
  char *name_part;		// Part of the name
  int build_position;		// Where we are in dest string
  int command_length;		// Length of command string
  char *tmp;

  int error = 0;


  filename2 = NULL;
  file = NULL;
  result = 0;
  build_position = 0;

  if ((world == NULL) || (country == NULL))
    return LIBNAMEGEN_ERROR_PARAMS;

	memset(command,0,LIBNAMEGEN_MAX_RULE_LENGTH);

  error =
    libnamegen_get_random_command (command, init, world, country, sex,
				   special_tags);
  if (error != LIBNAMEGEN_ERROR_NONE)
    {
      return error;
    }

#ifdef DEBUG
  printf ("Got random command: %s", command);
#endif
  // Go through command
  start_pointer = command;	// Start after :

  build_position = 0;
  memset (name, '\0', LIBNAMEGEN_MAX_NAMELENGTH);

  command_length = strlen (command);

  while ((command + command_length - 1 > start_pointer)
	 && (start_pointer != NULL))
    {
      // Doing Direct String
      fspointer = start_pointer;
      fepointer = strstr (start_pointer, LIBNAMEGEN_LEFT_OF_FILENAME);
      if (fepointer != NULL)
	{
	  length = fepointer - fspointer;
	  if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
	    {
#ifdef DEBUG
	      printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
	      error = LIBNAMEGEN_ERROR_RULE_PARSING;
	      goto clean_up_the_mess;
	    }
	  memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
	  strncpy (rule_part, fspointer, length);
	}
      else
	{
	  length = strlen (fspointer);
	  if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
	    {
#ifdef DEBUG
	      printf ("Error in line %d of %s\n", __LINE__, __FILE__);
#endif
	      error = LIBNAMEGEN_ERROR_RULE_PARSING;
	      goto clean_up_the_mess;
	    }
	  memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
	  strncpy (rule_part, fspointer, length);
	}
      tmp = strstr (rule_part, "\n");
      if (tmp != NULL)
	{
	  *tmp = 0;
	}
      if (build_position + length > LIBNAMEGEN_MAX_NAMELENGTH)
	{
	  error = LIBNAMEGEN_ERROR_NAME_TO_LONG;
	  goto clean_up_the_mess;
	}


      strncpy (name + build_position, rule_part, length);

      build_position += length;

      // Doing file name
      if (fepointer != NULL)
	{
	  fspointer = fepointer + 1;
	  // fspointer++;
	  fepointer = strstr (fspointer, LIBNAMEGEN_RIGHT_OF_FILENAME);
		
	  length = fepointer - fspointer;
	  // printf("%s|%s|%d\n", fspointer, fepointer, length);

	  if (length > LIBNAMEGEN_MAX_INSERT_LENGTH)
	    {
#ifdef DEBUG
	      printf ("Error in line %d of %s\n", __LINE__, __FILE__);
				if (fepointer==NULL)
					printf("Epointer is NULL\n");	
#endif
	      error = LIBNAMEGEN_ERROR_RULE_PARSING;
	      goto clean_up_the_mess;
	    }
	  memset (rule_part, '\0', LIBNAMEGEN_MAX_INSERT_LENGTH);
	  strncpy (rule_part, fspointer, length);

	  filename2 =
	    create_filename (init->db_base_path, world, country, rule_part);
	  

#ifdef DEBUG
	  printf ("Filename:%s|\n", filename2);
#endif

	  error = libnamegen_return_random_line (filename2, &name_part);
	  if (error)
	    {
	      printf ("Error: %d", error);
	      goto clean_up_the_mess;
	    }
	  free (filename2);
		filename2=NULL;

#ifdef DEBUG
	  printf ("Random chosen: |%s|\n", name_part);
#endif

	  length = strlen (name_part);
	  if (build_position + length > LIBNAMEGEN_MAX_NAMELENGTH)
	    {
	      free (name_part);
	      error = LIBNAMEGEN_ERROR_NAME_TO_LONG;
	      goto clean_up_the_mess;

	    }

	  if ((strlen (name) + strlen (name_part) + 1) >
	      LIBNAMEGEN_MAX_NAMELENGTH)
	    {
	      error = LIBNAMEGEN_ERROR_NAME_TO_LONG;
	      goto clean_up_the_mess;
	    }
	  strcat (name, name_part);
	  free (name_part);

	  build_position += length;


	  // Return to dissecting here.
	  start_pointer = fepointer + 1;
	}
      else
	{
	  start_pointer = NULL;
	}

    }


clean_up_the_mess:

	if (filename2)
	{
		free(filename2);
		filename2=NULL;
	}		
		
  return error;
}

*/

/** \brief inits namegen
*
* If you want to acess the database, you have to init namegen first.
* The resulted data has to be passed to every function using the database.
*
* If you don't use libnamegen_done on the data afterwards , you get leaks !
*
* \param libnamegen_initstruct Pass the data the library needs in here.
* \param internal_init
* \return Error code
* \author Thorsten Sick
* \date 1.10.05
* \callgraph
*/
int
libnamegen_init (struct libnamegen_initstruct init,
		 struct libnamegen_init_int **internal_init)
{
  struct libnamegen_init_int *result;

  if (internal_init == NULL)
    {
      return LIBNAMEGEN_ERROR_NO_MEMORY;
    }

  set_random ();

  if (init.expected_api != LIBNAMEGEN_API_VERSION)
    {
      return LIBNAMEGEN_ERROR_WRONG_API;
    }
  if (init.expected_db != LIBNAMEGEN_DB_VERSION)
    {
      return LIBNAMEGEN_ERROR_WRONG_DB;
    }


  result = malloc (sizeof (struct libnamegen_init_int));
  if (result == NULL)
    {
      return LIBNAMEGEN_ERROR_NO_MEMORY;
    }

  memset (result, 0, sizeof (struct libnamegen_init_int));

  result->db_base_path = init.db_base_path;
  *internal_init = result;

  return LIBNAMEGEN_ERROR_NONE;
}


/** \brief de-inits namegen
*
* If you don't use libnamegen_done on the data afterwards , you get leaks !
*
* \param preset_stuff You got this from init, you pass it in here
* \return Error, but there should be none
* \author Thorsten Sick
* \date 1.10.05
* \callgraph
*/
int
libnamegen_done (struct libnamegen_init_int **preset_stuff)
{
  free (*preset_stuff);
  return 0;
}



/** \brief Returns version information
*
*
* \param major major version of this lib
* \param minor minor version of this lib
* \return nothing, yet
* \author Thorsten Sick
* \date 1.10.05
* \callgraph
*/
int
libnamegen_getver (int *major, int *minor)
{
  *major = MAJOR_VERSION;
  *minor = MINOR_VERSION;
  return 0;
}


/** \brief Returns a list of countries in th given world
*
*
* \param preset_stuff We need to know where the DB is
* \param world The world to check
* \param list The list to be written. Free it afterwards !
* \return The error
* \author Thorsten Sick
* \date 7.6.07
* \callgraph
*/
int
libnamegen_get_countries (struct libnamegen_init_int *preset_stuff,
			  char *world,
			  struct libnamegen_linked_string_list **list)
{
  DIR *dir;
  struct dirent *dire;
  char *dir_name;

  struct libnamegen_linked_string_list *llist = NULL;

  dir_name = create_filename (preset_stuff->db_base_path, world, NULL, NULL);

  // Test
  dir = opendir (dir_name);
  dire = readdir (dir);
  while (dire)
    {
#ifndef WINCROSS
      if (dire->d_type == DT_DIR)
#endif
	{
	  // Every name starting with "." ignored. these are also hidden 
	  // dirs !
	  if (strncmp (dire->d_name, ".", 1) != 0)
	    {
	      llist =
		libnamegen_add_to_linked_string_list (llist,
						      (char *) dire->d_name);
	    }
	}
      dire = readdir (dir);
    }
  closedir (dir);

  free (dir_name);
  *list = llist;

  return 0;
}


/** \brief Returns a list of worlds
*
*
* \param preset_stuff We need to know where the DB is
* \param list The list to be written. Free it afterwards !
* \return The error
* \author Thorsten Sick
* \date 7.6.07
* \callgraph
*/
int
libnamegen_get_worlds (struct libnamegen_init_int *preset_stuff,
		       struct libnamegen_linked_string_list **list)
{
  DIR *dir;
  struct dirent *dire;
  char *dir_name;

  struct libnamegen_linked_string_list *llist = NULL;

  dir_name = create_filename (preset_stuff->db_base_path, NULL, NULL, NULL);

  // Test
  dir = opendir (dir_name);
  dire = readdir (dir);
  while (dire)
    {
#ifndef WINCROSS
      if (dire->d_type == DT_DIR)
#endif
	{
	  // Every name starting with "." ignored. these are also hidden 
	  // dirs !
	  if (strncmp (dire->d_name, ".", 1) != 0)
	    {
	      llist =
		libnamegen_add_to_linked_string_list (llist,
						      (char *) dire->d_name);
	    }
	}

      dire = readdir (dir);
    }
  closedir (dir);

  free (dir_name);
  *list = llist;

  return 0;
}

/** \brief Returns the path to the description file. Or NULL if none there.
*
*
* \param init the initialized data
* \param world The world to check
* \param country The country
* \param description The description file returned (or null)
* \return The error
* \author Thorsten Sick
* \date 13.7.07
* \callgraph
*/
int
libnamegen_get_description (struct libnamegen_init_int *init, char *world,
			    char *country, char **description)
{
  char *filename = NULL;
  FILE *file = NULL;

  filename =
    create_filename (init->db_base_path, world, country, "description.html");
  if (filename == NULL)
    {
      *description = NULL;
      return 0;
    }

  file = fopen (filename, "rt");
  if (file == NULL)
    {
      free (filename);
      *description = NULL;
      return 0;
    }
  fclose (file);
  *description = filename;
  return 0;
}

/** \brief Returns the path to the logo file. Or NULL if none there.
*
*
* \param init the initialized data
* \param world The world to check
* \param country The country
* \param description The description file returned (or null)
* \return The error
* \author Thorsten Sick
* \date 13.7.07
* \callgraph
*/
int
libnamegen_get_logo (struct libnamegen_init_int *init, char *world,
		     char *country, char **logo)
{
  char *filename = NULL;
  FILE *file = NULL;

  filename = create_filename (init->db_base_path, world, country, "logo.png");
  if (filename == NULL)
    {
      *logo = NULL;
      return 0;
    }

  file = fopen (filename, "rt");
  if (file == NULL)
    {
      free (filename);
      *logo = NULL;
      return 0;
    }
  fclose (file);
  *logo = filename;
  return 0;
}
