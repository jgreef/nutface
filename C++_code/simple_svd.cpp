//
//
// This file performs a simple version of SVD for predicting
//

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <cmath>
#include <numeric>
// #include <parallel/numeric>
#include "data_types.h"
#include "data_io.h"

using namespace std;

// Define the number of features in the SVD
#define NUM_FEATURES 40

// Define the learning rate
#define LEARNING_RATE 0.002
#define NUM_EPOCHS 60

// Define the regularization rate. This will change after each epoch.
//float regularization_rate = 0.04;
float regularization_rate = 0.02;

#define MOVIE_AVG 3.60861

// Declare the feature vectors for SVD
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];
float user_bias[NUM_USERS];
float movie_bias[NUM_MOVIES];

// Function declarations
static inline void svd_train(user_type user, movie_type movie, rating_type rating);
static inline float predict_rating(user_type user, movie_type movie);
float get_svd_rmse(data_set_t dset);
void get_qual(data_set_t dset);

int main()
{
	time_t begin_time, end_time;
	double seconds;
	float probe_rmse, train_rmse, valid_rmse;
	int epochs = 0;
	unsigned data_train_points, data_probe_points;
	int j, k;

	data_point * data;
	data_point * data_train_start, *data_probe_start;

	// Need to initialize the movie and user feature vectors
	for (j = 0; j < NUM_MOVIES; j++)
	{
		movie_bias[j] = 0;

		for (k = 0; k < NUM_FEATURES; k++)
		{
			movie_features[j][k] = 0.1;
		}
	}
	for (j = 0; j < NUM_USERS; j++)
	{
		user_bias[j] = 0;

		for (k = 0; k < NUM_FEATURES; k++)
		{
			user_features[j][k] = 0.1;
		}
	}

	// Open the output RMSE file
	ofstream out_rmse ("../../../data/solutions/simple_svd_rmse.csv", ios::trunc);

	// Need to initialize the data io
	init_data_io();
	// Get the data and verify that it's about right
	data_train_start = get_data(TRAIN_MU);
	// Get the size of the data
	data_train_points = get_data_size(TRAIN_MU);

	// Also need to get the probe data
	data_probe_start = get_data(PROBE_MU);
	data_probe_points = get_data_size(PROBE_MU);


	// Loop until the old RMSE and new RMSE differ by about 0.001
	while( epochs < NUM_EPOCHS )
	{

		epochs++;

		// Initialize the timer
		time(&begin_time);

		// Train on the training set
		data = data_train_start;
		for (int i = 0; i < data_train_points; i++)
		{
			// Train
			svd_train(data->user, data->movie, data->rating);
			// Increment the pointer
			data++;
		}


		// Train on Probe
		data = data_probe_start;
		for (int i = 0; i < data_probe_points; i++)
		{
			// Train
			svd_train(data->user, data->movie, data->rating);
			// Increment the pointer
			data++;
		}

		// Figure out how long it took
		time(&end_time);

		cout << "epoch " << epochs << " completed in " << (end_time - begin_time) << " seconds" << endl;

		// Update the RMSEs
		// train_rmse = get_svd_rmse(TRAIN_MU);
		// valid_rmse = get_svd_rmse(VALID_MU);
		probe_rmse = get_svd_rmse(PROBE_MU);
		// And write them to file
		out_rmse << train_rmse << "," << valid_rmse << "," << probe_rmse << endl;

        // Update regularization rate
        //regularization_rate *= 0.9;

		// Now evaluate and write the new qual file
		get_qual(QUAL_MU);
	}

	// And need to free the data
	free_data(TRAIN_MU);
	free_data(VALID_MU);
	free_data(QUAL_MU);

	return 0;

}

// This funciont is to be called on a point in the data set to train
//	all of the features on one point
static inline void svd_train(user_type user, movie_type movie, rating_type rating)
{
	float error, adjustment_term;
	float temp_user, temp_movie;
    float temp_user_bias, temp_movie_bias;

	// Calculate the error
	error = LEARNING_RATE*(rating - predict_rating(user, movie));
    adjustment_term =  LEARNING_RATE*regularization_rate;

	// Descend on each of the features
	for (int i = 0; i < NUM_FEATURES; i++)
	{
		// Adjust each of the features
		temp_user = user_features[user - 1][i];
		temp_movie = movie_features[movie - 1][i];
		// Don't forget to account for zero-indexing

		user_features[user - 1][i] += error*temp_movie - adjustment_term*temp_user;
		movie_features[movie - 1][i] += error*temp_user - adjustment_term*temp_movie;
	}

	// Descend on the user/movie biases
	user_bias[user - 1] += error - adjustment_term*user_bias[user - 1];
	movie_bias[movie - 1] += error - adjustment_term*movie_bias[movie - 1];

    //temp_user_bias = user_bias[user - 1];
    //temp_movie_bias = movie_bias[movie - 1];

    //user_bias[user - 1] += error - LEARNING_RATE*0.05*(temp_user_bias + temp_movie_bias);
    //movie_bias[movie - 1] += error - LEARNING_RATE*0.05*(temp_user_bias + temp_movie_bias);

}

// this function will predict a rating for a user and movie by doing the inner
//	product of the feature vectors
static inline float predict_rating(user_type user, movie_type movie)
{
	float prediction = MOVIE_AVG;

	for (int j = 0; j < NUM_FEATURES; j++)
	{
		// Don't forget to account for zero-indexing
		prediction += movie_features[movie - 1][j]*user_features[user - 1][j];
	}

	// Add in the movie average and the movie and user biases
	prediction += movie_bias[movie - 1] + user_bias[user - 1];

	float ret_val = (prediction > 5) ? 5 : prediction;
	ret_val = (prediction < 1) ? 1 : ret_val;

	//return prediction;
    return ret_val;
}

// this function calculates the out-of-sample error on the passed dataset
float get_svd_rmse(data_set_t dset)
{
	unsigned num_points;
	data_point* data;
	float error, rmse;
	time_t begin_time, end_time;
	float prediction;

	// First, need to get the data
	data = get_data(dset);
	num_points = get_data_size(dset);

	// Initialize the rmse
	rmse = 0;

	time(&begin_time);

	for (int i = 0; i < num_points; i++)
	{
		// Calculate the error
		error = data->rating - predict_rating(data->user, data->movie);;
		rmse += error*error;
		data++;
	}

	// Calculate the RMSE
	rmse = sqrt(rmse/num_points);

	time(&end_time);

	cout << dset << " RMSE of: " << rmse << ", took " << (end_time - begin_time) << " seconds" << endl;

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
		// Increment the pointer
		data++;
	}

}

