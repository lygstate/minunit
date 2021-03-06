/*
* Copyright (c) 2016 Yonggang Luo, luoyonggang@gmail.com
* Copyright (c) 2012 David Siñuela Pastor, siu.4coders@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __MINUNIT_H__
#define __MINUNIT_H__

#include <stdio.h>
#include <math.h>

#if defined(_WIN32)

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#define __func__ __FUNCTION__
#endif

#endif /* _WIN32 */

#define MU_EXPAND(x) x

#define MU_ARG_LENGTH_(_63, _62, _61, _60, _59, _58, _57, _56, _55, _54, _53,          \
            _52, _51, _50, _49, _48, _47, _46, _45, _44, _43, _42, _41, _40, _39, _38, \
            _37, _36, _35, _34, _33, _32, _31, _30, _29, _28, _27, _26, _25, _24, _23, \
            _22, _21, _20, _19, _18, _17, _16, _15, _14, _13, _12, _11, _10, _9, _8,   \
            _7, _6, _5, _4, _3, _2, _1, count, ...)     count

#define MU_ARG_LENGTH(...)                                                              \
    MU_EXPAND(MU_ARG_LENGTH_(__VA_ARGS__,                                               \
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
            44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, \
            25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, \
            5, 4, 3, 2, 1, 0))


#ifdef __GNUC__
#define GLOBAL_STAIC_CONSTRUCT(construct) \
static void construct ## _constructor(void) __attribute__((constructor)); \
static void construct ## _constructor(void) { construct(); } \


#define GLOBAL_STAIC_DESTRUCT(destruct) \
static void destruct ## _destructor(void) __attribute__((destructor)); \
static void destruct ## _destructor(void) { destruct(); }

#elif defined (_MSC_VER)

/* Only Vs 2008 and Upper support for __pragma keyword */
/* https://msdn.microsoft.com/en-us/library/d9x1s805(v=vs.90).aspx */
#if _MSC_VER>=1500

#ifdef _M_X64
#define INCLUDE_SYM(s) comment(linker, "/include:" #s)
#else
#define INCLUDE_SYM(s) comment(linker, "/include:_" #s)
#endif

#define MSVC_DECL_SECTION_FUNCTION_POINTER(sec_name, func_name) \
__pragma(data_seg(push)) \
__pragma(section(sec_name, read)) \
__declspec(allocate(sec_name)) \
int (__cdecl *func_name ## _section)(void) = func_name; \
__pragma(data_seg(pop)) \
__pragma(INCLUDE_SYM(func_name ## _section))


#define GLOBAL_STAIC_CONSTRUCT(construct) \
static int __cdecl construct ## _constructor(void) { construct(); return 0; } \
MSVC_DECL_SECTION_FUNCTION_POINTER(".CRT$XCU", construct ## _constructor)

#define GLOBAL_STAIC_DESTRUCT(destruct) \
static int __cdecl destruct ## _destructor(void) { destruct(); return 0; } \
MSVC_DECL_SECTION_FUNCTION_POINTER(".CRT$XTU", destruct ## _destructor)

#else
#message "Visual Studio 2005 and lower unsupported"
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*  Maximum length of last message */
#define MINUNIT_MESSAGE_LEN 1024
/*  Accuracy with which floats are compared */
#define MINUNIT_EPSILON 1E-12

typedef void(*test_function_t)();

/*  Test setup and teardown function pointers for test suite*/
typedef struct test_suite_info_t {
  const char* suite_name;
  test_function_t setup;
  test_function_t teardown;
} test_suite_info_t;

typedef struct test_function_info_t {
  const char* suite_name;
  const char* test_name;
  test_suite_info_t* suite;
  test_function_t test_ptr;
  double timer_real;
  double timer_cpu;
  int assert_count;
} test_function_info_t;

/*  Misc. counters */
extern int minunit_assert;
extern int minunit_status;

extern char* mu_get_last_message();
extern double mu_timer_real();
extern double mu_timer_cpu();

extern void mu_add_test(const char* suite_name, const char* test_name, test_function_t test_ptr);
extern void mu_add_suite(const char* suite_name, test_function_t setup, test_function_t teardown);

#define mu_last_message mu_get_last_message()

/*  MU Test without suite */
#define _MU_TEST_1(method_name) \
static void method_name(); \
static void method_name ## _add_test() { \
  mu_add_test(NULL, "" #method_name, method_name); \
} \
GLOBAL_STAIC_CONSTRUCT(method_name ## _add_test); \
static void method_name()

/*  MU Test with suite */
#define _MU_TEST_2(suite_name, method_name) \
static void method_name(); \
static void method_name ## _add_test() { \
  mu_add_test("" #suite_name, "" #method_name, method_name); \
} \
GLOBAL_STAIC_CONSTRUCT(method_name ## _add_test) \
static void method_name()

#define MU_TEST_APPLY__(n, ...) MU_EXPAND(_MU_TEST_ ## n(__VA_ARGS__))

#define MU_TEST_APPLY_(n, ...) MU_EXPAND(MU_TEST_APPLY__(n, __VA_ARGS__))

#define MU_TEST(...) MU_EXPAND(MU_TEST_APPLY_(MU_ARG_LENGTH(__VA_ARGS__), __VA_ARGS__))

#define MU_TEST_SUITE(suite_name, setup_fun, teardown_fun) \
static void suite_name ## _add_suite() { \
  mu_add_suite("" #suite_name, setup_fun, teardown_fun); \
} \
GLOBAL_STAIC_CONSTRUCT(suite_name ## _add_suite)

#define MU__SAFE_BLOCK(block) do {\
  block\
} while(0)

/*  Assertions */
#define mu_check(test) MU__SAFE_BLOCK(\
  minunit_assert++;\
  if (!(test)) {\
    snprintf(mu_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %s", __func__, __FILE__, __LINE__, #test);\
    minunit_status = 1;\
    return;\
  } else {\
    printf(".");\
  }\
)

#define mu_fail(message) MU__SAFE_BLOCK(\
  minunit_assert++;\
  snprintf(mu_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %s", __func__, __FILE__, __LINE__, message);\
  minunit_status = 1;\
  return;\
)

#define mu_assert(test, message) MU__SAFE_BLOCK(\
  minunit_assert++;\
  if (!(test)) {\
    snprintf(mu_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %s", __func__, __FILE__, __LINE__, message);\
    minunit_status = 1;\
    return;\
  } else {\
    printf(".");\
  }\
)

#define mu_assert_int_eq(expected, result) MU__SAFE_BLOCK(\
  int minunit_tmp_e;\
  int minunit_tmp_r;\
  minunit_assert++;\
  minunit_tmp_e = (expected);\
  minunit_tmp_r = (result);\
  if (minunit_tmp_e != minunit_tmp_r) {\
    snprintf(mu_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %d expected but was %d", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_r);\
    minunit_status = 1;\
    return;\
  } else {\
    printf(".");\
  }\
)

#define mu_assert_double_eq(expected, result) MU__SAFE_BLOCK(\
  double minunit_tmp_e;\
  double minunit_tmp_r;\
  minunit_assert++;\
  minunit_tmp_e = (expected);\
  minunit_tmp_r = (result);\
  if (fabs(minunit_tmp_e-minunit_tmp_r) > MINUNIT_EPSILON) {\
    int minunit_significant_figures = 1 - (int)log10(MINUNIT_EPSILON);\
    snprintf(mu_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %.*g expected but was %.*g", __func__, __FILE__, __LINE__, minunit_significant_figures, minunit_tmp_e, minunit_significant_figures, minunit_tmp_r);\
    minunit_status = 1;\
    return;\
  } else {\
    printf(".");\
  }\
)

#ifdef __cplusplus
}
#endif

#endif /* __MINUNIT_H__ */
