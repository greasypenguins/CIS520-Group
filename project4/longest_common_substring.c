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
uint32 lcs_dynamic(char * ret, char * a, uint32 a_size, char * b uint32 b_size)
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
    for(a_idx = 0; a_idx < a_size; a_idx++)
    {
        substring_lengths[a_idx] = (uint16 *)malloc(sizeof(uint16) * b_size);
    }

    /* Compare each character in a with each character in b */
    for(a_idx = 0; a_idx < a_size; a_idx++)
    {
        for(b_idx = 0; b_idx < b_size; b_idx++)
        {
            /* If the characters match */
            if(a[a_idx] == b[b_idx])
            {
                /* If one of the characters is the starting character for that string */
                if((a_idx == 0)
                || (b_idx == 0))
                {
                    substring_lengths[a_idx][b_idx] = 1;
                }
                else
                {
                    substring_lengths[a_idx][b_idx] = substring_lengths[a_idx - 1][b_idx - 1] + 1;
                }

                /* Only override longest common substring if a longer common substring is found */
                if(substring_lengths[a_idx][b_idx] > max_length)
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
    for(i = 0; i < max_length; i++)
    {
        ret[i] = a[a_idx_max - max_length + i + 1];
    }
    ret[max_length] = '\0';

    /* Free dynamically allocated memory for array */
    for(a_idx = 0; a_idx < a_size; a_idx++)
    {
        free(substring_lengths[a_idx]);
    }
    free(substring_lengths);

    return max_length;
}

