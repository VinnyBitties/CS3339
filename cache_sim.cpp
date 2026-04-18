#include<iostream>
#include<fstream>
#include<cstdlib>

using namespace std;

class Entry {
public:
  Entry();
  ~Entry();
  void display(ofstream& outfile);

  void set_tag(int _tag) { tag = _tag; }
  int get_tag() { return tag; }

  void set_valid(bool _valid) { valid = _valid; }
  bool get_valid() { return valid; }

  void set_ref(int _ref) { ref = _ref; }
  int get_ref() { return ref; }

private:
  bool valid;
  unsigned tag;
  int ref;
};

class Cache {
public:
  Cache(int, int);
  ~Cache();

  void display(ofstream& outfile);

  int get_index(unsigned long addr);
  int get_tag(unsigned long addr);

  unsigned long retrieve_addr(int way, int index);

  bool hit(ofstream& outfile, unsigned long addr);

  void update(ofstream& outfile, unsigned long addr);

private:
  int assoc;
  unsigned num_entries;
  int num_sets;
  Entry **entries;



  int *replacement_index;
};





Entry::Entry() {
  valid = false;
  tag = 0;
  ref = 0;
}

Entry::~Entry() {
}

//Helper func for debugging-> cahce contents
void Entry::display(ofstream& outfile) {
  outfile << "Valid: " << valid
          << " Tag: " << tag
          << " Ref: " << ref << endl;
}




// Build the cache as a 2D dynamic array.
//
// We are storing the cache as:
// entries[way][set]
//
// - first index tells us which line/way inside the set
// - second index tells us which set
Cache::Cache(int _num_entries, int _assoc) {
  num_entries = _num_entries;
  assoc = _assoc;
  num_sets = num_entries / assoc;

  entries = new Entry*[assoc];
  for (int i = 0; i < assoc; i++) {
    entries[i] = new Entry[num_sets];
  }

  replacement_index = new int[num_sets];
  for (int i = 0; i < num_sets; i++) {
    replacement_index[i] = 0;
  }
}

Cache::~Cache() {
  for (int i = 0; i < assoc; i++) {
    delete[] entries[i];
  }
  delete[] entries;

  delete[] replacement_index;
}

void Cache::display(ofstream& outfile) {
  for (int set = 0; set < num_sets; set++) {
    outfile << "Set " << set << ":" << endl;
    for (int way = 0; way < assoc; way++) {
      outfile << "  Way " << way << " -> ";
      entries[way][set].display(outfile);
    }
  }
}

// Finds which set the address belongs to.
int Cache::get_index(unsigned long addr) {
  return addr % num_sets;
}

// Finds the tag for the address.
// After removing the set portion, the remaining upper part is the tag.
int Cache::get_tag(unsigned long addr) {
  return addr / num_sets;
}

// Rebuilds an address from a stored cache entry.
unsigned long Cache::retrieve_addr(int way, int index) {
  return entries[way][index].get_tag() * num_sets + index;
}

// Checks whether the given address is already stored in the correct set.
// If found, it is a HIT. Otherwise, it is a MISS.
bool Cache::hit(ofstream& outfile, unsigned long addr) {
  int index = get_index(addr);
  int tag = get_tag(addr);

  // Search every way in the correct set
  for (int way = 0; way < assoc; way++) {
    if (entries[way][index].get_valid() &&
        entries[way][index].get_tag() == tag) {
      outfile << addr << " : HIT" << endl;
      return true;
    }
  }

  outfile << addr << " : MISS" << endl;
  return false;
}

// Miss function
// It adds the new address into the cache.
//
// Step 1: Try to place it in an empty line in that set.
// Step 2: If the set is already full, replace one line using round-robin.
void Cache::update(ofstream& outfile, unsigned long addr) {
  int index = get_index(addr);
  int tag = get_tag(addr);

  // First try to find an unused line in this set
  for (int way = 0; way < assoc; way++) {
    if (!entries[way][index].get_valid()) {
      entries[way][index].set_valid(true);
      entries[way][index].set_tag(tag);
      return;
    }
  }

  // If set is full.
  // Replace the line pointed to by replacement_index[index].
  int victim = replacement_index[index];

  entries[victim][index].set_valid(true);
  entries[victim][index].set_tag(tag);

  // Move to the next line in this set for future replacement
  replacement_index[index] = (replacement_index[index] + 1) % assoc;
}



int main(int argc, char* argv[]) {


  if (argc < 4) {
    cout << "Usage: " << endl;
    cout << "   ./cache_sim num_entries associativity input_file" << endl;
    return 0;
  }


  unsigned entries = atoi(argv[1]);
  unsigned assoc = atoi(argv[2]);
  string input_filename = argv[3];


  if (entries == 0 || assoc == 0 || entries % assoc != 0) {
    cerr << "Invalid cache configuration." << endl;
    return 1;
  }

  // Open input file
  ifstream input;
  input.open(input_filename.c_str());

  if (!input.is_open()) {
    cerr << "Could not open input file " << input_filename << "." << endl;
    return 1;
  }

  // Open output file
  ofstream output;
  output.open("cache_sim_output");

  if (!output.is_open()) {
    cerr << "Could not create output file." << endl;
    return 1;
  }

  // Build cache using command-line configuration
  Cache cache(entries, assoc);

  unsigned long addr;

  // Read one memory reference at a time from the input file
  while (input >> addr) {

    // First check whether this address is already in cache
    bool found = cache.hit(output, addr);

    // If it was not found, insert it into cache
    if (!found) {
      cache.update(output, addr);
    }
  }

  input.close();
  output.close();

  return 0;
}
