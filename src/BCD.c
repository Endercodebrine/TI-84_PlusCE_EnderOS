#include<inttypes.h>
#include<math.h>

inline bool read_bit(int value, int index){
	return (value >> index) & 1;
}

const uint8_t nibble = 4;
const int max_int = pow(10, 12);
struct BCD {
	static const uint8_t figure_offset   = 16;
	static const uint8_t exponent_offset = 8;

	uint64_t value;

	BCD() : value(0) {}

	BCD(int init_value){
		init();
	}

	// Gets the int value of the BCD, without exponent
	int to_int(){
		int output = 0;

		bool input = 0;

		for (uint8_t i = 0; i < 12; i++){
			for (uint8_t j = 0; j < nibble; j++){
				output |= (read_bit(value, j + (i * 4)) << i);
			}

			output *= pow(10, 11 - i);
		}
		
		return (read_bit(value, 0) ? -output : output);
	}

	static struct BCD add(struct BCD a, struct BCD b){
		
	}

	// Gets exponent of the BCD
	uint8_t get_exponent(){
		uint8_t tens = 0;
		uint8_t ones = 0;

		uint8_t i;

		for (i = 0; i < nibble; i++)
			ones |= (read_bit(value, i + 7) << i);

		for (i = 0; i < nibble; i++)
			tens |= (read_bit(value, i + 11) << i);

		return read_bit(value, 1) ? -(ones + (tens * 10)) : (ones + (tens * 10));
	}

	bool set_exponent(int8_t exponent){
		if (exponent > 99 || exponent < -99) return false;
	}

	void from_sint(signed int integer){
		bool is_negative = read_bit(integer, 31);

		if (is_negative) integer = -integer;

		from_uint(integer);

		value |= read_bit(integer, 31);
	}

	void from_uint(unsigned int integer){
		value = 0;	
			
		// No need for an exponent check, as int cannot be large enough to require one 
	}

	private:
	void init(){
		
	}
};
