#include <iostream>
#include "sigWrapper.h"

using namespace std;

string signedMsgToAddress(string message, string sig)
{
    Py_Initialize();

    PyRun_SimpleString("import sys, os");
    PyRun_SimpleString("sys.path.append(os.getcwd())");

    PyObject* module = PyImport_ImportModule("signed_msg_to_address");
    if (!module)
    {
        cout << "oops" << endl;
        PyErr_Print();
    }

    PyObject* function = PyObject_GetAttrString(module,(char*)"signed_msg_to_address");
    if (!function)
    {
        cout << "oops" << endl;
        PyErr_Print();
    }

    PyObject* result = PyObject_CallFunction(function, "ss", message.c_str(), sig.c_str());
    if (!result)
    {
        cout << "oops" << endl;
        PyErr_Print();
    }

    const char* resultCStr = PyUnicode_AsUTF8(result);

    return(string(resultCStr));
    // return string("");
}