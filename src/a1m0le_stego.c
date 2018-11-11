#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "image.h"

#define LAST 3

int naive_hide();
int naive_extract();
//static char mod_last_bit(char original, char to);
static char get_particular_bit(char byte, char position);
static char* read_file_name(char* possibleSize);
static char mod_particular_bit(char byte, char bit, char pos);

int main(int argc, char** argv){
  // the main execution of the program
  printf("\n\n");
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>    @1m0le Steganography  v.1.0.0  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
  printf("\nCurrently supported opertaions:\n");
  printf("         a. Hide data in an image file (using naive @1m0le)\n");
  printf("         b. Extract hidden data from an image file (using naive @1m0le)\n");
  printf("\n You choose:");
  char possibleSize = 0;
  char* choice = read_file_name(&possibleSize);
  // read user input
  while (choice[0] != 'a' && choice[0] != 'b'){
      printf("\n Invalid opetion, please re-choose:");
      choice = read_file_name(&possibleSize);
  }
  // branch out to hiding data
  if (choice[0] == 'a'){
    naive_hide();
  }else{
    naive_extract();
  }

}


// this procedure hides fata in a file
int naive_hide(){
   printf("\n\nPlease enter the name of the file you want to hide: ");
   // read the name and try to file that file
   char hf_name_size = 0;
   char* hf_name = read_file_name(&hf_name_size);
   if (hf_name == NULL){
     printf("[ERROR] filename is not entered, terminating........\n");
     return -1;
   }
   if (hf_name_size > 250){
     printf("[ERROR] filename length is over 250 characters, terminating.......]");
     return -1;
   }
   // base on the name to open the files
   char hf_path[256];
   sprintf(hf_path, "./input/%s", hf_name);
   hf_path[8+(int)hf_name_size] = '\0';
   FILE* hfile = fopen(hf_path, "rb");
   if (hfile == NULL){
     printf("[ERROR] Unbale to read the file you just provided\n");
     return -1;
   }

   printf("|---------------------   READING FILE   ------------------------- |\n");
   printf("|==========");
   fseek(hfile, 0, SEEK_END);
   int hfsize = ftell(hfile); // this size should be 4+fileszie+4 (0xDEADBEEF and 4-byte checksum)+hf_name_size + 1 (for the length of the )
   hfsize += 8 + hf_name_size+1;
   fseek(hfile, 0, SEEK_SET); // reset the file reading pointer
   char* h_file_data = malloc(hfsize);
   printf("=============");
   h_file_data[0] = 'b';
   h_file_data[1] = 'e';
   h_file_data[2] = 'e';
   h_file_data[3] = 'f'; // write the magical words
   for (char i=0; i<hf_name_size; i++){
     // data[8] is for the size of the name, then write the name;
     h_file_data[i+9] = hf_name[(int)i];
   }
   h_file_data[8] = hf_name_size;
   int actuall_read = 0;
   // starts the file reading from h_file_data[9+hf_name_size] because the previousbytes are reserved.
   actuall_read = fread(&h_file_data[9+hf_name_size], 1,hfsize-8, hfile);
   if (actuall_read != hfsize-8-1-hf_name_size){
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
   char img_name_size;
   char* img_name = read_file_name(&img_name_size);

   if (img_name == NULL){
     printf("[ERROR] filename is not entered, terminating........\n");
     return -1;
   }
   printf("|---------------------   READING FILE   ------------------------- |\n");
   printf("|==========");
   char img_path[256];
    sprintf(img_path, "./input/%s", img_name);
    img_path[8+(int)img_name_size] = '\0';
   image im = load_image(img_path);
   image region = load_image(img_path); // ***** REGION MAPPING *****
   int imgsize = im.w*im.h*im.c; // the size of the entire image
   if (imgsize < 12+3*(hfsize)){    // if img has total bytes less than 6+8*(4+4+hideen_size)+6, then cancel all operations and print error message
     printf("[ERROR] Image file is not large enough to store the data\n");
     return -1;
   }
   printf("=======================================================|\n");
   printf("|-----------------------   COMPLETED   -------------------------- |\n");
//   for (int i=0; i<10; i++){
//     printf("got %d\n",im.data[i]);
//   }
   printf("\n\n All files needed are set. Start the hidding process\n\n");
   printf("|----------------------------------------      Hiding Data     ------------------------------------- |\n");
   // If we have all the data, we can now start processing
   // step 1: the first 111111
   im.data[0] = mod_particular_bit(im.data[0],1, 0);
   im.data[0] = mod_particular_bit(im.data[0],1, 1);
   im.data[0] = mod_particular_bit(im.data[0],1, 2);
   im.data[1] = mod_particular_bit(im.data[1],1, 0);
   im.data[1] = mod_particular_bit(im.data[1],1, 1);
   im.data[1] = mod_particular_bit(im.data[1],1, 2);
   char internal_bit_counter = 0;
   // step 2: the rest of the data
   int current_byte_pos = 0;
   int bit_counter = 0;
   int consecutive_1_counter = 0;
   int img_byte_pos = 2;
   int bar_counter = 0;
   printf("|");
   fflush(stdout);
   while (current_byte_pos<hfsize){ // keep going as long as we still havent finished the last byte
     if (img_byte_pos>=imgsize){
       // oops, we have run out of data. report error and exit
       printf("\n[ERROR] The image file turns out to be not enough, please try another image file");
     }
      bar_counter++;
      if (bar_counter % 4 == 0 ){
        printf("*");
        if (bar_counter%400 ==0){
          printf("|\n|");
        }
        fflush(stdout);
      }
      if (consecutive_1_counter == 5){
        // we have written a 11111, next bit should be 0 anyway
        im.data[img_byte_pos] = mod_particular_bit(im.data[img_byte_pos], 0,internal_bit_counter);
        internal_bit_counter++;
        region.data[img_byte_pos] = 0; // ***** REGION MAPPING *****
        consecutive_1_counter = 0;
      }else{
        // write a regular data
        char next_bit = get_particular_bit(h_file_data[current_byte_pos], bit_counter);
        im.data[img_byte_pos] = mod_particular_bit(im.data[img_byte_pos], next_bit, internal_bit_counter);
        internal_bit_counter++;
        region.data[img_byte_pos] = next_bit * 255; // ***** REGION MAPPING *****
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
      if (internal_bit_counter >= LAST){
          img_byte_pos++;
          internal_bit_counter = 0;
      }

   }
   // now write the end of the data
   //   char backup =  im.data[img_byte_pos+0] ;
   if (img_byte_pos+6>=imgsize){
     // oops, we have run out of data. report error and exit
    printf("\n[ERROR] The image file turns out to be not enough, please try another image file");
  }
  for (int ii = 0; ii<6; ii++){
    im.data[img_byte_pos] = mod_particular_bit(im.data[img_byte_pos], 1,internal_bit_counter);
    internal_bit_counter++;
    if (internal_bit_counter >= LAST){
        img_byte_pos++;
        internal_bit_counter = 0;
    }
  }


   // now the image file should contain our hidden information. now we can output the image data
   printf("|\n");
   printf("|-----------------------------------------------COMPLETED------------------------------------------- |\n\n");
   // operations completed
   printf("Please enter the image name you want to output(no extension name):");
   char out_size;
   char* outname = read_file_name(&out_size);
   if (img_name == NULL){
     printf("[WARNNING] filename is not entered, using default name phantom\n");
     outname = "phantom";
   }else{
   }

   printf("\n\nOutputing file.................\n");
   printf("|---------------------   OUTPUTING FILE   ------------------------- |\n");
   printf("|==========");
   char out_path[256];
    sprintf(out_path, "./output/%s", outname);
    out_path[9+(int)out_size] = '\0';
   save_image(im, out_path);
   save_image(region, "./mapped_region");
   printf("=========================================================|\n");
   free(hf_name);
   free(img_name);
   printf("\n                   [SUCCESSFUL]\n");
   for (int i=0; i<hfsize; i++){
  //   printf(" %d",h_file_data[i]);
   }

//for (int i=0; i<10; i++){
//  printf("got %d\n",im.data[i]);
//}
//   printf("Is it?? %d %d\n", im.data[img_byte_pos], backup);
   free_image(im);

   return 0;
}

static char flag_bit(char byte, char pos){
   return !(!(byte & (1<<pos)));
}

static char mod_particular_bit(char byte, char bit, char pos){
  if (bit == 0){
    char mask = 1 << pos;
    mask = ~mask;
    return byte & mask;
  }else{
    char mask = 1 << pos;
    return byte | mask;
  }
}

// this method extract the data hidden in a image
int naive_extract(){
  // read the image file
  printf("\n\nPlease enter the name of the image file with hidden data: ");
  // read the name and try to file that file
  char img_name_size;
  char* img_name = read_file_name(&img_name_size);
  if (img_name == NULL){
    printf("[ERROR] filename is not entered, terminating........\n");
    return -1;
  }
  printf("|---------------------   READING FILE   ------------------------- |\n");
  printf("|==========");
  char img_path[256];
   sprintf(img_path, "./input/%s", img_name);
   img_path[8+(int)img_name_size] = '\0';
  image im = load_image(img_path);
  printf("=======================================================|\n");
  printf("|-----------------------   COMPLETED   -------------------------- |\n");
  printf("\n\n Image received. identifing validity information.......\n");
  char* buffer = (char*)malloc((im.w*im.h*im.c)/7); // cant go wrong with this size!!!!!
  char byte_buf = 0;
  char bits_written = 0;
  char consecutive_1_counter = 0;
  char flag_detected = 0;
  int next_img_byte = 2;
  int bar_counter = 0;
  char start_flag = (flag_bit(im.data[0],0)) & (flag_bit(im.data[0],1)) & (flag_bit(im.data[0],2)) & (flag_bit(im.data[1],0)) & (flag_bit(im.data[1],1)) & (flag_bit(im.data[1],2));
  if (start_flag != 1){
    printf("\n[ERROR] Data-start flag not found, terminating.........\n");
    for (int i=0; i<10; i++){
      printf("got %d\n",im.data[i]);
    }
    return -1;
  }
  printf("\nData-start-flag detected, starts extracting data\n\n");
    printf("|----------------------------------------      Extracting Data     ----------------------------------|\n");
  int bytes_read = 0;
   printf("|");
  char internal_bit_counter  = 0;
  while (flag_detected != 1 && bytes_read<(im.w*im.h*im.c)/7-4){ // have some space to prevent oveflowing
    // now starts for each byte in the image
    char bit_got = flag_bit(im.data[next_img_byte], internal_bit_counter);
    internal_bit_counter++;
    //printf("%d",bit_got);
    if (consecutive_1_counter == 5){
      // we have seen 5 1s before, the next bit is either a zero to omit or the end of the message
      if (bit_got == 1){
        // we have a flag to exit
        flag_detected = 1;
      }else{
        consecutive_1_counter = 0;
      }
    }else{
       // it is a normal bit to be read
       if (bit_got == 1){
       // we received a one, increment the consecutive_1_counter
       consecutive_1_counter++;
     }else{
       consecutive_1_counter =0;
     }
       byte_buf = mod_particular_bit(byte_buf, bit_got, bits_written);
       bits_written++;
       if (bits_written == 8){
         // we have written 8 bits, reset, and extract as a byte
         bits_written = 0;
         buffer[bytes_read] = byte_buf;
         bytes_read++;
       }
    }
    bar_counter++;
    if (bar_counter % 4 == 0 ){
      printf("*");
      if (bar_counter%400 ==0){
        printf("|\n|");
      }
      fflush(stdout);
    }
    if (internal_bit_counter >= LAST){
      next_img_byte ++;
      internal_bit_counter = 0;
    }
  }
  if (flag_detected != 1){
    // data has no end
    printf("\n[ERROR] No End detected, data is incomplete. terminating.......\n");
    return -1;
  }
  printf("\n\nData received, checking the first 4 byte for magical key:  ");
  if (buffer[0] != 'b'  || buffer[1] != 'e' || buffer[2] != 'e'  || buffer[3]!= 'f'){
    // the magical work is wrong
    printf(" INCORRECT\n");
    printf("\nExtraction failed, terminating....................\n");
    return -1;
  }

  printf("CORRECT :-)\n");
  printf("Checking the integrity of the file:");
  uint32_t expect_checksum = *((uint32_t*)&buffer[4]);
  uint32_t actual_checksum = 0;
  for (int i = 8; i<bytes_read; i++){
    actual_checksum += buffer[i];  // naively calculate the checksum
  }
  if (expect_checksum != actual_checksum){
    printf(" INCORRECT\n");
    printf("\nExtraction failed, terminating....................\n");
    return -1;
  }
  printf("CORRECT :-)\n");


  char name_size = buffer[8];
  char outname[256];
  for (char i = 0; i<name_size; i++){
      outname[(int)i] = buffer[(int)i+9];
  }
  outname[(int)name_size] = '\0';
  char out_path[512];
   sprintf(out_path, "./output/%s", outname);
   out_path[9+(int)name_size] = '\0';
      printf("\nOutputing data to file %s\n",out_path);
      FILE* output = fopen(out_path,"wb");
      fwrite(&buffer[9+name_size], 1, bytes_read-9-name_size, output);
      fclose(output);


printf("\n                   [SUCCESSFUL]\n");
for (int i=0; i<bytes_read; i++){
  //printf(" %d",buffer[i]);
}

return 0;

}

// I want to have 3-bit hiding, ***namesaving, and data_map_region visualization implemented. (REMEMBER TO CAHNGE README AS WELL !!!!!!!!!!!!!!!!!!!!)

// extract a particular bit
static char get_particular_bit(char byte, char position){
   char shifted = byte >> position;
   return shifted & 1;
}






// modify the last bit of a particular data
//static char mod_last_bit(char original, char to){
  // printf("%d",to);
//   if (to == 0){
//     return original & (~((char)LAST));
//   } else{
//     return original | LAST;
//   }
//}


static char* read_file_name(char* possibleSize){
  int size = 256;  // common max input length
  char* query = (char*)malloc(size);
  char read_bytes = 0;
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
        query[(int)read_bytes] = '\0';
        *possibleSize = read_bytes;
        return query;
    } else {
      // we have captured a character
      // set the query first
      query[(int)read_bytes] = c;
      read_bytes++;
      // if it is not a alphabet
      // increment wordsPossibl

    }
  } while (1);
}
