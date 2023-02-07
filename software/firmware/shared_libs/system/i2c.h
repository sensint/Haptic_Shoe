#ifndef __SENSINT_I2C_H__
#define __SENSINT_I2C_H__

#if SENSINT_WIRE == 0
#define SENSINT_I2C Wire
#else
#define SENSINT_I2C Wire1
#endif

#endif  // __SENSINT_I2C_H__
