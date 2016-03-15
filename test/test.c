#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#define MAX_PROC_ENTRIES 4
#define PROC_LEN 20
#define BUFF_LEN 1024

FILE *fd[MAX_PROC_ENTRIES];

/* Print driver statistics i.e data from proc entries */
void test_1()
{
        int i = 0, rc = 0;
        char proc_entry[PROC_LEN];
        for (i = 0; i < MAX_PROC_ENTRIES; i++) {
                char data[BUFF_LEN];
                memset(data, 0, BUFF_LEN);
                sprintf(proc_entry, "/proc/proc_%d", i + 1);
                fd[i] = fopen(proc_entry, "r");
                fread(data, BUFF_LEN, sizeof(char), fd[i]);
                if (ferror(fd[i]))
                        printf("Proc_%d: Error: %s\n", i + 1, strerror(errno));
                else
                        printf("Proc_%d: %s\n", i + 1, data);
        }
        
}

/* Write to proc entries */
void test_2()
{
    int i = 0, rc = 0;
        char proc_entry[PROC_LEN];
        for (i = 0; i < MAX_PROC_ENTRIES; i++) {

	    sprintf(proc_entry, "/proc/proc_%d", i + 1);
	    fd[i] = fopen(proc_entry, "w");
	    rc = fprintf(fd[i], "1");
	    fflush(fd[i]);
            if (errno) {
                printf("Proc_%d: Error: %s \n", i + 1, strerror(errno));            
                errno = 0;    /* Set to zero if any error number is set previously*/
            }
            else 
                printf("Proc_%d: %s \n", i + 1, strerror(errno));            
        }
        
}

int main(int argc, char *argv[])
{
        char *choice = NULL;
        int rc = 0, arg = 0;
        int test_num = 0;
        
        if (argc < 2) {
                printf("usage: %s <test_number> \n", argv[0]);
                goto out;
        }
        
        choice = argv[++arg];
        test_num = atoi(choice);
        switch (test_num) {
        case 1: /* Print driver statistics i.e data from proc entries */
                test_1();
                break;
        case 2: /* Write to proc entries */
                test_2();
                break;

        default:
                assert(2 == 1);
                return -1;
        }
out:
        return rc;
}
