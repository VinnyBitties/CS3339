#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// A cache line represents one spot in the cache.
// valid = tells if something has been stored there yet
// tag = used to identify which memory address is stored
struct CacheLine {
    bool valid;
    int tag;
};

int main(int argc, char* argv[]) {

    // The program should be run like this:
    // ./cache_sim <num_entries> <associativity> <input_file>
    // Example:
    // ./cache_sim 4 2 input0.txt
    if (argc != 4) {
        cerr << "Usage: ./cache_sim <num_entries> <associativity> <input_file>" << endl;
        return 1;
    }

    // Convert command line inputs into usable values
    int num_entries = stoi(argv[1]);       // total number of cache lines
    int associativity = stoi(argv[2]);     // lines per set
    string input_file_name = argv[3];      // file containing memory addresses

    // Make sure values are valid
    if (num_entries <= 0 || associativity <= 0 ||
        num_entries % associativity != 0) {
        cerr << "Invalid cache configuration." << endl;
        return 1;
    }

    // Number of sets = total lines divided by lines per set
    int num_sets = num_entries / associativity;

    // Open the input file that contains addresses
    ifstream input_file(input_file_name);

    if (!input_file.is_open()) {
        cerr << "Could not open input file." << endl;
        return 1;
    }

    // Create the required output file
    ofstream output_file("cache_sim_output");

    if (!output_file.is_open()) {
        cerr << "Could not create output file." << endl;
        return 1;
    }

    // Build the cache as a 2D vector:
    // first index = set number
    // second index = line inside that set
    // Start with everything empty (valid = false)
    vector<vector<CacheLine>> cache(
        num_sets,
        vector<CacheLine>(associativity, {false, -1})
    );

    // Keeps track of which line to replace next
    // for each set when the set becomes full
    vector<int> replacement_index(num_sets, 0);

    int address;

    // Read one memory address at a time from input file
    while (input_file >> address) {

        // Find which set this address belongs to
        // Example: if num_sets = 2, addresses alternate between set 0 and set 1
        int set_index = address % num_sets;

        // Find the tag value for this address
        // The tag is what we compare to know if the address is already cached
        int tag = address / num_sets;

        bool hit = false;

        // Search every line inside the correct set
        for (int i = 0; i < associativity; i++) {

            // If the line is being used AND the tags match,
            // then the address is already in cache
            if (cache[set_index][i].valid &&
                cache[set_index][i].tag == tag) {

                hit = true;
                break;
            }
        }

        // If address was found in cache
        if (hit) {
            output_file << address << " : HIT" << endl;
        }
        else {
            // Address was not found in cache
            output_file << address << " : MISS" << endl;

            bool inserted = false;

            // First check if there is an empty spot in this set
            for (int i = 0; i < associativity; i++) {

                if (!cache[set_index][i].valid) {

                    // Store the new address here
                    cache[set_index][i].valid = true;
                    cache[set_index][i].tag = tag;

                    inserted = true;
                    break;
                }
            }

            // If no empty space exists, the set is full
            // We must replace one old line
            if (!inserted) {

                int victim = replacement_index[set_index];

                // Overwrite that old line with new tag
                cache[set_index][victim].valid = true;
                cache[set_index][victim].tag = tag;

                // Move to next line for future replacement
                // This creates a round-robin replacement pattern
                replacement_index[set_index] =
                    (replacement_index[set_index] + 1) % associativity;
            }
        }
    }

    // Close files when finished
    input_file.close();
    output_file.close();

    return 0;
}