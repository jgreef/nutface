//
//
// K Nearest Neighbors
//
//

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include "data_types.h"
#include "data_io.h"

using namespace std;

#define K 30
#define BETA 500
#define EPSILON 0.05

// Constant for filename buffer size
#define FILENAME_BUF_SIZE 512

movie_type neighbors[K];
float similarities[NUM_MOVIES][NUM_MOVIES];
float A[NUM_MOVIES][NUM_MOVIES];

float littleA[K][K];
float B[K];
float weights[K];
float temp[K];
float temp2[K];

user_type common_users[NUM_USERS/4];
rating_type ratings1[NUM_USERS/4];
rating_type ratings2[NUM_USERS/4];

// Function declarations
//static inline void rank_movies(data_point * data, unsigned num_points);
static inline void sort_movie_avg(int left, int right);
static inline float partition(int left, int right, int pivot_idx);
static inline float get_movie_mean(data_point * data);
static inline void get_similarities(data_point* first, data_point* second);
static inline int get_common_users(data_point * first, data_point * second);
static inline void get_A_hat(data_point * data);
static inline void get_user_mean(data_point * data);
static inline void get_neighbors(movie_type movie);
static inline void predict_qual(data_point * data, unsigned num_points);
static inline void build_little_A_and_B(movie_type movie);
static inline void find_weights();
static inline float make_prediction(data_point * data, user_type user);
static inline void gather_ratings(data_point * data, user_type user);
static inline float temp_mag(void);
static inline float calc_alpha(void);
static inline void predict_probe(data_point * data, unsigned num_points);

// Perform the KNN collaberative filtering
int main() {

    data_point * data_start, *qual_start, *probe_start, *ptr1, *ptr2;
    unsigned num_points, qual_points, probe_points;

    cout << "Loading data..." << endl;
    init_data_io();

    data_start = get_data(TRAIN_MU);
    num_points = get_data_size(TRAIN_MU);

    qual_start = get_data(QUAL_MU);
    qual_points = get_data_size(QUAL_MU);

    probe_start = get_data(PROBE_MU);
    probe_points = get_data_size(PROBE_MU);

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

    FILE *f = fopen("similarity_matrix.data", "wb");
    fwrite(similarities, sizeof(float), sizeof(similarities), f);
    fclose(f);

    f = fopen("A_bar.data", "wb");
    fwrite(A, sizeof(float), sizeof(A), f);
    fclose(f);

    cout << "Getting A hat matrix..." << endl;
    get_A_hat(data_start);

    cout << "Predicting probe..." << endl;
    predict_probe(probe_start, probe_points);

    cout << "Predicting qual..." << endl;
    predict_qual(qual_start, qual_points);
}

static inline void get_A_hat(data_point * data) {

    float total = 0;
    int number = 0;

    // get average of entire A matrix
    for (int i = 0; i < NUM_MOVIES; i++) {

        for (int j = 0; j <= i; j++) {

            total += A[i][j];
            number++;
        }
    }

    total /= (float)number;

    data_point * ptr1 = data;

    // Calculate A hat matrix (just replace A bar matrix)
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

static inline float get_movie_mean(data_point * data) {

    int count = 0;
    float total = 0;
    movie_type curr_movie = data->movie;
    while (data->movie == curr_movie) {
        total += data->rating;
        count++;
        data++;
    }

    return total / (float)count;
}


// This function uses the Spearman correlation coefficient to get the similarities
// between each pair of movies in the dataset.
static inline void get_similarities(data_point* first, data_point* second) {

    // populate the movie_users vector
    int num_users = get_common_users(first, second);
    if (num_users == 0) {
        similarities[first->movie - 1][second->movie - 1] = -1.0;
        return;
    }
    float mean1 = get_movie_mean(first);
    float mean2 = get_movie_mean(second);

    float error1 = 0;
    float error2 = 0;
    float error1sq = 0;
    float error2sq = 0;
    float product = 0;
    float topproduct = 0;

    for (int i = 0; i < num_users; i++) {
        error1 = (ratings1[i] - mean1);
        error2 = (ratings2[i] - mean2);
        error1sq += error1*error1;
        error2sq += error2*error2;
        topproduct += error1*error2;
        product += error1 * error2;
    }

    if (num_users < 50) {
        similarities[first->movie - 1][second->movie - 1] = (topproduct / pow(error1sq * error2sq, (float)0.5)) * (num_users/(float)50);
    }
    else {
        similarities[first->movie - 1][second->movie - 1] = (topproduct / pow(error1sq * error2sq, (float)0.5));
    }
    A[first->movie - 1][second->movie - 1] = product / (float)num_users;
    if ((similarities[first->movie -1][second->movie - 1] < -1) || (similarities[first->movie -1][second->movie - 1] > 1)) {
        cout << "Similarity: " << similarities[first->movie -1][second->movie - 1] << endl;
    }
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
                ratings1[idx] = curr_first->rating;
                ratings2[idx] = curr_second->rating;
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

    for (; idx < NUM_USERS/4; idx++) {
        common_users[idx] = 0;
        ratings1[idx] = 0;
        ratings2[idx] = 0;
    }

    return retval;
}


static inline void predict_qual(data_point * data, unsigned num_points) {

    time_t timestamp;
    struct tm * curr_time;
    char filename[FILENAME_BUF_SIZE];

    // Get the time
    timestamp = time(0);
    // Convert the time to a string
    curr_time = localtime(&timestamp);

    snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/knn_qual_%d_%d_%dh%dm_.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min);

    ofstream qual_output (filename, ios::trunc);


    for (int i = 0; i < num_points; i++) {

        movie_type curr = data->movie;
        get_neighbors(data->movie);
        build_little_A_and_B(data->user);
        find_weights();
        float prediction = make_prediction(data, data->user);

        while (data->movie == curr) {
            data++;
        }

        float ret_val = (prediction > 5) ? 5 : prediction;
        ret_val = (ret_val < 1) ? 1 : ret_val;

        // Write it to the file
        qual_output << ret_val << endl;
    }
}

static inline void predict_probe(data_point * data, unsigned num_points) {

    time_t timestamp;
    struct tm * curr_time;
    char filename[FILENAME_BUF_SIZE];

    // Get the time
    timestamp = time(0);
    // Convert the time to a string
    curr_time = localtime(&timestamp);

    snprintf(filename, FILENAME_BUF_SIZE, "../../../data/solutions/knn_probe_%d_%d_%dh%dm_.out", curr_time->tm_mon, curr_time->tm_mday, curr_time->tm_hour, curr_time->tm_min);

    ofstream qual_output (filename, ios::trunc);


    for (int i = 0; i < num_points; i++) {

        movie_type curr = data->movie;
        get_neighbors(data->movie);
        build_little_A_and_B(data->user);
        find_weights();
        float prediction = make_prediction(data, data->user);

        while (data->movie == curr) {
            data++;
        }

        float ret_val = (prediction > 5) ? 5 : prediction;
        ret_val = (ret_val < 1) ? 1 : ret_val;

        cout << ret_val << endl;

        // Write it to the file
        qual_output << ret_val << endl;
    }
}

static inline void get_neighbors(movie_type movie) {

    float minimum = 0;
    int min_idx = 0;

    for (int i = 0; i < NUM_MOVIES; i++) {

        if ((similarities[movie-1][i] != 0) && (movie != i+1)) {

            if (similarities[movie-1][i] > minimum) {
                neighbors[min_idx] = i;
            }

            for (int j = 0; j < K; j++) {
                if (similarities[movie - 1][j] < minimum) {
                    minimum = similarities[movie - 1][j];
                    min_idx = j;
                }
            }

        }
    }

    for (int i = 0; i < NUM_MOVIES; i++) {

        if ((similarities[i][movie-1] != 0) && (movie != i+1)) {

            if (similarities[i][movie-1] > minimum) {
                neighbors[min_idx] = i;
            }

            for (int j = 0; j < K; j++) {
                if (similarities[j][movie - 1] < minimum) {
                    minimum = similarities[j][movie - 1];
                    min_idx = j;
                }
            }

        }
    }
}

static inline void build_little_A_and_B(movie_type movie) {

    for (int i = 0; i < K; i++) {

        for (int j = 0; j < K; j++) {

            littleA[i][j] = A[neighbors[i] - 1][neighbors[j] - 1];
        }

        B[i] = A[movie - 1][neighbors[i] - 1];
    }

}

static inline void find_weights(void) {

    float alpha;

    do {

        for (int i = 0; i < K; i++) {

            float product = 0;

            for (int j = 0; j < K; j++) {

                product += A[i][j] * weights[j];
            }

            temp[i] = product - B[i];
        }

        for (int i = 0; i < K; i++) {

            if ((weights[i] == 0) && temp[i] < 0) {

                temp[i] = 0;
            }
        }

        alpha = calc_alpha();

        for (int i = 0; i < K; i++) {
            if (temp[i] < 0) {
                alpha = min(alpha, -1*(weights[i]/temp[i]));
            }
        }

        for (int i = 0; i < K; i++) {

            weights[i] += alpha * temp[i];
        }

    } while (temp_mag() > EPSILON);

}

static inline float temp_mag(void) {

    float total = 0;

    for (int i = 0; i < K; i++) {

        total += temp[i]*temp[i];
    }

    return total / (float)K;
}

static inline float calc_alpha(void) {

    float rTr = 0;
    float rTAr = 0;

    for (int i = 0; i < K; i++) {

        for (int j = 0; j < K; j++) {

            temp2[i] = temp[i] * A[i][j];
        }
        rTr += temp[i]*temp[i];
    }

    for (int i = 0; i < K; i++) {

        rTAr += temp2[i] * temp[i];
    }

    return rTr / rTAr;
}

static inline float make_prediction(data_point * data, user_type user) {

    float predict = 0;
    gather_ratings(data, user);

    for (int i = 0; i < K; i++) {

        // temp is now the user's ratings on the neighboring movies
        predict += weights[i] * temp[i];
    }

    return predict;
}

static inline void gather_ratings(data_point * data, user_type user) {

    for (int i = 0; i < K; i++) {

        while (data->user != user) {
            data++;
        }

        if (data->user == user) {
            temp[i] = data->rating;
        }
        else {
            cout << "Uh oh I don't know how to deal with this issue" << endl;
        }
    }

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