#ifndef SIGWRAPPER_H
#define SIGWRAPPER_H

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <string>

using namespace std;

string signedMsgToAddress(string message, string sig);

#endif // SIGWARPPER_H