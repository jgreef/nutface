//
//
// This file performs a simple version of SVD for predicting 
//

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <cmath>
#include "data_types.h"
#include "data_io.h"

using namespace std;

// Define the number of features in the SVD
#define NUM_FEATURES 40

// Define the learning rate
#define LEARNING_RATE 0.001

// Declare the feature vectors for SVD
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];

// Function declarations
static inline void svd_train(user_type user, movie_type movie, rating_type rating);
static inline float predict_rating(user_type user, movie_type movie);
float get_svd_rmse(data_set_t dset);
void get_qual(data_set_t dset);

int main()
{
	time_t begin_time, end_time;
	double seconds;
	float old_rmse = 100.0;
	float new_rmse = 50.0;
	int epochs = 0;
	unsigned data_points;
	int j, k;

	data_point * data;
	data_point * data_start;

	// Need to initialize the movie and user feature vectors
	for (j = 0; j < NUM_MOVIES; j++)
	{
		for (k = 0; k < NUM_FEATURES; k++)
		{
			movie_features[j][k] = 0.1;
		}
	}
	for (j = 0; j < NUM_USERS; j++)
	{
		for (k = 0; k < NUM_FEATURES; k++)
		{
			user_features[j][k] = 0.1;
		}
	}

	// Need to initialize the data io
	init_data_io();
	// Get the data and verify that it's about right
	data_start = get_data(TRAIN_MU);
	// Get the size of the data
	data_points = get_data_size(TRAIN_MU);


	// Loop until the old RMSE and new RMSE differ by about 0.001
	while( (old_rmse - new_rmse) > 0.001)
	{

		epochs++;

		// Initialize the timer
		time(&begin_time);

		// Initialize the data pointer
		data = data_start;

		for (int i = 0; i < data_points; i++)
		{
			// Train
			svd_train(data->user, data->movie, data->rating);
			// Increment the pointer
			data++;
		}

		// Update the RMSE
		old_rmse = new_rmse;
		new_rmse = get_svd_rmse(VALID_MU);

		// Figure out how long it took
		time(&end_time);

		cout << "epoch " << epochs << " completed in " << (end_time - begin_time) << " seconds" << endl;
	}

	// Now evaluate qual
	get_qual(QUAL_MU);

	// And need to free the data
	free_data(TRAIN_MU);
	free_data(VALID_MU);

	return 0;

}

// This funciont is to be called on a point in the data set to train 
//	all of the features on one point
static inline void svd_train(user_type user, movie_type movie, rating_type rating)
{
	float error;
	float temp_val;

	// Calculate the error
	error = LEARNING_RATE*(rating - predict_rating(user, movie));

	for (int i = 0; i < NUM_FEATURES; i++)
	{
		// Adjust each of the features
		temp_val = user_features[user - 1][i];
		// Don't forget to account for zero-indexing
		user_features[user - 1][i] += error*movie_features[movie - 1][i];
		movie_features[movie - 1][i] += error*temp_val;
	}

}

// this function will predict a rating for a user and movie by doing the inner 
//	product of the feature vectors
static inline float predict_rating(user_type user, movie_type movie)
{
	float prediction = 0.0;

	for (int i = 0; i < NUM_FEATURES; i++)
	{
		// Don't forget to account for zero-indexing
		prediction += movie_features[movie - 1][i]*user_features[user - 1][i];
	}

	// if (prediction > 5)
	// {
	// 	prediction  = 5;
	// }
	// if (prediction < 0)
	// {
	// 	prediction = 0;
	// }

	return prediction;
}

// this function calculates the out-of-sample error on the passed dataset
float get_svd_rmse(data_set_t dset)
{
	unsigned data_points;
	data_point* data;
	float error, avg_sqerr;
	time_t begin_time, end_time;
	float rmse, sqerr;
	float prediction;

	// First, need to get the data
	data = get_data(dset);
	data_points = get_data_size(dset);

	// Initialize the rmse
	rmse = 0;

	time(&begin_time);

	for (int i = 0; i < data_points; i++)
	{
		// Calculate the error
		error = data->rating - predict_rating(data->user, data->movie);;
		rmse += error*error;
		data++;
	}

	// Calculate the RMSE
	rmse = sqrt(rmse/data_points);

	time(&end_time);

	cout << "RMSE of: " << rmse << ", took " << (end_time - begin_time) << " seconds" << endl;

	return rmse;
}

// This function will get the qual output for a svd trained model. DSET should
//	be either QUAL_MU or QUAL_UM
void get_qual(data_set_t dset)
{
	data_point * data;
	unsigned data_size;
	float prediction;

	ofstream qual_output ("../../../data/solutions/simple_svd_qual.out", ios::trunc);

	// Get the qual data
	data = get_data(dset);
	// Get the qual size
	data_size = get_data_size(dset);

	for(int i = 0; i < data_size; i++)
	{
		// Get the predicion
		prediction = predict_rating(data->user, data->movie);
		// Write it to the file
		qual_output << prediction << endl;
	}

	// Free the qual data
	free_data(dset);

}

