#ifndef stdlib_arm_h
#define stdlib_arm_h

#ifdef __cplusplus
extern "C"{
#endif

char * utoa( unsigned long value, char *string, int radix );
char * itoa( int value, char *string, int radix );
char * ltoa( long value, char *string, int radix );
char * ultoa( unsigned long value, char *string, int radix );

#ifdef __cplusplus
} // extern "C"
#endif

#endif
