/*
 * part3.c
 * 
 * This program simulates a paging system with LRU page replacement,
 * dynamically based on command-line parameters for page size, virtual memory,
 * and physical memory sizes.
 *
 * Usage: ./part3 BytesPerPage SizeOfVirtualMemory SizeOfPhysicalMemory SequenceFile OutputFile
 */

 #include <stdio.h>
 #include <stdlib.h>
 
 typedef struct {
     int valid_bit;
     int frame_number;
 } PTE;
 
 // Helper function to compute log2(x)
 int log2_int(unsigned long x) {
     int res = 0;
     while (x > 1) {
         x = x >> 1;
         res++;
     }
     return res;
 }
 
 // Helper function to select a victim frame using LRU
 int select_victim_frame(int *LRUcount, int num_frames) {
     int oldest_time = LRUcount[1];
     int victim_frame = 1;
     for (int i = 2; i < num_frames; i++) {
         if (LRUcount[i] < oldest_time) {
             oldest_time = LRUcount[i];
             victim_frame = i;
         }
     }
     return victim_frame;
 }
 
 int main(int argc, char *argv[]) {
     
    if (argc != 6) {
         printf("Usage: %s BytesPerPage SizeOfVirtualMemory SizeOfPhysicalMemory SequenceFile OutputFile\n", argv[0]);
         return 1;
     }
     int i;
     unsigned long BytesPerPage = atoi(argv[1]);
     unsigned long SizeOfVirtualMemory = atoi(argv[2]);
     unsigned long SizeOfPhysicalMemory = atoi(argv[3]);
     unsigned long LA, PA;
     int current_time = 0;
     int page_faults = 0;


     FILE *infile = fopen(argv[4], "rb");
     if (infile == NULL) {
         printf("Error opening input file");
         return 1;
     }
 
     FILE *outfile = fopen(argv[5], "wb");
     if (outfile == NULL) {
         printf("Error opening output file");
         fclose(infile);
         return 1;
     }
 
     //Calculate number of offset bits based on page size
     int d = log2_int(BytesPerPage); 
     
     //Calculate number of page bits based on virt mem size and offset
     int p = log2_int(SizeOfVirtualMemory) - d;

     //Calculate number of frame bits based on phys mem size and offset
     int f = log2_int(SizeOfPhysicalMemory) - d;
 
    //Create bitmask to extract offset from LA
     unsigned long offset_mask = ((unsigned long)1 << d) - 1;

     //Calculate number of pages and frames
     int num_pages = 1 << p;
     int num_frames = 1 << f;
 
     //Dynamically allocate mem for page table and tracking arrays
     PTE *PT = (PTE *)malloc(sizeof(PTE) * num_pages);
     
     //Track free frames
     int *freeframes = (int *)malloc(sizeof(int) * num_frames);
     
     //Frame to page reverse mapping
     int *revmap = (int *)malloc(sizeof(int) * num_frames);
     
     //Track last access time for LRU
     int *LRUcount = (int *)malloc(sizeof(int) * num_frames);
 
// Check for memory allocation errors
    if (PT == NULL || freeframes == NULL || revmap == NULL || LRUcount == NULL) {
        printf("Memory allocation failed\n");
        fclose(infile);
        fclose(outfile);
        free(PT);
        free(freeframes);
        free(revmap);
        free(LRUcount);
        return 1;
    }
 
     // Initializepage table entries
     for (i = 0; i < num_pages; i++) {
         PT[i].valid_bit = 0;
         PT[i].frame_number = -1;
     }

     //Initialize frame tracking arrays
     for (int i = 0; i < num_frames; i++) {
         if (i == 0){
             freeframes[i] = 0; //frame 0 reserved
         }
         //Other frames are initially free 
         else{
             freeframes[i] = 1;
        }
        //No pages mapped yet
        revmap[i] = -1;

        //No frames have been acecssed either
        LRUcount[i] = 0;
     }
 
     //While there is data to be read from the input file
     while (fread(&LA, sizeof(unsigned long), 1, infile) == 1) {
         //Calculate page number and offset bits
         unsigned long pnum = LA >> d;
         unsigned long dnum = LA & offset_mask;
         int fnum;
 
         //Check if page is valid
         if (PT[pnum].valid_bit == 1) {
             // Page is valid
             fnum = PT[pnum].frame_number;
         } 
         else { //Otherwise, page is not valid and needs to be loaded
             // Page fault
             page_faults++;
 
             // Find free frame
             int found_free = 0;
             for (int i = 1; i < num_frames; i++) {
                 if (freeframes[i] == 1) {
                     fnum = i;
                     freeframes[i] = 0;
                     found_free = 1;
                     break;
                 }
             }
 
             //If there is no free frame found, use LRU replacement
             if (found_free == 0) {
                 
                 int victim_frame = select_victim_frame(LRUcount, num_frames);
 
                // Invalidate the page currently using the victim frame
                 int victim_page = revmap[victim_frame];
                 if (victim_page != -1) {
                     PT[victim_page].valid_bit = 0;
                 }
                 // Assign the victim frame to the new page
                 fnum = victim_frame;
             }
 
             // Update page table and reverse map
             PT[pnum].valid_bit = 1;
             PT[pnum].frame_number = fnum;
             revmap[fnum] = pnum;
         }
 
        // Update the last-used time for the frame (for LRU tracking)         current_time++;
         LRUcount[fnum] = current_time;
 
         // Calculate physical address
         PA = (fnum << d) + dnum;
 
         //Write physical addresses to the output file
         if (fwrite(&PA, sizeof(unsigned long), 1, outfile) != 1) {
             perror("Error writing to output file");
             fclose(infile);
             fclose(outfile);
             free(PT);
             free(freeframes);
             free(revmap);
             free(LRUcount);
             return 1;
         }
     }
 
     printf("Page faults: %d\n", page_faults);
 
     fclose(infile);
     fclose(outfile);
     free(PT);
     free(freeframes);
     free(revmap);
     free(LRUcount);
 
     return 0;
 }
 