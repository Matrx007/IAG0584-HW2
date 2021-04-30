#define u_int8_t unsigned char

void trim(const char **c);
void nextChar(const char **c);
char next(const char **c);
u_int8_t eat(const char **c, char t);
char get(const char **c);
char read(const char **c);