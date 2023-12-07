//THIS COMPILED AND RAN PERFECTLY ON WINDOWS!!!!!!!!! - all outputs were seen. so please try pc if mac gives funny results for some reason :)
// 17/02/2021 : A GIF image viewer
//Eloise Hatton CID:01338627

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <math.h>
#include <iomanip>
#include <algorithm>

using namespace std;

struct LSD{ //information contained within the logical screen descriptor
  unsigned int width;
  unsigned int height;
  bool global_ct_flag;
  int colour_resolution;
  bool sort_flag;
  int size_global_ct;
  int backgroud_ci;
  int pixel_aspect_ratio;
};
struct ID{ //information contained within the image descriptor
  unsigned int left_position;
  unsigned int top_position;
  unsigned int width;
  unsigned int height;
  bool local_ct_flag;
  bool interlace_flag;
  bool sort_flag;
  int reserved;
  int local_ct_size;
};

string dec_to_bin(int x);
int bin_to_dec(vector<bool> vb);
vector<bool> multibyte_to_binary(string str, int num_bytes);
string swap_binary(string bin_str);
vector<bool> bit_string_to_binary(string str);
vector<vector<int>> global_colour_table(LSD& lsd, fstream& image);
vector<vector<int>> initialise_code_table(int num_initial_codes);
void add_row(vector<vector<int>> *code_table, vector<int> code_value);
void print_array(int width, int height, vector<int> vc, int spacing);
void print_table(vector<vector<int>> table);
vector<int> colour_channel(vector<int> vc_index, vector<int> vc_colour);


int main(){

cout << "\n\nGIF IMAGE VIEWER" << endl;

fstream image;
string file_name;

cout << "\nEnter file name: ";
getline(cin, file_name);
image.open(file_name, fstream::in | fstream::binary);
if(image.fail()){                         // outputs to error log if file fails to open and ends program
  cerr << "\nInput file opening failed.\n";
  cout << "Please make sure you have entered the file name correctly and are in the correct directory" << endl;
  exit(1);}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////HEADER////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// extracting the header bytes (first 3 bytes are the signature, second 3 are the version)
vector<char> header;
char c;
for(int i=1; i<=6; i++){
  image.get(c);
  if(image.fail()){
    cerr << "failed to read header";
    exit(1);
  }
  header.push_back(c);
}

string signature = {header[0], header[1], header[2]};
string version = {header[3], header[4], header[5]};

if(signature != "GIF"){ //every GIF file has this signature so end program if it is not there
  cerr << "Image not recognised as a GIF" << endl;
  exit(1);
}
else cout << "\n>> read header" << endl;

cout << "\nHeader:" << endl;
cout << "signature: " << setw(15) << signature << endl << "version: " << setw(17) << version << endl;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////LOGICAL SCREEN DESCRIPTOR/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LSD lsd; //declaring a struct of type LSD (Logical screen descriptor) - definition in global scope

//reading in next 7 bytes of file (the LSD bytes), putting in a vector to access when needed
vector<string> bytes;
string str;
for(int i=0; i<7; i++){
  image.get(c);
  str = c;
  bytes.push_back(str);
}
cout << endl << ">> read logical screen descriptor " << endl;

//WIDTH AND HEIGHT (bytes 1-4 in LSD):
//for multi-byte numbers, least significant byte comes first so need to swap the byte order read in

string width_bytes = bytes[1] + bytes[0];  //swapping byte order
string height_bytes = bytes[3] + bytes[2];

lsd.width = bin_to_dec(multibyte_to_binary(width_bytes, 2));  //converting string (2 bytes long) to binary vector and then converting that vector to a decimal int
lsd.height = bin_to_dec(multibyte_to_binary(height_bytes, 2));

//PACKED FIELD (5th byte in LSD):

//creating bitset (acts like a bool array) so each bit can be called separately by its index. (least significant bit will be at index 0, MSB at index 7)
bitset<8> packed_bits((bytes[4])[0]);
vector<bool> vb_colour_res = {packed_bits[6], packed_bits[5], packed_bits[4]}; //colour resolution and global ct size are read as integer numbers stored in 3 bits
lsd.colour_resolution = bin_to_dec(vb_colour_res);  //converting bitset to a decimal number
vector<bool> vb_size_global_ct = {packed_bits[2], packed_bits[1], packed_bits[0]};
lsd.size_global_ct = bin_to_dec(vb_size_global_ct);
lsd.global_ct_flag = packed_bits[7]; //Global Colour Table count flag is MSB
lsd.sort_flag = packed_bits[3];


//BACKGROUND COLOUR INDEX (6th byte in LSD) and PIXEL ASPECT RATIO (7th byte in LSD):
lsd.backgroud_ci = bin_to_dec(multibyte_to_binary(bytes[5], 1));      //converting string(1 byte long) to binary vector and then binary vector to decimal (int)
lsd.pixel_aspect_ratio = bin_to_dec(multibyte_to_binary(bytes[6], 1));

//Printing the LSD struct:
cout << endl << "Logical Screen Desciptor:" << endl;
cout << "width: " <<  setw(19) << lsd.width << endl;
cout << "height: " << setw(18) << lsd.height << endl;
cout << "global ct flag: " << setw(9) << lsd.global_ct_flag << endl;
cout << "colour res: " << setw(13) << lsd.colour_resolution << endl;
cout << "sort flag: " << setw(14) << lsd.sort_flag << endl;
cout << "global ct size: " << setw(9) << lsd.size_global_ct << endl;
cout << "background colour i: " << setw(4) << lsd.backgroud_ci << endl;
cout << "aspect ratio: " <<  setw(11) << lsd.pixel_aspect_ratio << endl;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////GLOBAL COLOUR TABLE//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//function that prints global color table and returns copy of it (Do not call more than once!!! - reads from current position in file)
vector<vector<int>> global_ct_copy = global_colour_table(lsd,image); //stores copy of ct for later use

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////IMAGE DESCRIPTOR/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char ch = 0;
while(int(ch) != 44){ //image descriptor starts with byte storing 0x2C (dec 44)- gif file may have optional extension blocks before this - want to skip to image descriptor
image.get(ch);
}
cout << endl << ">> read image descriptor" << endl;

ID id; //declaring a struct of type ID (image descriptor) - definition in global scope
//reading in the next 9 bytes of the I.D:
vector<string> id_bytes;
string str3;
for(int i = 1; i <= 9; i++){
  image.get(c);
  str3 = c;
  id_bytes.push_back(str3);
}
//swapping order of bytes to correctly read the multibyte numbers:
string id_left_bytes = id_bytes[1] + id_bytes[0];
string id_top_bytes = id_bytes[3] + id_bytes[2];
string id_width_bytes = id_bytes[5] + id_bytes[4];
string id_height_bytes = id_bytes[7] + id_bytes[6];

id.left_position = bin_to_dec(multibyte_to_binary(id_left_bytes, 2));  //2-byte-string -> binary vector -> decimal equivalent
id.top_position = bin_to_dec(multibyte_to_binary(id_top_bytes, 2));
id.width = bin_to_dec(multibyte_to_binary(id_width_bytes, 2));
id.height = bin_to_dec(multibyte_to_binary(id_height_bytes, 2));
//separating the bits of the packed field (as before):
bitset<8> id_packed_bits((id_bytes[8])[0]);
vector<bool> vb_reserved = {id_packed_bits[2], id_packed_bits[1], id_packed_bits[0]}; //colour resolution and global ct size are read as integer numbers stored in 3 bits
vector<bool> vb_sizeoflocalct = {id_packed_bits[4], id_packed_bits[3]};
id.local_ct_flag = id_packed_bits[7]; //local Colour Table count flag is MSB
id.interlace_flag = id_packed_bits[6];
id.sort_flag = id_packed_bits[5];
id.reserved = bin_to_dec(vb_reserved);
id.local_ct_size = bin_to_dec(vb_sizeoflocalct);

//printing image descriptor struct:
cout << endl << "Image Descriptor: " << endl;
cout << "left position: " << setw(9) << id.left_position << endl;
cout << "top position: " << setw(10) << id.top_position << endl;
cout << "image width: " << setw(12) << id.width << endl;
cout << "image height: " << setw(11) << id.height << endl;
cout << "local ct flag: " << setw(9) << id.local_ct_flag << endl;
cout << "interlace flag: " << setw(8) << id.interlace_flag << endl;
cout << "sort flag: " << setw(13) << id.sort_flag << endl;
cout << "reserved: " << setw(14) << id.reserved << endl;
cout << "local ct size: " << setw(9) << id.local_ct_size << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////TABLE BASED IMAGE DATA///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

if(id.local_ct_flag){ //program does not collect info on local colour table so cannot extract image data from gifs that use it
  cerr << endl << "Program cannot deal with use of local colour tables, will now end" << endl;
  exit(1);
}
// file is now open at the start of the table based image data
cout << endl << ">> read table based image data" << endl;
cout << endl << "Table Based Image Data:" << endl;
image.get(c);
int lzw_min_size = c;  //min code size is first byte of the image data (as integer)
cout << endl << "LZW Minimum Code Size: " << lzw_min_size << endl;

//no local colour table present (as checked for in line 201) so gif uses global colour table for the image's colour table:
//printing colour table:
cout << endl << "Colour Table:" << endl;
 for(int i = 0; i< pow(2, lsd.size_global_ct + 1); i++){
  cout << "#" << i;
  for(int j=0; j<3; j++)
    cout << "\t" << global_ct_copy[i][j];
  cout << endl;
}

//initial code table:
//initial code table should be 2^(LZW min code size) + 2 to account for the clear and eoi codes.
//alternatively, this is (at least) the number of colours to be represented + 2
int num_initial_codes = pow(2, lzw_min_size) + 2;

//creating an expandable code table:
vector<vector<int>> code_table = initialise_code_table(num_initial_codes);
//adjusting values for clr and eoi at the last two codes: (this is arbitrary output required from assignment brief -
//in the decoding section, clear = 4 and eoi = 5.
code_table[num_initial_codes - 2][0] = -1;
code_table[num_initial_codes - 1][0] = -2;

cout << endl << "Initial Code Table:";
print_table(code_table);


//Block Bytes:
//first block is block size:
int block_size = image.get();
cout << endl << "Block Bytes:" << endl;
cout << "block size: " << block_size;
//getting and printing block bytes:
vector<int> blocks;
for(int i = 0; i < block_size; i++){
  int x = image.get();
  blocks.push_back(x);
  cout << endl << "byte #" << i << ":  " << left << setw(3) <<blocks[i];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////CODE STREAM////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
string bit_stream;
// BYTE DECODING - here, the block byte integers are converted to 8N-bit binary equivalents (N is integer) and then added
// together in such a way that allows the bits to be read in groups of their codes (so the binary bytes are concatenated
//from last byte to first and then the whole sting is reversed).
for(int i = block_size - 1; i >=0 ; i--){
  string str = dec_to_bin(blocks[i]);
  bit_stream = bit_stream + str;
}
reverse(bit_stream.begin(), bit_stream.end()); //reversing order of bits - this is the order it should be read

//first code size is the LZW min size + 1 (the '+1' is to account for the clr/eoi codes)
int initial_code_size = lzw_min_size + 1; //so for squares.gif, this is 3.
//The code size needs to be increased by 1 as soon as the 2^(codesize + 1)th bit is read in

int code_size = initial_code_size;
vector<int> vc_code_stream;

int count = 0;  //count keeps track of how many bits from bit_stream have been read in so that the program does not try to read past end of the string
int start = 0;

while(count < bit_stream.size()){
//In this while loop, groups of 'code_size' bits are taken (e.g. for code_size = 3, bits are taken in 3s)- this string of bits is converted to decimal
//equivalent. For each code_size, this repeats 2^(code_size - 1) times, and then the code_size increments by 1. This repeats until the whole bit_stream is read in.
for(int i = start; i < (start + code_size * pow(2, code_size - 1)); i += code_size){

  if(count < bit_stream.size()){  //This makes sure that nothing is read in after the end of bit stream while the final for-loop is running

    if(bit_stream.size() - count < code_size){   //special case for the final code (which won't necessarily be the full code size long - need different formula to avoid reading past end of string)
      string str;
      int length_final_code = bit_stream.size() - count;
      for(int k = length_final_code - 1; k>=0; k--){ //codes stored LSB first in bit_stream, so here we reverse bit order to put MSB in first index so function bin_to_dec can be used to convert to decimal
        str = str + bit_stream[i + k];
        count = count + code_size;                 //breaks out of loop - makes count > bit_stream.size()
      }
      vector<bool> temp = bit_string_to_binary(str);
      vc_code_stream.push_back(bin_to_dec(temp));   //this adds to the final code to code_stream
      }

    else {
    string str1;
    for(int j = code_size - 1; j >= 0; j--){ //for all but the final code, this reads in successive bits, correct for the current code length, and 'swaps' the bits so that they are stored as MSB first
      str1 = str1 + bit_stream[i + j];
      count ++;
    }
    vector<bool> temp = bit_string_to_binary(str1);
    vc_code_stream.push_back(bin_to_dec(temp));     //entering the codes into the code stream
    }
}
}
start += code_size * pow(2, code_size -1);   //once 2^(code_size - 1) bits have been read in, code_size increments and start position changes for next for-loop
code_size++;
}

//printing the code stream:
cout << endl << endl << "Code Stream:";
for(int i = 0; i < vc_code_stream.size(); i++)
  cout << endl << "code #" << i << "  " << vc_code_stream[i];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////DECOMPRESSION///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Following the steps given in the LZW Decomopression algorithm:

const int CLEAR = pow(2, lzw_min_size);   //the values of the clear and eoi in stream are determined by the lZW min size (they take up the last 2 rows in the code table)
const int EOI = CLEAR + 1;
vector<vector<int>> vc_code_table;
vector<int> vc_index_stream;
vector<int>::iterator vit;                //will use iterator to keep track of where we are in code_stream as the algorithm involves accessing previous elements
vit = vc_code_stream.begin();

int code = *vit;   //code is the 'index': working way down code stream
if(code == CLEAR){ //should start with CLEAR
  vc_code_table = initialise_code_table(num_initial_codes);
  vit++;
  code = *vit;        //'code' moves to the next code in the code stream
  for(int i = 0; i < vc_code_table[code].size(); i++)
    vc_index_stream.push_back(vc_code_table[code][i]); //entering the value of code from the code table, {code}, into the index stream, element by elemnt
}
else {          //just in case it doesn't start with CLEAR (but should do)
  vc_code_table = initialise_code_table(num_initial_codes);
  for(int i = 0; i< vc_code_table[code].size(); i++)
    vc_index_stream.push_back(vc_code_table[code][i]);
}

//starting the loop section of the decompression algorithm:

while(*(vit+1) != EOI){ //loop runs until the code 1 before the EOI code (so that the EOI is not included as an index in stream)
  vit++;
  code = *vit; //letting code be next code in the code stream
  vector<int> k;

  if(code == CLEAR) //code table is reinitialised every time clear code is encountered (this won't happen in squares.gif but trying to be more general)
    vc_code_table = initialise_code_table(num_initial_codes);


  if(code < vc_code_table.size()){ //checking if code is already in code tabe
    //if yes, then output code to the index stream:
    for(int i = 0; i < vc_code_table[code].size(); i++) //outputting {code} to index stream
      vc_index_stream.push_back(vc_code_table[code][i]);
    k.push_back(vc_code_table[code][0]); //making k the first index in {code}
    //adding {code-1}+k to code table:
    int prev_code = *(vit - 1);  //prev_code is code of previous entry in code table
    vector<int> vc_temp = vc_code_table[prev_code];
    for(int i = 0; i< k.size(); i++)
      vc_temp.push_back(k[i]);  //joining {code-1} and k together
    add_row(&vc_code_table, vc_temp); //adding {code-1} +k to code table
    }

  else { //then code is not already in the code table
    int prev_code = *(vit-1);
    k.push_back(vc_code_table[prev_code][0]); //making k equal to the first index in {code-1}
    vector<int> vc_temp = vc_code_table[prev_code]; //this is {code-1}
    for(int i = 0; i<k.size(); i++)
      vc_temp.push_back(k[i]);       //this is {code-1} + k
    for(int i = 0; i< vc_temp.size(); i++)
      vc_index_stream.push_back(vc_temp[i]);  //outputting {code-1} +k to index stream
    add_row(&vc_code_table, vc_temp);  //adding {code-1} + k to code table
  }

}

cout << endl << endl << "Index Stream:";
cout << endl << "i_stream size: " << vc_index_stream.size() << endl;
//printing the index stream according to size of GIF (determined by the width and height stored in the id struct):
print_array(id.width, id.height, vc_index_stream, 1);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Colour Channels////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//use colour table to interepret the index stream: (each index refers to the colour of that pixel. The colour table contains the composition of each colour in terms of red/green/blue intensities)
vector<int> vc_red, vc_green, vc_blue;
for(int i = 0; i < global_ct_copy.size(); i++){   //Red Green and Blue are first, second and third column of colour table respectively
  vc_red.push_back(global_ct_copy[i][0]);         //Separating out the colour columns of the global colour table (which is used as the colour table for the image)
  vc_green.push_back(global_ct_copy[i][1]);
  vc_blue.push_back(global_ct_copy[i][2]);
}

vector<int> red_channel, green_channel, blue_channel;  //function colour_channel prints out each colour channel, showing which pixels each colour is present in
red_channel = colour_channel(vc_index_stream, vc_red);
green_channel = colour_channel(vc_index_stream, vc_green);
blue_channel = colour_channel(vc_index_stream, vc_blue);

//printing the colour channels:
cout << endl << "Image Data - Red Channel" << endl;
print_array(id.height, id.width, red_channel, 4);
cout << endl << endl << "Image Data - Green Channel" << endl;
print_array(id.height, id.width, green_channel, 4);
cout << endl << endl << "Image Data - Blue Channel" << endl;
print_array(id.height, id.width, blue_channel, 4);

//reading header- confirming end of gif file
//next byte should be 0 (block terminator). The one after that is the Trailer (decimal value 59)
int y = image.get();
if(y != 0){
  cout << endl<< "Gif too large"; //If next byte isn't block terminator then the gif contains multiple blocks - too large for this code to deal with
  exit(1);
}
else{
while(y != 59)  //The trailer that indicates end of file has decimal value 59. while loop is here in case there are any extensions after image descriptor - want to skip them
  y = image.get();
}
//once out of while loop, means the trailer has been read - end of file reached
cout << endl <<  ">> read trailer" << endl;
cout << endl << ">> program ended" << endl << endl;

return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////FUNCTIONS////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<vector <int>> global_colour_table(LSD& lsd, fstream& image){
  //creates and prints the global colour table.
  vector<vector<int>> vc_int;
  if(lsd.global_ct_flag){  //Global colour table only exists if colour flag = 1
    cout << endl << ">> read global table colour" << endl;
    char c;
    string str1;
    int size_of_ct = pow(2, lsd.size_global_ct + 1); // #bytes of colour table is 3*2^(global ct size + 1)
    for(int i = 0; i < size_of_ct; i++){  //creating a dynamically expandable table
      string str1;
      vector<int> row;
      for(int j = 0; j<3; j++){
        image.get(c);
        str1 = c;
        row.push_back(bin_to_dec(multibyte_to_binary(str1, 1)));  //converting the byte read in to a decimal - putting this into vector
      }
      vc_int.push_back(row);
    }
    //printing table:
    cout << endl << "Global Colour Table:" << endl;
    for(int i = 0; i< size_of_ct; i++){
      cout << "#" << i;
      for(int j=0; j<3; j++)
        cout << "\t" << vc_int[i][j];
      cout << endl;
    }
  }
return vc_int;
}


vector<bool> multibyte_to_binary(string str, int num_bytes){
  //Converts a string str into its binary number equivalent (stored in a bool vector). num_bytes is number of bytes in the string.
  vector<bool> bin_vec;
  for(int i = 0; i < num_bytes; i++){
    bitset<8> bitboi(str.c_str()[i]);
    for(int j = 0; j < 8; j++)
      bin_vec.push_back(bitboi[7-j]); //bitset puts LSB first - must reverse the order
  }
  return bin_vec;
}

vector<bool> bit_string_to_binary(string str){
  //for converting strings of less than 8bits to binary sequence - leading zeros added to make 8 bits
  vector<bool> bin_vec;
  bitset<8> bitboi(str);  //This puts binary value of string into the bitset, with leading 0s
  for(int j = 0; j< 8; j++)
    bin_vec.push_back(bitboi[7-j]);  //want to vector to be MSB first so reverse bitset order

  return bin_vec;
}

string dec_to_bin(int x){ //converts a decimal number to its binary equivalent as a string
  //inserts leading 0s to make 8N bit number, N is integer
  string str;
  while(x!=0){
    str = (x%2 == 0 ? "0" : "1") + str;
    x = x/2;
  }
  int n = str.size() % 8;  //if string isnt a full multiple of 8 bits, will pad with 0s to make it so.
  if(n != 0){
  str = string(8-n, '0') + str; //padding out with leading 0s
  }
  return str;
}

int bin_to_dec(vector<bool> vb){
  //Converts a vector of bools into a single decimal number, reading the vector
  //as one binary number. Reads the number as if most significant bit is at index 0.

  int n = vb.size() - 1;  //index n is the position of the least significant bit (the last bit of vector)
  int dec = 0;
  for(int i= n; i>=0; i--){
    dec = dec + vb[i] * pow(2,n-i);
  }
  return dec;
}


string swap_binary(string bin_str){
  //reverses the order of elements in a string
  string swapped_str;
  for(int i=0; i<bin_str.size(); i++){
    swapped_str[bin_str.size() - 1 - i] = bin_str[i];
  }
  return swapped_str;
}

void print_table(vector<vector<int>> table){
  //prints table with row numbering
  cout << endl;
  for(int i = 0; i< table.size(); i++){
    cout << "#" << i <<"  ";
    for(int j=0; j<table[0].size(); j++)
      cout << table[i][j] << "\t";
    cout << endl;
  }
}

vector<vector<int>> initialise_code_table(int num_initial_codes){
//intialises the code table for the lzw decompression algorithm
//the code values (the rows) can be of variable length
vector<vector<int>> code_table;
for(int i = 0; i<num_initial_codes; i++){
  vector<int> row;
  row.push_back(i);
  code_table.push_back(row);
}
return code_table;
}

void add_row(vector<vector<int>> *code_table, vector<int> code_value){
//adds new row (code_value) to code table.
  (*code_table).push_back(code_value);
}

vector<int> colour_channel(vector<int> vc_index_stream, vector<int> vc_colour){
//vc_colour is the colour column from the colour table (red, green or blue)  that you want to print the channel for
//the index stream gives the colour of every pixel in the image
  vector<int> colour_channel;
  for(int i = 0; i < vc_index_stream.size(); i++){
    int x = vc_index_stream[i];
    colour_channel.push_back(vc_colour[x]);   //This prints the chosen colour intensity for the given pixel to the colour channel
  }
return colour_channel;
}

void print_array(int width, int height, vector<int> vc, int spacing){
  //prints array of specified height, width and spacing between elements
  vector<int>::iterator it; //iterator moves through the array to access each element
  it = vc.begin();
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      cout << left << setw(spacing) << *it;
      it++;
    }
    cout << endl;
  }
}
