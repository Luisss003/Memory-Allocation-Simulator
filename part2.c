/*
* part2.c
* 
* This program simulates a simple paging system with LRU page replacement.
* It reads logical addresses from an input file, translates them to physical
* addresses, and writes the results to an output file. It also counts the
* number of page faults that occur during the translation process.
*
* Usage: ./part2 infile outfile
*/

#include <stdio.h>
#include <stdlib.h>

//Define constants for offset and mask
#define PAGE_SIZE 128
#define OFFSET_BITS 7
#define OFFSET_MASK 0x7F

//Define constants for number of frames and pages
#define NUM_FRAMES 8
#define NUM_PAGES 32

//Page Table Entry (PTE) structure
typedef struct {
    int valid_bit; //1 if valid, 0 if invalid
    int frame_number; //Frame number in physical memory
} PTE;

// Function to select victim frame based on LRU policy
int lru_vic_frame(int LRUcount[]) {
    int oldest_time = LRUcount[1];
    int victim_frame = 1;
    for (int i = 2; i < NUM_FRAMES; i++) {
        if (LRUcount[i] < oldest_time) {
            oldest_time = LRUcount[i];
            victim_frame = i;
        }
    }
    return victim_frame;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s infile outfile\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    FILE *outfile = fopen(argv[2], "wb");
    if (outfile == NULL) {
        printf("Error opening output file\n");
        fclose(infile);
        return 1;
    }

    //Initialize page table
    PTE PT[NUM_PAGES];
    // Initialize free frame table (1 = free, 0 = occupied)
    int freeframes[NUM_FRAMES] = {0, 1, 1, 1, 1, 1, 1, 1}; 
    
    //Initialize reverse mapping table
    int revmap[NUM_FRAMES];

    
    //Initialize LRU count array
    int LRUcount[NUM_FRAMES];
    
    //Initialize logical address (LA) and physical address (PA)
    unsigned long LA, PA;

    //Initialize current time and page fault count
    int current_time = 0;
    int page_faults = 0;

    //Initialize page table and reverse mapping
    for (int i = 0; i < NUM_PAGES; i++) {
        PT[i].valid_bit = 0; // Initially all pages are invalid
        PT[i].frame_number = -1; // No frame assigned
    }
    for (int i = 0; i < NUM_FRAMES; i++) {
        revmap[i] = -1; // No page assigned to any frame
        LRUcount[i] = 0; // Initialize LRU count
    }

    //While there are logical addresses to read
    while (fread(&LA, sizeof(unsigned long), 1, infile) == 1) {
        // Calculate the page number and offset from logical address
        unsigned long pnum = LA >> OFFSET_BITS;
        unsigned long dnum = LA & OFFSET_MASK;
        int fnum;

        //Check if extracted page number is valid
        if (PT[pnum].valid_bit == 1) {
            //Page is valid, just map it
            fnum = PT[pnum].frame_number;
        } else { //Page is invalid
            //Page fault occurs
            page_faults++;

            //Try to find a free frame first
            int found_free = 0;
            for (int i = 1; i < NUM_FRAMES; i++) {
                if (freeframes[i] == 1) {
                    fnum = i; //Assign free frameq
                    freeframes[i] = 0; //Mark it as occupied
                    found_free = 1; //Mark that we found a free frame
                    break;
                }
            }

            //If no free frame is found, use LRU replacement
            if (found_free == 0) {
                //Find the victim frame using LRU policy
                int victim_frame = lru_vic_frame(LRUcount);

                //Invalidate old page
                int victim_page = revmap[victim_frame];
                if (victim_page != -1) {
                    PT[victim_page].valid_bit = 0;
                }
                //Mark the victim frame as free
                fnum = victim_frame;
            }

            //Update page table and reverse map
            PT[pnum].valid_bit = 1;
            PT[pnum].frame_number = fnum;
            revmap[fnum] = pnum;
        }

        //Update LRU counter
        current_time++;
        //Update the LRU count for the current frame
        LRUcount[fnum] = current_time; 

        //Calculate physical address
        PA = (fnum << OFFSET_BITS) + dnum;

        //Write binary of unsigned long to file
        if (fwrite(&PA, sizeof(unsigned long), 1, outfile) != 1) {
            perror("Error writing to output file");
            fclose(infile);
            fclose(outfile);
            return 1;
        }
    }

    printf("Part 2 page faults: %d\n", page_faults);

    //Close the files
    fclose(infile);
    fclose(outfile);

    return 0;
}
