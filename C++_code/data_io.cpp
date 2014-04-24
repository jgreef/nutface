//
// This is the function that will handle 
//	getting the data from disk and loading 
//	it into RAM
//

#include <iostream>
#include <string>
#include <fstream>
#include "data_io.h"
#include "data_types.h"

using namespace std;

// Static pointers
data_point* data_pointers[NUM_DATA_SETS];
// Boolean array indicating whether or not the data is loaded
bool data_loaded[NUM_DATA_SETS];
// array of data set sizes
unsigned data_size[NUM_DATA_SETS];

// this function initializes all of the static variables. It MUST be called
//	before doing anything else
void init_data_io()
{
	for (int i = 0; i < NUM_DATA_SETS; i++)
	{
		data_pointers[i] = NULL;
		data_loaded[i] = false;
		data_size[i] = 0;
	}
}

// This function returns a pointer to the beginning of the
//	requested dataset
data_point* get_data(data_set_t dtype)
{
	ifstream data_file;
	unsigned filesize; 

	// Only reload if it's not loaded already
	if ( (data_pointers[dtype] == NULL) || (data_loaded[dtype] == false) )
	{
		// Load the right file. Yes, there's a prettier way to do this. 
		switch(dtype)
		{
			case TRAIN_MU:
				data_file.open("../../../data/mu/train.bin", ios::binary);
				break;
			case VALID_MU:
				data_file.open("../../../data/mu/valid.bin", ios::binary);
				break; 
			case HIDDEN_MU:
				data_file.open("../../../data/mu/hidden.bin", ios::binary);
				break;
			case PROBE_MU:
				data_file.open("../../../data/mu/probe.bin", ios::binary);
				break;
			case QUAL_MU:
				data_file.open("../../../data/mu/qual.bin", ios::binary);
				break;
			case TRAIN_UM:
				data_file.open("../../../data/um/train.bin", ios::binary);
				break;
			case VALID_UM:
				data_file.open("../../../data/um/valid.bin", ios::binary);
				break; 
			case HIDDEN_UM:
				data_file.open("../../../data/um/hidden.bin", ios::binary);
				break;
			case PROBE_UM:
				data_file.open("../../../data/um/probe.bin", ios::binary);
				break;
			case QUAL_UM:
				data_file.open("../../../data/um/qual.bin", ios::binary);
				break;
			default:
				return NULL;
		}

		// Now, figure out the file length, but make sure the file is open
		if (data_file.is_open())
		{
			// Seek to the end
			data_file.seekg(0, ios::end);
			// Get the size
			filesize = data_file.tellg();
			// Store the size
			data_size[dtype] = (filesize / sizeof(data_point));
			// Seek back to the beginning
			data_file.seekg(0, ios::beg);
			// Allocate the buffer for the file and assign the pointer
			//	into the pointer buffer
			data_pointers[dtype] = (data_point*)malloc(filesize);
			// and read in the input data into the buffer
			data_file.read((char*)data_pointers[dtype], filesize);
			// And note that we have read the data
			data_loaded[dtype] = true;
		}
		// File didn't open for some reason
		else
		{
			cout << "can't open desired file" << endl;
			return NULL;
		}

	}

	// Assuming everything went well if we got to here
	return data_pointers[dtype];

}

// This function returns the number of data points in any data set
unsigned get_data_size(data_set_t dtype)
{
	return data_size[dtype];
}

// This function frees a dataset
void free_data(data_set_t dtype)
{
	// Make sure the data is actually loaded before doing anything
	if ( (data_pointers[dtype] != NULL) && (data_loaded[dtype] == true) )
	{
		// If it is, just free the pointer and reset the variables
		free(data_pointers[dtype]);
		data_pointers[dtype] = NULL;
		data_loaded[dtype] = false;
		data_size[dtype] = 0;
	}
}