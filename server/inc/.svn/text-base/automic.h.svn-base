/*
 * automic.h
 *
 *  Created on: 2016年9月2日
 *      Author: Administrator
 */

#ifndef INC_AUTOMIC_H_
#define INC_AUTOMIC_H_
/* Check GCC version, just to be safe */
#if !defined(__GNUC__) || (__GNUC__ < 4) || (__GNUC_MINOR__ < 1)
	# error atomic.h works only with GCC newer than version 4.1
#endif /* GNUC >= 4.1 */

/*
ATOMIC_INIT(int i)                           // 定义原子变量时，将其值赋为i
int atomic_read(atomic_t *v)                 // 读v的值
void atomic_set(atomic_t *v, int i)          // 设置v的值为i
void atomic_add(int i, atomic *v)            // v的值增加i
void atomic_sub(int i, atomic *v)            // v的值减少i
void atomic_inc(atomic *v)                   // v的值加1
void atomic_dec(atomic *v)                   // v的值减1
int atomic_sub_and_test(int i, atomic_t *v)  // v的值减少i,且结果为0时返回true
int atomic_add_negative(int i, atomic_t *v)  // v的值增加i,且结果为负数时返回true
int atomic_add_return(int i, atomic_t *v)    // v的值增加i,且返回结果
int atomic_sub_return(int i, atomic_t *v)    // v的值减少i,且返回结果
int atomic_inc_return(atomic_t *v)           // v的值加1，且返回结果
int atomic_dec_return(atomic_t *v)           // v的值减1，且返回结果
int atomic_dec_and_test(atomic_t *v)         // v的值减1，且结果为0时返回true
int atomic_inc_and_test(atomic_t *v)         // v的值加1，且结果为0时返回true
 */

/**
 * Atomic type.
 */
typedef struct {
	volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i)  { (i) }

/**
 * Read atomic variable
 * @param v pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
#define atomic_read(v) ((v)->counter)

/**
 * Set atomic variable
 * @param v pointer of type atomic_t
 * @param i required value
 */
#define atomic_set(v,i) (((v)->counter) = (i))

/**
 * Add to the atomic variable
 * @param i integer value to add
 * @param v pointer of type atomic_t
 */
static inline void atomic_add( int i, atomic_t *v )
{
	(void)__sync_add_and_fetch(&v->counter, i);
}

/**
 * Subtract the atomic variable
 * @param i integer value to subtract
 * @param v pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static inline void atomic_sub( int i, atomic_t *v )
{
	(void)__sync_sub_and_fetch(&v->counter, i);
}

/**
 * Subtract value from variable and test result
 * @param i integer value to subtract
 * @param v pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_sub_and_test( int i, atomic_t *v )
{
	return !(__sync_sub_and_fetch(&v->counter, i));
}

/**
 * Increment atomic variable
 * @param v pointer of type atomic_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic_inc( atomic_t *v )
{
	(void)__sync_fetch_and_add(&v->counter, 1);
}

/**
 * @brief decrement atomic variable
 * @param v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
static inline void atomic_dec( atomic_t *v )
{
	(void)__sync_fetch_and_sub(&v->counter, 1);
}

/**
 * @brief Decrement and test
 * @param v pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline int atomic_dec_and_test( atomic_t *v )
{
	return !(__sync_sub_and_fetch(&v->counter, 1));
}

/**
 * @brief Increment and test
 * @param v pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic_inc_and_test( atomic_t *v )
{
	return !(__sync_add_and_fetch(&v->counter, 1));
}

/**
 * @brief add and test if negative
 * @param v pointer of type atomic_t
 * @param i integer value to add
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static inline int atomic_add_negative( int i, atomic_t *v )
{
	return (__sync_add_and_fetch(&v->counter, i) < 0);
}

#endif /* INC_AUTOMIC_H_ */
