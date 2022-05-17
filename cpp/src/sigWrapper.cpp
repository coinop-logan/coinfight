#include <iostream>
#include "sigWrapper.h"

using namespace std;

string getPythonErrorText()
{
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    if (pvalue)
        {
        PyObject *pstr = PyObject_Str(pvalue);
        if (pstr)
        {
            const char* err_msg = PyUnicode_AsUTF8(pstr);
            if(pstr)
            return string(err_msg);
        }
    }
    return "... but I can't get the error string for some reason :/";
}

optional<string> signedMsgToAddressString(string message, string sig, string *error)
{
    Py_Initialize();

    PyRun_SimpleString("import sys, os");
    PyRun_SimpleString("sys.path.append(os.getcwd())");

    PyObject* module = PyImport_ImportModule("signed_msg_to_address");
    if (!module)
    {
        *error = "Python error: error importing module: " + getPythonErrorText();
        return {};
    }

    PyObject* function = PyObject_GetAttrString(module,(char*)"signed_msg_to_address");
    if (!function)
    {
        *error = "Python error: error loading functino signed_msg_to_address: " + getPythonErrorText();
        return {};
    }

    PyObject* result = PyObject_CallFunction(function, "ss", message.c_str(), sig.c_str());
    if (!result)
    {
        *error = "Python error: error calling function signed_msg_to_address: " + getPythonErrorText();
        return {};
    }

    const char* resultCStr = PyUnicode_AsUTF8(result);
    return {string(resultCStr)};
}