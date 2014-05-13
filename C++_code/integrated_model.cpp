//
//
// This file performs the integrated model from Koren's 2008 paper.
//

#include <stdio.h>
#include <iostream>
#include <cmath>
#include "data_types.h"
#include "data_io.h"

using namespace std;

#define NUM_ITERATIONS 30
#define NUM_FEATURES 50

// Bound the feature vectors by these values
#define MIN_FEATURE_START -0.1
#define MAX_FEATURE_START 0.1

#define MOVIE_AVG 3.60861

float gammas[6] =  { 0.007, 0.007, 0.001, 0.005, 0.015, 0.015 };

// Parameters of integrated model
float user_bias[NUM_USERS];
float movie_bias[NUM_MOVIES];
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];
// number of movies rated by each user
float movie_count[NUM_USERS];
// the y value in tier two of the model
float implicit_y[NUM_MOVIES][NUM_FEATURES];
// a list of movies the current user has rated
float curr_user[NUM_MOVIES];
float user_implicit_y[NUM_FEATURES];

static inline void initialize_parameters(void);
static inline void get_curr_movies(data_point* data, user_type user);
static inline float predict_rating(user_type user, movie_type movie);
static inline void bias_train(user_type user, movie_type movie, float error);
static inline void feature_train(user_type user, movie_type movie, float error);
static inline void implicit_y_train(user_type user, movie_type movie, float error);



int main()
{
    data_point *data_train_start, *data;
    unsigned num_points;

    // need this to seed the "rand" function
    srand (static_cast <unsigned> (time(0)));

    cout << "Loading data..." << endl;
    init_data_io();

    data_train_start = get_data(TRAIN_UM);
    num_points = get_data_size(TRAIN_UM);

    // initialize some of the parameters for the model
    initialize_parameters();

    data = data_train_start;


    // initialize movie count for each user (# of movies the user rated)
    for (int j = 0; j < num_points; j++) {

        movie_count[data->user - 1]++;
        data++;
    }
    // and the only thing we care about is the movie count to the -1/2 power
    for (int j = 0; j < NUM_USERS; j++) {

        movie_count[j] = pow(movie_count[j], (float) -0.5);
    }


    cout << "Training data..." << endl;
    data = data_train_start;

    // train for this many iterations over the dataset
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        cout << "Iteration " << i+1 << endl;

        int last_user = 0;

        // for each point in the dataset, train all parameters
        for (int j = 0; j < num_points; j++) {

            if (j % 100000 == 0) {
                cout << "   Data point " << j << endl;
            }

            // if we've reached a new user, we need to update the list of current movies
            int this_user = data->user;

            if (this_user != last_user) {
                //find this user's rated movies
                get_curr_movies(data, data->user);
                last_user = this_user;
            }

            // get current error first
            float error = data->rating - predict_rating(data->user, data->movie);

            for (int k = 0; k < NUM_FEATURES; k++) {
                user_implicit_y[k] = 0;
            }

            // train all of the parameters for the model
            bias_train(data->user, data->movie, error);
            feature_train(data->user, data->movie, error);
            implicit_y_train(data->user, data->movie, error);
            //train wij
            //train cij

            data++;
        }

        // update the constants so we don't overfit
        for (int j = 0; j < 6; j++) {
            gammas[j] *= 0.9;
        }

        data = data_train_start;

    }

    // TODO: loop over qual set using predict_rating



    // And need to free the data
    free_data(TRAIN_UM);
}

// Initialize biases and feature vectors for model
static inline void initialize_parameters(void)
{
    cout << "Initalizing parameters..." << endl;

    // initialize user bias
    for (int i = 0; i < NUM_USERS; i++) {
        user_bias[i] = 0;
        movie_count[i] = 0;
    }
    // initialize movie bias
    for (int i = 0; i < NUM_MOVIES; i++) {
        movie_bias[i] = 0;
        curr_user[i] = 0;
    }

    for (int i = 0; i < NUM_FEATURES; i++) {
        user_implicit_y[i] = 0;
    }

    // initialize movie feature vector
    for (int j = 0; j < NUM_MOVIES; j++)
    {
        for (int k = 0; k < NUM_FEATURES; k++)
        {
            movie_features[j][k] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
            implicit_y[j][k] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
        }
    }
    // initialize user feature vector
    for (int j = 0; j < NUM_USERS; j++)
    {
        for (int k = 0; k < NUM_FEATURES; k++)
        {
            user_features[j][k] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
        }
    }

}

// Fill the curr_user array with the movies the current user has rated. Fill
// the rest of the array with zeros.
static inline void get_curr_movies(data_point* data, user_type user)
{
    int i = 0;
    while (data->user == user) {
        curr_user[i] = data->movie;
        i++;
        data++;
    }
    for (; curr_user[i] != 0; i++) {
        curr_user[i] = 0;
    }

}

// The actual rating prediction. Based on formula from paper.
static inline float predict_rating(user_type user, movie_type movie)
{
    // tier 1 is the normal movie mean, plus user and feature bias.
    float tier_one = MOVIE_AVG + movie_bias[movie - 1] + user_bias[user - 1];

    // tier two includes implicit information
    float tier_two = 0;

    // take the dot product of the movie feature vector with the implicit user feedback
    for (int i = 0; i < NUM_FEATURES; i++) {

        tier_two += movie_features[movie - 1][i] * (user_features[user - 1][i] + movie_count[user - 1]*user_implicit_y[i]);
    }

    // and add the two tiers together
    float prediction = tier_one + tier_two;

    // TODO: add tier 3

    float ret_val = (prediction > 5) ? 5 : prediction;
    ret_val = (ret_val < 1) ? 1 : ret_val;

    //cout << ret_val << endl;

    return ret_val;
}

// Trains the user and movie biases for each data point
static inline void bias_train(user_type user, movie_type movie, float error)
{
    user_bias[user - 1] += gammas[0]*(error - gammas[3]*user_bias[user - 1]);
    movie_bias[movie - 1] += gammas[0]*(error - gammas[3]*movie_bias[movie - 1]);
}


// trains the movie and user feature vectors for each data point
static inline void feature_train(user_type user, movie_type movie, float error)
{

    for (int i = 0; i < NUM_FEATURES; i++)
    {

        user_features[user - 1][i] += gammas[1]*(error*movie_features[movie - 1][i] - gammas[4]*user_features[user - 1][i]);
        movie_features[movie - 1][i] += gammas[1]*(error*(user_features[user - 1][i] + movie_count[user - 1]*user_implicit_y[i]) - gammas[4]*movie_features[movie - 1][i]);

    }

}

// trains the yj parameter for each data point
static inline void implicit_y_train(user_type user, movie_type movie, float error)
{
    // curr_user is a vector of all the movie numbers that the user has rated.
    // 0 indicates the end of the list of movies they've rated.
    int i = 0;
    int curr_movie = curr_user[i];
    while (curr_movie != 0) {
        for (int j = 0; j < NUM_FEATURES; j++) {
            implicit_y[curr_movie - 1][j] += gammas[1]*(error*movie_count[user - 1]*movie_features[user - 1][movie - 1] - gammas[4]*implicit_y[curr_movie - 1][j]);
            user_implicit_y[j] += implicit_y[curr_movie - 1][j];
        }
        i++;
        curr_movie = curr_user[i];
    }
}