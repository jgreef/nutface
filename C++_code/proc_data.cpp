// This file is a test of the C++ unordered multimap

#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using user_type = unsigned;
using movie_type = unsigned short;
using time_type = unsigned short;
using rating_type = unsigned char;

struct data_point
{	
	user_type user;
	movie_type movie;
	time_type timestamp;
	rating_type rating;
};

data_point* get_data_from_line(string line);

int main()
{

	string data_line, idx_line;
	data_point* data;
	int i = 0;

	// Get the input files
    ifstream mu_data ("../../../data/mu/all.dta");
    ifstream mu_idx  ("../../../data/mu/all.idx");

    ifstream um_data ("../../../data/um/all.dta");
    ifstream um_idx  ("../../../data/um/all.idx");

    // Create the output files
	ofstream mu_train 	("../../../data/mu/train.bin", ios::out | ios::trunc | ios::binary); 
	ofstream mu_hidden 	("../../../data/mu/hidden.bin", ios::out | ios::trunc | ios::binary); 
	ofstream mu_valid 	("../../../data/mu/valid.bin", ios::out | ios::trunc | ios::binary); 
	ofstream mu_probe 	("../../../data/mu/probe.bin", ios::out | ios::trunc | ios::binary); 
	ofstream mu_qual 	("../../../data/mu/qual.bin", ios::out | ios::trunc | ios::binary); 

	ofstream um_train  	("../../../data/um/train.bin", ios::out | ios::trunc | ios::binary); 
	ofstream um_hidden 	("../../../data/um/hidden.bin", ios::out | ios::trunc | ios::binary); 
	ofstream um_valid  	("../../../data/um/valid.bin", ios::out | ios::trunc | ios::binary); 
	ofstream um_probe  	("../../../data/um/probe.bin", ios::out | ios::trunc | ios::binary); 
	ofstream um_qual 	("../../../data/um/qual.bin", ios::out | ios::trunc | ios::binary); 

    if (mu_data.is_open() && mu_idx.is_open() && um_data.is_open() && um_idx.is_open())
    {
    	// Process MU first
	    while(getline(mu_data, data_line) && getline(mu_idx, idx_line))
	    {
	    	// Print a line status indicator
	    	cout << i << endl;
	    	i++;

	    	data = get_data_from_line(data_line);

	    	switch(idx_line[0])
	    	{
	    		case '1':
	    			mu_train.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '2':
	    			mu_valid.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '3':
	    			mu_hidden.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '4':
	    			mu_probe.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '5':
	    			mu_qual.write( (char*) data, sizeof(data_point));
	    			break;
	    		default:
	    			cout << "invalid index!" << endl;
	    			break;
	    	}
	    }

	    // close all MU files
	    mu_train.close();
	    mu_valid.close();
	    mu_hidden.close();
	    mu_probe.close();
	    mu_qual.close();
	    mu_data.close();
	    mu_idx.close();

	    // Process UM next
	    while(getline(um_data, data_line) && getline(um_idx, idx_line))
	    {
	    	// Print a line status indicator
	    	cout << i << endl;
	    	i++;

	    	data = get_data_from_line(data_line);

	    	switch(idx_line[0])
	    	{
	    		case '0':
	    			um_train.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '1':
	    			um_valid.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '2':
	    			um_hidden.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '3':
	    			um_probe.write( (char*) data, sizeof(data_point));
	    			break;
	    		case '4':
	    			um_qual.write( (char*) data, sizeof(data_point));
	    			break;
	    		default:
	    			break;
	    	}
	    }

	    // close all files
	    um_train.close();
	    um_valid.close();
	    um_hidden.close();
	    um_probe.close();
	    um_qual.close();
	    um_data.close();
	    um_idx.close();
	}

    return 0;
}

// This function puts the line data into a struct and 
//	returns the pointer to the struct
data_point* get_data_from_line(string line)
{
	int idx = 0;
	static data_point data;

	// Initialize the data point
	memset(&data, 0, sizeof(data_point));

	// Get the user number
	while(line[idx] != ' ')
	{
		data.user *= 10;
		data.user += (line[idx] - '0');
		idx++;
	}
	idx++;

	// Get the movie number
	while(line[idx] != ' ')
	{
		data.movie *= 10;
		data.movie += (line[idx] - '0');
		idx++;
	}
	idx++;

	// Get the time number
	while(line[idx] != ' ')
	{
		data.timestamp *= 10;
		data.timestamp += (line[idx] - '0'); 
		idx++;
	}
	idx++;

	// And finally get the rating
	data.rating = (line[idx] - '0');

	return &data;

}










