
#ifndef _TC_PROFILING_H
#define _TC_PROFILING_H

#if HAVE_STDATOMIC_H==1

#include <stdatomic.h>

#define TC_ATOMIC_MEMORY_ORDER_RELAXED memory_order_relaxed
#define TC_ATOMIC_MEMORY_ORDER_CONSUME memory_order_consume
#define TC_ATOMIC_MEMORY_ORDER_ACQUIRE memory_order_acquire
#define TC_ATOMIC_MEMORY_ORDER_RELEASE memory_order_release
#define TC_ATOMIC_MEMORY_ORDER_ACQ_REL memory_order_acq_rel
#define TC_ATOMIC_MEMORY_ORDER_SEQ_CST memory_order_seq_cst

/**
 *  \brief wrapper for declaring atomic variables.
 *
 *  \param type Type of the variable (char, short, int, long, long long)
 *  \param name Name of the variable.
 *
 *  We just declare the variable here as we rely on atomic operations
 *  to modify it, so no need for locks.
 *
 *  \warning variable is not initialized
 */
#define TC_ATOMIC_DECLARE(type, name) \
    _Atomic(type) name ## _sc_atomic__

/**
 *  \brief wrapper for referencing an atomic variable declared on another file.
 *
 *  \param type Type of the variable (char, short, int, long, long long)
 *  \param name Name of the variable.
 *
 *  We just declare the variable here as we rely on atomic operations
 *  to modify it, so no need for locks.
 *
 */
#define TC_ATOMIC_EXTERN(type, name) \
    extern _Atomic(type) (name ## _sc_atomic__)

/**
 *  \brief wrapper for declaring an atomic variable and initializing it.
 **/
#define TC_ATOMIC_DECL_AND_INIT(type, name) \
    _Atomic(type) (name ## _sc_atomic__) = 0

/**
 *  \brief wrapper for declaring an atomic variable and initializing it
 *  to a specific value
 **/
#define TC_ATOMIC_DECL_AND_INIT_WITH_VAL(type, name, val) _Atomic(type)(name##_sc_atomic__) = val

/**
 *  \brief wrapper for initializing an atomic variable.
 **/
#define TC_ATOMIC_INIT(name) \
    (name ## _sc_atomic__) = 0
#define TC_ATOMIC_INITPTR(name) \
    (name ## _sc_atomic__) = NULL

/**
 *  \brief wrapper for reinitializing an atomic variable.
 **/
#define TC_ATOMIC_RESET(name) \
    TC_ATOMIC_INIT(name)

/**
 *  \brief add a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to add to the variable
 */
#define TC_ATOMIC_ADD(name, val) \
    atomic_fetch_add(&(name ## _sc_atomic__), (val))

/**
 *  \brief sub a value from our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to sub from the variable
 */
#define TC_ATOMIC_SUB(name, val) \
    atomic_fetch_sub(&(name ## _sc_atomic__), (val))

/**
 *  \brief Bitwise OR a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to OR to the variable
 */
#define TC_ATOMIC_OR(name, val) \
    atomic_fetch_or(&(name ## _sc_atomic__), (val))

/**
 *  \brief Bitwise AND a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to AND to the variable
 */
#define TC_ATOMIC_AND(name, val) \
    atomic_fetch_and(&(name ## _sc_atomic__), (val))

/**
 *  \brief atomic Compare and Switch
 *
 *  \warning "name" is passed to us as "&var"
 */
#define TC_ATOMIC_CAS(name, cmpval, newval) \
    atomic_compare_exchange_strong((name ## _sc_atomic__), &(cmpval), (newval))

/**
 *  \brief Get the value from the atomic variable.
 *
 *  \retval var value
 */
#define TC_ATOMIC_GET(name) \
    atomic_load(&(name ## _sc_atomic__))

#define TC_ATOMIC_LOAD_EXPLICIT(name, order) \
    atomic_load_explicit(&(name ## _sc_atomic__), (order))

/**
 *  \brief Set the value for the atomic variable.
 *
 *  \retval var value
 */
#define TC_ATOMIC_SET(name, val)    \
    atomic_store(&(name ## _sc_atomic__), (val))

#else

#define TC_ATOMIC_MEMORY_ORDER_RELAXED
#define TC_ATOMIC_MEMORY_ORDER_CONSUME
#define TC_ATOMIC_MEMORY_ORDER_ACQUIRE
#define TC_ATOMIC_MEMORY_ORDER_RELEASE
#define TC_ATOMIC_MEMORY_ORDER_ACQ_REL
#define TC_ATOMIC_MEMORY_ORDER_SEQ_CST

/**
 *  \brief wrapper for OS/compiler specific atomic compare and swap (CAS)
 *         function.
 *
 *  \param addr Address of the variable to CAS
 *  \param tv Test value to compare the value at address against
 *  \param nv New value to set the variable at addr to
 *
 *  \retval 0 CAS failed
 *  \retval 1 CAS succeeded
 */
#define SCAtomicCompareAndSwap(addr, tv, nv) \
    __sync_bool_compare_and_swap((addr), (tv), (nv))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and add
 *         function.
 *
 *  \param addr Address of the variable to add to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicFetchAndAdd(addr, value) \
    __sync_fetch_and_add((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and sub
 *         function.
 *
 *  \param addr Address of the variable to add to
 *  \param value Value to sub from the variable at addr
 */
#define SCAtomicFetchAndSub(addr, value) \
    __sync_fetch_and_sub((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and add
 *         function.
 *
 *  \param addr Address of the variable to add to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicAddAndFetch(addr, value) \
    __sync_add_and_fetch((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and sub
 *         function.
 *
 *  \param addr Address of the variable to add to
 *  \param value Value to sub from the variable at addr
 */
#define SCAtomicSubAndFetch(addr, value) \
    __sync_sub_and_fetch((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and "AND"
 *         function.
 *
 *  \param addr Address of the variable to AND to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicFetchAndAnd(addr, value) \
    __sync_fetch_and_and((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and "NAND"
 *         function.
 *
 *  \param addr Address of the variable to NAND to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicFetchAndNand(addr, value) \
    __sync_fetch_and_nand((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and "XOR"
 *         function.
 *
 *  \param addr Address of the variable to XOR to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicFetchAndXor(addr, value) \
    __sync_fetch_and_xor((addr), (value))

/**
 *  \brief wrapper for OS/compiler specific atomic fetch and or
 *         function.
 *
 *  \param addr Address of the variable to or to
 *  \param value Value to add to the variable at addr
 */
#define SCAtomicFetchAndOr(addr, value) \
    __sync_fetch_and_or((addr), (value))

/**
 *  \brief wrapper for declaring atomic variables.
 *
 *  \warning Only char, short, int, long, long long and their unsigned
 *           versions are supported.
 *
 *  \param type Type of the variable (char, short, int, long, long long)
 *  \param name Name of the variable.
 *
 *  We just declare the variable here as we rely on atomic operations
 *  to modify it, so no need for locks.
 *
 *  \warning variable is not initialized
 */
#define TC_ATOMIC_DECLARE(type, name) \
    type name ## _sc_atomic__

/**
 *  \brief wrapper for referencing an atomic variable declared on another file.
 *
 *  \warning Only char, short, int, long, long long and their unsigned
 *           versions are supported.
 *
 *  \param type Type of the variable (char, short, int, long, long long)
 *  \param name Name of the variable.
 *
 *  We just declare the variable here as we rely on atomic operations
 *  to modify it, so no need for locks.
 *
 */
#define TC_ATOMIC_EXTERN(type, name) \
    extern type name ## _sc_atomic__

/**
 *  \brief wrapper for declaring an atomic variable and initializing it
 *  to a specific value
 **/
#define TC_ATOMIC_DECL_AND_INIT_WITH_VAL(type, name, val) type name##_sc_atomic__ = val

/**
 *  \brief wrapper for declaring an atomic variable and initializing it.
 **/
#define TC_ATOMIC_DECL_AND_INIT(type, name) \
    type name ## _sc_atomic__ = 0

/**
 *  \brief wrapper for initializing an atomic variable.
 **/
#define TC_ATOMIC_INIT(name) \
    (name ## _sc_atomic__) = 0

#define TC_ATOMIC_INITPTR(name) \
    (name ## _sc_atomic__) = NULL

/**
 *  \brief wrapper for reinitializing an atomic variable.
 **/
#define TC_ATOMIC_RESET(name) \
    (name ## _sc_atomic__) = 0

/**
 *  \brief add a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to add to the variable
 */
#define TC_ATOMIC_ADD(name, val) \
    SCAtomicFetchAndAdd(&(name ## _sc_atomic__), (val))

/**
 *  \brief sub a value from our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to sub from the variable
 */
#define TC_ATOMIC_SUB(name, val) \
    SCAtomicFetchAndSub(&(name ## _sc_atomic__), (val))

/**
 *  \brief Bitwise OR a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to OR to the variable
 */
#define TC_ATOMIC_OR(name, val) \
    SCAtomicFetchAndOr(&(name ## _sc_atomic__), (val))

/**
 *  \brief Bitwise AND a value to our atomic variable
 *
 *  \param name the atomic variable
 *  \param val the value to AND to the variable
 */
#define TC_ATOMIC_AND(name, val) \
    SCAtomicFetchAndAnd(&(name ## _sc_atomic__), (val))

/**
 *  \brief atomic Compare and Switch
 *
 *  \warning "name" is passed to us as "&var"
 */
#define TC_ATOMIC_CAS(name, cmpval, newval) \
    SCAtomicCompareAndSwap((name ## _sc_atomic__), cmpval, newval)

/**
 *  \brief Get the value from the atomic variable.
 *
 *  \retval var value
 */
#define TC_ATOMIC_GET(name) \
    (name ## _sc_atomic__)

#define TC_ATOMIC_LOAD_EXPLICIT(name, order) \
    (name ## _sc_atomic__)

/**
 *  \brief Set the value for the atomic variable.
 *
 *  \retval var value
 */
#define TC_ATOMIC_SET(name, val) ({       \
    while (TC_ATOMIC_CAS(&name, TC_ATOMIC_GET(name), val) == 0) \
        ;                                                       \
        })

#endif /* no c11 atomics */
/* atomic end*************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/




/*************************************************************************************************************************/
/*************************************************************************************************************************/
/* profiling start */

uint64_t cpu_ticks_get(void);

typedef enum {
	TC_CNT_DEQ = 0,
	TC_CNT_DO_FWD,
	TC_CNT_SRV_NEW,
	TC_CNT_CLI_NEW,
	TC_CNT_SOCKET_SEND,
	TC_CNT_MEM_FREE,
	TC_CNT_ENQ,
	
	TC_CNT_MAX,
}TC_CNT_E;


//#define DEBUG_TICKS 1
#ifdef DEBUG_TICKS

#define TC_TICKS_INIT() \
	uint64_t start = 0;\
	uint64_t end = 0;

#define TC_TICKS_GET(val) \
	val = cpu_ticks_get()

#define TC_TICKS_ADD(type, start, end)\
	tc_tick_cnt_add(type, start, end)

#define TC_TICKS_DUMP()  \
	tc_tick_cnt_dump()

#else

#define TC_TICKS_INIT() 

#define TC_TICKS_GET(val)

#define TC_TICKS_ADD(type, start, end)

#define TC_TICKS_DUMP()

#endif

int tc_tick_cnt_file_dump(FILE *fp);


#endif

