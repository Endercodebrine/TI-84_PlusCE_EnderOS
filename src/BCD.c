#include<stdint.h>
#include<math.h>

//Reads the value at the specified index in the passed data
template<typename T>
inline bool read_bit(T value, unsigned int index){
	return (value >> index) & 1;
}

const uint8_t nibble = 4;
const int max_int = pow(10, 12);
struct BCD {
	static const uint8_t figure_offset   = 12;
	static const uint8_t exponent_offset = 4;

	uint64_t value = 0;

	// 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000   0000 0000     00        00
	// |--------------------------------------------------------------|   |-------|     ||        ||
	//                        Value                                       Exponents   Useless    Signs

	// The value is stored in nibbles, such that each nibble is one digit.
	// For example:
	// 1001 1000 0111 0110 0101 0100 0011 0010 0001
	// equals
	// 987654321
	// 
	// Probably

	// Exponents are stored very similarly to digits, so the exponent
	// 1001 1001
	// would be
	// 99
	
	// Useless bits are self-descriptive
	
	// Sign bits work like this
	// 01 - Value is negative
	// 10 - Exponent is negative
	// 11 - Both values are negative

	~BCD()  {}

	BCD() {}

	BCD(int init_value){
		from_sint(init_value);
		set_exponent(0);
	}

	BCD(int init_value, int8_t exponent){
		from_sint(init_value);
		set_exponent(exponent);	
	}

	BCD(int64_t init_value){

		set_exponent(0);
	}

	BCD(int64_t init_value, int8_t init_exponent){
			
		set_exponent(init_exponent);
	}

	// Gets the int64_t value of the BCD, without exponent
	int64_t to_int64(){
		int64_t output = 0;
		uint8_t temp   = 0;
		

		bool input = 0;

		// So this is very not right
		// THINK AGAIN!
		for (uint8_t i = 0; i < 13; i++){
			for (uint8_t j = 0; j < nibble; j++){
				// Read the bit at the indexed value, shift it by j so it's in the right position, 
				// and bitwise OR it into the result.
				temp |= (read_bit(value, figure_offset + j + (i * nibble)) << j);
			}

			// Multiply the result by its 10 exponent and add it to the output
			output += (temp * pow(10, i)); 
			temp = 0;
		}
		
		return (read_bit(value, 0) ? -output : output);
	}

	static struct BCD add(struct BCD a, struct BCD b){
		int64_t int_a = a.to_int64();
		int64_t int_b = b.to_int64();

		bool neg_a = (int_a < 0);
		bool neg_b = (int_b < 0);

		if (neg_a) int_a = -int_a;
		if (neg_b) int_b = -int_b;

		uint8_t shift_a = 0;
		uint8_t shift_b = 0;

		while (!read_bit(int_a, 63)){
			int_a <<= 1;
			shift_a++;
		} 

		while (!read_bit(int_a, 63)){
			int_b <<= 1;
			shift_b++;
		} 

		int8_t exp_a = a.get_exponent();
		int8_t exp_b = b.get_exponent();

		if (exp_a != exp_b){
			bool exp_a_larger = (exp_a > exp_b);

			if (exp_a_larger){
				int_b /= pow(10, exp_a - exp_b);
			}
			else{
				int_a /= pow(10, exp_b - exp_a);
			}

			return BCD((int_a >> shift_a) + (int_b >> shift_b), (exp_a_larger) ? exp_a : exp_b);
		}

		return BCD(int_a + int_b, exp_a);
	}

	// Gets exponent of the BCD
	int8_t get_exponent(){
		uint8_t tens = 0;
		uint8_t ones = 0;

		uint8_t i;

		for (i = 0; i < nibble; i++)
			ones |= (read_bit(value, i + 7) << i);

		for (i = 0; i < nibble; i++)
			tens |= (read_bit(value, i + 11) << i);

		return (read_bit(value, 1) ? -(ones + (tens * 10)) : (ones + (tens * 10)));
	}

	void clear_exponent(){
		// Clears the exponent without touching anything else
		value &= ~(0x00FF << exponent_offset);
		//CLears the exponent sign without touching anything else
		value &= ~(1 << 1);
	}

	bool set_exponent(int8_t exponent){
		if (exponent > 99 || exponent < -99) return false;

		clear_exponent();

		if (exponent < 0) {
			value |= (1 << 1);
			exponent = -exponent;
		}

		if (exponent < 10) value |= (exponent << exponent_offset);
		else{
			value |= ((exponent % 10) << exponent_offset);
			value |= ((exponent / 10) << (exponent_offset + nibble));
		}

		return true;
	}

	void from_sint(signed int integer){
		bool is_negative = read_bit(integer, (sizeof(int) * 8) - 1);

		if (is_negative) integer = -integer;

		from_uint(integer);

		value |= is_negative;
	}

	void from_uint(unsigned int integer){
		value = 0;	
			
		// No need for an exponent check, as int cannot be large enough to require one 
		// The i < sizeof(int) also works because ints are so small

		

		for (int i = 0; i < (int)sizeof(int); i++){
			for (int j = 0; j < nibble; j++){
				value |= (
						(((int)floor(integer / pow(10, i)) % 10) >> 0) << 
						(figure_offset + 0 + (i * nibble)));
			}	
		}
	}

	private:
	void init(){
		
	}
};
