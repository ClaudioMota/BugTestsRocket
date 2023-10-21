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

#ifndef TEST_HEADER
#define TEST_HEADER 1

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
#define assert(boolean) _assert(_currentEnv, boolean, __LINE__, #boolean)
#define refute(boolean) _assert(_currentEnv, !(boolean), __LINE__, #boolean)
#define context(name) _context = name;

#define beginTests \
  int _allTests(TestEnvironment* testEnv){ char* _context = ""; if(_context){}; int _testCount = 0; int _testRunning = 0; {

#define _finishLastScope() if(_testRunning > 0){ _testRunning--; onTestPass(testEnv); } }

#define test(description) \
  _finishLastScope()\
  testEnv->testIndex = _testCount++;\
  testEnv->testDescription = description;\
  testEnv->testLine = __LINE__;\
  testEnv->testContext = _context;\
  if(_shouldRunTest(testEnv)){\
    if(_testRunning > 0){ printf("\nError nested tests detected %s:%i\n", _sourceFile, __LINE__); }\
    _testRunning++;\
    setupFunction(testEnv);

#define mock(function, newFunction) _mock(#function, newFunction, _mocks);

#define endTests _finishLastScope() return _testCount; }\
  int main(int numArgs, char** args){\
    _sourceFile = __FILE__;\
    return _testFileMain(numArgs, args, _allTests);\
  }
  