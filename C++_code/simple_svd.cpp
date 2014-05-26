//
//
// This file performs a simple version of SVD for predicting
//

// TODO: Play around with how many features/epochs we want. Maybe try
// 120 features with 100, 200, and 300 epochs, or a fixed number
// of epochs on like 100, 200, 300 features.

// TODO: Try not training on probe? Then we could use probe to
// get out-of-sample error and we could stop training when
// successive RMSEs don't differ by very much. Unless taking probe
// out significantly decreases our performance.


#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <cmath>
#include <numeric>
#include <stdio.h>
// #include <parallel/numeric>
#include "data_types.h"
#include "data_io.h"

using namespace std;

// Define the number of features in the SVD
#define NUM_FEATURES 200

// Define the learning rate
#define LEARNING_RATE 0.002
#define NUM_EPOCHS 200
#define REGULARIZATION_RATE 0.04

// average score across all movies in train
#define MOVIE_AVG 3.60861

// Bound the feature vectors by these values
#define MIN_FEATURE_START -0.1
#define MAX_FEATURE_START 0.1

// Constant for calculating user/movie bias
#define BIAS_CONSTANT 25

// Constant for filename buffer size
#define FILENAME_BUF_SIZE 512

// Declare the feature vectors for SVD
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];
float user_bias[NUM_USERS];
float movie_bias[NUM_MOVIES];

// Function declarations
static inline void svd_train(user_type user, movie_type movie, rating_type rating);
static inline float predict_rating(user_type user, movie_type movie);
float get_svd_rmse(data_set_t dset);
void get_qual(data_set_t dset, int epochs);
void init_bias_vectors(data_point * train_start, unsigned train_points);

int main()
{
	time_t begin_time, end_time, timestamp;
	double seconds;
	float probe_rmse, train_rmse, valid_rmse;
	int epochs = 0;
	unsigned data_train_points, data_probe_points;
	int j, k;
	struct tm * curr_time;
	char filename[FILENAME_BUF_SIZE];

	data_point * data;
	data_point * data_train_start, *data_probe_start;

    // need this to seed the "rand" function
    srand (static_cast <unsigned> (time(0)));

    // Need to initialize the data io
    init_data_io();
    // Get the MU data and verify that it's about right
    data_train_start = get_data(TRAIN_MU);
    // Get the size of the data
    data_train_points = get_data_size(TRAIN_MU);

    init_bias_vectors(data_train_start, data_train_points);


	// Need to initialize the movie feature vector
	for (j = 0; j < NUM_MOVIES; j++)
	{
		for (k = 0; k < NUM_FEATURES; k++)
		{
			movie_features[j][k] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
		}
	}

    // Need to initialize the user feature vector
	for (j = 0; j < NUM_USERS; j++)
	{
		for (k = 0; k < NUM_FEATURES; k++)
		{
			user_features[j][k] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
		}
	}

	// Get the time
	timestamp = time(0);
	// Convert the time to a string
	curr_time = localtime(&timestamp);


	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_rmse_%d_%d_%dh%dm__%d_features.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES);
	ofstream out_rmse (filename, ios::trunc);


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
		// data = data_probe_start;
		// for (int i = 0; i < data_probe_points; i++)
		// {
		// 	// Train
		// 	svd_train(data->user, data->movie, data->rating);
		// 	// Increment the pointer
		// 	data++;
		// }

		// Figure out how long it took
		time(&end_time);

		cout << "epoch " << epochs << " completed in " << (end_time - begin_time) << " seconds" << endl;

		// Update the RMSEs
		// train_rmse = get_svd_rmse(TRAIN_MU);
		// valid_rmse = get_svd_rmse(VALID_MU);
		probe_rmse = get_svd_rmse(PROBE_MU);
		// And write them to file
		out_rmse << probe_rmse << endl;

		// Now evaluate and write the new qual file
		get_qual(QUAL_MU, epochs);
		get_qual(PROBE_MU, epochs);
	}

	// And need to free the data
	free_data(TRAIN_MU);
	free_data(VALID_MU);
	free_data(QUAL_MU);

	// Get the time
	timestamp = time(0);
	// Convert the time to a string
	curr_time = localtime(&timestamp);

	// Finally, we should write the SVD features to disk as well so that we can use it in other applications
	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_movie_features_%d_%d_%dh%dm__%d_features_%d_epochs.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES, NUM_EPOCHS);
	ofstream movie_feature_out (filename, ios::binary);
	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_user_features_%d_%d_%dh%dm__%d_features_%d_epochs.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES, NUM_EPOCHS);
	ofstream user_feature_out (filename, ios::binary);
	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_movie_biases_%d_%d_%dh%dm__%d_features_%d_epochs.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES, NUM_EPOCHS);
	ofstream movie_bias_out (filename, ios::binary);
	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_user_biases_%d_%d_%dh%dm__%d_features_%d_epochs.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES, NUM_EPOCHS);
	ofstream user_bias_out (filename, ios::binary);

	// And write the features to file
	movie_feature_out.write((char*)movie_features, NUM_MOVIES*NUM_FEATURES*sizeof(float));
	user_feature_out.write((char*)user_features, NUM_USERS*NUM_FEATURES*sizeof(float));
	movie_bias_out.write((char*)movie_bias, NUM_MOVIES*sizeof(float));
	user_bias_out.write((char*)user_bias, NUM_USERS*sizeof(float));

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
    adjustment_term =  LEARNING_RATE*REGULARIZATION_RATE;

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
		error = data->rating - predict_rating(data->user, data->movie);
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
void get_qual(data_set_t dset, int epochs)
{
	data_point * data;
	unsigned data_size;
	float prediction;

	time_t timestamp;
	struct tm * curr_time;
	char filename[FILENAME_BUF_SIZE];

	// Get the time
	timestamp = time(0);
	// Convert the time to a string
	curr_time = localtime(&timestamp);



	snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/simple_svd_%d_%d_%d_%dh%dm__%d_features_epoch_%d.out", dset, curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES, epochs);

	ofstream qual_output (filename, ios::trunc);

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

// This function initializes the bias vectors for users and movies. It's called once.
void init_bias_vectors(data_point * train_start, unsigned train_points)
{

    data_point * data = train_start;

    // Vectors to keep track of how many movies/users, and the total ratings of
    // movies/users
    int movie_num[NUM_MOVIES];
    int movie_rating_sum[NUM_MOVIES];
    int user_num[NUM_USERS];
    int user_rating_sum[NUM_USERS];

    // Initialize movie and user vectors
    for (int i = 0; i < NUM_MOVIES; i++)
    {
        movie_num[i] = 0;
        movie_rating_sum[i] = 0;
    }
    for (int i = 0; i < NUM_USERS; i++)
    {
        user_num[i] = 0;
        user_rating_sum[i] = 0;
    }

    // Run through each data point, updating the number of users/movies that
    // rated it, as well as the user/movie total sum.
    for (int i = 0; i < train_points; i++)
    {
        movie_num[(data->movie)-1] += 1;
        movie_rating_sum[(data->movie)-1] += data->rating;

        user_num[(data->user)-1] += 1;
        user_rating_sum[(data->user)-1] += data->rating;

        data++;
    }

    // For each movie/user, calculate the bias. This is the average rating for
    // the movie/user (NOT the movie/users's average rating), minus the average
    // rating. Formulas found from the funny article (BetterMean).
    for (int i = 0; i < NUM_MOVIES; i++) {

        movie_bias[i] = ((MOVIE_AVG * BIAS_CONSTANT + movie_rating_sum[i])/(BIAS_CONSTANT + movie_num[i])) - MOVIE_AVG;
    }

    // For each user, calculate the user bias. This is the average rating for
    // a user (NOT the user's average rating), minus the user's average rating.
    for (int i = 0; i < NUM_USERS; i++) {

        user_bias[i] = ((MOVIE_AVG * BIAS_CONSTANT + user_rating_sum[i])/(BIAS_CONSTANT + user_num[i])) - MOVIE_AVG;
    }
}


