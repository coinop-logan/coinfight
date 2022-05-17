#ifndef SIGWRAPPER_H
#define SIGWRAPPER_H

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <string>
#include <optional>

using namespace std;

optional<string> signedMsgToAddressString(string message, string sig, string *error);

#endif // SIGWARPPER_H