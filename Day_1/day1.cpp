#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <vector>

using namespace std;


bool replace(string& str, const string& from, const string& to);
void replaceAll(string& str, const string& from, const string& to);
int calibration(string file_name);

int main( int argc, char *argv[] ) {

    cout << calibration("cal_input.txt") << endl;

}



int calibration(string file_name){
    // vector<int> output_sums;
    ifstream file;
    file.open( file_name );
    if(file.fail()){                   
        cerr << "\nInput file opening failed.\n";
        cout << "Please make sure you have entered the file name correctly and are in the correct directory" << endl;
        exit(1);
    }

    const string search_strings[10] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    const string replace_strings[10] = {"ze0ro", "o1ne", "t2wo", "th3ree", "fo4ur", "fi5ve", "si6x", "se7ven", "ei8ght", "ni9ne"}; 
    int sum = 0;
    
    string line;
             
    while( getline(file, line) ){
        int digit1 = -1; int digit2 = -1;
        for (int i = 0; i < sizeof(search_strings) / sizeof(search_strings[0]); i++){
            replaceAll(line, search_strings[i], replace_strings[i]);
        }

        
        for (int i = 0; i < line.length(); i++) {
            if (isdigit(line[i])){
                digit1 = line[i] - '0';
                break;
            }
        }

        for (int i = line.length() - 1; i >= 0; i--) {
            if (isdigit(line[i])){
                digit2 = line[i] - '0';
                break;
            }
        }
        if (digit1 != -1 && digit2 != -1) {
            sum  += stoi(to_string(digit1) +  to_string(digit2));
            // output_sums.push_back(stoi(to_string(digit1) +  to_string(digit2)));
        } else {
            cout << "oh nooooooo" << endl;
        }
        
        
    }
//     ofstream output_file("./coutput.txt");
//     if (!output_file) {
//     cerr << "Failed to open coutput.txt for writing." << endl;
//     return -1; // or handle the error as appropriate
// }

//     ostream_iterator<int> output_iterator(output_file, "\n");
//     copy(begin(output_sums), end(output_sums), output_iterator);
    return sum;
}

bool replace(string& str, const string& from, const string& to){
    size_t start_pos = str.find(from);
    if (start_pos == string::npos) return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(string& str, const string& from, const string& to){
    if (from.empty()) return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos){
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

