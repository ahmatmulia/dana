// See LICENSE for license details.

#ifndef SRC_MAIN_C_XFILES_H_
#define SRC_MAIN_C_XFILES_H_

#include <stdint.h>
#include <stddef.h>

// [TODO] Any changes to these types need to occur in conjunction with
// the Chisel code and with the TID extraction part of
// new_write_request.
typedef int32_t nnid_type;
typedef int16_t tid_type;
typedef int32_t element_type;
typedef uint64_t xlen_t;

typedef enum {
  xfiles_reg_batch_items = 0,
  xfiles_reg_learning_rate,
  xfiles_reg_weight_decay_lambda
} xfiles_reg;

#define t_USR_READ_DATA 4
#define t_USR_WRITE_DATA 5
#define t_USR_NEW_REQUEST 6
#define t_USR_WRITE_DATA_LAST 7
#define t_USR_WRITE_REGISTER 8
#define t_USR_XFILES_DEBUG 9
#define t_USR_XFILES_DANA_ID 10

typedef enum {
  FEEDFORWARD = 0,
  TRAIN_INCREMENTAL = 1,
  TRAIN_BATCH = 2
} learning_type_t;

typedef enum {
  err_XFILES_UNKNOWN = 0,
  err_XFILES_NOASID,
  err_XFILES_TTABLEFULL,
  err_XFILES_INVALIDTID
} xfiles_err_t;

typedef enum {
  resp_OK = 0,
  resp_TID,
  resp_READ,
  resp_NOT_DONE,
  resp_QUEUE_ERR,
  resp_XFILES
} xfiles_resp_t;

typedef enum {
  err_UNKNOWN     = 0,
  err_DANA_NOANTP = 1,
  err_INVASID     = 2,
  err_INVNNID     = 3,
  err_ZEROSIZE    = 4,
  err_INVEPB      = 5
} dana_err_t;

typedef enum {
  int_INVREQ      = 0,
  int_DANA_NOANTP = 1,
  int_INVASID     = 2,
  int_INVNNID     = 3,
  int_NULLREAD    = 4,
  int_ZEROSIZE    = 5,
  int_INVEPB      = 6,
  int_MISALIGNED  = 7,
  int_UNKNOWN     = -1
} xfiles_interrupt_t;

#define RESP_CODE_WIDTH 3

// Macros for using XCustom instructions. Four different macros are
// provided depending on whether or not the passed arguments should be
// communicated as registers or immediates.
#define XCUSTOM 0

#define STR1(x) #x
#ifndef STR
#define STR(x) STR1(x)
#endif
#define EXTRACT(a, size, offset) (((~(~0 << size) << offset) & a) >> offset)

#define CUSTOMX_OPCODE(x) CUSTOM_##x
#define CUSTOM_0 0b0001011
#define CUSTOM_1 0b0101011
#define CUSTOM_2 0b1011011
#define CUSTOM_3 0b1111011

#define CUSTOMX(X, rd, rs1, rs2, funct) \
  CUSTOMX_OPCODE(X)                   | \
  (rd                   << (7))       | \
  (0x7                  << (7+5))     | \
  (rs1                  << (7+5+3))   | \
  (rs2                  << (7+5+3+5)) | \
  (EXTRACT(funct, 7, 0) << (7+5+3+5+5))

#define CUSTOMX_R_R_R(X, rd, rs1, rs2, funct)           \
  asm ("mv a4, %[_rs1]\n\t"                             \
       "mv a5, %[_rs2]\n\t"                             \
       ".word "STR(CUSTOMX(X, 15, 14, 15, funct))"\n\t" \
       "mv %[_rd], a5"                                  \
       : [_rd] "=r" (rd)                                \
       : [_rs1] "r" (rs1), [_rs2] "r" (rs2)             \
       : "a4", "a5");

// Standard macro that passes rd_, rs1_, and rs2_ via registers
#define XFILES_INSTRUCTION(rd, rs1, rs2, funct)     \
  XFILES_INSTRUCTION_R_R_R(rd, rs1, rs2, funct)
#define XFILES_INSTRUCTION_R_R_R(rd, rs1, rs2, funct)               \
  CUSTOMX_R_R_R(XCUSTOM, rd, rs1, rs2, funct)

// Macro to pass rs2_ as an immediate
#define XFILES_INSTRUCTION_R_R_I(rd, rs1, rs2, funct)               \
  CUSTOMX_R_R_R(XCUSTOM, rd, rs1, rs2, funct)

// Macro to pass rs1_ and rs2_ as immediates
#define XFILES_INSTRUCTION_R_I_I(rd, rs1, rs2, funct)               \
  CUSTOMX_R_R_R(XCUSTOM, rd, rs1, rs2, funct)

#endif  // SRC_MAIN_C_XFILES_H_
