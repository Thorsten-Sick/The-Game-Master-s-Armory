/* Created by Anjuta version 1.2.0 */
/*	This file will not be overwritten */
/*
* Parameters:
* rpg_names m midgard alba
*  m/f: male/female
*  midgard: the name of the rpg
*  alba: the name of the country
*
*
*
* data is read from
* basedir/midgard/names/alba
*
*
* Needed files
* m male names
* f femine names
* fam family names
* style Name-style Contains one line (more to come)
* 1
* ---8< ----
* 1 is the name-style. At the moment, there is only on style.
*
* Written by Thorsten Sick
* thorstensick@users.sourceforge.net
* under the Terms of the LGPL (Lesser GPL).
* For more: See readme.txt
*/



#include "libnamegen.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>


/** \brief prints a list of the database
*
*
*
* \param cmdline_test prin additional data
* \author Thorsten Sick
* \date 13.3.2007
* \return 0 on success
* \callgraph
*/
int list_db(gint      cmdline_test)
{
	//GPtrArray * array=NULL;
		int error;
	
	int ierror;
	guint64 i;
	struct libnamegen_linked_string_list *tags=NULL;
	struct libnamegen_linked_string_list *list_runner=NULL;
		
	gchar command[LIBNAMEGEN_MAX_RULE_LENGTH];

	// Version information
	int major;
	int minor;
	
	// Init information
	struct libnamegen_initstruct init; // Here initialization data is passed
	struct libnamegen_init_int * data=NULL; // The initialized data is in here
	
	struct libnamegen_linked_string_list *worldlist=NULL;
	struct libnamegen_linked_string_list *world=NULL;
		
	struct libnamegen_linked_string_list *countrylist=NULL;
	struct libnamegen_linked_string_list *country=NULL;
	
	char * description=NULL;
	
	//GError *gerror = NULL;

	gulong statf=0, statm=0, smallest_stat=0;	// Statistics for the databases
	
	#ifdef WINCROSS
		char * total_dir=NULL;
		char * dir=NULL;
	#endif
	
	

		libnamegen_getver(&major, &minor);
		#ifndef DEBUG
		printf("Name generation library %d.%d / built for %s\n",major, minor, OS);
		#else
		printf("Name generation library %d.%d (debug version) / built for %s\n",major, minor, OS);
		#endif	
		memset(&init,0,sizeof(init));
		#ifndef WINCROSS
			init.db_base_path=BASEDIR;
		#else
			dir=getcwd(NULL,0);
			total_dir=malloc(strlen(dir)+strlen(G_DIR_SEPARATOR_S)*2+strlen("data")+1);
			if (total_dir==NULL)
				goto clean_up_the_mess;
			strcpy(total_dir,dir);
			strcat(total_dir,G_DIR_SEPARATOR_S);
			strcat(total_dir,"data");
			strcat(total_dir,G_DIR_SEPARATOR_S);
			init.db_base_path=total_dir;
		#endif
		init.expected_api=LIBNAMEGEN_API_VERSION;
		init.expected_db=LIBNAMEGEN_DB_VERSION;
		error=libnamegen_init(init, &data);
		if (error)
		{
			printf("Error initializing\n");
		}
		printf("Installed worlds:\n");
		
		libnamegen_get_worlds(data,&worldlist);
		
		world=worldlist;
		
		while (world != NULL)
		{
			printf("%s\n", world->string);
			libnamegen_get_countries(data, (char *)world->string, &countrylist);
			
			country=countrylist;
			while (country != NULL)
			{
				printf(" %s", country->string);
				libnamegen_get_description (data,(char *)world->string,(char *)country->string,&description);
				if (description)
				{
					printf(" D ");
					free(description);
					description=NULL;
				}
				
				ierror=libnamegen_get_db_stats (data, (char *)world->string, (char *)country->string, LIBNAMEGEN_FEMALE,&statf);
				if (ierror)
				{
					printf("Error %d\n", ierror);
					statf=0;
				}
				ierror=libnamegen_get_db_stats (data, (char *)world->string, (char *)country->string, LIBNAMEGEN_MALE,&statm);
				if (ierror)
				{
					printf("Error %d\n", ierror);
					statm=0;
				}
				if (statf<statm)
					smallest_stat=statf;
				else
					smallest_stat=statm;
				// Print world-country,femal,male
				printf("\t");
				if (strlen((char *)country->string)<12) // One tab if countrylength is smaler then 12
					printf("\t");
				if (strlen((char *)country->string)<7) // Add another tab if countrylength is smaler then 7
					printf("\t");
				printf("Female names: %lu Male names: %lu  ", // %lu = long unsigned (%ul)?		
														statf,
														statm);
				// Print rating. This is for rating by the number of names available.
				if (smallest_stat==0)
					printf("There\n");
				if ((smallest_stat>0) && (smallest_stat<=100))
					printf("Well...\n");
				if ((smallest_stat>100) && (smallest_stat<=1000))
					printf("Good\n");
				if ((smallest_stat>1000) && (smallest_stat<=10000))
					printf("Very Good\n");
				if ((smallest_stat>10000) && (smallest_stat<=100000))
					printf("Impressive\n");
				if ((smallest_stat>100000) && (smallest_stat<=1000000))
					printf("Very Impressive\n");
				if ((smallest_stat>1000000))
					printf("Freakish\n");

				error=libnamegen_extract_tags (data, (char *)world->string,(char *)country->string,  LIBNAMEGEN_SEX_WILDCARD,&tags);
				if (error)
				{
					printf("TAG Error %d\n", error);
					error=0;
				}
				if (tags)
				{
					g_printf("Tags present: ");
					list_runner=tags;
					while (list_runner)
					{
						g_printf("%s ",list_runner->string);
						list_runner=list_runner->next;
					}					
					g_printf("\n");
				}				
				libnamegen_free_linked_string_list(tags);
				tags=NULL;
				
				if (cmdline_test==1)
				{ // Additional: Print all the male and female rules
					i=0;
					printf("Male rules\n");					
					while (libnamegen_get_unstripped_command_by_number (command,data,(char *)world->string,(char *)country->string, LIBNAMEGEN_MALE, "", i)==LIBNAMEGEN_ERROR_NONE_CONTINUE)
					{
									
						printf("%s\n", command);		
						i++;					
					}

					i=0;
					printf("Female rules\n");					
					while (libnamegen_get_unstripped_command_by_number (command,data,(char *)world->string,(char *)country->string, LIBNAMEGEN_FEMALE, "", i)==LIBNAMEGEN_ERROR_NONE_CONTINUE)
					{	
						
						printf("%s\n", command);		
						i++;					
					}	
				}
				if (cmdline_test==2)
				{
					ierror=libnamegen_get_db_stats (data, (char *)world->string,(char *)country->string,  LIBNAMEGEN_FEMALE,&statf);
					if (ierror)
					{
						printf("Error %d\n", ierror);

					}
					ierror=libnamegen_get_db_stats (data, (char *)world->string, (char *)country->string,  LIBNAMEGEN_MALE,&statm);
					if (ierror)
					{
						printf("Error %d\n", ierror);

					}
					if (statf<statm)
						smallest_stat=statf;
					else
						smallest_stat=statm;
					// Print world-country,female,male
					printf("DB-quality: %s-%s,%lu,%lu,", 
					world->string, country->string, 
					statf,
					statm);
					// Print rating
					if (smallest_stat==0)
						printf("There\n");
					if ((smallest_stat>0) && (smallest_stat<=100))
						printf("Well...\n");
					if ((smallest_stat>100) && (smallest_stat<=1000))
						printf("Good\n");
					if ((smallest_stat>1000) && (smallest_stat<=10000))
						printf("Very Good\n");
					if ((smallest_stat>10000) && (smallest_stat<=100000))
						printf("Impressive\n");
					if ((smallest_stat>100000) && (smallest_stat<=1000000))
						printf("Very Impressive\n");
					if ((smallest_stat>1000000))
						printf("Freakish\n");

				
				}
				if (cmdline_test==3)
				{
					// Print all rules in this rule file
					i=0;
					
					while (libnamegen_get_unstripped_command_by_number (command, data, (char *)world->string, (char *)country->string,	 	
						LIBNAMEGEN_SEX_WILDCARD, "*", i)==LIBNAMEGEN_ERROR_NONE_CONTINUE)
					{						
						libnamegen_strip_command(command);
						g_printf("Command  %s\n",command);
						i++;
					}
					error=libnamegen_extract_tags (data, (char *)world->string,(char *)country->string,  LIBNAMEGEN_SEX_WILDCARD,&tags);
					if (error)
					{
						printf("Error %d\n", error);
						error=0;
					}
					if (tags)
					{
						g_printf("Tags present: ");
						list_runner=tags;
						while (list_runner)
						{
							g_printf("Data from array: %s\n",list_runner->string);
							list_runner=list_runner->next;
						}						
						g_printf("\n");						
					}
					libnamegen_free_linked_string_list(tags);
					tags=NULL;
				}
				
				
				
				country=country->next;
				
			}
			libnamegen_free_linked_string_list(countrylist);
			countrylist=NULL;
			world=world->next;
			
		}
			
		
		#ifdef WINCROSS
		clean_up_the_mess:
		#endif
				
		
		libnamegen_free_linked_string_list(worldlist);
		libnamegen_free_linked_string_list(countrylist);
		libnamegen_done(&data);	
		return 0;
}



int main(int argc, char *argv[]) // The int is for the optional amount of names to be generated, the string (char*) is for the arguments.
{
	gchar name[LIBNAMEGEN_MAX_NAMELENGTH];
	gchar name2[LIBNAMEGEN_MAX_NAMELENGTH];
		int ierror;
	//gchar name_written[LIBNAMEGEN_MAX_NAMELENGTH];
	//gchar ** tags;
	//glong a;

	int error;

	guint64 i;
	
	char * description=NULL;
	char * logo=NULL;
	
	// Init information
	struct libnamegen_initstruct init; // Here initialization data is passed
	struct libnamegen_init_int * data=NULL; // The initialized data is in here
	
	// The sex being questioned
	int sex=0;

	gulong statf;//, statm, smallest_stat;	// Statistics for the databases
										// gulong corresponds to the standard C unsigned long type

	// Command line settings
	static gboolean  cmdline_list_db = FALSE;            /**< if set, lists databases*/
	static gchar     *cmdline_sex = NULL;                /**< The sex the user requested*/
	static gint      cmdline_number_of_names = 1;        /**< The number of names the used wants*/
	static gchar     *cmdline_world = NULL;              /**< The world requested*/
	static gchar     *cmdline_country = NULL;            /**< The country requested*/
	static gchar     *cmdline_style = NULL;              /**< Style of name*/
	static gint      cmdline_test = 0;                   /**< 0: run no test, every thing else will test*/
	static gchar     *cmdline_tags = NULL;               /**< The user can chose tags for a better rule selection.*/

	#ifdef WINCROSS
	char * dir;
	char * total_dir;
	#endif

	//GPtrArray * array=NULL;

	static GOptionEntry entries[] = 
	{
		{ "number-of-names", 'n', 0, G_OPTION_ARG_INT, &cmdline_number_of_names, "Set number of names to generate", NULL },
		{ "sex", 's', 0, G_OPTION_ARG_STRING, &cmdline_sex, "Set kind of sex to request: m for male, f for female, v=village, c=city, h=mountain, r=river, l=lake, t=tavern/hotel ", NULL },
		{ "style", 0, 0, G_OPTION_ARG_STRING, &cmdline_style, "if you want something other than just the ascii name, you can submit \n\t'all' for full name \n\t'html' for a html file, \n\t'unicode' for unicode name only \n\t'ascii' for ascii name only \n\t'notag' removes HTML tags like links", NULL },
  	{ "list-db", 'l', 0, G_OPTION_ARG_NONE, &cmdline_list_db, "First list the database", NULL },
  	{ "world", 'w', 0, G_OPTION_ARG_STRING, &cmdline_world, "Select the world to generate names for", NULL },
  	{ "country", 'c', 0, G_OPTION_ARG_STRING, &cmdline_country, "Select the country to generate names for", NULL },
		{ "test", 0 ,0, G_OPTION_ARG_INT, &cmdline_test, "Only for testing purpose of the lib, run test number ...",NULL},
		{ "tags",'t',0,G_OPTION_ARG_STRING, &cmdline_tags,"You can specify a set of tags to chose the rule you want. Default: *", NULL},
  	{ NULL }
	};	

	GOptionContext *context = NULL;
	GError *gerror = NULL;


	// Parse Command line
	context = g_option_context_new ("Generate names");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &gerror);
	


	if (cmdline_list_db)
	{
		list_db(cmdline_test);

	}
	
	
	memset(name,'\0',LIBNAMEGEN_MAX_NAMELENGTH);
	

	if (
		(cmdline_sex) &&
	   	(cmdline_world) &&
	   	(cmdline_country)
	   )
	{
		if (strcmp(cmdline_sex,"m")==0)
			sex=LIBNAMEGEN_MALE;
		else if (strcmp(cmdline_sex,"f")==0)
			sex=LIBNAMEGEN_FEMALE;
		else if (strcmp(cmdline_sex,"v")==0)
			sex=LIBNAMEGEN_VILLAGE;
		else if (strcmp(cmdline_sex,"c")==0)
			sex=LIBNAMEGEN_CITY;
		else if (strcmp(cmdline_sex,"h")==0)
			sex=LIBNAMEGEN_MOUNTAIN;
		else if (strcmp(cmdline_sex,"r")==0)
			sex=LIBNAMEGEN_RIVER;
		else if (strcmp(cmdline_sex,"l")==0)
			sex=LIBNAMEGEN_LAKE;
		else if (strcmp(cmdline_sex,"t")==0)
			sex=LIBNAMEGEN_TAVERN;
	}	
	if (sex)
	{
		
		memset(&init,0,sizeof(init));
		#ifndef WINCROSS
			init.db_base_path=BASEDIR;
		#else
			dir=getcwd(NULL,0);
			total_dir=malloc(strlen(dir)+strlen(G_DIR_SEPARATOR_S)*2+strlen("data")+1);
			if (total_dir==NULL)
				goto clean_up_the_mess;
			strcpy(total_dir,dir);
			strcat(total_dir,G_DIR_SEPARATOR_S);
			strcat(total_dir,"data");
			strcat(total_dir,G_DIR_SEPARATOR_S);
			init.db_base_path=total_dir;
		#endif
		init.expected_api=LIBNAMEGEN_API_VERSION;
		init.expected_db=LIBNAMEGEN_DB_VERSION;
		error=libnamegen_init(init, &data);
		if (error)
		{
			printf("Error initializing\n");
		}
		else
		{
		if ((cmdline_style != NULL) && 
		    (strcmp(cmdline_style,"html")==0))
		{
			/********************
			*					*
			*Generate HTML Code *
			*					*
			*					*
			*********************/
		/*printf("<html>\n<head><title>Libnamegen - %s %s</title></head><body><h1>%s %s</h1>Libnamegen is an Open Source Library to generate names. It can be found at http://libnamegen.sourceforge.net/<br>", cmdline_world, cmdline_country,cmdline_world, cmdline_country);*/
		printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
		printf("<html>\n <head>\n <title>Libnamegen --- Example %s %s </title>\n ", cmdline_world, cmdline_country);
		printf("</head>\n <body>\n <table width=100%%>\n <tr><!-- Titel row-->\n");
		printf("<td width=100%%>\n <center>\n  <img src=\"logo.png\">\n  </center>\n<td>\n </tr>\n<tr><!-- Control row-->\n<td width=100%%>\n <center>\n<font size=\"-1\">\n  <a href=index.html>About</a> \n <a href=examples.html>Examples</a> \n  <a href=documentation.html>Documentation</a> \n <a href=download.html>Download</a>\n   <a href=authors.html>Authors</a>\n	      </font>\n	    </center>\n	  </td>\n    </tr>\n	<tr height=10> <!-- Spacer-->\n	</tr>\n	<tr> <!-- Content row-->\n  <table width=100%%>\n    <tr>\n<td>\n</td>\n	<td width=80%%> ");
		printf("<h1>%s %s </h1>Libnamegen is an Open Source Library to generate names. It can be found at http://libnamegen.sourceforge.net/<br>", cmdline_world, cmdline_country);
		libnamegen_get_description (data,(char *)cmdline_world,(char *)cmdline_country,(char **)&description);
		if (description)
		{			
			// Add a line lining to the file desctiption-$world-$country.html
			printf("<a href='description-%s-%s.html'>Description available</a>", cmdline_world, cmdline_country);
			free(description);
			description=NULL;
		}		
		libnamegen_get_logo (data,cmdline_world,cmdline_country,&logo);
		if (logo)
		{			
			// Add a line lining to the file desctiption-$world-$country.html
			printf("<center><img src='logo-%s-%s.png'></center>", cmdline_world, cmdline_country);
			free(logo);
			logo=NULL;
		}		
			
		// Iterate through sexes
		for (sex=1; sex<=8; sex++)
		{



			ierror=libnamegen_get_db_stats (data, cmdline_world,cmdline_country, sex,&statf);
			if (ierror)
			{
				printf("Error %d\n", ierror);

			}
			if (statf>0)
			{
			if (sex==LIBNAMEGEN_MALE)
				printf("<h2>Male names</h2>");
			if (sex==LIBNAMEGEN_FEMALE)
				printf("<h2>Female names</h2>");
			if (sex==LIBNAMEGEN_VILLAGE)
				printf("<h2>Village names</h2>");
			if (sex==LIBNAMEGEN_CITY)
				printf("<h2>City names</h2>");
			if (sex==LIBNAMEGEN_MOUNTAIN)
				printf("<h2>Mountain names</h2>");
			if (sex==LIBNAMEGEN_RIVER)
				printf("<h2>River names</h2>");
			if (sex==LIBNAMEGEN_LAKE)
				printf("<h2>Lake names</h2>");
			if (sex==LIBNAMEGEN_TAVERN)
				printf("<h2>Tavern names</h2>");

			printf("<table>\n");
			for (i=0;i<cmdline_number_of_names; i++)
			{ // Get names for HTML
				memset(name, 0, LIBNAMEGEN_MAX_NAMELENGTH);
				if (cmdline_tags)
				{
					error=libnamegen_getname(data, cmdline_world, cmdline_country, sex,cmdline_tags,name);
				}
				else
				{
					error=libnamegen_getname(data, cmdline_world, cmdline_country, sex,"*", name);
				}
				if (error)
				{
					printf("Error :%d\n", error);					
				}
				else
				{
					if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"ascii")==0))
					{						
						libnamegen_remove_unicode(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"notags")==0))
					{						
						libnamegen_remove_unicode(name);
						libnamegen_remove_html_tags(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"unicode")==0))
					{							
						libnamegen_keep_unicode(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"html")==0))
									
				     {								
						memcpy(name2, name, LIBNAMEGEN_MAX_NAMELENGTH); 
						libnamegen_remove_unicode(name);
						libnamegen_keep_unicode(name2);
						printf("<tr><td>%s</td><td>%s</td></tr>\n", name, name2);
					}
					else
					{
						g_printf("%s\n", name);
					}
						
						
				}	
				
			}
				printf("</table>\n");
			}	
				
		
			}	
			/*printf("<br><center><a href=\"http://sourceforge.net\"><img src=\"http://sourceforge.net/sflogo.php?group_id=148538&amp;type=6\" width=\"210\" height=\"62\" border=\"0\" alt=\"SourceForge.net Logo\" /></a></center></body></html>\n"); */

printf("</td>\n	<td>\n	</td>\n	 </tr>\n </table>\n	</tr>\n   </table>\n<p>\n<center>\n<a href=\"http://sourceforge.net\"><img src=\"http://sourceforge.net/sflogo.php?group_id=148538&amp;type=6\" width=\"210\" height=\"62\" border=\"0\" alt=\"SourceForge.net Logo\" /></a>\n</center>\n  </body>\n</html>");
		}
		else	
		{		
			for (i=0;i<cmdline_number_of_names; i++)
			{
				if (cmdline_tags)
				{
					error=libnamegen_getname(data, cmdline_world, cmdline_country, sex,cmdline_tags, name);
				}
				else
				{
					error=libnamegen_getname(data, cmdline_world, cmdline_country, sex,"*",	 name);
				}
				if (error)
				{
					printf("Error :%d\n", error);					
				}
				else
				{
					if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"ascii")==0))
					{						
						libnamegen_remove_unicode(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"notags")==0))
					{						
						libnamegen_remove_unicode(name);
						libnamegen_remove_html_tags(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"unicode")==0))
					{							
						libnamegen_keep_unicode(name);
						g_printf("%s\n", name);
					}
					else if ((cmdline_style != NULL) && 
					    (strcmp(cmdline_style,"html")==0))
					{									
						memcpy(name2, name, LIBNAMEGEN_MAX_NAMELENGTH); 
						libnamegen_remove_unicode(name);
						libnamegen_keep_unicode(name2);
						printf("<tr><td>%s</td><td>%s</td></tr>\n", name, name2)	;
					}
					else
					{
						libnamegen_remove_unicode(name);
						libnamegen_remove_html_tags(name);						
						g_printf("%s\n", name);
					}
						
						
						
				}
			}
		}
		}
		libnamegen_done(&data);		
		
	}
	
	#ifdef WINCROSS
	clean_up_the_mess:
	#endif
	g_option_context_free (context)	;	
	#ifdef WINCROSS
		g_free(dir);
		g_free(total_dir);
	#endif
	return (0);
}
