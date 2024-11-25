/**
 ******************************************************************************
 * @file      sysmem.c
 * @author
 * @brief     STM32CubeIDE System Memory calls file
 *
 *            For more information about which C functions
 *            need which of these lowlevel functions
 *            please consult the newlib libc manual
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes */
#include <errno.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Pointer to the current high watermark of the heap usage
 */
static uint8_t *__sbrk_heap_end = NULL;

/**
 * @brief _sbrk() allocates memory to the newlib heap and is used by malloc
 *        and others from the C library
 *
 * @verbatim
 * ############################################################################
 * #  .data  #  .bss  #       newlib heap       #          MSP stack          #
 * #         #        #                         # Reserved by _Min_Stack_Size #
 * ############################################################################
 * ^-- RAM start      ^-- _end                             _estack, RAM end --^
 * @endverbatim
 *
 * This implementation starts allocating at the '_end' linker symbol
 * The '_Min_Stack_Size' linker symbol reserves a memory for the MSP stack
 * The implementation considers '_estack' linker symbol to be RAM end
 * NOTE: If the MSP stack, at any point during execution, grows larger than the
 * reserved size, please increase the '_Min_Stack_Size'.
 *
 * @param incr Memory size
 * @return Pointer to allocated memory
 */
void *_sbrk(ptrdiff_t incr)
{
    extern uint8_t _end;               /* Symbol defined in the linker script */
    extern uint8_t _estack;            /* Symbol defined in the linker script */
    extern uint32_t _Min_Stack_Size;   /* Symbol defined in the linker script */

    /* Corrected: Subtract the value of _Min_Stack_Size, not its address */
    const uintptr_t estack_addr = (uintptr_t)&_estack;
    const uintptr_t stack_limit = estack_addr - (uintptr_t)_Min_Stack_Size;
    const uint8_t *max_heap = (const uint8_t *)stack_limit;
    uint8_t *prev_heap_end;

    /* Initialize heap end at first call */
    if (__sbrk_heap_end == NULL)
    {
        __sbrk_heap_end = &_end;
    }

    /* Protect heap from growing into the reserved MSP stack */
    if ((__sbrk_heap_end + incr) > max_heap)
    {
        errno = ENOMEM;
        return (void *)-1;
    }

    prev_heap_end = __sbrk_heap_end;
    __sbrk_heap_end += incr;

    return (void *)prev_heap_end;
}
