#include<stdint.h>
#include<math.h>

//Reads the value at the specified index in the passed data
template<typename T>
bool read_bit(T value, unsigned int index){
	return (value >> index) & 1;
}

constexpr uint8_t nibble = 4;
constexpr uint8_t byte   = 8;
struct BCD {
	static constexpr uint8_t figure_offset      = 16;
	static constexpr uint8_t digit_count_offset = 12;
	static constexpr uint8_t exponent_offset    = 4;

	static constexpr uint8_t max_digit_count = (64 - figure_offset) / nibble;

	const int max_int = pow(10, max_digit_count);

	uint64_t value = 0;

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

	~BCD()  {}

	BCD() {}

	BCD(int64_t init_value){
		from_sint64(init_value);
		set_exponent(0);

		truncate_value();
		update_digit_count();
	}

	BCD(int64_t init_value, int8_t exponent){
		from_sint(init_value);
		set_exponent(exponent);	

		truncate_value();
		update_digit_count();
	}

	// Gets the int64_t value of the BCD, without exponent
	int64_t to_int64(){
		truncate_value();

		int64_t output = 0;
		uint8_t temp   = 0;
		
		// So this is very not right
		// THINK AGAIN!
		for (uint8_t i = 0; i < max_digit_count; i++){
			for (uint8_t j = 0; j < nibble; j++){
				// Read the bit at the indexed value, 
				// shift it by j so it's in the right position, 
				// and bitwise OR it into the result.

				temp |= (read_bit(value, figure_offset + j + (i * nibble)) << j);
			}

			// Multiply the result by its 10 exponent and add it to the output
			output += (temp * pow(10, i)); 
			temp = 0;
		}
		
		return (read_bit(value, 0) ? -output : output);
	}

	bool update_digit_count(){
		uint8_t digit_count = -1;

		for (int i = max_digit_count - 1; i >= 0; i--){
			if (0x0F & (value >> ((i * nibble) + figure_offset))){
				digit_count = i;
				goto EndFor;
			}  
		}
		return false;

	EndFor:
		set_digit_count(digit_count);
		return true;
	}

	bool set_digit_count(uint8_t count){
		if (count < 0 || count > max_digit_count) return false;

		reset_digit_count();
		
		value |= (count << digit_count_offset);
		return true;
	}

	void add(struct BCD b){
		int64_t int_a = to_int64();
		int64_t int_b = b.to_int64();

		//bool neg_a = (int_a < 0);
		//bool neg_b = (int_b < 0);	

		int8_t exp_a = get_exponent();
		int8_t exp_b = b.get_exponent();	

		if (exp_a == exp_b) {
			//TODO: Make from_int64_t and from_uint64_t functions
			from_sint64(int_a + int_b);
			set_exponent(exp_a);
			return;	
		}
		else{
			int64_t sum = 0;

			//TODO: Fix this logic
			if (exp_a > exp_b){
				sum = int_a + (int_b * pow(10, exp_b - exp_a));	
				from_sint64(sum);
				set_exponent(exp_a);
				return;
			}
			else{
				sum = int_b + (int_a * pow(10, exp_a - exp_b));	
				from_sint64(sum);
				set_exponent(exp_b);
				return;	
			}
		}
	}

	static struct BCD add(struct BCD a, struct BCD b){
		int64_t int_a = a.to_int64();
		int64_t int_b = b.to_int64();

		//bool neg_a = (int_a < 0);
		//bool neg_b = (int_b < 0);	

		int8_t exp_a = a.get_exponent();
		int8_t exp_b = b.get_exponent();	

		if (exp_a == exp_b) {
			return BCD(int_a + int_b, exp_a);	
		}
		else{
			int64_t sum = 0;

			if (exp_a > exp_b){
				sum = int_a + (int_b * pow(10, exp_b - exp_a));	
				return BCD(sum, exp_a);
			}
			else{
				sum = int_b + (int_a * pow(10, exp_a - exp_b));	
				return BCD(sum, exp_b);	
			}
		}
	}

	void from_sint64(int64_t sint64){
		bool is_negative = (sint64 < 0);	

		if (is_negative) sint64 = -sint64;

		from_uint64(sint64);

		value |= is_negative;
	}

	void from_uint64(uint64_t uint64){
		int8_t exp = 0;
	
		if (value > 99999999999999){
			//TODO: This
			exp = 13;
			uint64 /= pow(10, 13);
		}

		from_uint(value);

		set_exponent(exp);
	}

	// Gets exponent of the BCD
	int8_t get_exponent(){
		uint8_t tens = 0;
		uint8_t ones = 0;

		uint8_t i;

		for (i = 0; i < nibble; i++)
			ones |= (read_bit(value, i + exponent_offset) << i);

		for (i = 0; i < nibble; i++)
			tens |= (read_bit(value, i + (exponent_offset + nibble)) << i);

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
		bool is_negative = (integer < 0);

		if (is_negative) integer = -integer;

		from_uint(integer);

		value |= is_negative;
	}

	void from_uint(unsigned int integer){
		value = 0;	
			
		// No need for an exponent check, as int cannot be large enough to require one 

		for (int i = 0; i < max_digit_count; i++){
			value |= (
					((uint64_t)floor(integer / pow(10, i)) % 10) << 
					(figure_offset + (i * nibble)));
		}
	}

	private:
	void truncate_value(){
		if (!(value >> (64 - (figure_offset)))) return;

		uint16_t temp = 0;

		uint8_t shift = 0;

		temp |= (value << nibble);

		do { 
			shift++;
		} while (value >>= (shift * nibble));

		value &= ~(0 << (figure_offset));
		value |= (temp >> nibble);

		set_exponent(get_exponent() + shift);
	}

	void reset_digit_count(){
		value &= ~(0x0000FF << digit_count_offset);	
	}

	void init(){
		
	}
};
