//
//
// K Nearest Neighbors
//
//

// FIX: When you get the common users vector, you also need get two vectors of rankings --
// one for the 1st movie and one for 2nd.
// each index corresponds to a different USER
// No full ranking of whole dataset.
// Don't use Spearman's because I don't know how to adjust it. Just use the one from the article.

#include <stdio.h>
#include <iostream>
#include <cmath>
#include "data_types.h"
#include "data_io.h"

using namespace std;

#define K 30
#define BETA 500

float similarities[NUM_MOVIES][NUM_MOVIES];
float A[NUM_MOVIES][NUM_MOVIES];

float B[K];
float weights[NUM_MOVIES];

user_type common_users[NUM_USERS/4];
rating_type ratings1[NUM_USERS/4];
rating_type ratings2[NUM_USERS/4];

// Function declarations
//static inline void rank_movies(data_point * data, unsigned num_points);
static inline void sort_movie_avg(int left, int right);
static inline float partition(int left, int right, int pivot_idx);
static inline void get_movie_avg(data_point * data, unsigned num_points);
static inline void get_similarities(data_point* first, data_point* second);
static inline int get_common_users(data_point * first, data_point * second);
static inline void get_A_hat(data_point * data);

// Perform the KNN collaberative filtering
int main() {

    data_point * data_start, *ptr1, *ptr2;
    unsigned num_points;

    cout << "Loading data..." << endl;
    init_data_io();

    data_start = get_data(TRAIN_MU);
    num_points = get_data_size(TRAIN_MU);

    ptr1 = data_start;

    //cout << "Ranking movies..." << endl;
    //rank_movies(data_start, num_points);

    cout << "Calculating similarities..." << endl;
    // For each pair of movies, get similarities.
    for (int i = 0; i < NUM_MOVIES; i++) {

        cout << "    Movie " << i+1 << endl;
        ptr2 = data_start;

        int retval;
        // get to first movie
        while (ptr1->movie < (i+1)) {
            ptr1++;
        }

        for (int j = 0; j <= i; j++) {

            // get to second movie
            while (ptr2->movie < (j+1)) {
                ptr2++;
            }

            get_similarities(ptr1, ptr2);
        }
    }

    get_A_hat(data_start);
}

static inline void get_A_hat(data_point * data) {

    float total = 0;
    int number = 0;

    for (int i = 0; i < NUM_MOVIES; i++) {

        for (int j = 0; j <= i; j++) {

            total += A[i][j];
            number++;
        }
    }

    total /= (float)number;

    data_point * ptr1 = data;

    for (int i = 0; i < NUM_MOVIES; i++) {

        cout << "    Movie " << i+1 << endl;
        data_point * ptr2 = data;

        // get to first movie
        while (ptr1->movie < (i+1)) {
            ptr1++;
        }

        for (int j = 0; j <= i; j++) {

            // get to second movie
            while (ptr2->movie < (j+1)) {
                ptr2++;
            }

            number = get_common_users(ptr1, ptr2);

            A[i][j] = (number * A[i][j] + BETA*total)/(number + BETA);

        }
    }

}


// This function uses the Spearman correlation coefficient to get the similarities
// between each pair of movies in the dataset.
static inline void get_similarities(data_point* first, data_point* second) {

    // populate the movie_users vector
    int num_users = get_common_users(first, second);
    cout << common_users[0] << endl;
    cout << (int)ratings1[0] << endl;
    cout << (int)ratings2[0] << endl;

    cout << "first " << first->movie << endl;
    cout << "second " << second->movie << endl;

    int i = 0;
    float error1 = 0;
    float error2 = 0;
    float error1sq = 0;
    float error2sq = 0;
    int product = 0;
    float mean1 = 0;
    float mean2 = 0;
    float topproduct = 0;
    // Solve for Pearson correlation coefficient.
    while (common_users[i] != 0) {
        mean1 += ratings1[i];
        mean2 += ratings2[i];
        i++;
    }

    mean1 /= (float)num_users;
    mean2 /= (float)num_users;

    i = 0;
    while (common_users[i] != 0) {
        error1 = (ratings1[i] - mean1);
        error2 = (ratings2[i] - mean2);
        error1sq += error1*error1;
        error2sq += error2*error2;
        topproduct += error1*error2;
        product += ratings1[i]*ratings2[i];
        i++;
    }

    similarities[first->movie - 1][second->movie - 1] = (topproduct / pow(error1sq * error2sq, (float)0.5))*((float)i/(float)i + 0.1);
    A[first->movie - 1][second->movie - 1] = product / (float)i;
    cout << "Similarity: " << similarities[first->movie -1][second->movie - 1] << endl;
}

// This function gets the set of common users that have rated both movies, and
// populates the common_users vector. Takes in pointers to start of movie ratings in
// the dataset.
static inline int get_common_users(data_point * first, data_point * second) {

    int idx = 0;
    data_point* curr_first = first;
    data_point* curr_second = second;
    while (curr_first->movie == first->movie) {

        while (curr_second->movie == second->movie) {

            // if both ratings have the same user, add the
            if (curr_first->user == curr_second->user) {
                common_users[idx] = curr_first->user;
                ratings1[idx] = (int)curr_first->rating;
                cout << "idx " << idx << " is " << ratings1[idx] << endl;
                ratings2[idx] = (int)curr_second->rating;
                idx++;
                break;
            }
            curr_second++;
        }
        // start over and look at the beginning of the second user's data points
        curr_second = second;
        curr_first++;
    }

    int retval = idx;

    cout << "idx" << idx << endl;
    for (; idx < NUM_USERS; idx++) {
        common_users[idx] = 0;
        ratings1[idx] = 0;
        ratings2[idx] = 0;
    }

    cout << "rating : " << ratings1[0] << endl;

    return retval;
}
/*
// After calling this function, ranks should be populated.
static inline void rank_movies(data_point * data, unsigned num_points) {

    for (int i = 0; i < NUM_MOVIES; i++) {

        ranks_tmp[i] = i+1;
    }

    get_movie_avg(data, num_points);
    sort_movie_avg(0, NUM_MOVIES - 1);

    for (int i = 0; i < NUM_MOVIES; i++) {

        ranks[ranks_tmp[i] - 1] = i + 1;
    }

    // swap order so the largest averages have lowest rank
    for (int i = 0; i < NUM_MOVIES; i++) {

        ranks[i] = NUM_MOVIES - ranks[i] + 1;
    }

}


//in-place quicksort rankings (simultaneously taking along movie numbers)
static inline void sort_movie_avg(int left, int right) {

    if (left < right) {
        int pivot_idx = left;
        int new_pivot_idx = partition(left, right, pivot_idx);
        sort_movie_avg(left, new_pivot_idx - 1);
        sort_movie_avg(new_pivot_idx + 1, right);
    }

}

static inline float partition(int left, int right, int pivot_idx) {

    float pivot, save_val;
    float pivot_cpy, save_cpy;
    int save_idx;

    pivot = movie_avg[pivot_idx];
    pivot_cpy = ranks_tmp[pivot_idx];

    movie_avg[pivot_idx] = movie_avg[right];
    ranks_tmp[pivot_idx] = ranks_tmp[right];

    movie_avg[right] = pivot;
    ranks_tmp[right] = pivot_cpy;

    save_idx = left;
    for (int i = left; i < right; i++) {
        if (movie_avg[i] <= pivot) {
            save_val = movie_avg[i];
            save_cpy = ranks_tmp[i];

            movie_avg[i] = movie_avg[save_idx];
            ranks_tmp[i] = ranks_tmp[save_idx];

            movie_avg[save_idx] = save_val;
            ranks_tmp[save_idx] = save_cpy;

            save_idx++;
        }
    }
    save_val = movie_avg[save_idx];
    save_cpy = ranks_tmp[save_idx];

    movie_avg[save_idx] = movie_avg[right];
    ranks_tmp[save_idx] = ranks_tmp[right];

    movie_avg[right] = save_val;
    ranks_tmp[right] = save_cpy;

    return save_idx;
}

// get movie averages for each movie and populate the movie_avg array
static inline void get_movie_avg(data_point * data, unsigned num_points) {

    float total;
    int num_movies;
    movie_type curr_movie = data->movie;

    for (int i = 0; i < NUM_MOVIES; i++) {

        total = 0;
        num_movies = 0;
        while (data->movie == curr_movie) {
            total += data->rating;
            num_movies++;
            data++;
        }
        movie_avg[curr_movie - 1] = total/num_movies;
        curr_movie = data->movie;
    }
}
*/


// Process:
// Solve for similarity values with Spearman coefficient (for each pair of movies)
// Find the nearest neighbors for each movie

// Solve for A bar and b bar (for each pair of movies)
// Find avg, which is just the average value of A bar (look in article for how to improve this)
// Solve for A hat (using values of A bar matrix)
// For every rating we want to predict
    // Solve for b hat (using values from A hat matrix)
    // Solve for weight vector using Figure 1 algorithm (using A hat and b hat)
    // Solve for predictions using weight vector on nearest neighbors (using neighbors)



// Algorithm:
// For each pair of movies:
//      Solve for similarity values
//      Solve for A bar
// For each movie:
//      Find its nearest neighbors
// Find avg
// For each point in qual:
//      Solve for A hat
//      Solve for b hat
//      Solve for weight vector
//      Solve for prediction