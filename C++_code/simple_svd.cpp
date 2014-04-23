//
//
// This file performs a simple version of SVD for predicting 
//

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include "data_types.h"

using namespace std;

// Define the number of features in the SVD
#define NUM_FEATURES 40

// Define the learning rate
#define LEARNING_RATE 0.1

// Declare the feature vectors for SVD
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];

int main()
{
	time_t begin, end;
	double seconds;
	unsigned filesize;
	char * train_data;
	data_point * data;
	// Need to initialize the movie and user feature vectors
	memset(movie_features, 1, NUM_FEATURES*NUM_MOVIES*sizeof(float)/sizeof(int));
	memset(user_features, 1, NUM_FEATURES*NUM_USERS*sizeof(float)/sizeof(int));

	// Determine the size of the training data
	ifstream train_mu_data ("../../../data/mu/train.bin", ios::binary);
	if (train_mu_data.is_open())
	{
		train_mu_data.seekg(0, ios::end);
		filesize = train_mu_data.tellg();
		// Seeking back to the beginning helps
		train_mu_data.seekg(0, ios::beg);
	}

	// Now, need to allocate the space for the input data
	train_data = (char *)malloc(filesize);

	time(&begin);

	// and read in the input data
	train_mu_data.read(train_data, filesize);

	time(&end);

	// Report the elapsed time to read in the data
	seconds = difftime(end, begin);

	cout << seconds << " seconds elapsed to read data" << endl;

	// And verify that the data seems about right
	data = (data_point*)train_data;
	cout << data->user << ";" << data->movie << ";" << data->timestamp << ";" << (int)data->rating << endl;

	// Finally, free the malloc'd space
	free(train_data);

	return 0;

}