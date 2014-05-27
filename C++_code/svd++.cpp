#include <stdio.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include "data_types.h"
#include "data_io.h"

using namespace std;

#define NUM_EPOCHS 100
#define NUM_FEATURES 30

#define MOVIE_AVG 3.60861
#define BIAS_CONSTANT 25

// Bound the feature vectors by these values
#define MIN_FEATURE_START -0.1
#define MAX_FEATURE_START 0.1

// Maximum number of movies rated by any one user
#define MAX_MOVIES 3341

// Constant for filename buffer size
#define FILENAME_BUF_SIZE 512

// Parameters for learning
#define LAMBDA 0.002
#define LEARNING_RATE 0.0001
#define BIAS_RATE 0.002

// Parameters of integrated model

// normal bias and feature parameters
float user_bias[NUM_USERS];
float movie_bias[NUM_MOVIES];
float movie_features[NUM_MOVIES][NUM_FEATURES];
float user_features[NUM_USERS][NUM_FEATURES];
// Copies of feature matrices for algorithm
float user_f2[NUM_USERS][NUM_FEATURES];
float user_f3[NUM_USERS][NUM_FEATURES];

// number of movies rated by each user
int movie_count[NUM_USERS];
// the y value in tier two of the model
float implicit_y[NUM_MOVIES][NUM_FEATURES];
// list of movies each user has rated, filled out with zeros.
int user_movies[MAX_MOVIES];
// list of ratings for each of theose movies, filled out with zeros.
rating_type user_movie_ratings[MAX_MOVIES];

static inline void initialize_bias_and_features(void);
static inline void initialize_movie_count(data_point* data, unsigned num_points);
static inline void initialize_user_movies(data_point* data);
static inline data_point * get_curr_movies(data_point* data, user_type user);
static inline data_point * get_qual_movies(data_point* data, unsigned num_points, user_type user);
static inline float sum_implicit_y(int feature_idx);
static inline float mult_feat_vec(user_type real_user, movie_type real_movie);
static inline void predict_qual();
static inline void predict_probe();
static inline void get_svd_rmse();
void init_bias_vectors(data_point* data, unsigned num_points);


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

    cout << "Initalizing parameters..." << endl;
    // initialize parameters for the model
    initialize_bias_and_features();
    init_bias_vectors(data_train_start, num_points);
    initialize_movie_count(data_train_start, num_points);

    cout << "Training data..." << endl;

    for (int i = 0; i < NUM_EPOCHS; i++) {
        cout << "Iteration " << i+1 << endl;

        data = data_train_start;

        // for each point in the dataset, train all parameters
        for (int j = 0; j < NUM_USERS; j++) {

            // get the movies rated by this user
            data = get_curr_movies(data, data->user);

            // update the user features, and save them
            for (int k = 0; k < NUM_FEATURES; k++) {

                user_features[j][k] = pow(movie_count[j], (float) -0.5) * sum_implicit_y(k);
                user_f2[j][k] = user_features[j][k];
            }

            //for each of the movies rated by this user...
            for (int k = 0; k < movie_count[j]; k++) {

                movie_type curr_movie = user_movies[k];
                rating_type curr_rating = user_movie_ratings[k];

                // get the error
                float error = curr_rating - (user_bias[j] + movie_bias[curr_movie - 1] + mult_feat_vec(j, curr_movie - 1));

                // train the user and movie features
                for (int l = 0; l < NUM_FEATURES; l++) {

                    user_f3[j][l] = user_features[j][l];
                    user_features[j][l] += LEARNING_RATE*error*(movie_features[curr_movie - 1][l] - LAMBDA*user_features[j][l]);
                    movie_features[curr_movie - 1][l] += LEARNING_RATE*error*(user_f3[j][l] - LAMBDA*movie_features[curr_movie - 1][l]);
                }

                // train the user and movie biases
                user_bias[j] += BIAS_RATE*error;
                movie_bias[curr_movie - 1] += BIAS_RATE*error;

            }

            // train the "implicit y" parameters
            for (int k = 0; k < movie_count[j]; k++) {

                movie_type curr_movie = user_movies[k];

                for (int l = 0; l < NUM_FEATURES; l++) {

                    implicit_y[curr_movie - 1][l] += pow(movie_count[j], (float) -0.5)*(user_features[j][l] - user_f2[j][l]);

                }
            }
        }

        get_svd_rmse();
    }

    // Now predict on qual.
    cout << "Predicting data..." << endl;
    predict_qual();
    predict_probe();

    return 0;

}


// Initialize all biases, features, implicit y values to random values centered at 0
static inline void initialize_bias_and_features(void) {

/*
    // initialize user bias
    for (int i = 0; i < NUM_USERS; i++) {
        user_bias[i] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
    }
    // initialize movie bias
    for (int i = 0; i < NUM_MOVIES; i++) {
        movie_bias[i] = MIN_FEATURE_START + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(MAX_FEATURE_START-MIN_FEATURE_START)));
    }

*/
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
            user_f2[j][k] = user_features[j][k];
            user_f3[j][k] = user_features[j][k];
        }
    }
}

// Get the number of movies each user has rated.
static inline void initialize_movie_count(data_point* data, unsigned num_points) {

    // initialize movie count for each user (# of movies the user rated)
    for (int i = 0; i < num_points; i++) {

        movie_count[data->user - 1]++;
        data++;
    }
}


// Put all movies rated by the current user into an array. Returns a pointer to
// the current location in the dataset (will be the first rating by the next
// user after this one).
static inline data_point* get_curr_movies(data_point* data, user_type user)
{
    int i = 0;
    while (data->user == user) {
        user_movies[i] = data->movie;
        user_movie_ratings[i] = data->rating;
        i++;
        data++;
    }
    for (; i < MAX_MOVIES; i++) {
        user_movies[i] = 0;
        user_movie_ratings[i] = 0;
    }

    return data;

}

// Get the movies rated by this user from qual. Different function than for training
// because we start from the beginning of the dataset every time.
static inline data_point* get_qual_movies(data_point* data, unsigned num_points, user_type user) {

    while (data->user < user) {
        data++;
    }

    data = get_curr_movies(data, user);
    return data;
}

// Function to do the sum of "implicit y" values for a certain feature over
// all the movies rated by the current user.
static inline float sum_implicit_y(int feature_idx) {

    float total = 0.0;

    for (int i = 0; i < MAX_MOVIES; i++) {

        if (user_movies[i] == 0) {
            return total;
        }
        else {
            total += implicit_y[user_movies[i] - 1][feature_idx];
        }

    }

    return total;

}

// Function to get the product of the user and movie feature vectors, given a
// user and a movie.
static inline float mult_feat_vec(user_type real_user, movie_type real_movie) {

    float total = 0;

    for (int i = 0; i < NUM_FEATURES; i++) {
        total += user_features[real_user][i] * movie_features[real_movie][i];
    }

    return total;
}

// Do the prediction for qual.
static inline void predict_qual() {

    data_point * data = get_data(QUAL_UM);
    unsigned data_size = get_data_size(QUAL_UM);

    data_point * train_data = get_data(TRAIN_UM);
    unsigned num_points = get_data_size(TRAIN_UM);

    float prediction;

    time_t timestamp;
    struct tm * curr_time;
    char filename[FILENAME_BUF_SIZE];

    // Get the time
    timestamp = time(0);
    // Convert the time to a string
    curr_time = localtime(&timestamp);

    snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/nsvd1_qual_%d_%d_%dh%dm__%d_features_.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES);

    ofstream qual_output (filename, ios::trunc);

    user_type curr_user = 0;

    for(int i = 0; i < data_size; i++)
    {
        user_type user = data->user;
        movie_type movie = data->movie;

        if (curr_user != user) {

            train_data = get_qual_movies(train_data, num_points, user);
        }

        curr_user = user;

        // start prediction with the normal movie mean, plus user and feature bias.
        float prediction = movie_bias[movie - 1] + user_bias[user - 1];

        for (int l = 0; l < NUM_FEATURES; l++) {

            prediction += movie_features[movie - 1][l] * pow(movie_count[user - 1], (float) -0.5) * sum_implicit_y(l);
        }

        float ret_val = (prediction > 5) ? 5 : prediction;
        ret_val = (ret_val < 1) ? 1 : ret_val;

        // Write it to the file
        qual_output << ret_val << endl;
        // Increment the pointer
        data++;
    }
}

// Do the prediction for qual.
static inline void predict_probe() {

    data_point * data = get_data(PROBE_UM);
    unsigned data_size = get_data_size(PROBE_UM);

    data_point * train_data = get_data(TRAIN_UM);
    unsigned num_points = get_data_size(TRAIN_UM);

    float prediction;

    time_t timestamp;
    struct tm * curr_time;
    char filename[FILENAME_BUF_SIZE];

    // Get the time
    timestamp = time(0);
    // Convert the time to a string
    curr_time = localtime(&timestamp);

    snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/nsvd1_probe_%d_%d_%dh%dm__%d_features_.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min, NUM_FEATURES);

    ofstream qual_output (filename, ios::trunc);

    user_type curr_user = 0;

    for(int i = 0; i < data_size; i++)
    {
        user_type user = data->user;
        movie_type movie = data->movie;

        if (curr_user != user) {

            train_data = get_qual_movies(train_data, num_points, user);
        }

        curr_user = user;

        // start prediction with the normal movie mean, plus user and feature bias.
        float prediction = movie_bias[movie - 1] + user_bias[user - 1];

        for (int l = 0; l < NUM_FEATURES; l++) {

            prediction += movie_features[movie - 1][l] * pow(movie_count[user - 1], (float) -0.5) * sum_implicit_y(l);
        }

        float ret_val = (prediction > 5) ? 5 : prediction;
        ret_val = (ret_val < 1) ? 1 : ret_val;

        // Write it to the file
        qual_output << ret_val << endl;
        // Increment the pointer
        data++;
    }
}

// Do the prediction for qual.
static inline void get_svd_rmse() {

    data_point * data = get_data(PROBE_UM);
    unsigned data_size = get_data_size(PROBE_UM);

    data_point * train_data = get_data(TRAIN_UM);
    unsigned num_points = get_data_size(TRAIN_UM);

    float prediction, rmse, error;

    user_type curr_user = 0;
    rmse = 0;
    error = 0;

    for(int i = 0; i < data_size; i++)
    {
        user_type user = data->user;
        movie_type movie = data->movie;

        if (curr_user != user) {

            train_data = get_qual_movies(train_data, num_points, user);
        }

        curr_user = user;

        // start prediction with the normal movie mean, plus user and feature bias.
        float prediction = movie_bias[movie - 1] + user_bias[user - 1];

        for (int l = 0; l < NUM_FEATURES; l++) {

            prediction += movie_features[movie - 1][l] * pow(movie_count[user - 1], (float) -0.5) * sum_implicit_y(l);
        }

        float ret_val = (prediction > 5) ? 5 : prediction;
        ret_val = (ret_val < 1) ? 1 : ret_val;

        error = data->rating - ret_val;
        rmse += error*error;

        // Increment the pointer
        data++;
    }

    // Calculate the RMSE
    rmse = sqrt(rmse/data_size);
    cout << " RMSE of: " << rmse << endl;
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
    // a user (NOT the user's average rating), minus the average rating.
    for (int i = 0; i < NUM_USERS; i++) {

        user_bias[i] = ((MOVIE_AVG * BIAS_CONSTANT + user_rating_sum[i])/(BIAS_CONSTANT + user_num[i])) - MOVIE_AVG;
    }
}