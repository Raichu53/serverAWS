#ifndef __PARSER_HEADER__
#define __PARSER_HEADER__

struct Frame;

bool parse_buffer ( unsigned char* buffer, Frame* f );
void printFrame( Frame* f );
#endif //__PARSER_HEADER__
