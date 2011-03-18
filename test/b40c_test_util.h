/******************************************************************************
 * Copyright 2010 Duane Merrill
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 * 
 * For more information, see our Google Code project site: 
 * http://code.google.com/p/back40computing/
 * 
 * Thanks!
 ******************************************************************************/

#pragma once

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <map>
#include <string>
#include <sstream>
#include <iostream>

namespace b40c {


/******************************************************************************
 * Command-line parsing
 ******************************************************************************/

class CommandLineArgs
{
protected:

	std::map<std::string, std::string> pairs;

public:

	// Constructor
	CommandLineArgs(int argc, char **argv)
	{
		using namespace std;

	    for (int i = 1; i < argc; i++)
	    {
	        string arg = argv[i];

	        if ((arg[0] != '-') || (arg[1] != '-')) {
	        	continue;
	        }

        	string::size_type pos;
		    string key, val;
	        if ((pos = arg.find( '=')) == string::npos) {
	        	key = string(arg, 2, arg.length() - 2);
	        	val = "";
	        } else {
	        	key = string(arg, 2, pos - 2);
	        	val = string(arg, pos + 1, arg.length() - 1);
	        }
        	pairs[key] = val;
	    }
	}

	bool CheckCmdLineFlag(const char* arg_name)
	{
		using namespace std;
		map<string, string>::iterator itr;
		if ((itr = pairs.find(arg_name)) != pairs.end()) {
			return true;
	    }
		return false;
	}

	template <typename T>
	void GetCmdLineArgument(const char *arg_name, T &val);

};

template <typename T>
void CommandLineArgs::GetCmdLineArgument(const char *arg_name, T &val)
{
	using namespace std;
	map<string, string>::iterator itr;
	if ((itr = pairs.find(arg_name)) != pairs.end()) {
		istringstream strstream(itr->second);
		strstream >> val;
    }
}

template <>
void CommandLineArgs::GetCmdLineArgument<char*>(const char* arg_name, char* &val)
{
	using namespace std;
	map<string, string>::iterator itr;
	if ((itr = pairs.find(arg_name)) != pairs.end()) {

		string s = itr->second;
		val = (char*) malloc(sizeof(char) * (s.length() + 1));
		strcpy(val, s.c_str());

	} else {
    	val = NULL;
	}
}





/******************************************************************************
 * Device initialization
 ******************************************************************************/

void DeviceInit(CommandLineArgs &args)
{
	int deviceCount;
	cudaGetDeviceCount(&deviceCount);
	if (deviceCount == 0) {
		fprintf(stderr, "No devices supporting CUDA.\n");
		exit(1);
	}
	int dev = 0;
	args.GetCmdLineArgument("device", dev);
	if (dev < 0) {
		dev = 0;
	}
	if (dev > deviceCount - 1) {
		dev = deviceCount - 1;
	}
	cudaDeviceProp deviceProp;
	cudaGetDeviceProperties(&deviceProp, dev);
	if (deviceProp.major < 1) {
		fprintf(stderr, "Device does not support CUDA.\n");
		exit(1);
	}
    if (!args.CheckCmdLineFlag("quiet")) {
        printf("Using device %d: %s\n", dev, deviceProp.name);
    }

	cudaSetDevice(dev);
}




/******************************************************************************
 * Templated routines for printing keys/values to the console 
 ******************************************************************************/

template<typename T> 
void PrintValue(T val) {
	printf("%d", val);
}

template<>
void PrintValue<float>(float val) {
	printf("%f", val);
}

template<>
void PrintValue<double>(double val) {
	printf("%f", val);
}

template<>
void PrintValue<unsigned char>(unsigned char val) {
	printf("%u", val);
}

template<>
void PrintValue<unsigned short>(unsigned short val) {
	printf("%u", val);
}

template<>
void PrintValue<unsigned int>(unsigned int val) {
	printf("%u", val);
}

template<>
void PrintValue<long>(long val) {
	printf("%ld", val);
}

template<>
void PrintValue<unsigned long>(unsigned long val) {
	printf("%lu", val);
}

template<>
void PrintValue<long long>(long long val) {
	printf("%lld", val);
}

template<>
void PrintValue<unsigned long long>(unsigned long long val) {
	printf("%llu", val);
}



/******************************************************************************
 * Helper routines for list construction and validation 
 ******************************************************************************/


/**
 * Generates random 32-bit keys.
 * 
 * We always take the second-order byte from rand() because the higher-order 
 * bits returned by rand() are commonly considered more uniformly distributed
 * than the lower-order bits.
 * 
 * We can decrease the entropy level of keys by adopting the technique 
 * of Thearling and Smith in which keys are computed from the bitwise AND of 
 * multiple random samples: 
 * 
 * entropy_reduction	| Effectively-unique bits per key
 * -----------------------------------------------------
 * -1					| 0
 * 0					| 32
 * 1					| 25.95
 * 2					| 17.41
 * 3					| 10.78
 * 4					| 6.42
 * ...					| ...
 * 
 */
template <typename K>
void RandomBits(K &key, int entropy_reduction = 0, int lower_key_bits = sizeof(K) * 8)
{
	const unsigned int NUM_UCHARS = (sizeof(K) + sizeof(unsigned char) - 1) / sizeof(unsigned char);
	unsigned char key_bits[NUM_UCHARS];
	
	do {
	
		for (int j = 0; j < NUM_UCHARS; j++) {
			unsigned char quarterword = 0xff;
			for (int i = 0; i <= entropy_reduction; i++) {
				quarterword &= (rand() >> 7);
			}
			key_bits[j] = quarterword;
		}
		
		if (lower_key_bits < sizeof(K) * 8) {
			unsigned long long base = 0;
			memcpy(&base, key_bits, sizeof(K));
			base &= (1 << lower_key_bits) - 1;
			memcpy(key_bits, &base, sizeof(K));
		}
		
		memcpy(&key, key_bits, sizeof(K));
		
	} while (key != key);		// avoids NaNs when generating random floating point numbers 
}


/**
 * Compares the equivalence of two arrays
 */
template <typename T>
int CompareResults(T* computed, T* reference, size_t len, bool verbose = true)
{
	for (size_t i = 0; i < len; i++) {

		if (computed[i] != reference[i]) {
			printf("Incorrect: [%lu]: ", (unsigned long) i);
			PrintValue<T>(computed[i]);
			printf(" != ");
			PrintValue<T>(reference[i]);

			if (verbose) {
				printf("\nresult[...");
				for (size_t j = (i >= 5) ? i - 5 : 0; (j < i + 5) && (j < len); j++) {
					PrintValue<T>(computed[j]);
					printf(", ");
				}
				printf("...]");
				printf("\nreference[...");
				for (size_t j = (i >= 5) ? i - 5 : 0; (j < i + 5) && (j < len); j++) {
					PrintValue<T>(reference[j]);
					printf(", ");
				}
				printf("...]");
			}

			return 1;
		}
	}

	printf("Correct");
	return 0;
}


/**
 * Verify the contents of a device array match those
 * of a host array
 */
template <typename T>
int CompareDeviceResults(
	T *h_reference,
	T *d_data,
	size_t num_elements,
	bool verbose = true,
	bool display_data = false)
{
	// Allocate array on host
	T *h_data = (T*) malloc(num_elements * sizeof(T));

	// Reduction data back
	cudaMemcpy(h_data, d_data, sizeof(T) * num_elements, cudaMemcpyDeviceToHost);

	// Display data
	if (display_data) {
		printf("Reference:\n");
		for (int i = 0; i < num_elements; i++) {
			PrintValue(h_reference[i]);
			printf(", ");
		}
		printf("\n\nData:\n");
		for (int i = 0; i < num_elements; i++) {
			PrintValue(h_data[i]);
			printf(", ");
		}
		printf("\n\n");
	}

	// Check
	int retval = CompareResults(h_data, h_reference, num_elements, verbose);

	// Cleanup
	if (h_data) free(h_data);

	return retval;
}


/**
 * Verify the contents of a device array match those
 * of a host array
 */
template <typename T>
void DisplayDeviceResults(
	T *d_data,
	size_t num_elements)
{
	// Allocate array on host
	T *h_data = (T*) malloc(num_elements * sizeof(T));

	// Reduction data back
	cudaMemcpy(h_data, d_data, sizeof(T) * num_elements, cudaMemcpyDeviceToHost);

	// Display data
	if (display_data) {
		printf("\n\nData:\n");
		for (int i = 0; i < num_elements; i++) {
			PrintValue(h_data[i]);
			printf(", ");
		}
		printf("\n\n");
	}

	// Cleanup
	if (h_data) free(h_data);
}


}// namespace b40c