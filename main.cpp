#include <bitset>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Reinterpret float as a 32-bit unsigned integer using memory copy
static uint32_t FloatToBits(float x) {
  uint32_t u = 0;
  memcpy(&u, &x, sizeof(float));
  return u;
}

// Reinterpret 32-bit unsigned integer back into a float
static float BitsToFloat(uint32_t u) {
  float x = 0.0f;
  memcpy(&x, &u, sizeof(float));
  return x;
}

// Displays the bits in IEEE 754 format: [Sign] [Exponent] [Fraction]
static void PrintIeeeFormatted(uint32_t bits) {
  bitset<32> b(bits);

  // Sign bit (bit 31)
  cout << b[31] << " ";

  // Exponent bits (30 to 23)
  for (int i = 30; i >= 23; --i) cout << b[i];
  cout << " ";

  // Fraction bits (22 to 0)
  for (int i = 22; i >= 0; --i) cout << b[i];
}

// Extracts the raw 8-bit biased exponent from the bit pattern
static int GetBiasedExponent(uint32_t bits) {
  return static_cast<int>((bits >> 23) & 0xFFu);
}

// Converts biased exponent to unbiased (real) power of 2
static int GetUnbiasedExponentFromBits(uint32_t bits) {
  int exp = GetBiasedExponent(bits);

  // Handle special cases: subnormals (0) and Inf/NaN (255)
  if (exp == 0) return -126;
  if (exp == 255) return 128; 
  return exp - 127; // Standard bias for float is 127
}

int main(int argc, char* argv[]) {
  // Validate command line arguments
  if (argc != 3) {
    cout << "usage: ./fp_overflow_checker loop_bound loop_counter\n\n";
    cout << "\tloop_bound is a positive floating-point value\n";
    cout << "\tloop_counter is a positive floating-point value\n";
    return 0;
  }

  float loop_bound = 0.0f;
  float loop_counter = 0.0f;

  // Parse input strings to float
  try {
    loop_bound = static_cast<float>(stof(string(argv[1])));
    loop_counter = static_cast<float>(stof(string(argv[2])));
  } catch (...) {
    cout << "Invalid input. Please provide numeric values.\n";
    return 0;
  }

  uint32_t bound_bits = FloatToBits(loop_bound);
  uint32_t counter_bits = FloatToBits(loop_counter);

  // Print binary representations for debugging/grading
  cout << "Loop bound:   ";
  PrintIeeeFormatted(bound_bits);
  cout << "\n";

  cout << "Loop counter: ";
  PrintIeeeFormatted(counter_bits);
  cout << "\n\n";

  int eB = GetUnbiasedExponentFromBits(bound_bits);
  int eI = GetUnbiasedExponentFromBits(counter_bits);

  // Rule: If the difference in exponents exceeds the precision (24 bits), 
  // adding the smaller number to the larger results in no change (overflow/loss of significance).
  if (eB - eI >= 24) {
    cout << "Warning: Possible overflow!\n";
    cout << "Overflow threshold:\n";

    // Calculate the point where the counter becomes too small relative to the bound
    int threshold_unbiased = eI + 24;
    int threshold_biased = threshold_unbiased + 127;

    // Construct the bit pattern for the threshold value (sign 0, empty fraction)
    uint32_t threshold_bits = (static_cast<uint32_t>(threshold_biased) & 0xFFu) << 23;
    float threshold_val = BitsToFloat(threshold_bits);

    cout << "    " << scientific << setprecision(5) << threshold_val << "\n";
    cout << "    ";
    PrintIeeeFormatted(threshold_bits);
    cout << "\n";
  } else {
    cout << "There is no overflow!\n";
  }

  return 0;
}