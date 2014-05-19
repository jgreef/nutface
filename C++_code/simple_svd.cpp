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
#define NUM_FEATURES 50

// Define the learning rate
#define LEARNING_RATE 0.002
#define REGULARIZATION_RATE 0.04
#define NUM_EPOCHS 200

//
// TODO: Actually compute this value and
//	see if it helps. The error is higher than it 
//	should be by using this predefined value
//
#define MOVIE_AVG 3.7

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
int do_svd(void);
void get_probe(void);

int main(void)
{
	get_probe();
}

int do_svd()
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

		// Now evaluate and write the new qual file
		get_qual(QUAL_MU);
	}

	// And need to free the data
	free_data(TRAIN_MU);
	free_data(VALID_MU);
	free_data(QUAL_MU);

	return 0;

}

// This will get the probe based off of some sketchy files
void get_probe(void)
{
	// First, need to declare and open the files
	ifstream user_feature_file;
	user_feature_file.open("../../../data/solutions/simple_svd_user_features_4_1_6h30m__200_features_200_epochs.out", ios::binary);
	ifstream movie_feature_file;
	movie_feature_file.open("../../../data/solutions/simple_svd_movie_features_4_1_6h30m__200_features_200_epochs.out", ios::binary);
	ifstream user_bias_file;
	user_bias_file.open("../../../data/solutions/simple_svd_user_biases_4_1_6h30m__200_features_200_epochs.out", ios::binary);
	ifstream movie_bias_file;
	movie_bias_file.open("../../../data/solutions/simple_svd_movie_biases_4_1_6h30m__200_features_200_epochs.out", ios::binary);

	// Now, read in the shtuff
	user_feature_file.read((char*)user_features, (NUM_USERS*NUM_FEATURES)*sizeof(float));
	movie_feature_file.read((char*)movie_features, (NUM_MOVIES*NUM_FEATURES)*sizeof(float));
	user_bias_file.read((char*)user_bias, NUM_USERS*sizeof(float));
	movie_bias_file.read((char*)movie_bias, NUM_MOVIES*sizeof(float));

	// Predict the RMSE on probe to make sure that it worked OK and write the outfile
	get_svd_rmse(PROBE_MU);
}

// This funciont is to be called on a point in the data set to train 
//	all of the features on one point
static inline void svd_train(user_type user, movie_type movie, rating_type rating)
{
	float error;
	float temp_user, temp_movie;

	// Calculate the error
	error = LEARNING_RATE*(rating - predict_rating(user, movie));

	// Descend on each of the features
	for (int i = 0; i < NUM_FEATURES; i++)
	{
		// Adjust each of the features
		temp_user = user_features[user - 1][i];
		temp_movie = movie_features[movie - 1][i];
		// Don't forget to account for zero-indexing

		user_features[user - 1][i] += error*temp_movie - LEARNING_RATE*REGULARIZATION_RATE*temp_user;
		movie_features[movie - 1][i] += error*temp_user - LEARNING_RATE*REGULARIZATION_RATE*temp_movie;
	}

	// Descend on the user/movie biases
	user_bias[user - 1] += error - LEARNING_RATE*REGULARIZATION_RATE*user_bias[user - 1];
	movie_bias[movie - 1] += error - LEARNING_RATE*REGULARIZATION_RATE*movie_bias[movie - 1];

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

	// ret_val = (prediction > 5) ? 5 : prediction;
	// ret_val = (prediction < 1) ? 1 : prediction;

	return prediction;
}

// this function calculates the out-of-sample error on the passed dataset
float get_svd_rmse(data_set_t dset)
{
	unsigned num_points;
	data_point* data;
	float error, rmse;
	time_t begin_time, end_time;
	float prediction;

	ofstream probe_out;
	probe_out.open("../../../data/solutions/simple_svd_200_200_probe.out", ios::trunc);

	// First, need to get the data
	data = get_data(dset);
	num_points = get_data_size(dset);

	// Initialize the rmse
	rmse = 0;

	time(&begin_time);

	for (int i = 0; i < num_points; i++)
	{
		// Calculate the error
		prediction = predict_rating(data->user, data->movie);
		// write the prediction to the output file
		probe_out << prediction << endl;
		// And now update the error
		error = data->rating - prediction;
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

