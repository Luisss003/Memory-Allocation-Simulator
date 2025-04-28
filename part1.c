/*
 * part1.c
 * 
 * This program simulates a simple paging system based on a fixed page table.
 * It reads logical addresses from an input file, translates them to physical
 * addresses, and writes the results to an output file.
 * 
 * Usage: ./part1 infile outfile
 */

#include <stdio.h>
#include <stdlib.h>

//Define constants for offset and mask
#define OFFSET_BITS 7      
#define OFFSET_MASK 0x7F   

int main(int argc, char *argv[]) {
    unsigned long LA, PA;
    int PT[8] = {2, 4, 1, 7, 3, 5, 6, -1}; 
    int fnum;

    if (argc != 3) {
        printf("Usage: %s infile outfile\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        printf("Error opening input file");
        return 1;
    }

    FILE *outfile = fopen(argv[2], "wb");
    if (outfile == NULL) {
        printf("Error opening output file");
        fclose(infile);
        return 1;
    }

    //While there are logical addresses to read
    while (fread(&LA, sizeof(unsigned long), 1, infile) == 1) {
        //Calculate the page number and offset from logical address
        unsigned long pnum = LA >> OFFSET_BITS; 
        unsigned long dnum = LA & OFFSET_MASK;

        //Check if extracted page number is valid
        if (pnum >= 8 || PT[pnum] == -1) {
            printf("Invalid Page Number\n");
			fclose(infile);
            fclose(outfile);
            return 1;
        }
		
        //Use page # as index in page table
        fnum = PT[pnum];
        //Compute physical address
        PA = (fnum << OFFSET_BITS) + dnum;

		//Write binary of unsigned long to file
        if (fwrite(&PA, sizeof(unsigned long), 1, outfile) != 1) {
            printf("Error writing to output file");
            fclose(infile);
            fclose(outfile);
            return 1;
        }

        //Print logical and physical addresses
        printf("LA = %lx   PA = %lx\n", LA, PA);
    }

    //Close the files
    fclose(infile);
    fclose(outfile);

    return 0;
}
