#include<stdint.h>

typedef uint64_t BCD;

//Functions
void BCD_constants_init();
int64_t BCD_to_int64(BCD *value);
void BCD_reset_digit_count(BCD *value);
bool BCD_set_digit_count(BCD *value, uint8_t count);
void BCD_update_digit_count(BCD *value);
int8_t BCD_get_exponent(BCD *value);
void BCD_clear_exponent(BCD *value);
bool BCD_set_exponent(BCD *value, int8_t exponent);
void BCD_clear_digits(BCD *value);
//void BCD_simplify_value(BCD *value);
void BCD_from_uint64(BCD *value, uint64_t uint64);
void BCD_from_sint64(BCD *value, int64_t sint64);
void BCD_add(BCD *a, BCD *b);
inline void BCD_init(BCD *value, int64_t init_value, int8_t exponent);
