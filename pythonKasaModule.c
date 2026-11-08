//pythonKasaModule.c


//g++ -shared -fPIC pythonKasaModule.c kasaLight.cpp     $(python3-config --cflags)     $(python3-config --ldflags --embed)     -o pythonKasaModule.cpython-312-x86_64-linux-gnu.so

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "kasaLight.h"
#include <string.h>
#include <iostream>

typedef struct{
    PyObject_HEAD
    kasaLight* cLight;
} pyLight;


//destroy object in Python and C
static void lightDealloc(pyLight* self){

    delete self->cLight;
    Py_TYPE(self)->tp_free((PyObject*)self); //free python mem

}


//init Python object
static PyObject* pyLightNew(PyTypeObject* type, PyObject*, PyObject*){

    pyLight* self = (pyLight*)type->tp_alloc(type, 0);
    self->cLight = nullptr;

    return (PyObject*)self;

}

//init c++ object
static int pyLightInit(pyLight* self, PyObject* args){

    const char* ip;
    if (!PyArg_ParseTuple(args, "s", &ip))
        return -1;


    std::string* s_ip = new std::string(ip);
    self->cLight = new kasaLight(*s_ip);
    delete s_ip;

    return(0);
}


//methods -----------------------------------------------------------------------

//light on
static PyObject* pyLightON(pyLight* self, PyObject* args){

    if(self->cLight->ON()){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;

}

//light off
static PyObject* pyLightOFF(pyLight* self, PyObject* args){

    if(self->cLight->OFF()){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;


}

//blink
static PyObject* pyLightBlink(pyLight* self, PyObject* args){

    if(self->cLight->blink()){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;


}

static PyObject* pyLightSetWhite(pyLight* self, PyObject* args){

    long value = PyLong_AsLong(args);

    if (value == -1 && PyErr_Occurred()) {
        return NULL;
    }

    if(self->cLight->setWhite((int)value)){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;

}


static PyObject* pyLightSetBrightness(pyLight* self, PyObject* args){

    long value = PyLong_AsLong(args);

    if (value == -1 && PyErr_Occurred()) {
        return NULL;
    }
  
    if(self->cLight->setBrightness((int)value)){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;

}


static PyObject* pyLightGetPower(pyLight* self, PyObject* args){

    if(self->cLight->getPower()){
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;

}


//light info
static PyObject* pyLightInfo(pyLight* self, PyObject* args){

    std::string s = self->cLight->getInfoString();
    return PyUnicode_FromStringAndSize(s.c_str(), s.size()); 

}


static PyMethodDef methods[] = {
    {"on", (PyCFunction)pyLightON, METH_NOARGS, "Turn light on"},
    {"off", (PyCFunction)pyLightOFF, METH_NOARGS, "Turn light off"},
    {"getInfo", (PyCFunction)pyLightInfo, METH_NOARGS, "Get bulb info"},
    {"blink", (PyCFunction)pyLightBlink, METH_NOARGS, "Blink light"},
    {"getPower", (PyCFunction)pyLightGetPower, METH_NOARGS, "Get power state"},
    {"setBrightness", (PyCFunction)pyLightSetBrightness, METH_O, "Set bulb brightness"},
    {"setWhite", (PyCFunction)pyLightSetWhite, METH_O, "Set bulb white level"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef pythonKasaModule = {
    PyModuleDef_HEAD_INIT,
    "pythonKasaModule",
    "Kasa Light Module",
    -1,
    methods
};

//set up object type
static PyTypeObject pyLightType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pythonKasaModule.pyLight",
    .tp_basicsize = sizeof(pyLight),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)lightDealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Kasa Light Objects",
    .tp_methods = methods, 
    .tp_init = (initproc)pyLightInit,
    .tp_new = pyLightNew,
};


// python entry point
PyMODINIT_FUNC PyInit_pythonKasaModule(void) {
    PyObject* m;

    if (PyType_Ready(&pyLightType) < 0)
        return NULL;

    m = PyModule_Create(&pythonKasaModule);
    if (m == NULL)
        return NULL;

    //protect class definition from garbage collector
    Py_INCREF(&pyLightType);

    //add pylight class ot module
    if (PyModule_AddObject(m, "pyLight", (PyObject*)&pyLightType) < 0) {
        Py_DECREF(&pyLightType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
