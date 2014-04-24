//
//
// This file performs a simple version of SVD for predicting 
//

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include "data_types.h"
#include "data_io.h"

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

	// Need to initialize the data io
	init_data_io();

	// Get the data and verify that it's about right
	data = get_data(TRAIN_MU);
	if (data == NULL)
	{
		cout << "well shit" << endl;
	}

	cout << data->user << ";" << data->movie << ";" << data->timestamp << ";" << (int)data->rating << endl;

	// And need to free the data
	free_data(TRAIN_MU);

	return 0;

}