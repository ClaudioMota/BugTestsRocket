// This header file was generated by BugTestsRocket test framework
// It must be included after test.h and any eventual required typedefs
#ifdef BUG_TESTS_ROCKET_TEST_HEADER
#ifdef __cplusplus
extern "C"{
#define _BTR_CONVERT(what, to) reiterpret_cast<to>(what)
#else
#define _BTR_CONVERT(what, to) ((to)(what))
#endif
int 🐛getRandomInput🚀();
void* _mocked_getRandomInput = _BTR_CONVERT(🐛getRandomInput🚀, void*);
void 🐛_ZN7MyClass15internalProcessEv🚀(void*);
void* _mocked__ZN7MyClass15internalProcessEv = _BTR_CONVERT(🐛_ZN7MyClass15internalProcessEv🚀, void*);
int getRandomInput(){ _mocks[0].calls++; return _BTR_CONVERT(_mocked_getRandomInput, int (*)())(); }
void _ZN7MyClass15internalProcessEv(void* a0){ _mocks[1].calls++; return _BTR_CONVERT(_mocked__ZN7MyClass15internalProcessEv, void (*)(void*))(a0); }
FunctionMock _mocks[] = {
  {true, (int)0, (void*)&_mocked_getRandomInput, "getRandomInput"},
  {true, (int)0, (void*)&_mocked__ZN7MyClass15internalProcessEv, "_ZN7MyClass15internalProcessEv"},
  {false, (int)0, (void*)0, ""}
};
#ifdef __cplusplus
}
#endif
#endif
