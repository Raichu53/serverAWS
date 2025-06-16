#ifndef __PARSER_HEADER__
#define __PARSER_HEADER__

struct Frame;

bool parse_buffer ( unsigned char* buffer, Frame* f );
void printFrame( Frame* f );
uint8_t checkFrame( Frame* f );
bool isCMDidValid(uint8_t cID);
const char* codeTostring(uint8_t errCode);
bool checkParamSz( uint8_t cID, uint8_t sz );

#endif //__PARSER_HEADER__

