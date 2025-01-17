/*
Name:
ID number (Matricula):

Programming Fundamentals
Final Exam

Hotel occupancy and revenue program
This program will read a file containing daily information about several hotels
belonging to a chain, and will report the total ocupancy and revenue from
all hotels combined.

HINT: Write your name right now!!! You will loose 10 points if you don't
HINT: Read the provided code to understand what you have to do and how to
make it work with the existing program.

QUESTIONS:
1. Complete the data structure "hotel_data" that will hold the information for a single hotel
    read from the provided text file.
2. Create the function "read_file" that will read all the data from the text file and
	store it in an array of the data structure you created previously.
	Parameters received: the name of the file, the array of structures
	Return value: number of data entries read from the file or -1 if the file could not be opened
3. Create the function "print_hotel_data" that will show the contents of one structure on the screen
	Parameters received: a single structure
	Return value: nothing
4. Create the function "get_occupancy_percent" that will inform of the percent of rooms used in a
    single hotel. It should look for the hotel with the name provided, and then report the percent of rooms used.
    Parameters received: a string with the name of the hotel desired, the array of structures, the number of hotels
    Return value: percent of occupied rooms, or -1 if the hotel specified is not in the records
5. Create the function "compute_totals" that will compute the combined number of rooms, occupancy
    and revenue of all hotels during the day represented by the file.
    The results should be printed to the screen, and also to another file called: "hotel_totals.txt"
	Parameters received: the array of structures, the number of hotels
	Return value: nothing
*/
#define _CRT_SECURE_NO_DEPRECATE	// Stop the complaining from MS VS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 20
#define STRING_SIZE 25

// Structures
typedef struct hotel_data
{
	// QUESTION 1

} hotel_data;

// Function prototypes
void menu_loop(void);
// QUESTIONS 2, 3, 4, 5




int main(int argc, const char * argv[])
{
    printf("HOTEL REVENUE SYSTEM\n");
    printf("====================\n\n");
	menu_loop();

	return 0;
}

/*
Function that displays the menu and manages the flow of the program.
*/
void menu_loop()
{
	char * filename = "hotel_data_04122015.txt";
	hotel_data all_hotels[ARRAY_SIZE];
    char hotel_name[STRING_SIZE];
	int num_hotels = 0;
	char option = 'a';
	int i;
    float occupancy;

	// Init the data from the file
    // Call to function of QUESTION 2
	num_hotels = read_file(filename, all_hotels);
	if (num_hotels == -1)
	{
		printf("Fatal Error! Better luck next time.\n");
		exit(-1);
	}
	printf("The file '%s' contained data for %d hotels.\n", filename, num_hotels);

	// Start the user menu
	while (option != 'q')
	{
		printf("\nUser menu:\n");
		printf("\ta. Print information of all hotels\n");
        printf("\tb. Print the percent of rooms used in a single hotel\n");
		printf("\tc. Compute the totals for the hotels\n");
		printf("\tq. Exit the program\n");
		printf("Choose an option: ");
		scanf(" %c", &option);
		switch (option)
		{
		case 'a':
			// Show information for all hotels
			for (i = 0; i < num_hotels; i++)
			{
                // Call to function of QUESTION 3
				print_hotel_data(all_hotels[i]);
			}
			break;
		case 'b':
			// Print percent of rooms occupied
            printf("For which hotel do you need the occupancy? ");
            scanf("%s", hotel_name);
            // Call to function of QUESTION 4
            occupancy = get_occupancy_percent(hotel_name, all_hotels, num_hotels);
            if (occupancy != -1)
                printf("The hotel '%s' is occupied to the %.2f percent.\n", hotel_name, occupancy);
            else
                printf("There is no hotel named '%s'\n", hotel_name);
			break;
		case 'c':
            // Compute the totals of occupancy for all hotels
            // Call to function of QUESTION 5
			compute_totals(all_hotels, num_hotels);
			break;
		case 'q':
			printf("Thanks for using the system\n");
			break;
		default:
			printf("Invalid option! Try again ...\n");
			break;
		}
	}
}

// QUESTION 2
/*
Read a text file with the following format, and store the contents
in the array of structures received as a parameter
Returns the number of hotels read from the file, or -1 if
the file could not be opened for reading
*/


// QUESTION 3
/*
Print the contents of a single game structure
*/


// QUESTION 4
/*
Find the percent of rooms occupied for the hotel named as a parameter. You must look for
the hotel in the array of structures. If the hotel is found, return the percentage.
If the hotel is not found, return -1.
*/


// QUESTION 5
/*
Compute the total of number of rooms, rooms occupied and total revenue.
Print the results to the screen, and write them to a file.
*/


