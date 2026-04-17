#include<iostream>
#include<fstream>
#include<string>

using namespace std;

// One cache entry represents one line in cache memory
class Entry {
public:
    Entry() {
        // Start with an empty line
        valid = false;
        tag = 0;
        ref = 0;
    }

    ~Entry() {
    }

    // Print entry contents if needed
    void display(ofstream& outfile) {
        outfile << valid << " " << tag << " " << ref;
    }

    void set_tag(int _tag) { tag = _tag; }
    int get_tag() { return tag; }

    void set_valid(bool _valid) { valid = _valid; }
    bool get_valid() { return valid; }

    void set_ref(int _ref) { ref = _ref; }
    int get_ref() { return ref; }

private:
    bool valid;     // tells if this line contains data
    unsigned tag;   // used to identify stored address
    int ref;        // used for replacement tracking
};

// Cache class controls all cache operations
class Cache {
public:
    Cache(int _num_entries, int _assoc) {
        assoc = _assoc;
        num_entries = _num_entries;

        // Number of sets = total lines / lines per set
        num_sets = num_entries / assoc;

        // Build 2D dynamic array
        // rows = ways
        // columns = sets
        entries = new Entry*[assoc];

        for (int i = 0; i < assoc; i++) {
            entries[i] = new Entry[num_sets];
        }
    }

    ~Cache() {
        // Free memory when program ends
        for (int i = 0; i < assoc; i++) {
            delete[] entries[i];
        }

        delete[] entries;
    }

    // Optional function to print cache contents
    void display(ofstream& outfile) {
        for (int i = 0; i < assoc; i++) {
            for (int j = 0; j < num_sets; j++) {
                outfile << "Way " << i
                        << ", Set " << j << ": ";
                entries[i][j].display(outfile);
                outfile << endl;
            }
        }
    }

    // Finds which set an address belongs to
    int get_index(unsigned long addr) {
        return addr % num_sets;
    }

    // Finds the tag for an address
    int get_tag(unsigned long addr) {
        return addr / num_sets;
    }

    // Rebuild original address from tag + set index
    unsigned long retrieve_addr(int way, int index) {
        return (entries[way][index].get_tag() * num_sets) + index;
    }

    // Checks if address already exists in cache
    bool hit(ofstream& outfile, unsigned long addr) {
        int index = get_index(addr);
        int tag_val = get_tag(addr);

        // Search every way in this set
        for (int i = 0; i < assoc; i++) {

            // Valid line + matching tag = HIT
            if (entries[i][index].get_valid() &&
                entries[i][index].get_tag() == tag_val) {

                outfile << addr << " : HIT" << endl;
                return true;
            }
        }

        // Address not found
        outfile << addr << " : MISS" << endl;
        return false;
    }

    // Adds new address after a miss
    void update(ofstream& outfile, unsigned long addr) {
        int index = get_index(addr);
        int tag_val = get_tag(addr);

        // First look for an empty line
        for (int i = 0; i < assoc; i++) {

            if (!entries[i][index].get_valid()) {

                // Store new block here
                entries[i][index].set_valid(true);
                entries[i][index].set_tag(tag_val);
                entries[i][index].set_ref(0);
                return;
            }
        }

        // If no empty space exists,
        // replace the line with lowest ref value
        int victim = 0;
        int lowest_ref = entries[0][index].get_ref();

        for (int i = 1; i < assoc; i++) {

            if (entries[i][index].get_ref() < lowest_ref) {
                lowest_ref = entries[i][index].get_ref();
                victim = i;
            }
        }

        // Replace chosen line
        entries[victim][index].set_valid(true);
        entries[victim][index].set_tag(tag_val);
        entries[victim][index].set_ref(0);

        // Increase age of all other lines
        for (int i = 0; i < assoc; i++) {
            if (i != victim) {
                entries[i][index].set_ref(
                    entries[i][index].get_ref() + 1
                );
            }
        }
    }

private:
    int assoc;              // lines per set
    unsigned num_entries;  // total lines
    int num_sets;          // total sets
    Entry **entries;       // 2D cache array
};

int main(int argc, char* argv[]) {

    // Program must be run like:
    // ./cache_sim 4 2 input0.txt
    if (argc != 4) {
        cerr << "Usage: ./cache_sim <num_entries> "
             << "<associativity> <input_file>" << endl;
        return 1;
    }

    // Read command line values
    int num_entries = stoi(argv[1]);
    int associativity = stoi(argv[2]);
    string input_file_name = argv[3];

    // Make sure cache setup is valid
    if (num_entries <= 0 ||
        associativity <= 0 ||
        num_entries % associativity != 0) {

        cerr << "Invalid cache configuration." << endl;
        return 1;
    }

    // Open file containing memory addresses
    ifstream input_file(input_file_name);

    if (!input_file.is_open()) {
        cerr << "Could not open input file." << endl;
        return 1;
    }

    // Create output file
    ofstream output_file("cache_sim_output");

    if (!output_file.is_open()) {
        cerr << "Could not create output file." << endl;
        return 1;
    }

    // Build cache object
    Cache cache(num_entries, associativity);

    unsigned long address;

    // Read one address at a time
    while (input_file >> address) {

        // If not found, place it into cache
        if (!cache.hit(output_file, address)) {
            cache.update(output_file, address);
        }
    }

    input_file.close();
    output_file.close();

    return 0;
}
