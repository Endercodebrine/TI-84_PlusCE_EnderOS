#include<stdint.h>
#include<math.h>
#include<stdbool.h>

//Reads the value at the specified index in the passed data
//template<typename T>
bool read_bit(uint64_t value, uint8_t index){
	return (value >> index) & 1;
}

//void DEBUG_print_out(uint64_t value){
	//printf("Value: ");

	//for (int8_t i = 63;  i >= 0; i--){
		//printf("%d", read_bit(value, i));
	//}

	//printf("\n");
//}

typedef uint64_t BCD;

constexpr uint8_t nibble = 4;
constexpr uint8_t byte   = 8;

static constexpr uint8_t BCD_figure_offset      = 16;
static constexpr uint8_t BCD_digit_count_offset = 12;
static constexpr uint8_t BCD_exponent_offset    = 4;

static constexpr uint8_t BCD_max_digit_count = (64 - BCD_figure_offset) / nibble;

uint64_t RO_BCD_max_int = -1;

void BCD_constants_init(){
	RO_BCD_max_int = pow(10, BCD_max_digit_count);
}

// 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000   0000   0000 0000     00        00
// |----------------------------\_____/----------------------|   \__/   \_______/     \/        \/
//                               Value                           Digit  Exponents   Useless    Signs
//                                                               Count

// The value is stored in nibbles, such that each nibble is one digit.
// For example:
// 1001 1000 0111 0110 0101 0100 0011 0010 0001
// equals
// 987654321
// 
// Probably

// The digit count is a count of the digits, stored as a regular integer, albiet four bits
// For example, if the value was 10 or 11 or any other two-digit number, the digit count would be
// 0010
// or 2
// And a 12 digit number would be
// 1100
// which is 12

// Exponents are stored very similarly to digits, so the exponent
// 1001 1001
// would be
// 99

// Useless bits are self-descriptive

// Sign bits work like this
// 01 - Value is negative
// 10 - Exponent is negative
// 11 - Both values are negative

//~BCD()  {}

// Gets the int64_t value of the BCD, without exponent
int64_t BCD_to_int64(BCD *value){
	//simplify_value();

	int64_t output = 0;
	uint8_t temp   = 0;

	// So this is very not right
	// THINK AGAIN!
	for (uint8_t i = 0; i < BCD_max_digit_count; i++){
		for (uint8_t j = 0; j < nibble; j++){
			// Read the bit at the indexed value, 
			// shift it by j so it's in the right position, 
			// and bitwise OR it into the result.

			temp |= (read_bit(*value, BCD_figure_offset + j + (i * nibble)) << j);
		}

		// Multiply the result by its 10 exponent and add it to the output
		output += (temp * pow(10, i)); 
		temp = 0;
	}

	return (read_bit(*value, 0) ? -output : output);
}

void BCD_reset_digit_count(BCD *value){
	*value &= ~0x000000000000F000;	
}

bool BCD_set_digit_count(BCD *value, uint8_t count){
	if (count < 0 || count > 16) return false;

	BCD_reset_digit_count(value);

	*value |= (count << BCD_digit_count_offset);
	return true;
}

void BCD_update_digit_count(BCD* value){
	uint8_t digit_count;

	for (int i = BCD_max_digit_count; i > 0; i--){
		if (0x0F & (*value >> (((i - 1) * nibble) + BCD_figure_offset))){
			digit_count = i; 
			goto EndFor;
		}  
	}

	BCD_reset_digit_count(value);
	return;

EndFor:
	BCD_set_digit_count(value, digit_count);
}

// Gets exponent of the BCD
int8_t BCD_get_exponent(BCD *value){
	uint8_t tens = 0;
	uint8_t ones = 0;

	uint8_t i;

	for (i = 0; i < nibble; i++)
		ones |= (read_bit(*value, i + BCD_exponent_offset) << i);

	for (i = 0; i < nibble; i++)
		tens |= (read_bit(*value, i + (BCD_exponent_offset + nibble)) << i);

	return (read_bit(*value, 1) ? -(ones + (tens * 10)) : (ones + (tens * 10)));
}

void BCD_clear_exponent(BCD* value){
	// Clears the exponent without touching anything else
	*value &= ~(0x00FF << BCD_exponent_offset);
	//CLears the exponent sign without touching anything else
	*value &= ~(1 << 1);
}

bool BCD_set_exponent(BCD *value, int8_t exponent){
	if (exponent > 99 || exponent < -99) return false;

	BCD_clear_exponent(value);

	if (exponent < 0) {
		*value |= (1 << 1);
		exponent = -exponent;
	}

	if (exponent < 10) *value |= (exponent << BCD_exponent_offset);
	else{
		*value |= ((exponent % 10) << BCD_exponent_offset);
		*value |= ((exponent / 10) << (BCD_exponent_offset + nibble));
	}

	return true;
}

void BCD_clear_digits(BCD *value){
	*value &= 0x000000000000FFFF;
}

void BCD_simplify_value(BCD *value){
	int8_t exp = BCD_get_exponent(value);

	uint64_t digits = 0xFFFFFFFFFFFF0000 & *value;

	if (exp > 0){
		const uint64_t mask = 0xF000000000000000;
		while (!(exp == 0 || (digits & mask))){
			digits <<= nibble;
			exp--;
		}
	}
	else if (exp < 0){
		const uint64_t mask = 0x0000000000F0000;
		while (!(exp == 0 || (digits & mask))){
			digits >>= nibble;
			exp++;
		}
	}

	BCD_clear_digits(value);

	*value |= digits;

	BCD_set_exponent(value, exp);

	// If 
	//if (!(value >> (64 - (BCD_figure_offset)))) return;

	//uint16_t temp = 0;

	//uint8_t shift = 0;

	//temp |= (value << nibble);

	// Value shifted left to
	//do { 
	//shift++;
	//} while (value >>= (shift * nibble));

	//value &= ~((uint32_t)0 << BCD_figure_offset);
	//value |= (temp >> nibble);

	//set_exponent(get_exponent() + shift);
}

void BCD_from_uint64(BCD *value, uint64_t uint64){
	int8_t exp = 0;

	//if (uint64 > (1 * pow(10, BCD_max_digit_count + 1)) - 1){
	//TODO: This
	//exp = 13;
	//uint64 /= pow(10, 13);
	//}

	BCD_clear_digits(value);

	uint64_t max = (1 * pow(10, BCD_max_digit_count));

	while (uint64 >= max){
		uint64 /= 10;
		exp++;
	}

	for (uint8_t i = 0; i < BCD_max_digit_count; i++){
		*value |= ((uint64 % 10) << (BCD_figure_offset + (i * nibble)));

		uint64 /= 10;

		if (!uint64) break;
	}


	BCD_set_exponent(value, exp + BCD_get_exponent(value));
	BCD_simplify_value(value);
	BCD_update_digit_count(value);
}

void BCD_from_sint64(BCD *value, int64_t sint64){
	bool is_negative = (sint64 < 0);	

	if (is_negative) sint64 = -sint64;

	BCD_from_uint64(value, sint64);

	*value |= is_negative;
}

void BCD_add(BCD *a, BCD *b){
	int64_t int_a = BCD_to_int64(a);
	int64_t int_b = BCD_to_int64(b);

	//bool neg_a = (int_a < 0);
	//bool neg_b = (int_b < 0);	

	int8_t exp_a = BCD_get_exponent(a);
	int8_t exp_b = BCD_get_exponent(b);	

	if (int_a == 0){
		BCD_from_sint64(a, int_b);
		BCD_set_exponent(a, exp_b);
		return;
	}

	if (int_b == 0) return;

	if (exp_a == exp_b) {
		//TODO: Make from_int64_t and from_uint64_t functions
		BCD_from_sint64(a, int_a + int_b);
		BCD_set_exponent(a, exp_a);
		return;	
	}
	else{
		int64_t sum = 0;

		{	
			const uint64_t mask = 0xF000000000000000;

			uint64_t tmp_a = *a;
			uint64_t tmp_b = *b;

			while (!(mask & (tmp_a <<= nibble))){ 
				int_a *= (int64_t)10;
				exp_a--;
			}

			while (!(mask & (tmp_b <<= nibble))){ 
				int_b *= (int64_t)10;
				exp_b--;
			}
		}

		//TODO: Fix this logic
		//Okay, but it works
		if (exp_a > exp_b){
			sum = int_a + (int_b * pow(10, exp_b - exp_a));	
			BCD_set_exponent(a, exp_a);
			BCD_from_sint64(a, sum);
			return;
		}
		else{
			sum = int_b + (int_a * pow(10, exp_a - exp_b));	
			BCD_set_exponent(a, exp_b);
			BCD_from_sint64(a, sum);
			return;	
		}
	}
}

inline void BCD_init(BCD* value, int64_t init_value, int8_t exponent){
	*value = 0;

	BCD_from_sint64(value, init_value);
	BCD_set_exponent(value, exponent);	
}
