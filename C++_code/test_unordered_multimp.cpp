// This file is a test of the C++ unordered multimap

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>

using namespace std;

struct data_point
{	
	unsigned user;
	unsigned short movie;
	unsigned short timestamp;
	unsigned short rating;
};

struct movie_data
{
	unsigned short movie;
	unsigned short timestamp;
	unsigned short rating;
};

struct user_data
{
	unsigned user;
	unsigned short timestamp;
	unsigned short rating;
};

enum data_type
{
	TRAIN, 
	VALID, 
	HIDDEN, 
	PROBE, 
	QUAL
}

data_point* get_data_from_line(string line);
data_type get_type_from_line(string line);

int main()
{

	string line;
	data_point* data;
	data_type type;
	user_data udata;
	int i = 0;

	cout << sizeof(unsigned short) << endl;
	//
	// This stuff was all for testing multimaps, 
	//	now want to try to load the input file
	//

	unordered_multimap<unsigned, user_data> mu_dbase;
	// unordered_multimap<unsigned, movie_data> um_dbase;


	// string_map.emplace(234,567);
	// string_map.emplace(234,789);
	// string_map.emplace(127,203);

	// auto range = string_map.equal_range(127);

	// // for (auto i = range.first; i != range.second; i++)
	// for (auto i = string_map.begin(); i != string_map.end(); i++)
	// {
	 //    	std::cout << i->first << ": " << i->second << std::endl;
	 //    }

    // std::cout << sizeof(int) << std::endl;

    ifstream mu_data ("../../../data/mu/all.dta");
    ifstream mu_idx  ("../../../data/mu/all.idx");

    if (mu_data.is_open())
    {
	    while(getline(mu_data, data_line) && getline(mu_idx, idx_line))
	    {
	    	cout << i << endl;
	    	i++;
	    	data = get_data_from_line(data_line);
	    	type = get_type_from_line(idx_line);

	    	switch(type)
	    	{
	    		case TRAIN:

	    		case VALID:

	    		case HIDDEN:

	    		case PROBE:

	    		case QUAL:

	    	}

	    	udata.user = data->user;
	    	udata.timestamp = data->timestamp;
	    	udata.rating = data->rating;
	    	mu_dbase.emplace(data->movie, udata);
	    }

	    mu_data.close();
	}

	// Now read out the database to make sure it's being stored properly
	// for (auto x : mu_dbase)
	// {
	// 	cout << x.first << ";" << x.second.user << ";" << x.second.timestamp << ";" << x.second.rating << endl;
	// }

    return 0;
}

// This function puts the line data into a struct and 
//	returns the pointer to the struct
data_point* get_data_from_line(string line)
{
	int idx = 0;
	static data_point data;

	// Initialize the data point
	memset(&data, 0, sizeof(data_point));

	// Get the user number
	while(line[idx] != ' ')
	{
		data.user *= 10;
		data.user += (line[idx] - '0');
		idx++;
	}
	idx++;

	// Get the movie number
	while(line[idx] != ' ')
	{
		data.movie *= 10;
		data.movie += (line[idx] - '0');
		idx++;
	}
	idx++;

	// Get the time number
	while(line[idx] != ' ')
	{
		data.timestamp *= 10;
		data.timestamp += (line[idx] - '0'); 
		idx++;
	}
	idx++;

	// And finally get the rating
	data.rating = (line[idx] - '0');

	return &data;

}

// This function returns an enum which represents the type of 
//	data the point is: train, valid, hidden, probe or quiz
data_type get_type_from_line(string line)
{
	return (line[0] - '0');
}










