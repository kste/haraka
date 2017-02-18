#ifndef SETTINGS_H
#define SETTINGS_H

#ifndef MPAR
#define MPAR (1)
#endif

#ifndef ROUNDS
#define ROUNDS (4)
#endif

#ifndef AES_PER_ROUND
#define AES_PER_ROUND (2)
#endif

#ifndef MIX_PER_ROUND
#define MIX_PER_ROUND (1)
#endif

#define DEBUG (0)

#define MIX_METHOD (0)	// 0 : blend
						// 1 : shuffle + xor
						// 2 : AESQ method

#endif
