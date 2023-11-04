// MIT License

// Copyright (c) 2023 Claudio Mota Oliveira (cmoliveira1000@gmail.com)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// See https://github.com/ClaudioMota/BugTestsRocket/ for updated versions
// If this file is incomplete try running _internal/mergeHeaders.sh before copying

#ifndef BUG_TESTS_ROCKET_TEST_HEADER
#define BUG_TESTS_ROCKET_TEST_HEADER 1

#ifdef __cplusplus
extern "C"
{
#define _C_STRING_LITERAL(literal) (char*)literal
#define class struct
#define private public
#define protected public
#else
#define _C_STRING_LITERAL(literal) literal
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define 🐛 beginTests
#define 🚀 endTests

#define _TEST_HELPER_BLOCK_SIZE 1024
#define assert(boolean) _assert(_C_STRING_LITERAL(__FILE__), __LINE__, boolean, _C_STRING_LITERAL(#boolean))
#define refute(boolean) _assert(_C_STRING_LITERAL(__FILE__), __LINE__, !(boolean), _C_STRING_LITERAL(#boolean))

#define beginTests \
  int _allTests(){ int _testCount = 0; int _testRunning = 0; int _testDefinition = 0; {

#define _finishLastScope() if(_testRunning > 0){ _testRunning--; onTestPass(); } }\
  if(_testDefinition > 0){ _testDefinition--;\
  if(_testDefinition != 0) onFail(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL("test scope has been compromised"));}\
  
#define context(name) _finishLastScope() _setContext(_C_STRING_LITERAL(name)); {

#define test(description) \
  _finishLastScope()\
  _testDefinition++;\
  if(_shouldRunTest(_testCount++, __LINE__, testEnv->_candidateContext)){\
    _initializeTest(_testCount-1, __LINE__, _C_STRING_LITERAL(description));\
    _testRunning++;\
    setupFunction();

#define mock(function, newFunction) _mock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), (void*)newFunction, _mocks)

#define mockReset(function) _mockReset(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), (void*)newFunction, _mocks)

#define mockCalls(function) _getMock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), _mocks)->calls

#define mockGetOrginal(function) _getMock(_C_STRING_LITERAL(__FILE__), __LINE__, _C_STRING_LITERAL(#function), _mocks)->original

#define helperBlockAs(env, type, index) (type*)&(((char*)env->helperBlock)[sizeof(type)*index])

#define endTests _finishLastScope() return _testCount; }\
  int main(int numArgs, char** args){\
    _sourceFile = _C_STRING_LITERAL(__FILE__);\
    return _testFileMain(numArgs, args, _allTests);\
  }

#define _BTR_MAX_NAME_SIZE 512