/**
 *
 * mycommand.c
 * Grundlagen der Programmierung 2
 * Testprogramm zu getopt()
 * Author: Alexander Nimmervoll
 * Last Modified: 2005-04-04
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>

/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
   fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] …[filenameN] \n", program_name);
   exit(EXIT_FAILURE);
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[])
{
   std::string searchpath = "";  
   std::vector<std::string> filenames = {"" };


   int c;
   int error = 0;
   char *inputFile = NULL;
   int cOptionA = 0;

   program_name = argv[0];

   while ((c = getopt(argc, argv, "iR:")) != EOF)
   {
     printf("%d: argR\n", c);
      switch (c)
      {
      case 'R':        /* Option ohne Argument */
         if (cOptionA) /* mehrmalige Verwendung? */
         {
            error = 1;
            break;
         }
         cOptionA = 1;
         printf("%s: parsing option R\n", program_name);
         break;
      case 'i':                 /* Option mit Argument */
         if (inputFile != NULL) /* mehrmalige Verwendung? */
         {
            error = 1;
            break;
     
      }
   }
   if (error) /* Optionen fehlerhaft ? */
   {
      print_usage();
   }
   if ((argc < optind + 1) || (argc > optind + 2)) /* falsche Anzahl an Optionen */
   {
      print_usage();
   }

   /* Die restlichen Argumente, die keine Optionen sind, befinden sich in
    * argv[optind] bis argv[argc-1]
    */
   while (optind < argc)
   {
      /* aktuelles Argument: argv[optind] */
      filenames.push_back (argv[optind]);
      optind++;
   }


    for(unsigned int i = 0; i < filenames.size(); i++)
  {
    std::cout << "filenames[i]" << std::endl;
    
  }

   return EXIT_SUCCESS;
}
}