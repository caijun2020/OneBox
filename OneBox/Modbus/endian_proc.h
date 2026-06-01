#ifndef ENDIAN_PROC_H
#define ENDIAN_PROC_H

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------
FUNCTION:    swapUint16
PURPOSE:     Transform the endian of unsigned 16-bit value.
ARGUMENTS:   val [in] - unsigned 16-bit value.
RETURNS:     Unsigned 16-bit value with transformed endian.
-----------------------------------------------------------------------*/
inline uint16_t swapUint16(uint16_t val)
{
  return (val << 8) | (val >> 8);
}

/*-----------------------------------------------------------------------
FUNCTION:    swapUint32
PURPOSE:     Transform the endian of unsigned 32-bit value.
ARGUMENTS:   val [in] - unsigned 32-bit value.
RETURNS:     Unsigned 32-bit value with transformed endian.
-----------------------------------------------------------------------*/
inline uint32_t swapUint32(uint32_t val)
{
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
  return (val << 16) | (val >> 16);
}

/*-----------------------------------------------------------------------
FUNCTION:    swapUint64
PURPOSE:     Transform the endian of unsigned 64-bit value.
ARGUMENTS:   val [in] - unsigned 64-bit value.
RETURNS:     Unsigned 64-bit value with transformed endian.
-----------------------------------------------------------------------*/
inline uint64_t swapUint64(uint64_t val)
{
  val = ((val << 8)  & 0xFF00FF00FF00FF00ULL) | ((val >> 8)  & 0x00FF00FF00FF00FFULL);
  val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
  return (val << 32) | (val >> 32);
}

#ifdef __cplusplus
}
#endif

#endif
