#include <ti/screen.h>
#include <ti/getcsc.h>
#include <stdlib.h>
#include<graphx.h>
#include<string.h>
#include<stdio.h>
#include"BCD.c"

// Index legend:
// 0 = Character
// 1 = Line
int current_line_index[2] = { 0, 0 };

// The values must be the same as their indexes, or this program will break.
uint16_t ids[0x0FFF] = {
	0x0000, // 0
	0x0001, // 1
	0x0002, // 2
	0x0003, // 3
	0x0004, // 4
	0x0005, // 5
	0x0006, // 6
	0x0007, // 7
	0x0008, // 8
	0x0009, // 9
	0x000A, // Separator
	0x000B, // a
	0x000C, // A
	0x000D, // b
	0x000E, // B
	0x000F, // c
	0x0010, // C
	0x0011, // d
	0x0012, // D
	0x0013, // e
	0x0014, // E
	0x0015, // f
	0x0016, // F
	0x0017, // g
	0x0018, // G
	0x0019, // h
	0x001A, // H
	0x001B, // i
	0x001C, // I
	0x001D, // j
	0x001E, // J
	0x001F, // k
	0x0020, // K
	0x0021, // l
	0x0022, // L
	0x0023, // m
	0x0024, // M
	0x0025, // n
	0x0026, // N
	0x0027, // o
	0x0028, // O
	0x0029, // p
	0x002A, // P
	0x002B, // q
	0x002C, // Q
	0x002D, // r
	0x002E, // R
	0x002F, // s
	0x0030, // S
	0x0031, // t
	0x0032, // T
	0x0033, // u
	0x0034, // U
	0x0035, // v
	0x0036, // V
	0x0037, // w
	0x0038, // W
	0x0039, // x
	0x003A, // X
	0x003B, // y
	0x003C, // Y
	0x003D, // z
	0x003E, // Z
	0x003F, // Theta
	0x0040, // Negative
	0x0041, // Plus
	0x0042, // Minus
	0x0043, // Multiply
	0x0044, // Divide
	0x0045, // Modulo
	0x0046, // Decimal Point
};

#define MAX_LINE_BUFFER 128
#define MAX_TEXT_BUFFER 30

#define SEPARATOR_INDEX 0x000A

#define PLUS_INDEX      0x0041
#define MINUS_INDEX     0x0042
#define MULTIPLY_INDEX  0x0043
#define DIVIDE_INDEX    0x0044
#define MODULO_INDEX    0x0045

#define DECIMAL_INDEX   0x0046

const uint8_t Max_answer_length = 15;
const uint8_t Line_spacing = 17;
const uint8_t Test_start_pos[2] = { 7, 40 };

uint8_t text_colour = 0xFE;

uint16_t line_buffer[MAX_LINE_BUFFER] = { [0 ... MAX_LINE_BUFFER - 1] = SEPARATOR_INDEX };
uint16_t text_buffer[MAX_TEXT_BUFFER][MAX_LINE_BUFFER];


uint8_t line_index = 0;
uint8_t text_index = 0;

uint8_t current_line = 0;


bool print_int(uint8_t integer){

	if (line_index > MAX_LINE_BUFFER - 1) return false;

	switch (integer){
		case 0 ... 9:
			line_buffer[line_index] = ids[integer];
			break;

		default:
			return false;
	}

	gfx_PrintInt(integer, 1);
	line_index++;
	return true;
}

bool print_operator(char oper){
	
	if (line_index > MAX_LINE_BUFFER - 1) return false;

	switch (oper){
		case '+':
			line_buffer[line_index] = ids[PLUS_INDEX];
			break;

		case '-':
			line_buffer[line_index] = ids[MINUS_INDEX];
			break;

		case '*':
			line_buffer[line_index] = ids[MULTIPLY_INDEX];
			break;

		case '/':
			line_buffer[line_index] = ids[DIVIDE_INDEX];
			break;

		default: 
			return false;
	}

	gfx_PrintChar(oper);
	line_index++;
	return true;
}

void clear_line_buffer(){

	for (int i = 0; i < MAX_LINE_BUFFER; line_buffer[i] = ids[SEPARATOR_INDEX], i++);

	line_index = 0;
}

// Logs the line to the main text buffer, and clears the line if you tell it to
void log_line_buffer(bool clear_line){
	if (text_index > MAX_TEXT_BUFFER){
		for (int i = 0; i < MAX_TEXT_BUFFER - 1; i++){
			memcpy(text_buffer[i], text_buffer[i + 1], sizeof(text_buffer[i + 1]));
		}
	
		text_index--;
	}

	memcpy(text_buffer[text_index], line_buffer, sizeof(line_buffer));
	text_index++;

	if (clear_line) clear_line_buffer();
}

// Prints the passed answer to a new line
void print_answer(double answer, bool is_double){
	char buffer[Max_answer_length];

	if (!is_double){
		sprintf(buffer, "%.g", answer);
	}
	else{	
		sprintf(buffer, "%.5f", answer);
		// When you're handling different DP of output,
		// sprintf("%.*f", n, answer);
		// You'll thank you later.
	}

	current_line++;

	gfx_SetTextXY(Test_start_pos[0], Test_start_pos[1] + (Line_spacing * current_line));
	gfx_PrintChar('=');

	int answer_location = GFX_LCD_WIDTH - gfx_GetStringWidth(buffer) - Test_start_pos[0];

	if (answer_location <= Test_start_pos[0] + (int)gfx_GetCharWidth('=')){
		// TODO: Something here
	}

	//gfx_SetTextXY(GFX_LCD_WIDTH - Test_start_pos[0], Test_start_pos[1] + (Line_spacing * current_line));
	gfx_PrintString(buffer);

	current_line++;
	gfx_SetTextXY(Test_start_pos[0], Test_start_pos[1] + (Line_spacing * current_line));
}

double apply_operator(uint16_t oper, double a, double b){
	if (oper == 0x0000) return b;

	switch(oper){
		case PLUS_INDEX:
			return a + b;

		case MINUS_INDEX:
			return a - b;

		case MULTIPLY_INDEX:
			return a * b;

		case DIVIDE_INDEX:
			return a / b;

		case MODULO_INDEX:
			//TODO: Make a mathematically correct modulo
			return b;

		default:
			return b;
	}
}

// TODO: Order of operations
bool calculate(){	
	if (line_buffer[0] == 0x0000) return false;

	uint16_t value;	

	double current_number = 0;

	double answer = 0;

	bool allow_separator = false;
	bool is_double = false;
	
	uint16_t queued_operator = 0x0000;

	uint8_t number_count = 0;

	uint8_t depth = 0;
	uint8_t nulls = 0;

	for (int i = 0; i < MAX_LINE_BUFFER; i++){
		value = line_buffer[i];

		switch (value){
			case 0x0000 ... 0x0009:
				for (number_count = 0; 
						line_buffer[i + number_count] >= 0x0000 && line_buffer[i + number_count] <= 0x0009;
					 	number_count++);

				while (number_count > 0){
					current_number += (line_buffer[i++] * pow(10, --number_count)); 
				}

				// Without this, skips the next operator
				i--; 
				break;

			case PLUS_INDEX ... MODULO_INDEX:
				answer = apply_operator(queued_operator, answer, current_number);
				queued_operator = value;
				if (!answer) answer = current_number;
				current_number = 0;
				break;

			case DECIMAL_INDEX:
				is_double = true;
				break;

			case SEPARATOR_INDEX:
				if (!allow_separator){
					answer = apply_operator(queued_operator, answer, current_number);
					goto exit_loop;
				}
				// TODO: This
				break;
		
		}
	}

	apply_operator(queued_operator, current_number, answer);

exit_loop:
	print_answer(answer, is_double);
	log_line_buffer(true);
}

int main(void)
{

		//Begin drawing to screen
		gfx_Begin();	

		gfx_SetMonospaceFont(8);

		// Fill in the screen
		gfx_FillScreen(0x00);

		//Sets the colour to draw
		gfx_SetColor(0xFF);

		gfx_FillRectangle(2, 30, GFX_LCD_WIDTH - 4, 4);

		gfx_Line(2, 2, GFX_LCD_WIDTH - 2, 2);
		gfx_Line(GFX_LCD_WIDTH - 2, 2, GFX_LCD_WIDTH - 2, GFX_LCD_HEIGHT - 4);
		gfx_Line(2, 2, 2, GFX_LCD_HEIGHT - 4);	
		gfx_Line(2, GFX_LCD_HEIGHT - 2, GFX_LCD_WIDTH - 2, GFX_LCD_HEIGHT - 2);

		gfx_SetTextFGColor(text_colour);

		gfx_PrintStringXY("EnderOS", 5, 15);

		gfx_SetTextScale(2, 2);

		gfx_SetTextXY(Test_start_pos[0], Test_start_pos[1]);

    while (true){
			switch (os_GetCSC()){
				case sk_Clear:
					 goto exit_loop;

				case sk_0:
					 print_int(0);
					 break;

				case sk_1:
					 print_int(1);
					 break;

				case sk_2:
					 print_int(2);
					 break;

				case sk_3:
					 print_int(3);
					 break;

				case sk_4:
					 print_int(4);
					 break;

				case sk_5:
					 print_int(5);
					 break;

				case sk_6:
					 print_int(6);
					 break;

				case sk_7:
					 print_int(7);
					 break;

				case sk_8:
					 print_int(8);
					 break;

				case sk_9:
					 print_int(9);
					 break;

				case sk_Add:
					 print_operator('+');
					 break;

				case sk_Sub:
					 print_operator('-');
					 break;

				case sk_Mul:
					 print_operator('*');
					 break;

				case sk_Div:
					 print_operator('/');
					 break;

				case sk_Enter:
					 calculate();
					 break;
			}
		}

	exit_loop:

		//End drawing to screen
		gfx_End();


    return 0;
}
