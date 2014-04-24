//
// This is the public header that defines the I/O routines for 
//	processing data
//

#ifndef DATA_IO_H
#define DATA_IO_H

#include "data_types.h"

// Function declarations
void init_data_io();
data_point* get_data(data_set_t dtype);
void free_data(data_set_t dtype);
unsigned get_data_size(data_set_t dtype);

#endif