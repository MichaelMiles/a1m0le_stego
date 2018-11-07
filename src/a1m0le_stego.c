#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "image.h"

int naive_hide();
static char mod_last_bit(char original, char to);
static char get_particular_bit(char byte, char position);
static char* read_file_name(int* possibleSize);
int main(int argc, char** argv){
  // the main execution of the program
  printf("\n\n");
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>    @1m0le Steganography  v.1.0.0  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
  printf("\nCurrently supported opertaions:\n");
  printf("         a. Hide data in an image file (using naive @1m0le)\n");
  printf("         b. Extract hidden data from an image file (using naive @1m0le)\n");
  printf("\n You choose:\n\n");
  // read user input

  // branch out to hiding data
  naive_hide();

}


// this procedure hides fata in a file
int naive_hide(){
   printf("Please enter the name of the file you want to hide: ");
   // read the name and try to file that file
   int possibleSize = 0;
   char* hf_name = read_file_name(&possibleSize);
   if (hf_name == NULL){
     printf("[ERROR] filename is not entered, terminating........\n");
     return -1;
   }
   FILE* hfile = fopen(hf_name, "rb");
   if (hfile == NULL){
     printf("[ERROR] Unbale to read the file you just provided\n");
     return -1;
   }
   printf("|---------------------   READING FILE   ------------------------- |\n");
   printf("|==========");
   fseek(hfile, 0, SEEK_END);
   int hfsize = ftell(hfile); // this size should be 4+fileszie+4 (0xDEADBEEF and 4-byte checksum)
   hfsize += 8;
   fseek(hfile, 0, SEEK_SET);
   char* h_file_data = malloc(hfsize);
   printf("=============");
   h_file_data[0] = 0xde;
   h_file_data[1] = 0xad;
   h_file_data[2] = 0xbe;
   h_file_data[3] = 0xef;
   int actuall_read = 0;
   actuall_read = fread(&h_file_data[8], 1,hfsize-8, hfile);
   if (actuall_read != hfsize-8){
     printf("DID NOT READ THE WHOLE FILE actual=%d  expect=%d\n",actuall_read,hfsize);
     return -1;
   }
   printf("===========================");
   uint32_t checksum = 0;
   for (int i = 8; i<hfsize; i++){
     checksum += h_file_data[i];  // naively calculate the checksum
   }
   *((uint32_t*)&h_file_data[4]) = checksum;
   printf("===============|\n");
   printf("|-----------------------   COMPLETED   -------------------------- |\n");
   fclose(hfile);
   // now the image file
   printf("\n\nPlease enter the name of the image file to store the hidden: ");
   // read the name and try to file that file
   char* img_name = read_file_name(&possibleSize);
   if (img_name == NULL){
     printf("[ERROR] filename is not entered, terminating........\n");
     return -1;
   }
   printf("|---------------------   READING FILE   ------------------------- |\n");
   printf("|==========");
   image im = load_image(img_name);
   int imgsize = im.w*im.h*im.c; // the size of the entire image
   if (imgsize < 12+8*(hfsize)){    // if img has total bytes less than 6+8*(4+4+hideen_size)+6, then cancel all operations and print error message
     printf("[ERROR] Image file is not large enough to store the data\n");
     return -1;
   }
   printf("=======================================================|\n");
   printf("|-----------------------   COMPLETED   -------------------------- |\n");
   printf("\n\n All files needed are set. Start the hidding process\n\n");
   printf("|----------------------------------------      Hiding Data     ------------------------------------- |\n");
   // If we have all the data, we can now start processing
   // step 1: the first 111111
   im.data[0] = mod_last_bit(im.data[0], 1);
   im.data[1] = mod_last_bit(im.data[1], 1);
   im.data[2] = mod_last_bit(im.data[2], 1);
   im.data[3] = mod_last_bit(im.data[3], 1);
   im.data[4] = mod_last_bit(im.data[4], 1);
   im.data[5] = mod_last_bit(im.data[5], 1);

   // step 2: the rest of the data
   int current_byte_pos = 0;
   int bit_counter = 0;
   int consecutive_1_counter = 0;
   int img_byte_pos = 6;
   int bar_counter = 0;
   printf("|");
   fflush(stdout);
   while (current_byte_pos<hfsize){ // keep going as long as we still havent finished the last byte
      bar_counter++;
      if (bar_counter % 4 == 0 ){
        printf("*");
        if (bar_counter%400 ==0){
          printf("|\n");
        }
        fflush(stdout);
      }
      if (consecutive_1_counter == 5){
        // we have written a 11111, next bit should be 0 anyway
        im.data[img_byte_pos] = mod_last_bit(im.data[img_byte_pos], 0);
        consecutive_1_counter = 0;
      }else{
        // write a regular data
        char next_bit = get_particular_bit(h_file_data[current_byte_pos], bit_counter);
        im.data[img_byte_pos] = mod_last_bit(im.data[img_byte_pos], next_bit);
        bit_counter++;
        if (next_bit == 1){
          // increment the consecutive_1_counter
          consecutive_1_counter ++;
        }else{
          // we have a zero, stop counting
          consecutive_1_counter = 0;
        }
        if (bit_counter == 8){
          // we have just written the last bit, reset
          bit_counter = 0;
          current_byte_pos++;
        }
        // this is the opertaions needed for each byte.
      }
      img_byte_pos++;
   }
   // now write the end of the data
//   char backup =  im.data[img_byte_pos+0] ;
   im.data[img_byte_pos+0] = mod_last_bit(im.data[img_byte_pos+0], 1);
   im.data[img_byte_pos+1] = mod_last_bit(im.data[img_byte_pos+1], 1);
   im.data[img_byte_pos+2] = mod_last_bit(im.data[img_byte_pos+2], 1);
   im.data[img_byte_pos+3] = mod_last_bit(im.data[img_byte_pos+3], 1);
   im.data[img_byte_pos+4] = mod_last_bit(im.data[img_byte_pos+4], 1);
   im.data[img_byte_pos+5] = mod_last_bit(im.data[img_byte_pos+5], 1);
   // now the image file should contain our hidden information. now we can output the image data
   printf("|\n");
   printf("|-----------------------------------------------COMPLETED------------------------------------------- |\n");
   // operations completed
   printf("\n\nData has been hidden, now outputing file.................\n");
   printf("|---------------------   OUTPUTING FILE   ------------------------- |\n");
   printf("|==========");
   save_image(im, "phantom");
   printf("=========================================================|\n");
   free(hf_name);
   free(img_name);
   printf("\n                   [SUCCESSFUL]\n");

//   printf("Is it?? %d %d\n", im.data[img_byte_pos], backup);
   free_image(im);

   return 0;
}


// this method extract the data hidden in a image
int naive_extract(){

return 0;

}


// extract a particular bit
static char get_particular_bit(char byte, char position){
   char shifted = byte >> position;
   return shifted & 1;
}





// modify the last bit of a particular data
static char mod_last_bit(char original, char to){
   if (to == 0){
     return original & (~((char)1));
   } else{
     return original | 1;
   }
}


static char* read_file_name(int* possibleSize){
  int size = 256;  // common max input length
  char* query = (char*)malloc(size);
  int read_bytes = 0;
  int wordsPossible = 1;
  do {
    if (read_bytes == size) {
      // not enough space, reallocate
      size *= 2;
      query = (char*)realloc(query, size);
    }
    // continuously reading
    char c = fgetc(stdin);
    if (c == EOF) {
      // we have reached the end of the file
      if (read_bytes == 0) {
        // not read a thing, return NULL
        *possibleSize = 0;
        free(query);
        return NULL;
      }
    }
    if (c == '\n') {
        // we have finished reading something
        query[read_bytes] = '\0';
        *possibleSize = wordsPossible;
        return query;
    } else {
      // we have captured a character
      // set the query first
      query[read_bytes] = c;
      read_bytes++;
      // if it is not a alphabet
      // increment wordsPossibl

    }
  } while (1);
}
