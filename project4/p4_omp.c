/*
Command to compile:
gcc p4_omp.c -fopenmp -o p4_omp
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>

#define NUM_THREADS 1
#define NUM_LINES 20
#define LINE_LENGTH 2005
#define FILENAME "/homes/dan/625/wiki_dump.txt"
#define NUM_LINES_PER_THREAD (NUM_LINES / NUM_THREADS)

typedef unsigned long int uint32;
typedef unsigned int uint16;

uint32 actual_num_lines; /* Number of lines successfully read from file */
char data[NUM_LINES][LINE_LENGTH]; /* All data read in from file */
char lcs_data[NUM_LINES - 1][LINE_LENGTH]; /* longest common substrings */

void open_file(void);
uint32 lcs_dynamic(char*, const char*, uint32, const char*, uint32);
void thandle(int);

void open_file() 
{	
	int count;
	FILE *file;

	file = fopen(FILENAME, "r");
	if (file == NULL)
	{
		perror(FILENAME);
		return;
	}

	for(count = 0; count < NUM_LINES; count++)
	{
		if(fgets(data[count], LINE_LENGTH, file) == NULL)
		{
			break;
		}
	}

	actual_num_lines = count;

	fclose(file);
}


void thandle(int tid) {
	char temp[LINE_LENGTH]; /* temporary string storage     */
	char ret[LINE_LENGTH];  /* formatted string with lcs    */
	char * s1;              /* pointer to string 1          */
	char * s2;              /* pointer to string 2          */
	uint32 len_lcs;         /* length of lcs                */
	uint32 line_number;     /* index into data for s1       */
	uint32 start;           /* index into data for first s1 */
	uint32 end;             /* index into data for last s2  */

	#pragma omp private (tid, temp, ret, s1, s2, len_lcs, line_number, start, end)
	{
		start = tid * NUM_LINES_PER_THREAD;
		end = start + NUM_LINES_PER_THREAD;

		/* Avoid reading past the end of data */
		if(end > (actual_num_lines - 1))
		{
			end = actual_num_lines - 1;
		}

		for (line_number = start; line_number < end; line_number++)
		{
			s1 = data[line_number];
			s2 = data[line_number + 1];

			len_lcs = lcs_dynamic(ret, s1, strlen(s1), s2, strlen(s2));
			if (len_lcs > 0)
			{
				strcpy(lcs_data[line_number], ret);
			}
			else
			{
				strcpy(lcs_data[line_number], "No common substring.");
			}

		}
	}

	return;
}

/*
Find the longest common substring
Options are:
	suffix tree         O(m + n)
	dynamic programming O(m * n)
Dynamic programming is easier though

~dan/625/wiki_dump.txt

The longest line in wiki_dump.txt is 2000 characters.

So with dynamic programming we need 2000*2000*2 = 8MB of memory for each table in memory, assuming constant size tables.
With dynamic memory allocation, this is a maximum, not a constant.
*/




/*
Find the longest common substring between two strings using dynamic programming
Return the length of the LCS
Write LCS to ret
*/
uint32 lcs_dynamic(char * ret, const char * a, uint32 a_size, const char * b, uint32 b_size)
{
	uint32 i; /* loop index */
	uint32 a_idx; /* a index */
	uint32 b_idx; /* b index */

	uint32 a_idx_max = 0; /* a index of end of longest common substring */
	uint32 b_idx_max = 0; /* b index of end of longest common substring */
	uint32 max_length = 0; /* length of longest common substring */

	uint16 ** substring_lengths; /* uint16 limits max substring length to about 65535 characters, which is more than we need */

	/* Allocate space for array */
	substring_lengths = (uint16 **)malloc(sizeof(uint16 *) * a_size);
	for (a_idx = 0; a_idx < a_size; a_idx++)
	{
		substring_lengths[a_idx] = (uint16 *)malloc(sizeof(uint16) * b_size);
	}

	/* Compare each character in a with each character in b */
	for (a_idx = 0; a_idx < a_size; a_idx++)
	{
		for (b_idx = 0; b_idx < b_size; b_idx++)
		{
			/* If the characters match */
			if (a[a_idx] == b[b_idx])
			{
				/* If one of the characters is the starting character for that string */
				if ((a_idx == 0)
					|| (b_idx == 0))
				{
					substring_lengths[a_idx][b_idx] = 1;
				}
				else
				{
					substring_lengths[a_idx][b_idx] = substring_lengths[a_idx - 1][b_idx - 1] + 1;
				}

				/* Only override longest common substring if a longer common substring is found */
				if (substring_lengths[a_idx][b_idx] > max_length)
				{
					max_length = substring_lengths[a_idx][b_idx];
					a_idx_max = a_idx;
					b_idx_max = b_idx;
				}
			}
			/* If the characters don't match */
			else
			{
				substring_lengths[a_idx][b_idx] = 0;
			}
		}
	}

	/* Copy longest common substring to ret */
	for (i = 0; i < max_length; i++)
	{
		ret[i] = a[a_idx_max - max_length + i + 1];
	}
	ret[max_length] = '\0';

	/* Free dynamically allocated memory for array */
	for (a_idx = 0; a_idx < a_size; a_idx++)
	{
		free(substring_lengths[a_idx]);
	}
	free(substring_lengths);

	return max_length;
}



int main(void)
{
	int i; /* Loop counter */

	//struct timeval t1, t2, t3;
	omp_set_num_threads(NUM_THREADS);

	//gettimeofday(&t1, NULL);
	open_file();
	//gettimeofday(&t2, NULL);
	#pragma omp parallel
	{
		thandle(omp_get_thread_num());
	}
	
	for(i = 0; i < actual_num_lines - 1; i++)
	{
		printf("%3u - %3u: %s\n", i, i + 1, lcs_data[i]);
	}
	//gettimeofday(&t3, NULL);
	//double time = (t2.tv_sec = t1.tv_sec) * 1000.0;
	//time += (t2.tv_usec = t1.tv_usec) / 1000.0;
	//printf("Time to read data: %f\n", time);

	//time = t3.tv_sec - t2.tv_sec) * 1000.0;
	//time += (t3.tv_usec - t2.tv_usec) * 1000.0;
	//printf("Time to determine LCS: &f\n", time);
	printf("Program Completed. \n");

	return 0;
}
