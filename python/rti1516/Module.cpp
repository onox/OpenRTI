/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Python.h>

#include <map>
#include <set>
#include "RTI/RTI1516.h"
#include "RTI/FederateAmbassador.h"
#include "RTI/HLAfloat64Time.h"
#include "RTI/HLAfloat64Interval.h"
#include "RTI/HLAinteger64Time.h"
#include "RTI/HLAinteger64Interval.h"

class PySharedPtr {
public:
  PySharedPtr(void) : _ptr(0)
  {}
  PySharedPtr(const PySharedPtr& p) : _ptr(0)
  { assign(_ptr); }
  ~PySharedPtr(void)
  { put(); }

  PySharedPtr& operator=(const PySharedPtr& p)
  { assign(p.get()); return *this; }

  PyObject* operator->(void) const
  { return _ptr; }

  PyObject& operator*(void) const
  { return *_ptr; }

  PyObject* get() const
  { return _ptr; }

  bool valid(void) const
  { return 0 != _ptr; }

  void clear()
  { put(); }

  PySharedPtr& setBorrowedRef(PyObject* p)
  { assign(p); return *this; }
  PySharedPtr& setNewRef(PyObject* p)
  { assignNonRef(p); return *this; }

private:
  void assign(PyObject* p)
  { if (p) Py_IncRef(p); put(); _ptr = p; }
  void assignNonRef(PyObject* p)
  { put(); _ptr = p; }
  void put(void)
  { if (!_ptr) return; Py_DecRef(_ptr); _ptr = 0; }

  // The reference itself.
  PyObject* _ptr;
};

static PyObject*
PyObject_NewString(const std::wstring& string)
{
  return PyUnicode_FromWideChar(string.c_str(), string.size());
}

static bool
PyObject_GetString(std::wstring& string, PyObject* o)
{
#if PY_MAJOR_VERSION < 3
  if (PyString_Check(o)) {
    Py_ssize_t size = PyString_Size(o);
    string.resize(size);
    if (size) {
      const char* ptr = PyString_AsString(o);
      std::copy(ptr, ptr + size, string.begin());
    }
    return true;

  } else
#else
  if (PyBytes_Check(o)) {
    Py_ssize_t size = PyBytes_Size(o);
    string.resize(size);
    if (size) {
      const char* ptr = PyBytes_AsString(o);
      std::copy(ptr, ptr + size, string.begin());
    }
    return true;

  } else
#endif
  if (PyUnicode_Check(o)) {
#if PY_VERSION_HEX < 0x03020000
    Py_ssize_t size = PyUnicode_GetSize(o);
    string.resize(size);
    if (size) {
      const Py_UNICODE* ptr = PyUnicode_AsUnicode(o);
      std::copy(ptr, ptr + size, string.begin());
    }
#else
    Py_ssize_t size;
    wchar_t* ptr = PyUnicode_AsWideCharString(o, &size);
    if (!ptr)
      return false;

    string.resize(size);
    std::copy(ptr, ptr + size, string.begin());

    PyMem_Free(ptr);
#endif
    return true;

  } else {

    return false;
  }
}

// Don't care for errors, just try to get something printable
static std::wstring
PyObject_GetString(PyObject* o)
{
  std::wstring string;
  if (PyObject_GetString(string, o))
    return string;

  PyObject* r = PyObject_Repr(o);
  if (!r)
    return string;

  PyObject_GetString(string, r);
  Py_DecRef(r);
  return string;
}

static PyObject*
PyObject_NewVariableLengthData(const rti1516::VariableLengthData& variableLengthData)
{
  return PyByteArray_FromStringAndSize(static_cast<const char *>(variableLengthData.data()), variableLengthData.size());
}

static bool
PyObject_GetVariableLengthData(rti1516::VariableLengthData& variableLengthData, PyObject* o)
{
  if (PyByteArray_Check(o)) {
    variableLengthData.setData(PyByteArray_AsString(o), PyByteArray_Size(o));
    return true;
  } else {
    PyObject *bytearray = PyByteArray_FromObject(o);
    if (!bytearray)
      return false;
    variableLengthData.setData(PyByteArray_AsString(bytearray), PyByteArray_Size(bytearray));
    Py_DecRef(bytearray);
    return true;
  }
}

static bool
PyObject_GetInt(long& value, PyObject* o)
{
  PyObject* i = PyNumber_Long(o);
  if (!i)
    return false;
  value = PyLong_AsLong(i);
  Py_DecRef(i);
  return true;
}

static bool
PyObject_GetLong(PY_LONG_LONG& value, PyObject* o)
{
  PyObject* i = PyNumber_Long(o);
  if (!i)
    return false;
  value = PyLong_AsLongLong(i);
  Py_DecRef(i);
  return true;
}

static bool
PyObject_GetDouble(double& value, PyObject* o)
{
  PyObject* f = PyNumber_Float(o);
  if (!f)
    return false;
  value = PyFloat_AsDouble(f);
  Py_DecRef(f);
  return true;
}

static PyObject*
PyObject_NewTransportationType(const rti1516::TransportationType& transportationType)
{
  return PyLong_FromLong(transportationType);
}

static bool
PyObject_GetTransportationType(rti1516::TransportationType& transportationType, PyObject* o)
{
  long value;
  if (PyObject_GetInt(value, o)) {
    if (value == rti1516::BEST_EFFORT)
      transportationType = rti1516::BEST_EFFORT;
    else
      transportationType = rti1516::RELIABLE;

    return true;
  } else {
    std::wstring string;
    if (PyObject_GetString(string, o)) {
      if (string == L"BEST_EFFORT")
        transportationType = rti1516::BEST_EFFORT;
      else
        transportationType = rti1516::RELIABLE;

      return true;
    }
  }
  return false;
}

static PyObject*
PyObject_NewOrderType(const rti1516::OrderType& orderType)
{
  return PyLong_FromLong(orderType);
}

static bool
PyObject_GetOrderType(rti1516::OrderType& orderType, PyObject* o)
{
  long value;
  if (PyObject_GetInt(value, o)) {
    if (value == rti1516::TIMESTAMP)
      orderType = rti1516::TIMESTAMP;
    else
      orderType = rti1516::RECEIVE;

    return true;
  } else {
    std::wstring string;
    if (PyObject_GetString(string, o)) {
      if (string == L"TIMESTAMP")
        orderType = rti1516::TIMESTAMP;
      else
        orderType = rti1516::RECEIVE;
      return true;
    }
  }
  return false;
}

static bool
PyObject_GetServiceGroupIndicator(rti1516::ServiceGroupIndicator& serviceGroup, PyObject* o)
{
  long value;
  if (PyObject_GetInt(value, o)) {
    if (value == rti1516::FEDERATION_MANAGEMENT)
      serviceGroup = rti1516::FEDERATION_MANAGEMENT;
    else if (value == rti1516::DECLARATION_MANAGEMENT)
      serviceGroup = rti1516::DECLARATION_MANAGEMENT;
    else if (value == rti1516::OBJECT_MANAGEMENT)
      serviceGroup = rti1516::OBJECT_MANAGEMENT;
    else if (value == rti1516::OWNERSHIP_MANAGEMENT)
      serviceGroup = rti1516::OWNERSHIP_MANAGEMENT;
    else if (value == rti1516::TIME_MANAGEMENT)
      serviceGroup = rti1516::TIME_MANAGEMENT;
    else if (value == rti1516::DATA_DISTRIBUTION_MANAGEMENT)
      serviceGroup = rti1516::DATA_DISTRIBUTION_MANAGEMENT;
    else if (value == rti1516::SUPPORT_SERVICES)
      serviceGroup = rti1516::SUPPORT_SERVICES;
    else
      serviceGroup = rti1516::SUPPORT_SERVICES;

    return true;
  } else {
    std::wstring string;
    if (PyObject_GetString(string, o)) {
      if (string == L"FEDERATION_MANAGEMENT")
        serviceGroup = rti1516::FEDERATION_MANAGEMENT;
      else if (string == L"DECLARATION_MANAGEMENT")
        serviceGroup = rti1516::DECLARATION_MANAGEMENT;
      else if (string == L"OBJECT_MANAGEMENT")
        serviceGroup = rti1516::OBJECT_MANAGEMENT;
      else if (string == L"OWNERSHIP_MANAGEMENT")
        serviceGroup = rti1516::OWNERSHIP_MANAGEMENT;
      else if (string == L"TIME_MANAGEMENT")
        serviceGroup = rti1516::TIME_MANAGEMENT;
      else if (string == L"DATA_DISTRIBUTION_MANAGEMENT")
        serviceGroup = rti1516::DATA_DISTRIBUTION_MANAGEMENT;
      else if (string == L"SUPPORT_SERVICES")
        serviceGroup = rti1516::SUPPORT_SERVICES;
      else
        serviceGroup = rti1516::SUPPORT_SERVICES;

      return true;
    }
  }
  return false;
}

static PyObject*
PyObject_NewLogicalTime(const rti1516::LogicalTime& logicalTime)
{
  if (logicalTime.implementationName() == L"HLAfloat64Time") {
    return PyFloat_FromDouble(static_cast<const HLAfloat64Time&>(logicalTime).getTime());
  } else if (logicalTime.implementationName() == L"HLAinteger64Time") {
    return PyLong_FromLongLong(static_cast<const HLAinteger64Time&>(logicalTime).getTime());
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return 0;
  }
}

static bool
PyObject_GetLogicalTime(RTI_UNIQUE_PTR<rti1516::LogicalTime>& logicalTime, const std::wstring& implementationName)
{
  if (implementationName == L"HLAfloat64Time") {
    logicalTime.reset(new HLAfloat64Time(0));
    return true;
  } else if (implementationName == L"HLAinteger64Time") {
    logicalTime.reset(new HLAinteger64Time(0));
    return true;
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return false;
  }
}

static bool
PyObject_GetLogicalTime(RTI_UNIQUE_PTR<rti1516::LogicalTime>& logicalTime, PyObject* o, const std::wstring& implementationName)
{
  if (implementationName == L"HLAfloat64Time") {
    double value;
    if (!PyObject_GetDouble(value, o))
      return false;
    logicalTime.reset(new HLAfloat64Time(value));
    return true;
  } else if (implementationName == L"HLAinteger64Time") {
    PY_LONG_LONG value;
    if (!PyObject_GetLong(value, o))
      return false;
    logicalTime.reset(new HLAinteger64Time(value));
    return true;
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return false;
  }
}

static PyObject*
PyObject_NewLogicalTimeInterval(const rti1516::LogicalTimeInterval& logicalTimeInterval)
{
  if (logicalTimeInterval.implementationName() == L"HLAfloat64Time") {
    return PyFloat_FromDouble(static_cast<const HLAfloat64Interval&>(logicalTimeInterval).getInterval());
  } else if (logicalTimeInterval.implementationName() == L"HLAinteger64Time") {
    return PyLong_FromLongLong(static_cast<const HLAinteger64Interval&>(logicalTimeInterval).getInterval());
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return 0;
  }
}

static bool
PyObject_GetLogicalTimeInterval(RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>& logicalTimeInterval, PyObject* o, const std::wstring& implementationName)
{
  if (implementationName == L"HLAfloat64Time") {
    double value;
    if (!PyObject_GetDouble(value, o))
      return false;
    logicalTimeInterval.reset(new HLAfloat64Interval(value));
    return true;
  } else if (implementationName == L"HLAinteger64Time") {
    PY_LONG_LONG value;
    if (!PyObject_GetLong(value, o))
      return false;
    logicalTimeInterval.reset(new HLAinteger64Interval(value));
    return true;
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return false;
  }
}

static bool
PyObject_GetLogicalTimeInterval(RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>& logicalTimeInterval, const std::wstring& implementationName)
{
  if (implementationName == L"HLAfloat64Time") {
    logicalTimeInterval.reset(new HLAfloat64Interval(0));
    return true;
  } else if (implementationName == L"HLAinteger64Time") {
    logicalTimeInterval.reset(new HLAinteger64Interval(0));
    return true;
  } else {
    // FIXME would be possible to models something like the abstract time factory also here in python
    return false;
  }
}

static long
VariableLengthData_hash(const rti1516::VariableLengthData& variableLengthData)
{
  // Almost taken from pythons string hash, but should be ok for the encoded handles as a hash
  unsigned long len = variableLengthData.size();
  unsigned char *p = (unsigned char *) variableLengthData.data();
  long x = *p << 7;
  while (0 < len--)
    x = (1000003*x) ^ *p++;
  x ^= variableLengthData.size();
  if (x == -1)
    x = -2;
  return x;
}

static PyObject*
PyObject_NewRangeBounds(const rti1516::RangeBounds& rangeBounds)
{
  PyObject *first = PyLong_FromLongLong(rangeBounds.getLowerBound());
  if (!first)
    return 0;
  PyObject *second = PyLong_FromLongLong(rangeBounds.getUpperBound());
  if (!second) {
    Py_DecRef(first);
    return 0;
  }
  PyObject *pair = PyTuple_Pack(2, first, second);
  if (!pair) {
    Py_DecRef(second);
    Py_DecRef(first);
    return 0;
  }
  return pair;
}

static bool
PyObject_GetRangeBounds(rti1516::RangeBounds& rangeBounds, PyObject* o)
{
  if (!PySequence_Check(o))
    return false;
  if (PySequence_Size(o) != 2)
    return false;

  PyObject* first = PySequence_GetItem(o, 0);
  PY_LONG_LONG value;
  if (!PyObject_GetLong(value, first)) {
    Py_DecRef(first);
    return false;
  }
  Py_DecRef(first);
  rangeBounds.setLowerBound(value);

  first = PySequence_GetItem(o, 1);
  if (!PyObject_GetLong(value, first)) {
    Py_DecRef(first);
    return false;
  }
  Py_DecRef(first);
  rangeBounds.setUpperBound(value);
  return true;
}

#define IMPLEMENT_HANDLE_CLASS(HandleKind)                              \
                                                                        \
  struct Py ## HandleKind {                                             \
    PyObject_HEAD                                                       \
    rti1516::HandleKind ob_value;                                       \
    long ob_hash;                                                       \
  };                                                                    \
                                                                        \
  static void                                                           \
  HandleKind ## _dealloc(Py ## HandleKind *o)                           \
  {                                                                     \
    o->ob_value.rti1516::HandleKind::~HandleKind();                     \
    Py_TYPE(o)->tp_free(o);                                             \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _richcmp(Py ## HandleKind *o0, Py ## HandleKind *o1,    \
                         int op)                                        \
  {                                                                     \
    switch (op) {                                                       \
    case Py_LT:                                                         \
      if (o0->ob_value < o1->ob_value)                                  \
        return PyLong_FromLong(1);                                      \
      else                                                              \
        return PyLong_FromLong(0);                                      \
      break;                                                            \
    case Py_LE:                                                         \
      if (o1->ob_value < o0->ob_value)                                  \
        return PyLong_FromLong(0);                                      \
      else                                                              \
        return PyLong_FromLong(1);                                      \
      break;                                                            \
    case Py_EQ:                                                         \
      if (o0->ob_value == o1->ob_value)                                 \
        return PyLong_FromLong(1);                                      \
      else                                                              \
        return PyLong_FromLong(0);                                      \
      break;                                                            \
    case Py_NE:                                                         \
      if (o0->ob_value != o1->ob_value)                                 \
        return PyLong_FromLong(1);                                      \
      else                                                              \
        return PyLong_FromLong(0);                                      \
      break;                                                            \
    case Py_GT:                                                         \
      if (o1->ob_value < o0->ob_value)                                  \
        return PyLong_FromLong(1);                                      \
      else                                                              \
        return PyLong_FromLong(0);                                      \
      break;                                                            \
    case Py_GE:                                                         \
      if (o0->ob_value < o1->ob_value)                                  \
        return PyLong_FromLong(0);                                      \
      else                                                              \
        return PyLong_FromLong(1);                                      \
      break;                                                            \
    default:                                                            \
      return 0;                                                         \
    };                                                                  \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _repr(Py ## HandleKind *o)                              \
  {                                                                     \
    return PyObject_NewString(o->ob_value.toString());                  \
  }                                                                     \
                                                                        \
  static long                                                           \
  HandleKind ## _hash(Py ## HandleKind *o)                              \
  {                                                                     \
    return o->ob_hash;                                                  \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _encode(Py ## HandleKind *o, PyObject *args)            \
  {                                                                     \
    if (!PyArg_UnpackTuple(args, "encode", 0, 0))                       \
      return 0;                                                         \
    return PyObject_NewVariableLengthData(o->ob_value.encode());        \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _isValid(Py ## HandleKind *o, PyObject *args)           \
  {                                                                     \
    if (!PyArg_UnpackTuple(args, "isValid", 0, 0))                      \
      return 0;                                                         \
    return PyBool_FromLong(o->ob_value.isValid());                      \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _new(PyTypeObject *type, PyObject *args, PyObject *);   \
                                                                        \
  static PyMethodDef HandleKind ## _methods[] =                         \
  {                                                                     \
    {"encode", (PyCFunction)HandleKind ## _encode, METH_VARARGS, ""},   \
    {"isValid", (PyCFunction)HandleKind ## _isValid, METH_VARARGS, ""}, \
    {0,}                                                                \
  };                                                                    \
                                                                        \
  static PyTypeObject Py ## HandleKind ## Type = {                      \
    PyVarObject_HEAD_INIT(NULL, 0)                                      \
    # HandleKind ,                      /* tp_name */                   \
    sizeof(Py##HandleKind),             /* tp_basicsize */              \
    0,                                  /* tp_itemsize */               \
    (destructor)HandleKind ## _dealloc, /* tp_dealloc */                \
    0,                                  /* tp_print */                  \
    0,                                  /* tp_getattr */                \
    0,                                  /* tp_setattr */                \
    0,                                  /* tp_compare */                \
    (reprfunc)HandleKind ## _repr,      /* tp_repr */                   \
    0,                                  /* tp_as_number */              \
    0,                                  /* tp_as_sequence */            \
    0,                                  /* tp_as_mapping */             \
    (hashfunc)HandleKind ## _hash,      /* tp_hash */                   \
    0,                                  /* tp_call */                   \
    0,                                  /* tp_str */                    \
    0,                                  /* tp_getattro */               \
    0,                                  /* tp_setattro */               \
    0,                                  /* tp_as_buffer */              \
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */                  \
    # HandleKind ,                      /* tp_doc */                    \
    0,                                  /* tp_traverse */               \
    0,                                  /* tp_clear */                  \
    (richcmpfunc)HandleKind ## _richcmp,/* tp_richcompare */            \
    0,                                  /* tp_weaklistoffset */         \
    0,                                  /* tp_iter */                   \
    0,                                  /* tp_iternext */               \
    HandleKind ## _methods,             /* tp_methods */                \
    0,                                  /* tp_members */                \
    0,                                  /* tp_getset */                 \
    0,                                  /* tp_base */                   \
    0,                                  /* tp_dict */                   \
    0,                                  /* tp_descr_get */              \
    0,                                  /* tp_descr_set */              \
    0,                                  /* tp_dictoffset */             \
    0,                                  /* tp_init */                   \
    0,                                  /* tp_alloc */                  \
    (newfunc)HandleKind ## _new,        /* tp_new */                    \
  };                                                                    \
                                                                        \
  static PyObject*                                                      \
  HandleKind ## _new(PyTypeObject *type, PyObject *args, PyObject *)    \
  {                                                                     \
    Py ## HandleKind *self;                                             \
    self = PyObject_New(Py ## HandleKind, &Py ## HandleKind ## Type);   \
    if (!self)                                                          \
      return 0;                                                         \
    new (&self->ob_value) rti1516:: HandleKind();                       \
    self->ob_hash = VariableLengthData_hash(self->ob_value.encode());   \
    return (PyObject*)self;                                             \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  PyObject_New ## HandleKind(const rti1516:: HandleKind& handle)        \
  {                                                                     \
    Py ## HandleKind *self;                                             \
    self = PyObject_New(Py ## HandleKind, &Py ## HandleKind ## Type);   \
    if (!self)                                                          \
      return 0;                                                         \
    new (&self->ob_value) rti1516::HandleKind(handle);                  \
    self->ob_hash = VariableLengthData_hash(handle.encode());           \
    return (PyObject*)self;                                             \
  }                                                                     \
                                                                        \
  static bool                                                           \
  PyObject_Get ## HandleKind(rti1516:: HandleKind& handle, PyObject* o) \
  {                                                                     \
    if (!PyObject_TypeCheck(o, &Py ## HandleKind ## Type))              \
      return false;                                                     \
    handle = ((Py ## HandleKind*)o)->ob_value;                          \
    return true;                                                        \
  }                                                                     \
                                                                        \
  static PyObject*                                                      \
  PyObject_New ## HandleKind ## Set(const std::set<rti1516::HandleKind>& handleSet) \
  {                                                                     \
    PyObject* set = PySet_New(0);                                       \
    std::set<rti1516::HandleKind>::const_iterator i;                    \
    for (i = handleSet.begin(); i != handleSet.end(); ++i) {            \
      PyObject *key = PyObject_New ## HandleKind(*i);                   \
      if (!key) {                                                       \
        Py_DecRef(set);                                                 \
        return 0;                                                       \
      }                                                                 \
      if (PySet_Add(set, key)) {                                        \
        Py_DecRef(key);                                                 \
        Py_DecRef(set);                                                 \
        return 0;                                                       \
      }                                                                 \
    }                                                                   \
    return set;                                                         \
  }                                                                     \
                                                                        \
  static bool                                                           \
  PyObject_Get ## HandleKind ## Set(std::set<rti1516::HandleKind>& handleSet, PyObject* o) \
  {                                                                     \
    PyObject* iterator = PyObject_GetIter(o);                           \
    if (!iterator)                                                      \
      return false;                                                     \
    while (PyObject* item = PyIter_Next(iterator)) {                    \
      rti1516:: HandleKind handle;                                      \
      if (!PyObject_Get ## HandleKind(handle, item)) {                  \
        Py_DecRef(item);                                                \
        Py_DecRef(iterator);                                            \
        return false;                                                   \
      }                                                                 \
      handleSet.insert(handle);                                         \
      Py_DecRef(item);                                                  \
    }                                                                   \
    Py_DecRef(iterator);                                                \
    return true;                                                        \
  }

IMPLEMENT_HANDLE_CLASS(FederateHandle)
IMPLEMENT_HANDLE_CLASS(ObjectClassHandle)
IMPLEMENT_HANDLE_CLASS(InteractionClassHandle)
IMPLEMENT_HANDLE_CLASS(ObjectInstanceHandle)
IMPLEMENT_HANDLE_CLASS(AttributeHandle)
IMPLEMENT_HANDLE_CLASS(ParameterHandle)
IMPLEMENT_HANDLE_CLASS(DimensionHandle)
IMPLEMENT_HANDLE_CLASS(RegionHandle)
IMPLEMENT_HANDLE_CLASS(MessageRetractionHandle)

#undef IMPLEMENT_HANDLE_CLASS

static PyObject*
PyObject_NewFederateHandleSaveStatusPairVector(const rti1516::FederateHandleSaveStatusPairVector& federateStatusVector)
{
  PyObject* list = PyList_New(federateStatusVector.size());
  for (size_t i = 0; i < federateStatusVector.size(); ++i) {
    PyObject *first = PyObject_NewFederateHandle(federateStatusVector[i].first);
    if (!first) {
      Py_DecRef(list);
      return 0;
    }
    PyObject *second = PyLong_FromLong(federateStatusVector[i].second);
    if (!second) {
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
    PyObject *pair = PyTuple_Pack(2, first, second);
    if (!pair) {
      Py_DecRef(second);
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
    if (PyList_SetItem(list, i, pair)) {
      Py_DecRef(pair);
      Py_DecRef(second);
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
  }
  return list;
}

static PyObject*
PyObject_NewFederateHandleRestoreStatusPairVector(const rti1516::FederateHandleRestoreStatusPairVector& federateStatusVector)
{
  PyObject* list = PyList_New(federateStatusVector.size());
  for (size_t i = 0; i < federateStatusVector.size(); ++i) {
    PyObject *first = PyObject_NewFederateHandle(federateStatusVector[i].first);
    if (!first) {
      Py_DecRef(list);
      return 0;
    }
    PyObject *second = PyLong_FromLong(federateStatusVector[i].second);
    if (!second) {
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
    PyObject *pair = PyTuple_Pack(2, first, second);
    if (!pair) {
      Py_DecRef(second);
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
    if (PyList_SetItem(list, i, pair)) {
      Py_DecRef(pair);
      Py_DecRef(second);
      Py_DecRef(first);
      Py_DecRef(list);
      return 0;
    }
  }
  return list;
}

static PyObject*
PyObject_NewAttributeHandleValueMap(const rti1516::AttributeHandleValueMap& attributeHandleValueMap)
{
  PyObject* dict = PyDict_New();
  rti1516::AttributeHandleValueMap::const_iterator i;
  for (i = attributeHandleValueMap.begin(); i != attributeHandleValueMap.end(); ++i) {
    PyObject *key = PyObject_NewAttributeHandle(i->first);
    if (!key) {
      Py_DecRef(dict);
      return 0;
    }
    PyObject *value = PyObject_NewVariableLengthData(i->second);
    if (!value) {
      Py_DecRef(key);
      Py_DecRef(dict);
      return 0;
    }
    if (PyDict_SetItem(dict, key, value)) {
      Py_DecRef(value);
      Py_DecRef(key);
      Py_DecRef(dict);
      return 0;
    }
  }
  return dict;
}

static bool
PyObject_GetAttributeHandleValueMap(rti1516::AttributeHandleValueMap& attributeHandleValueMap, PyObject* o)
{
  PyObject* items = PyMapping_Items(o);
  if (!items)
    return false;
  PyObject* iterator = PyObject_GetIter(items);
  if (!iterator) {
    Py_DecRef(items);
    return false;
  }
  while (PyObject* item = PyIter_Next(iterator)) {
    PyObject* key = PySequence_GetItem(item, 0);
    if (!key) {
      Py_DecRef(item);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    rti1516::AttributeHandle attributeHandle;
    if (!PyObject_GetAttributeHandle(attributeHandle, key)) {
      Py_DecRef(key);
      Py_DecRef(item);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    Py_DecRef(key);

    PyObject* value = PySequence_GetItem(item, 1);
    Py_DecRef(item);
    if (!value) {
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    if (!PyObject_GetVariableLengthData(attributeHandleValueMap[attributeHandle], value)) {
      Py_DecRef(value);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    Py_DecRef(value);
  }
  Py_DecRef(iterator);
  Py_DecRef(items);
  return true;
}

static PyObject*
PyObject_NewParameterHandleValueMap(const rti1516::ParameterHandleValueMap& parameterHandleValueMap)
{
  PyObject* dict = PyDict_New();
  rti1516::ParameterHandleValueMap::const_iterator i;
  for (i = parameterHandleValueMap.begin(); i != parameterHandleValueMap.end(); ++i) {
    PyObject *key = PyObject_NewParameterHandle(i->first);
    if (!key) {
      Py_DecRef(dict);
      return 0;
    }
    PyObject *value = PyObject_NewVariableLengthData(i->second);
    if (!value) {
      Py_DecRef(key);
      Py_DecRef(dict);
      return 0;
    }
    if (PyDict_SetItem(dict, key, value)) {
      Py_DecRef(value);
      Py_DecRef(key);
      Py_DecRef(dict);
      return 0;
    }
  }
  return dict;
}

static bool
PyObject_GetParameterHandleValueMap(rti1516::ParameterHandleValueMap& parameterHandleValueMap, PyObject* o)
{
  PyObject* items = PyMapping_Items(o);
  if (!items)
    return false;
  PyObject* iterator = PyObject_GetIter(items);
  if (!iterator) {
    Py_DecRef(items);
    return false;
  }
  while (PyObject* item = PyIter_Next(iterator)) {
    PyObject* key = PySequence_GetItem(item, 0);
    if (!key) {
      Py_DecRef(item);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    rti1516::ParameterHandle parameterHandle;
    if (!PyObject_GetParameterHandle(parameterHandle, key)) {
      Py_DecRef(key);
      Py_DecRef(item);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    Py_DecRef(key);

    PyObject* value = PySequence_GetItem(item, 1);
    Py_DecRef(item);
    if (!value) {
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    if (!PyObject_GetVariableLengthData(parameterHandleValueMap[parameterHandle], value)) {
      Py_DecRef(value);
      Py_DecRef(iterator);
      Py_DecRef(items);
      return false;
    }
    Py_DecRef(value);
  }
  Py_DecRef(iterator);
  Py_DecRef(items);
  return true;
}

static bool
PyObject_GetAttributeHandleSetRegionHandleSetPairVector(rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector, PyObject* o)
{
  PyObject* iterator = PyObject_GetIter(o);
  if (!iterator)
    return false;
  while (PyObject* item = PyIter_Next(iterator)) {
    PyObject* attribute = PySequence_GetItem(item, 0);
    if (!attribute) {
      Py_DecRef(item);
      Py_DecRef(iterator);
      return false;
    }
    rti1516::AttributeHandleSet attributeHandleSet;
    if (!PyObject_GetAttributeHandleSet(attributeHandleSet, attribute)) {
      Py_DecRef(attribute);
      Py_DecRef(item);
      Py_DecRef(iterator);
      return false;
    }
    Py_DecRef(attribute);

    PyObject* region = PySequence_GetItem(item, 1);
    Py_DecRef(item);
    if (!region) {
      Py_DecRef(iterator);
      return false;
    }
    rti1516::RegionHandleSet regionHandleSet;
    if (!PyObject_GetRegionHandleSet(regionHandleSet, region)) {
      Py_DecRef(region);
      Py_DecRef(iterator);
      return false;
    }
    Py_DecRef(region);

    attributeHandleSetRegionHandleSetPairVector.resize(attributeHandleSetRegionHandleSetPairVector.size() + 1);
    attributeHandleSetRegionHandleSetPairVector.back().first.swap(attributeHandleSet);
    attributeHandleSetRegionHandleSetPairVector.back().second.swap(regionHandleSet);
  }
  Py_DecRef(iterator);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

static PySharedPtr PyRTI1516Exception;
static PySharedPtr PyRTI1516AsynchronousDeliveryAlreadyDisabled;
static PySharedPtr PyRTI1516AsynchronousDeliveryAlreadyEnabled;
static PySharedPtr PyRTI1516AttributeAcquisitionWasNotCanceled;
static PySharedPtr PyRTI1516AttributeAcquisitionWasNotRequested;
static PySharedPtr PyRTI1516AttributeAlreadyBeingAcquired;
static PySharedPtr PyRTI1516AttributeAlreadyBeingDivested;
static PySharedPtr PyRTI1516AttributeAlreadyOwned;
static PySharedPtr PyRTI1516AttributeDivestitureWasNotRequested;
static PySharedPtr PyRTI1516AttributeNotDefined;
static PySharedPtr PyRTI1516AttributeNotOwned;
static PySharedPtr PyRTI1516AttributeNotPublished;
static PySharedPtr PyRTI1516AttributeNotRecognized;
static PySharedPtr PyRTI1516AttributeNotSubscribed;
static PySharedPtr PyRTI1516AttributeRelevanceAdvisorySwitchIsOff;
static PySharedPtr PyRTI1516AttributeRelevanceAdvisorySwitchIsOn;
static PySharedPtr PyRTI1516AttributeScopeAdvisorySwitchIsOff;
static PySharedPtr PyRTI1516AttributeScopeAdvisorySwitchIsOn;
static PySharedPtr PyRTI1516BadInitializationParameter;
static PySharedPtr PyRTI1516CouldNotCreateLogicalTimeFactory;
static PySharedPtr PyRTI1516CouldNotDecode;
static PySharedPtr PyRTI1516CouldNotDiscover;
static PySharedPtr PyRTI1516CouldNotEncode;
static PySharedPtr PyRTI1516CouldNotOpenFDD;
static PySharedPtr PyRTI1516CouldNotInitiateRestore;
static PySharedPtr PyRTI1516DeletePrivilegeNotHeld;
static PySharedPtr PyRTI1516RequestForTimeConstrainedPending;
static PySharedPtr PyRTI1516NoRequestToEnableTimeConstrainedWasPending;
static PySharedPtr PyRTI1516RequestForTimeRegulationPending;
static PySharedPtr PyRTI1516NoRequestToEnableTimeRegulationWasPending;
static PySharedPtr PyRTI1516ErrorReadingFDD;
static PySharedPtr PyRTI1516FederateAlreadyExecutionMember;
static PySharedPtr PyRTI1516FederateHasNotBegunSave;
static PySharedPtr PyRTI1516FederateInternalError;
static PySharedPtr PyRTI1516FederateNotExecutionMember;
static PySharedPtr PyRTI1516FederateOwnsAttributes;
static PySharedPtr PyRTI1516FederateServiceInvocationsAreBeingReportedViaMOM;
static PySharedPtr PyRTI1516FederateUnableToUseTime;
static PySharedPtr PyRTI1516FederatesCurrentlyJoined;
static PySharedPtr PyRTI1516FederationExecutionAlreadyExists;
static PySharedPtr PyRTI1516FederationExecutionDoesNotExist;
static PySharedPtr PyRTI1516IllegalName;
static PySharedPtr PyRTI1516IllegalTimeArithmetic;
static PySharedPtr PyRTI1516InteractionClassNotDefined;
static PySharedPtr PyRTI1516InteractionClassNotPublished;
static PySharedPtr PyRTI1516InteractionClassNotRecognized;
static PySharedPtr PyRTI1516InteractionClassNotSubscribed;
static PySharedPtr PyRTI1516InteractionParameterNotDefined;
static PySharedPtr PyRTI1516InteractionParameterNotRecognized;
static PySharedPtr PyRTI1516InteractionRelevanceAdvisorySwitchIsOff;
static PySharedPtr PyRTI1516InteractionRelevanceAdvisorySwitchIsOn;
static PySharedPtr PyRTI1516InTimeAdvancingState;
static PySharedPtr PyRTI1516InvalidAttributeHandle;
static PySharedPtr PyRTI1516InvalidDimensionHandle;
static PySharedPtr PyRTI1516InvalidFederateHandle;
static PySharedPtr PyRTI1516InvalidInteractionClassHandle;
static PySharedPtr PyRTI1516InvalidLogicalTime;
static PySharedPtr PyRTI1516InvalidLogicalTimeInterval;
static PySharedPtr PyRTI1516InvalidLookahead;
static PySharedPtr PyRTI1516InvalidObjectClassHandle;
static PySharedPtr PyRTI1516InvalidOrderName;
static PySharedPtr PyRTI1516InvalidOrderType;
static PySharedPtr PyRTI1516InvalidParameterHandle;
static PySharedPtr PyRTI1516InvalidRangeBound;
static PySharedPtr PyRTI1516InvalidRegion;
static PySharedPtr PyRTI1516InvalidRegionContext;
static PySharedPtr PyRTI1516InvalidRetractionHandle;
static PySharedPtr PyRTI1516InvalidServiceGroup;
static PySharedPtr PyRTI1516InvalidTransportationName;
static PySharedPtr PyRTI1516InvalidTransportationType;
static PySharedPtr PyRTI1516JoinedFederateIsNotInTimeAdvancingState;
static PySharedPtr PyRTI1516LogicalTimeAlreadyPassed;
static PySharedPtr PyRTI1516MessageCanNoLongerBeRetracted;
static PySharedPtr PyRTI1516NameNotFound;
static PySharedPtr PyRTI1516NoAcquisitionPending;
static PySharedPtr PyRTI1516ObjectClassNotDefined;
static PySharedPtr PyRTI1516ObjectClassNotKnown;
static PySharedPtr PyRTI1516ObjectClassNotPublished;
static PySharedPtr PyRTI1516ObjectClassRelevanceAdvisorySwitchIsOff;
static PySharedPtr PyRTI1516ObjectClassRelevanceAdvisorySwitchIsOn;
static PySharedPtr PyRTI1516ObjectInstanceNameInUse;
static PySharedPtr PyRTI1516ObjectInstanceNameNotReserved;
static PySharedPtr PyRTI1516ObjectInstanceNotKnown;
static PySharedPtr PyRTI1516OwnershipAcquisitionPending;
static PySharedPtr PyRTI1516RTIinternalError;
static PySharedPtr PyRTI1516RegionDoesNotContainSpecifiedDimension;
static PySharedPtr PyRTI1516RegionInUseForUpdateOrSubscription;
static PySharedPtr PyRTI1516RegionNotCreatedByThisFederate;
static PySharedPtr PyRTI1516RestoreInProgress;
static PySharedPtr PyRTI1516RestoreNotRequested;
static PySharedPtr PyRTI1516SaveInProgress;
static PySharedPtr PyRTI1516SaveNotInitiated;
static PySharedPtr PyRTI1516SpecifiedSaveLabelDoesNotExist;
static PySharedPtr PyRTI1516SynchronizationPointLabelNotAnnounced;
static PySharedPtr PyRTI1516TimeConstrainedAlreadyEnabled;
static PySharedPtr PyRTI1516TimeConstrainedIsNotEnabled;
static PySharedPtr PyRTI1516TimeRegulationAlreadyEnabled;
static PySharedPtr PyRTI1516TimeRegulationIsNotEnabled;
static PySharedPtr PyRTI1516UnableToPerformSave;
static PySharedPtr PyRTI1516UnknownName;
static PySharedPtr PyRTI1516InternalError;

#define CATCH_C_EXCEPTION(ExceptionKind)                                             \
  catch(const rti1516:: ExceptionKind& e) {                                          \
    PyErr_SetObject(PyRTI1516 ## ExceptionKind.get(), PyObject_NewString(e.what())); \
    return 0;                                                                        \
  }

#define CATCH_C_EXCEPTION_THREADS(ExceptionKind)                                     \
  catch(const rti1516:: ExceptionKind& e) {                                          \
    PyEval_RestoreThread(_save);                                                     \
    PyErr_SetObject(PyRTI1516 ## ExceptionKind.get(), PyObject_NewString(e.what())); \
    return 0;                                                                        \
  }

static std::wstring PyErr_GetExceptionString()
{
  PyObject* ptype = 0;
  PyObject* pvalue = 0;
  PyObject* ptraceback = 0;
  PyErr_Fetch(&ptype, &pvalue, &ptraceback);
  PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
  if (ptype)
    Py_DecRef(ptype);
  std::wstring s;
  if (pvalue) {
    s += PyObject_GetString(pvalue);
    Py_DecRef(pvalue);
  }
  if (ptraceback) {
    s += PyObject_GetString(ptraceback);
    Py_DecRef(ptraceback);
  }
  PyErr_Clear();
  return s;
}

#define CATCH_PYTHON_EXCEPTION(ExceptionKind, exception)                             \
  do {                                                                               \
    if (PyErr_GivenExceptionMatches(exception, PyRTI1516 ## ExceptionKind.get())) {  \
      PyErr_Print();                                                                 \
      throw rti1516::ExceptionKind(PyErr_GetExceptionString());                      \
    }                                                                                \
  } while(0)

#define CATCH_UNEXPECTED_EXCEPTION(exception)                                        \
  do {                                                                               \
    if (PyErr_GivenExceptionMatches(exception, PyExc_KeyboardInterrupt)) {           \
      PyErr_Clear();                                                                 \
      PyErr_SetInterrupt();                                                          \
    } else {                                                                         \
      PyErr_Print();                                                                 \
      throw rti1516::FederateInternalError(PyErr_GetExceptionString());              \
    }                                                                                \
  } while(0)

///////////////////////////////////////////////////////////////////////////////

struct PyRTI1516FederateAmbassador : public rti1516::FederateAmbassador {
  PyRTI1516FederateAmbassador(PyObject* federateAmbassador = 0)
    RTI_THROW ((rti1516::FederateInternalError)) :
    ob_federateAmbassador(federateAmbassador)
  {
    if (ob_federateAmbassador)
      Py_IncRef(ob_federateAmbassador);
  }

  virtual ~PyRTI1516FederateAmbassador()
    RTI_NOEXCEPT
  {
    if (ob_federateAmbassador)
      Py_DecRef(ob_federateAmbassador);
  }

  void setObject(PyObject* federateAmbassador = 0)
  {
    if (ob_federateAmbassador == federateAmbassador)
      return;
    if (federateAmbassador)
      Py_IncRef(federateAmbassador);
    if (ob_federateAmbassador)
      Py_DecRef(ob_federateAmbassador);
    ob_federateAmbassador = federateAmbassador;
  }

  struct GILStateScope {
    GILStateScope() :
      _gstate(PyGILState_Ensure())
    { }
    ~GILStateScope()
    { PyGILState_Release(_gstate); }
    PyGILState_STATE _gstate;
  };

  virtual void synchronizationPointRegistrationSucceeded(std::wstring const & label)
      RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"synchronizationPointRegistrationSucceeded", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void synchronizationPointRegistrationFailed(std::wstring const & label,
                                                      rti1516::SynchronizationFailureReason reason)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* arg1 = PyLong_FromLong(reason);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"synchronizationPointRegistrationFailed", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void announceSynchronizationPoint(std::wstring const & label,
                                            rti1516::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* arg1 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"announceSynchronizationPoint", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationSynchronized(std::wstring const & label)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationSynchronized", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void initiateFederateSave(std::wstring const & label)
    RTI_THROW ((rti1516::UnableToPerformSave,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"initiateFederateSave", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(UnableToPerformSave, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void initiateFederateSave(std::wstring const & label, rti1516::LogicalTime const & theTime)
    RTI_THROW ((rti1516::UnableToPerformSave,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* arg1 = PyObject_NewLogicalTime(theTime);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"initiateFederateSave", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(UnableToPerformSave, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationSaved()
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationSaved", 0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationNotSaved(rti1516::SaveFailureReason theSaveFailureReason)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyLong_FromLong(theSaveFailureReason);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationNotSaved", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationSaveStatusResponse(rti1516::FederateHandleSaveStatusPairVector const & theFederateStatusVector)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewFederateHandleSaveStatusPairVector(theFederateStatusVector);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationSaveStatusResponse", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestFederationRestoreSucceeded(std::wstring const & label)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestFederationRestoreSucceeded", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestFederationRestoreFailed(std::wstring const & label)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestFederationRestoreFailed", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationRestoreBegun()
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationRestoreBegun", 0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void initiateFederateRestore(std::wstring const & label, rti1516::FederateHandle handle)
    RTI_THROW ((rti1516::SpecifiedSaveLabelDoesNotExist,
           rti1516::CouldNotInitiateRestore,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(label);
    PyObject* arg1 = PyObject_NewFederateHandle(handle);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"initiateFederateRestore", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(SpecifiedSaveLabelDoesNotExist, exception);
      CATCH_PYTHON_EXCEPTION(CouldNotInitiateRestore, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationRestored()
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationRestored", 0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationNotRestored(rti1516::RestoreFailureReason theRestoreFailureReason)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyLong_FromLong(theRestoreFailureReason);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationNotRestored", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void federationRestoreStatusResponse(rti1516::FederateHandleRestoreStatusPairVector const & theFederateStatusVector)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewFederateHandleRestoreStatusPairVector(theFederateStatusVector);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"federationRestoreStatusResponse", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void startRegistrationForObjectClass(rti1516::ObjectClassHandle theClass)
      RTI_THROW ((rti1516::ObjectClassNotPublished,
             rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectClassHandle(theClass);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"startRegistrationForObjectClass", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectClassNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void stopRegistrationForObjectClass(rti1516::ObjectClassHandle theClass)
    RTI_THROW ((rti1516::ObjectClassNotPublished,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectClassHandle(theClass);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"stopRegistrationForObjectClass", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectClassNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void turnInteractionsOn(rti1516::InteractionClassHandle theHandle)
    RTI_THROW ((rti1516::InteractionClassNotPublished,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theHandle);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"turnInteractionsOn", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void turnInteractionsOff(rti1516::InteractionClassHandle theHandle)
    RTI_THROW ((rti1516::InteractionClassNotPublished,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theHandle);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"turnInteractionsOff", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void objectInstanceNameReservationSucceeded(std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516::UnknownName,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(theObjectInstanceName);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"objectInstanceNameReservationSucceeded", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(UnknownName, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void objectInstanceNameReservationFailed(std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516::UnknownName,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewString(theObjectInstanceName);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"objectInstanceNameReservationFailed", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(UnknownName, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void discoverObjectInstance(rti1516::ObjectInstanceHandle theObject, rti1516::ObjectClassHandle theObjectClass,
                                      std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516::CouldNotDiscover,
           rti1516::ObjectClassNotKnown,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewObjectClassHandle(theObjectClass);
    PyObject* arg2 = PyObject_NewString(theObjectInstanceName);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"discoverObjectInstance", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(CouldNotDiscover, exception);
      CATCH_PYTHON_EXCEPTION(ObjectClassNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg6 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg7 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg8 = Py_None;
    Py_IncRef(Py_None);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType, rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg6 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg7 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType, rti1516::LogicalTime const & theTime, rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg8 = Py_None;
    Py_IncRef(Py_None);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                                      rti1516::OrderType receivedOrder, rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = Py_None;
    Py_IncRef(Py_None);
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                                      rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* arg8 = Py_None;
    Py_IncRef(Py_None);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                      rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                                      rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle theHandle,
                                      rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleValueMap(theAttributeValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"reflectAttributeValues",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                                  rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                  rti1516::TransportationType theType)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = Py_None;
    PyObject* arg6 = Py_None;
    PyObject* arg7 = Py_None;
    PyObject* arg8 = Py_None;
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNOOOO", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                                  rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                  rti1516::TransportationType theType, rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = Py_None;
    PyObject* arg6 = Py_None;
    PyObject* arg7 = Py_None;
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNOOON", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                                  rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                  rti1516::TransportationType theType, rti1516::LogicalTime const & theTime, rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = Py_None;
    PyObject* arg8 = Py_None;
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNNNOO", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                                  rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                                  rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                                  rti1516::OrderType receivedOrder, rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = Py_None;
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNNNON", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual
  void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* arg8 = Py_None;
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNNNNO", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual
  void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction, rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & theUserSuppliedTag, rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType, rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle theHandle,
                     rti1516::RegionHandleSet const & theSentRegionHandleSet)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewInteractionClassHandle(theInteraction);
    PyObject* arg1 = PyObject_NewParameterHandleValueMap(theParameterValues);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg3 = PyObject_NewOrderType(sentOrder);
    PyObject* arg4 = PyObject_NewTransportationType(theType);
    PyObject* arg5 = PyObject_NewLogicalTime(theTime);
    PyObject* arg6 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg7 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* arg8 = PyObject_NewRegionHandleSet(theSentRegionHandleSet);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"receiveInteraction",
                                           (char*)"NNNNNNNNN", arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InteractionClassNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionParameterNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(InteractionClassNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  // 6.11
  virtual
  void
  removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                       rti1516::VariableLengthData const & theUserSuppliedTag,
                       rti1516::OrderType sentOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg2 = PyObject_NewOrderType(sentOrder);
    PyObject* arg3 = Py_None;
    PyObject* arg4 = Py_None;
    PyObject* arg5 = Py_None;
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"removeObjectInstance",
                                           (char*)"NNNOOO", arg0, arg1, arg2, arg3, arg4, arg5);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual
  void
  removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                       rti1516::VariableLengthData const & theUserSuppliedTag,
                       rti1516::OrderType sentOrder,
                       rti1516::LogicalTime const & theTime,
                       rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg2 = PyObject_NewOrderType(sentOrder);
    PyObject* arg3 = PyObject_NewLogicalTime(theTime);
    PyObject* arg4 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg5 = Py_None;
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"removeObjectInstance",
                                           (char*)"NNNNNO", arg0, arg1, arg2, arg3, arg4, arg5);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual
  void
  removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                       rti1516::VariableLengthData const & theUserSuppliedTag,
                       rti1516::OrderType sentOrder,
                       rti1516::LogicalTime const & theTime,
                       rti1516::OrderType receivedOrder,
                       rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* arg2 = PyObject_NewOrderType(sentOrder);
    PyObject* arg3 = PyObject_NewLogicalTime(theTime);
    PyObject* arg4 = PyObject_NewOrderType(receivedOrder);
    PyObject* arg5 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"removeObjectInstance",
                                           (char*)"NNNNNN", arg0, arg1, arg2, arg3, arg4, arg5);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributesInScope", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributesOutOfScope", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotSubscribed, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  // 6.18
  virtual void provideAttributeValueUpdate(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes,
                                           rti1516::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotOwned,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"provideAttributeValueUpdate", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void turnUpdatesOnForObjectInstance(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotOwned,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"turnUpdatesOnForObjectInstance", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void turnUpdatesOffForObjectInstance(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotOwned,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"turnUpdatesOffForObjectInstance", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestAttributeOwnershipAssumption(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & offeredAttributes,
                                                   rti1516::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeAlreadyOwned,
           rti1516::AttributeNotPublished,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(offeredAttributes);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestAttributeOwnershipAssumption", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestDivestitureConfirmation(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & releasedAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotOwned,
           rti1516::AttributeDivestitureWasNotRequested,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(releasedAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestDivestitureConfirmation", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(AttributeDivestitureWasNotRequested, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void attributeOwnershipAcquisitionNotification(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & securedAttributes,
                                                         rti1516::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeAcquisitionWasNotRequested,
           rti1516::AttributeAlreadyOwned,
           rti1516::AttributeNotPublished,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(securedAttributes);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributeOwnershipAcquisitionNotification", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAcquisitionWasNotRequested, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAlreadyOwned, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotPublished, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void attributeOwnershipUnavailable(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeAlreadyOwned,
           rti1516::AttributeAcquisitionWasNotRequested,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributeOwnershipUnavailable", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAlreadyOwned, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAcquisitionWasNotRequested, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestAttributeOwnershipRelease(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandleSet const & candidateAttributes,
                                                rti1516::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotOwned,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(candidateAttributes);
    PyObject* arg2 = PyObject_NewVariableLengthData(theUserSuppliedTag);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestAttributeOwnershipRelease", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotOwned, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(rti1516::ObjectInstanceHandle theObject,
                                                                rti1516::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeAlreadyOwned,
           rti1516::AttributeAcquisitionWasNotCanceled,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandleSet(theAttributes);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"confirmAttributeOwnershipAcquisitionCancellation", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAlreadyOwned, exception);
      CATCH_PYTHON_EXCEPTION(AttributeAcquisitionWasNotCanceled, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void informAttributeOwnership(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandle theAttribute,
                                        rti1516::FederateHandle theOwner)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandle(theAttribute);
    PyObject* arg2 = PyObject_NewFederateHandle(theOwner);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"informAttributeOwnership", (char*)"NNN", arg0, arg1, arg2);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void attributeIsNotOwned(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandle theAttribute)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandle(theAttribute);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributeIsNotOwned", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void attributeIsOwnedByRTI(rti1516::ObjectInstanceHandle theObject, rti1516::AttributeHandle theAttribute)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewObjectInstanceHandle(theObject);
    PyObject* arg1 = PyObject_NewAttributeHandle(theAttribute);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"attributeIsOwnedByRTI", (char*)"NN", arg0, arg1);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(ObjectInstanceNotKnown, exception);
      CATCH_PYTHON_EXCEPTION(AttributeNotRecognized, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void timeRegulationEnabled(rti1516::LogicalTime const & theFederateTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeRegulationWasPending,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewLogicalTime(theFederateTime);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"timeRegulationEnabled", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(NoRequestToEnableTimeRegulationWasPending, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  // 8.6
  virtual void timeConstrainedEnabled(rti1516::LogicalTime const & theFederateTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeConstrainedWasPending,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewLogicalTime(theFederateTime);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"timeConstrainedEnabled", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(NoRequestToEnableTimeConstrainedWasPending, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void timeAdvanceGrant(rti1516::LogicalTime const & theTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::JoinedFederateIsNotInTimeAdvancingState,
           rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewLogicalTime(theTime);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"timeAdvanceGrant", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(InvalidLogicalTime, exception);
      CATCH_PYTHON_EXCEPTION(JoinedFederateIsNotInTimeAdvancingState, exception);
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  virtual void requestRetraction(rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::FederateInternalError))
  {
    if (!ob_federateAmbassador)
      return;

    GILStateScope gilStateScope;

    PyObject* arg0 = PyObject_NewMessageRetractionHandle(theHandle);
    PyObject* result = PyObject_CallMethod(ob_federateAmbassador, (char*)"requestRetraction", (char*)"N", arg0);
    if (result) {
      Py_DecRef(result);
      return;
    } else {
      PyObject *exception = PyErr_Occurred();
      CATCH_PYTHON_EXCEPTION(FederateInternalError, exception);
      CATCH_UNEXPECTED_EXCEPTION(exception);
    }
  }

  PyObject* ob_federateAmbassador;
};

struct PyRTIambassadorObject {
  PyObject_HEAD
  RTI_UNIQUE_PTR<rti1516::RTIambassador> ob_value;
  PyRTI1516FederateAmbassador _federateAmbassador;
  std::wstring _logicaltimeFactoryName;
};

static std::wstring
validateLogicalTimeFactory(const std::wstring& implementationName)
{
  RTI_UNIQUE_PTR<rti1516::LogicalTimeFactory> logicalTimeFactory(rti1516::LogicalTimeFactoryFactory::makeLogicalTimeFactory(implementationName));
  if (!logicalTimeFactory.get())
    return std::wstring();

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime(logicalTimeFactory->makeLogicalTime());
  if (!logicalTime.get())
    return std::wstring();

  return logicalTime->implementationName();
}

static PyObject *
PyRTIambassador_setLogicalTimeFactory(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "setLogicalTimeFactory", 1, 1, &arg1))
    return 0;

  std::wstring logicalTimeImplementationName;
  if (!PyObject_GetString(logicalTimeImplementationName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "logicalTimeImplementationName needs to be a string!");
    return 0;
  }

  // Validate the time factory
  logicalTimeImplementationName = validateLogicalTimeFactory(logicalTimeImplementationName);
  if (logicalTimeImplementationName.empty()) {
    PyErr_SetString(PyRTI1516CouldNotCreateLogicalTimeFactory.get(), "Given logicalTimeImplementationName does not yield a C++ factory!");
    return 0;
  }

  if (logicalTimeImplementationName != L"HLAfloat64Time" &&
      logicalTimeImplementationName != L"HLAinteger64Time") {
    PyErr_SetString(PyRTI1516CouldNotCreateLogicalTimeFactory.get(), "Unsupported logicalTimeImplementationName!");
    return 0;
  }

  self->_logicaltimeFactoryName = logicalTimeImplementationName;

  Py_IncRef(Py_None);
  return Py_None;
}

static PyObject *
PyRTIambassador_getLogicalTimeFactory(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "getLogicalTimeFactory", 0, 0))
    return 0;

  return PyObject_NewString(self->_logicaltimeFactoryName);
}

static PyObject *
PyRTIambassador_createFederationExecution(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "createFederationExecution", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  std::wstring federationExecutionName;
  if (!PyObject_GetString(federationExecutionName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "federationExecutionName needs to be a string!");
    return 0;
  }

  std::wstring fullPathNameToTheFDDfile;
  if (!PyObject_GetString(fullPathNameToTheFDDfile, arg2)) {
    PyErr_SetString(PyExc_TypeError, "fullPathNameToTheFDDfile needs to be a string!");
    return 0;
  }

  std::wstring logicalTimeImplementationName;
  if (arg3) {
    if (!PyObject_GetString(logicalTimeImplementationName, arg3)) {
      PyErr_SetString(PyExc_TypeError, "logicalTimeImplementationName needs to be a string!");
      return 0;
    }

    logicalTimeImplementationName = validateLogicalTimeFactory(logicalTimeImplementationName);
    if (logicalTimeImplementationName.empty()) {
      PyErr_SetString(PyRTI1516CouldNotCreateLogicalTimeFactory.get(), "Given logicalTimeImplementationName does not yield a C++ factory!");
      return 0;
    }

    if (logicalTimeImplementationName != L"HLAfloat64Time" &&
        logicalTimeImplementationName != L"HLAinteger64Time") {
      PyErr_SetString(PyRTI1516CouldNotCreateLogicalTimeFactory.get(), "Unsupported logicalTimeImplementationName!");
      return 0;
    }
  } else {
    if (self->_logicaltimeFactoryName.empty())
      logicalTimeImplementationName = validateLogicalTimeFactory(logicalTimeImplementationName);
    else
      logicalTimeImplementationName = self->_logicaltimeFactoryName;
  }

  try {

    self->ob_value->createFederationExecution(federationExecutionName, fullPathNameToTheFDDfile, logicalTimeImplementationName);
    self->_logicaltimeFactoryName = logicalTimeImplementationName;

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederationExecutionAlreadyExists)
  CATCH_C_EXCEPTION(CouldNotOpenFDD)
  CATCH_C_EXCEPTION(ErrorReadingFDD)
  CATCH_C_EXCEPTION(CouldNotCreateLogicalTimeFactory)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_destroyFederationExecution(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "destroyFederationExecution", 1, 1, &arg1))
    return 0;

  std::wstring federationExecutionName;
  if (!PyObject_GetString(federationExecutionName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "federationExecutionName needs to be a string!");
    return 0;
  }

  try {

    self->ob_value->destroyFederationExecution(federationExecutionName);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederatesCurrentlyJoined)
  CATCH_C_EXCEPTION(FederationExecutionDoesNotExist)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_joinFederationExecution(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "joinFederationExecution", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  std::wstring federateType;
  if (!PyObject_GetString(federateType, arg1)) {
    PyErr_SetString(PyExc_TypeError, "federateType needs to be a string!");
    return 0;
  }

  std::wstring federationExecutionName;
  if (!PyObject_GetString(federationExecutionName, arg2)) {
    PyErr_SetString(PyExc_TypeError, "federationExecutionName needs to be a string!");
    return 0;
  }

  try {

    rti1516::FederateHandle federateHandle;
    federateHandle = self->ob_value->joinFederationExecution(federateType, federationExecutionName, self->_federateAmbassador);
    self->_federateAmbassador.setObject(arg3);

    return PyObject_NewFederateHandle(federateHandle);
  }
  CATCH_C_EXCEPTION(FederateAlreadyExecutionMember)
  CATCH_C_EXCEPTION(FederationExecutionDoesNotExist)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(CouldNotCreateLogicalTimeFactory)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_resignFederationExecution(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "resignFederationExecution", 1, 1, &arg1))
    return 0;

  std::wstring resignActionString;
  if (!PyObject_GetString(resignActionString, arg1)) {
    PyErr_SetString(PyExc_TypeError, "resignAction needs to be a string!");
    return 0;
  }

  rti1516::ResignAction resignAction;
  if (resignActionString == L"UNCONDITIONALLY_DIVEST_ATTRIBUTES")
    resignAction = rti1516::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  else if (resignActionString == L"DELETE_OBJECTS")
    resignAction = rti1516::DELETE_OBJECTS;
  else if (resignActionString == L"CANCEL_PENDING_OWNERSHIP_ACQUISITIONS")
    resignAction = rti1516::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  else if (resignActionString == L"DELETE_OBJECTS_THEN_DIVEST")
    resignAction = rti1516::DELETE_OBJECTS_THEN_DIVEST;
  else if (resignActionString == L"CANCEL_THEN_DELETE_THEN_DIVEST")
    resignAction = rti1516::CANCEL_THEN_DELETE_THEN_DIVEST;
  else if (resignActionString == L"NO_ACTION")
    resignAction = rti1516::NO_ACTION;
  else
    resignAction = rti1516::CANCEL_THEN_DELETE_THEN_DIVEST;

  try {

    self->ob_value->resignFederationExecution(resignAction);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(OwnershipAcquisitionPending)
  CATCH_C_EXCEPTION(FederateOwnsAttributes)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_registerFederationSynchronizationPoint(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "registerFederationSynchronizationPoint", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  std::wstring label;
  if (!PyObject_GetString(label, arg1)) {
    PyErr_SetString(PyExc_TypeError, "label needs to be a string!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg2)) {
    PyErr_SetString(PyExc_TypeError, "tag needs to be a bytearray!");
    return 0;
  }

  rti1516::FederateHandleSet federateHandleSet;
  if (arg3 && !PyObject_GetFederateHandleSet(federateHandleSet, arg3)) {
    PyErr_SetString(PyExc_TypeError, "syncSet needs to be a set of FederateHandles!");
    return 0;
  }

  try {

    self->ob_value->registerFederationSynchronizationPoint(label, tag, federateHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_synchronizationPointAchieved(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "synchronizationPointAchieved", 1, 1, &arg1))
    return 0;

  std::wstring label;
  if (!PyObject_GetString(label, arg1)) {
    PyErr_SetString(PyExc_TypeError, "label needs to be a string!");
    return 0;
  }

  try {

    self->ob_value->synchronizationPointAchieved(label);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(SynchronizationPointLabelNotAnnounced)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_requestFederationSave(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "requestFederationSave", 1, 2, &arg1, &arg2))
    return 0;

  std::wstring label;
  if (!PyObject_GetString(label, arg1)) {
    PyErr_SetString(PyExc_TypeError, "label needs to be a string!");
    return 0;
  }

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (arg2 && !PyObject_GetLogicalTime(logicalTime, arg2, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    if (logicalTime.get())
      self->ob_value->requestFederationSave(label, *logicalTime);
    else
      self->ob_value->requestFederationSave(label);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(FederateUnableToUseTime)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_federateSaveBegun(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "federateSaveBegun", 0, 0))
    return 0;

  try {

    self->ob_value->federateSaveBegun();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(SaveNotInitiated)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_federateSaveComplete(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "federateSaveComplete", 0, 0))
    return 0;

  try {

    self->ob_value->federateSaveComplete();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateHasNotBegunSave)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_federateSaveNotComplete(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "federateSaveNotComplete", 0, 0))
    return 0;

  try {

    self->ob_value->federateSaveNotComplete();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateHasNotBegunSave)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryFederationSaveStatus(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryFederationSaveStatus", 0, 0))
    return 0;

  try {

    self->ob_value->queryFederationSaveStatus();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_requestFederationRestore(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "requestFederationRestore", 1, 1, &arg1))
    return 0;

  std::wstring label;
  if (!PyObject_GetString(label, arg1)) {
    PyErr_SetString(PyExc_TypeError, "label needs to be a string!");
    return 0;
  }

  try {

    self->ob_value->requestFederationRestore(label);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_federateRestoreComplete(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "federateRestoreComplete", 0, 0))
    return 0;

  try {

    self->ob_value->federateRestoreComplete();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(RestoreNotRequested)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_federateRestoreNotComplete(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "federateRestoreNotComplete", 0, 0))
    return 0;

  try {

    self->ob_value->federateRestoreNotComplete();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(RestoreNotRequested)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryFederationRestoreStatus(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryFederationRestoreStatus", 0, 0))
    return 0;

  try {

    self->ob_value->queryFederationRestoreStatus();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_publishObjectClassAttributes(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "publishObjectClassAttributes", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->publishObjectClassAttributes(objectClassHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unpublishObjectClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "unpublishObjectClass", 1, 1, &arg1))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  try {

    self->ob_value->unpublishObjectClass(objectClassHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(OwnershipAcquisitionPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unpublishObjectClassAttributes(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unpublishObjectClassAttributes", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->unpublishObjectClassAttributes(objectClassHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(OwnershipAcquisitionPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_publishInteractionClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "publishInteractionClass", 1, 1, &arg1))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  try {

    self->ob_value->publishInteractionClass(interactionClassHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unpublishInteractionClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "unpublishInteractionClass", 1, 1, &arg1))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  try {

    self->ob_value->unpublishInteractionClass(interactionClassHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_subscribeObjectClassAttributes(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "subscribeObjectClassAttributes", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  bool active = true;
  if (arg3) {
    int t = PyObject_IsTrue(arg3);
    if (t == -1) {
      PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bool!");
      return 0;
    }
    active = (t == 1);
  }

  try {

    self->ob_value->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, active);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unsubscribeObjectClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "unsubscribeObjectClass", 1, 1, &arg1))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  try {

    self->ob_value->unsubscribeObjectClass(objectClassHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unsubscribeObjectClassAttributes(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unsubscribeObjectClassAttributes", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_subscribeInteractionClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "subscribeInteractionClass", 1, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  bool active = true;
  if (arg2) {
    int t = PyObject_IsTrue(arg2);
    if (t == -1) {
      PyErr_SetString(PyExc_TypeError, "Second argument needs to be a bool!");
      return 0;
    }
    active = (t == 1);
  }

  try {

    self->ob_value->subscribeInteractionClass(interactionClassHandle, active);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unsubscribeInteractionClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "unsubscribeInteractionClass", 1, 1, &arg1))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  try {

    self->ob_value->unsubscribeInteractionClass(interactionClassHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_reserveObjectInstanceName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "reserveObjectInstanceName", 1, 1, &arg1))
    return 0;

  std::wstring objectInstanceName;
  if (!PyObject_GetString(objectInstanceName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First argument needs to be a string!");
    return 0;
  }

  try {

    self->ob_value->reserveObjectInstanceName(objectInstanceName);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(IllegalName)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_registerObjectInstance(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "registerObjectInstance", 1, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  std::wstring objectInstanceName;
  if (arg2 && !PyObject_GetString(objectInstanceName, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a string!");
    return 0;
  }

  try {

    rti1516::ObjectInstanceHandle objectInstanceHandle;
    if (!arg2)
      objectInstanceHandle = self->ob_value->registerObjectInstance(objectClassHandle);
    else
      objectInstanceHandle = self->ob_value->registerObjectInstance(objectClassHandle, objectInstanceName);

    return PyObject_NewObjectInstanceHandle(objectInstanceHandle);
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(ObjectClassNotPublished)
  CATCH_C_EXCEPTION(ObjectInstanceNameNotReserved)
  CATCH_C_EXCEPTION(ObjectInstanceNameInUse)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_updateAttributeValues(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0, *arg4 = 0;
  if (!PyArg_UnpackTuple(args, "updateAttributeValues", 3, 4, &arg1, &arg2, &arg3, &arg4))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleValueMap attributeHandleValueMap;
  if (!PyObject_GetAttributeHandleValueMap(attributeHandleValueMap, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleValueMap!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (arg4 && !PyObject_GetLogicalTime(logicalTime, arg4, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Fourth argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    if (logicalTime.get()) {
      rti1516::MessageRetractionHandle messageRetractionHandle;
      messageRetractionHandle = self->ob_value->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, *logicalTime);

      return PyObject_NewMessageRetractionHandle(messageRetractionHandle);
    } else {
      self->ob_value->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag);

      Py_IncRef(Py_None);
      return Py_None;
    }
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_sendInteraction(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0, *arg4 = 0;
  if (!PyArg_UnpackTuple(args, "sendInteraction", 3, 4, &arg1, &arg2, &arg3, &arg4))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::ParameterHandleValueMap parameterHandleValueMap;
  if (!PyObject_GetParameterHandleValueMap(parameterHandleValueMap, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an ParameterHandleValueMap!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (arg4 && !PyObject_GetLogicalTime(logicalTime, arg4, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Fourth argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    if (logicalTime.get()) {
      rti1516::MessageRetractionHandle messageRetractionHandle;
      messageRetractionHandle = self->ob_value->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag, *logicalTime);

      return PyObject_NewMessageRetractionHandle(messageRetractionHandle);
    } else {
      self->ob_value->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag);

      Py_IncRef(Py_None);
      return Py_None;
    }
  }
  CATCH_C_EXCEPTION(InteractionClassNotPublished)
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InteractionParameterNotDefined)
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_deleteObjectInstance(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "deleteObjectInstance", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a bytearray!");
    return 0;
  }

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (arg3 && !PyObject_GetLogicalTime(logicalTime, arg3, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    if (logicalTime.get()) {
      rti1516::MessageRetractionHandle messageRetractionHandle;
      messageRetractionHandle = self->ob_value->deleteObjectInstance(objectInstanceHandle, tag, *logicalTime);

      return PyObject_NewMessageRetractionHandle(messageRetractionHandle);
    } else {
      self->ob_value->deleteObjectInstance(objectInstanceHandle, tag);

      Py_IncRef(Py_None);
      return Py_None;
    }
  }
  CATCH_C_EXCEPTION(DeletePrivilegeNotHeld)
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_localDeleteObjectInstance(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "localDeleteObjectInstance", 1, 1, &arg1))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  try {

    self->ob_value->localDeleteObjectInstance(objectInstanceHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(FederateOwnsAttributes)
  CATCH_C_EXCEPTION(OwnershipAcquisitionPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_changeAttributeTransportationType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "changeAttributeTransportationType", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::TransportationType transportationType;
  if (!PyObject_GetTransportationType(transportationType, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a TransportationType string!");
    return 0;
  }

  try {

    self->ob_value->changeAttributeTransportationType(objectInstanceHandle, attributeHandleSet, transportationType);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_changeInteractionTransportationType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "changeInteractionTransportationType", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::TransportationType transportationType;
  if (!PyObject_GetTransportationType(transportationType, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a TransportationType string!");
    return 0;
  }

  try {

    self->ob_value->changeInteractionTransportationType(interactionClassHandle, transportationType);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InteractionClassNotPublished)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_requestAttributeValueUpdate(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "requestAttributeValueUpdate", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  bool isClassHandle;
  rti1516::ObjectInstanceHandle objectInstanceHandle;
  rti1516::ObjectClassHandle objectClassHandle;
  if (PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    isClassHandle = true;
  } else if (PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    isClassHandle = false;
  } else {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle or ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  try {

    if (isClassHandle)
      self->ob_value->requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
    else
      self->ob_value->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unconditionalAttributeOwnershipDivestiture(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unconditionalAttributeOwnershipDivestiture", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_negotiatedAttributeOwnershipDivestiture(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "negotiatedAttributeOwnershipDivestiture", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  try {

    self->ob_value->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet, tag);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(AttributeAlreadyBeingDivested)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_confirmDivestiture(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "confirmDivestiture", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  try {

    self->ob_value->confirmDivestiture(objectInstanceHandle, attributeHandleSet, tag);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(AttributeDivestitureWasNotRequested)
  CATCH_C_EXCEPTION(NoAcquisitionPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_attributeOwnershipAcquisition(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "attributeOwnershipAcquisition", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  try {

    self->ob_value->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet, tag);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(ObjectClassNotPublished)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotPublished)
  CATCH_C_EXCEPTION(FederateOwnsAttributes)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_attributeOwnershipAcquisitionIfAvailable(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "attributeOwnershipAcquisitionIfAvailable", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(ObjectClassNotPublished)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotPublished)
  CATCH_C_EXCEPTION(FederateOwnsAttributes)
  CATCH_C_EXCEPTION(AttributeAlreadyBeingAcquired)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_attributeOwnershipDivestitureIfWanted(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "attributeOwnershipDivestitureIfWanted", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    rti1516::AttributeHandleSet divestedAttributeHandleSet;
    self->ob_value->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleSet, divestedAttributeHandleSet);

    return PyObject_NewAttributeHandleSet(divestedAttributeHandleSet);
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(ObjectClassNotPublished)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotPublished)
  CATCH_C_EXCEPTION(FederateOwnsAttributes)
  CATCH_C_EXCEPTION(AttributeAlreadyBeingAcquired)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_cancelNegotiatedAttributeOwnershipDivestiture(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "cancelNegotiatedAttributeOwnershipDivestiture", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(AttributeDivestitureWasNotRequested)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_cancelAttributeOwnershipAcquisition(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "cancelAttributeOwnershipAcquisition", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  try {

    self->ob_value->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(AttributeAcquisitionWasNotRequested)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryAttributeOwnership(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "queryAttributeOwnership", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandle attributeHandle;
  if (!PyObject_GetAttributeHandle(attributeHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandle!");
    return 0;
  }

  try {

    self->ob_value->queryAttributeOwnership(objectInstanceHandle, attributeHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_isAttributeOwnedByFederate(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "isAttributeOwnedByFederate", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandle attributeHandle;
  if (!PyObject_GetAttributeHandle(attributeHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandle!");
    return 0;
  }

  try {

    bool owned = self->ob_value->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);

    return PyBool_FromLong(owned);
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableTimeRegulation(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "enableTimeRegulation", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> logicalTimeInterval;
  if (!PyObject_GetLogicalTimeInterval(logicalTimeInterval, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTimeInterval!");
    return 0;
  }

  try {

    self->ob_value->enableTimeRegulation(*logicalTimeInterval);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(TimeRegulationAlreadyEnabled)
  CATCH_C_EXCEPTION(InvalidLookahead)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableTimeRegulation(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableTimeRegulation", 0, 0))
    return 0;

  try {

    self->ob_value->disableTimeRegulation();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(TimeRegulationIsNotEnabled)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableTimeConstrained(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableTimeConstrained", 0, 0))
    return 0;

  try {

    self->ob_value->enableTimeConstrained();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(TimeConstrainedAlreadyEnabled)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableTimeConstrained(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableTimeConstrained", 0, 0))
    return 0;

  try {

    self->ob_value->disableTimeConstrained();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(TimeConstrainedIsNotEnabled)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_timeAdvanceRequest(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "timeAdvanceRequest", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    self->ob_value->timeAdvanceRequest(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_timeAdvanceRequestAvailable(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "timeAdvanceRequestAvailable", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    self->ob_value->timeAdvanceRequestAvailable(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_nextMessageRequest(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "nextMessageRequest", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    self->ob_value->nextMessageRequest(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_nextMessageRequestAvailable(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "nextMessageRequestAvailable", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    self->ob_value->nextMessageRequestAvailable(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_flushQueueRequest(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "flushQueueRequest", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    self->ob_value->flushQueueRequest(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(LogicalTimeAlreadyPassed)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(RequestForTimeRegulationPending)
  CATCH_C_EXCEPTION(RequestForTimeConstrainedPending)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableAsynchronousDelivery(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableAsynchronousDelivery", 0, 0))
    return 0;

  try {

    self->ob_value->enableAsynchronousDelivery();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AsynchronousDeliveryAlreadyEnabled)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableAsynchronousDelivery(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableAsynchronousDelivery", 0, 0))
    return 0;

  try {

    self->ob_value->disableAsynchronousDelivery();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AsynchronousDeliveryAlreadyDisabled)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryGALT(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryGALT", 0, 0))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Cannot get LogicalTime for given factory!");
    return 0;
  }

  try {

    if (self->ob_value->queryGALT(*logicalTime))
      return PyObject_NewLogicalTime(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryLogicalTime(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryLogicalTime", 0, 0))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Cannot get LogicalTime for given factory!");
    return 0;
  }

  try {

    self->ob_value->queryLogicalTime(*logicalTime);

    return PyObject_NewLogicalTime(*logicalTime);
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryLITS(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryLITS", 0, 0))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (!PyObject_GetLogicalTime(logicalTime, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Cannot get LogicalTime for given factory!");
    return 0;
  }

  try {

    if (self->ob_value->queryLITS(*logicalTime))
      return PyObject_NewLogicalTime(*logicalTime);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_modifyLookahead(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "modifyLookahead", 1, 1, &arg1))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> logicalTimeInterval;
  if (!PyObject_GetLogicalTimeInterval(logicalTimeInterval, arg1, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Argument needs to be a LogicalTimeInterval!");
    return 0;
  }

  try {

    self->ob_value->modifyLookahead(*logicalTimeInterval);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(TimeRegulationIsNotEnabled)
  CATCH_C_EXCEPTION(InvalidLookahead)
  CATCH_C_EXCEPTION(InTimeAdvancingState)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_queryLookahead(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "queryLookahead", 0, 0))
    return 0;

  RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> logicalTimeInterval;
  if (!PyObject_GetLogicalTimeInterval(logicalTimeInterval, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Cannot get LogicalTimeInterval for given factory!");
    return 0;
  }

  try {

    self->ob_value->queryLookahead(*logicalTimeInterval);

    return PyObject_NewLogicalTimeInterval(*logicalTimeInterval);
  }
  CATCH_C_EXCEPTION(TimeRegulationIsNotEnabled)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_retract(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "retract", 1, 1, &arg1))
    return 0;

  rti1516::MessageRetractionHandle messageRetractionHandle;
  if (!PyObject_GetMessageRetractionHandle(messageRetractionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an MessageRetractionHandle!");
    return 0;
  }

  try {

    self->ob_value->retract(messageRetractionHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidRetractionHandle)
  CATCH_C_EXCEPTION(TimeRegulationIsNotEnabled)
  CATCH_C_EXCEPTION(MessageCanNoLongerBeRetracted)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_changeAttributeOrderType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "changeAttributeOrderType", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSet attributeHandleSet;
  if (!PyObject_GetAttributeHandleSet(attributeHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSet!");
    return 0;
  }

  rti1516::OrderType orderType;
  if (!PyObject_GetOrderType(orderType, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an OrderType!");
    return 0;
  }

  try {

    self->ob_value->changeAttributeOrderType(objectInstanceHandle, attributeHandleSet, orderType);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotOwned)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_changeInteractionOrderType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "changeInteractionOrderType", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::OrderType orderType;
  if (!PyObject_GetOrderType(orderType, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a OrderType string!");
    return 0;
  }

  try {

    self->ob_value->changeInteractionOrderType(interactionClassHandle, orderType);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InteractionClassNotPublished)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_createRegion(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "createRegion", 1, 1, &arg1))
    return 0;

  rti1516::DimensionHandleSet dimensionHandleSet;
  if (!PyObject_GetDimensionHandleSet(dimensionHandleSet, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be a DimensionHandleSet!");
    return 0;
  }

  try {

    rti1516::RegionHandle regionHandle;
    regionHandle = self->ob_value->createRegion(dimensionHandleSet);

    return PyObject_NewRegionHandle(regionHandle);
  }
  CATCH_C_EXCEPTION(InvalidDimensionHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_commitRegionModifications(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "commitRegionModifications", 1, 1, &arg1))
    return 0;

  rti1516::RegionHandleSet regionHandleSet;
  if (!PyObject_GetRegionHandleSet(regionHandleSet, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be a RegionHandleSet!");
    return 0;
  }

  try {

    self->ob_value->commitRegionModifications(regionHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_deleteRegion(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "deleteRegion", 1, 1, &arg1))
    return 0;

  rti1516::RegionHandle regionHandle;
  if (!PyObject_GetRegionHandle(regionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be a RegionHandle!");
    return 0;
  }

  try {

    self->ob_value->deleteRegion(regionHandle);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(RegionInUseForUpdateOrSubscription)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_registerObjectInstanceWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "registerObjectInstanceWithRegions", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  std::wstring objectInstanceName;
  if (arg3 && !PyObject_GetString(objectInstanceName, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a string!");
    return 0;
  }

  try {

    rti1516::ObjectInstanceHandle objectInstanceHandle;
    if (!arg3)
      objectInstanceHandle = self->ob_value->registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector);
    else
      objectInstanceHandle = self->ob_value->registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, objectInstanceName);

    return PyObject_NewObjectInstanceHandle(objectInstanceHandle);
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(ObjectClassNotPublished)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(AttributeNotPublished)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(ObjectInstanceNameNotReserved)
  CATCH_C_EXCEPTION(ObjectInstanceNameInUse)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_associateRegionsForUpdates(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "associateRegionsForUpdates", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  try {

    self->ob_value->associateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unassociateRegionsForUpdates(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unassociateRegionsForUpdates", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectInstanceHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  try {

    self->ob_value->unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_subscribeObjectClassAttributesWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "subscribeObjectClassAttributesWithRegions", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  bool active = true;
  if (arg3) {
    int t = PyObject_IsTrue(arg3);
    if (t == -1) {
      PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bool!");
      return 0;
    }
    active = (t == 1);
  }

  try {

    self->ob_value->subscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, active);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unsubscribeObjectClassAttributesWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unsubscribeObjectClassAttributesWithRegions", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  try {

    self->ob_value->unsubscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_subscribeInteractionClassWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "subscribeInteractionClassAttributesWithRegions", 2, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::RegionHandleSet regionHandleSet;
  if (!PyObject_GetRegionHandleSet(regionHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a RegionHandleSet!");
    return 0;
  }

  bool active = true;
  if (arg3) {
    int t = PyObject_IsTrue(arg3);
    if (t == -1) {
      PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bool!");
      return 0;
    }
    active = (t == 1);
  }

  try {

    self->ob_value->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet, active);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_unsubscribeInteractionClassWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "unsubscribeInteractionClassAttributesWithRegions", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::RegionHandleSet regionHandleSet;
  if (!PyObject_GetRegionHandleSet(regionHandleSet, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be a RegionHandleSet!");
    return 0;
  }

  try {

    self->ob_value->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_sendInteractionWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0, *arg4 = 0, *arg5 = 0;
  if (!PyArg_UnpackTuple(args, "sendInteractionWithRegions", 4, 5, &arg1, &arg2, &arg3, &arg4, &arg5))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::ParameterHandleValueMap parameterHandleValueMap;
  if (!PyObject_GetParameterHandleValueMap(parameterHandleValueMap, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an ParameterHandleValueMap!");
    return 0;
  }

  rti1516::RegionHandleSet regionHandleSet;
  if (!PyObject_GetRegionHandleSet(regionHandleSet, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a RegionHandleSet!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg4)) {
    PyErr_SetString(PyExc_TypeError, "Fourth argument needs to be a bytearray!");
    return 0;
  }

  RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime;
  if (arg5 && !PyObject_GetLogicalTime(logicalTime, arg5, self->_logicaltimeFactoryName)) {
    PyErr_SetString(PyExc_TypeError, "Fifth argument needs to be a LogicalTime!");
    return 0;
  }

  try {

    if (logicalTime.get()) {
      rti1516::MessageRetractionHandle messageRetractionHandle;
      messageRetractionHandle = self->ob_value->sendInteractionWithRegions(interactionClassHandle, parameterHandleValueMap, regionHandleSet, tag, *logicalTime);

      return PyObject_NewMessageRetractionHandle(messageRetractionHandle);
    } else {
      self->ob_value->sendInteractionWithRegions(interactionClassHandle, parameterHandleValueMap, regionHandleSet, tag);

      Py_IncRef(Py_None);
      return Py_None;
    }
  }
  CATCH_C_EXCEPTION(InteractionClassNotPublished)
  CATCH_C_EXCEPTION(InteractionClassNotDefined)
  CATCH_C_EXCEPTION(InteractionParameterNotDefined)
  CATCH_C_EXCEPTION(InvalidLogicalTime)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_requestAttributeValueUpdateWithRegions(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "requestAttributeValueUpdateWithRegions", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector;
  if (!PyObject_GetAttributeHandleSetRegionHandleSetPairVector(attributeHandleSetRegionHandleSetPairVector, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be an AttributeHandleSetRegionHandleSetPairVector!");
    return 0;
  }

  rti1516::VariableLengthData tag;
  if (!PyObject_GetVariableLengthData(tag, arg3)) {
    PyErr_SetString(PyExc_TypeError, "Third argument needs to be a bytearray!");
    return 0;
  }

  try {

    self->ob_value->requestAttributeValueUpdateWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, tag);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassNotDefined)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(InvalidRegionContext)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}


static PyObject *
PyRTIambassador_getObjectClassHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getObjectClassHandle", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::ObjectClassHandle objectClassHandle = self->ob_value->getObjectClassHandle(theName);
    return PyObject_NewObjectClassHandle(objectClassHandle);
  }
  CATCH_C_EXCEPTION(NameNotFound)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getObjectClassName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getObjectClassName", 1, 1, &arg1))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an ObjectClassHandle!");
    return 0;
  }

  try {
    std::wstring objectClassName = self->ob_value->getObjectClassName(objectClassHandle);
    return PyObject_NewString(objectClassName);
  }
  CATCH_C_EXCEPTION(InvalidObjectClassHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getAttributeHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getAttributeHandle", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "whichClass needs to be an ObjectClassHandle!");
    return 0;
  }

  std::wstring theAttributeName;
  if (!PyObject_GetString(theAttributeName, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theAttributeName needs to be a string!");
    return 0;
  }

  try {
    rti1516::AttributeHandle attributeHandle = self->ob_value->getAttributeHandle(objectClassHandle, theAttributeName);
    return PyObject_NewAttributeHandle(attributeHandle);
  }
  CATCH_C_EXCEPTION(InvalidObjectClassHandle)
  CATCH_C_EXCEPTION(NameNotFound)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getAttributeName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getAttributeName", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "whichClass needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandle attributeHandle;
  if (!PyObject_GetAttributeHandle(attributeHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an AttributeHandle!");
    return 0;
  }

  try {
    std::wstring attributeName = self->ob_value->getAttributeName(objectClassHandle, attributeHandle);
    return PyObject_NewString(attributeName);
  }
  CATCH_C_EXCEPTION(InvalidObjectClassHandle)
  CATCH_C_EXCEPTION(InvalidAttributeHandle)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getInteractionClassHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getInteractionClassHandle", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::InteractionClassHandle interactionClassHandle = self->ob_value->getInteractionClassHandle(theName);
    return PyObject_NewInteractionClassHandle(interactionClassHandle);
  }
  CATCH_C_EXCEPTION(NameNotFound)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getInteractionClassName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getInteractionClassName", 1, 1, &arg1))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an InteractionClassHandle!");
    return 0;
  }

  try {
    std::wstring interactionClassName = self->ob_value->getInteractionClassName(interactionClassHandle);
    return PyObject_NewString(interactionClassName);
  }
  CATCH_C_EXCEPTION(InvalidInteractionClassHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getParameterHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getParameterHandle", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "whichClass needs to be an InteractionClassHandle!");
    return 0;
  }

  std::wstring theParameterName;
  if (!PyObject_GetString(theParameterName, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theParameterName needs to be a string!");
    return 0;
  }

  try {
    rti1516::ParameterHandle parameterHandle = self->ob_value->getParameterHandle(interactionClassHandle, theParameterName);
    return PyObject_NewParameterHandle(parameterHandle);
  }
  CATCH_C_EXCEPTION(InvalidInteractionClassHandle)
  CATCH_C_EXCEPTION(NameNotFound)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getParameterName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getParameterName", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "whichClass needs to be an InteractionClassHandle!");
    return 0;
  }

  rti1516::ParameterHandle parameterHandle;
  if (!PyObject_GetParameterHandle(parameterHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an ParameterHandle!");
    return 0;
  }

  try {
    std::wstring parameterName = self->ob_value->getParameterName(interactionClassHandle, parameterHandle);
    return PyObject_NewString(parameterName);
  }
  CATCH_C_EXCEPTION(InvalidInteractionClassHandle)
  CATCH_C_EXCEPTION(InvalidParameterHandle)
  CATCH_C_EXCEPTION(InteractionParameterNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getObjectInstanceHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getObjectInstanceHandle", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::ObjectInstanceHandle objectInstanceHandle = self->ob_value->getObjectInstanceHandle(theName);
    return PyObject_NewObjectInstanceHandle(objectInstanceHandle);
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getObjectInstanceName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getObjectInstanceName", 1, 1, &arg1))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an ObjectInstanceHandle!");
    return 0;
  }

  try {
    std::wstring objectInstanceName = self->ob_value->getObjectInstanceName(objectInstanceHandle);
    return PyObject_NewString(objectInstanceName);
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getDimensionHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getDimensionHandle", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::DimensionHandle dimensionHandle = self->ob_value->getDimensionHandle(theName);
    return PyObject_NewDimensionHandle(dimensionHandle);
  }
  CATCH_C_EXCEPTION(NameNotFound)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getDimensionName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getDimensionName", 1, 1, &arg1))
    return 0;

  rti1516::DimensionHandle dimensionHandle;
  if (!PyObject_GetDimensionHandle(dimensionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an DimensionHandle!");
    return 0;
  }

  try {
    std::wstring dimensionName = self->ob_value->getDimensionName(dimensionHandle);
    return PyObject_NewString(dimensionName);
  }
  CATCH_C_EXCEPTION(InvalidDimensionHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getDimensionUpperBound(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getDimensionUpperBound", 1, 1, &arg1))
    return 0;

  rti1516::DimensionHandle dimensionHandle;
  if (!PyObject_GetDimensionHandle(dimensionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an DimensionHandle!");
    return 0;
  }

  try {
    unsigned long upperBound = self->ob_value->getDimensionUpperBound(dimensionHandle);
    return PyLong_FromUnsignedLong(upperBound);
  }
  CATCH_C_EXCEPTION(InvalidDimensionHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getAvailableDimensionsForClassAttribute(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getAvailableDimensionsForClassAttribute", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::ObjectClassHandle objectClassHandle;
  if (!PyObject_GetObjectClassHandle(objectClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theClass needs to be an ObjectClassHandle!");
    return 0;
  }

  rti1516::AttributeHandle attributeHandle;
  if (!PyObject_GetAttributeHandle(attributeHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theHandle needs to be an AttributeHandle!");
    return 0;
  }

  try {
    rti1516::DimensionHandleSet dimensionHandleSet;
    dimensionHandleSet = self->ob_value->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle);
    return PyObject_NewDimensionHandleSet(dimensionHandleSet);
  }
  CATCH_C_EXCEPTION(InvalidObjectClassHandle)
  CATCH_C_EXCEPTION(InvalidAttributeHandle)
  CATCH_C_EXCEPTION(AttributeNotDefined)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getKnownObjectClassHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getKnownObjectClassHandle", 1, 1, &arg1))
    return 0;

  rti1516::ObjectInstanceHandle objectInstanceHandle;
  if (!PyObject_GetObjectInstanceHandle(objectInstanceHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theObject needs to be an ObjectInstanceHandle!");
    return 0;
  }

  try {
    rti1516::ObjectClassHandle objectClassHandle = self->ob_value->getKnownObjectClassHandle(objectInstanceHandle);
    return PyObject_NewObjectClassHandle(objectClassHandle);
  }
  CATCH_C_EXCEPTION(ObjectInstanceNotKnown)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getAvailableDimensionsForInteractionClass(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getAvailableDimensionsForInteractionClass", 1, 1, &arg1))
    return 0;

  rti1516::InteractionClassHandle interactionClassHandle;
  if (!PyObject_GetInteractionClassHandle(interactionClassHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theClass needs to be an InteractionClassHandle!");
    return 0;
  }

  try {
    rti1516::DimensionHandleSet dimensionHandleSet;
    dimensionHandleSet = self->ob_value->getAvailableDimensionsForInteractionClass(interactionClassHandle);
    return PyObject_NewDimensionHandleSet(dimensionHandleSet);
  }
  CATCH_C_EXCEPTION(InvalidInteractionClassHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getTransportationType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getTransportationType", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::TransportationType transportationType = self->ob_value->getTransportationType(theName);
    return PyObject_NewTransportationType(transportationType);
  }
  CATCH_C_EXCEPTION(InvalidTransportationName)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getTransportationName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getTransportationName", 1, 1, &arg1))
    return 0;

  rti1516::TransportationType transportationType;
  if (!PyObject_GetTransportationType(transportationType, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theType needs to be an TransportationType!");
    return 0;
  }

  try {
    std::wstring transportationName = self->ob_value->getTransportationName(transportationType);
    return PyObject_NewString(transportationName);
  }
  CATCH_C_EXCEPTION(InvalidTransportationType)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getOrderType(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getOrderType", 1, 1, &arg1))
    return 0;

  std::wstring theName;
  if (!PyObject_GetString(theName, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theName needs to be a string!");
    return 0;
  }

  try {
    rti1516::OrderType transportationType = self->ob_value->getOrderType(theName);
    return PyObject_NewOrderType(transportationType);
  }
  CATCH_C_EXCEPTION(InvalidOrderName)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getOrderName(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getOrderName", 1, 1, &arg1))
    return 0;

  rti1516::OrderType transportationType;
  if (!PyObject_GetOrderType(transportationType, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theType needs to be an OrderType!");
    return 0;
  }

  try {
    std::wstring transportationName = self->ob_value->getOrderName(transportationType);
    return PyObject_NewString(transportationName);
  }
  CATCH_C_EXCEPTION(InvalidOrderType)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableObjectClassRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableObjectClassRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->enableObjectClassRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOn)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableObjectClassRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableObjectClassRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->disableObjectClassRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOff)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableAttributeRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableAttributeRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->enableAttributeRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOn)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableAttributeRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableAttributeRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->disableAttributeRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOff)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableAttributeScopeAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableAttributeScopeAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->enableAttributeScopeAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AttributeScopeAdvisorySwitchIsOn)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableAttributeScopeAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableAttributeScopeAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->disableAttributeScopeAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(AttributeScopeAdvisorySwitchIsOff)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableInteractionRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableInteractionRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->enableInteractionRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOn)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableInteractionRelevanceAdvisorySwitch(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableInteractionRelevanceAdvisorySwitch", 0, 0))
    return 0;

  try {

    self->ob_value->disableInteractionRelevanceAdvisorySwitch();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOff)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getDimensionHandleSet(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "getDimensionHandleSet", 1, 1, &arg1))
    return 0;

  rti1516::RegionHandle regionHandle;
  if (!PyObject_GetRegionHandle(regionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theRegionHandle needs to be an RegionHandle!");
    return 0;
  }

  try {
    rti1516::DimensionHandleSet dimensionHandleSet;
    dimensionHandleSet = self->ob_value->getDimensionHandleSet(regionHandle);
    return PyObject_NewDimensionHandleSet(dimensionHandleSet);
  }
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_getRangeBounds(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "getRangeBounds", 2, 2, &arg1, &arg2))
    return 0;

  rti1516::RegionHandle regionHandle;
  if (!PyObject_GetRegionHandle(regionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theRegionHandle needs to be an RegionHandle!");
    return 0;
  }

  rti1516::DimensionHandle dimensionHandle;
  if (!PyObject_GetDimensionHandle(dimensionHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theDimensionHandle needs to be an DimensionHandle!");
    return 0;
  }

  try {
    rti1516::RangeBounds rangeBounds = self->ob_value->getRangeBounds(regionHandle, dimensionHandle);
    return PyObject_NewRangeBounds(rangeBounds);
  }
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionDoesNotContainSpecifiedDimension)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_setRangeBounds(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0, *arg3 = 0;
  if (!PyArg_UnpackTuple(args, "setRangeBounds", 3, 3, &arg1, &arg2, &arg3))
    return 0;

  rti1516::RegionHandle regionHandle;
  if (!PyObject_GetRegionHandle(regionHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theRegionHandle needs to be an RegionHandle!");
    return 0;
  }

  rti1516::DimensionHandle dimensionHandle;
  if (!PyObject_GetDimensionHandle(dimensionHandle, arg2)) {
    PyErr_SetString(PyExc_TypeError, "theDimensionHandle needs to be an DimensionHandle!");
    return 0;
  }

  rti1516::RangeBounds rangeBounds;
  if (!PyObject_GetRangeBounds(rangeBounds, arg3)) {
    PyErr_SetString(PyExc_TypeError, "theRangeBounds needs to be a 2 Tuple of unsigned int!");
    return 0;
  }

  try {
    self->ob_value->setRangeBounds(regionHandle, dimensionHandle, rangeBounds);

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(InvalidRegion)
  CATCH_C_EXCEPTION(RegionNotCreatedByThisFederate)
  CATCH_C_EXCEPTION(RegionDoesNotContainSpecifiedDimension)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_normalizeFederateHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "normalizeFederateHandle", 1, 1, &arg1))
    return 0;

  rti1516::FederateHandle federateHandle;
  if (!PyObject_GetFederateHandle(federateHandle, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theFederateHandle needs to be a FederateHandle!");
    return 0;
  }

  try {
    unsigned long normalized = self->ob_value->normalizeFederateHandle(federateHandle);
    return PyLong_FromLongLong(normalized);
  }
  CATCH_C_EXCEPTION(InvalidFederateHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_normalizeServiceGroup(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "normalizeServiceGroup", 1, 1, &arg1))
    return 0;

  rti1516::ServiceGroupIndicator serviceGroup;
  if (!PyObject_GetServiceGroupIndicator(serviceGroup, arg1)) {
    PyErr_SetString(PyExc_TypeError, "theFederateHandle needs to be a FederateHandle!");
    return 0;
  }

  try {
    unsigned long normalized = self->ob_value->normalizeServiceGroup(serviceGroup);
    return PyLong_FromLongLong(normalized);
  }
  CATCH_C_EXCEPTION(InvalidFederateHandle)
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_evokeCallback(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "evokeCallback", 1, 1, &arg1))
    return 0;

  double approximateMinimumTimeInSeconds;
  if (!PyObject_GetDouble(approximateMinimumTimeInSeconds, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be a number!");
    return 0;
  }

  PyThreadState *_save;
  _save = PyEval_SaveThread();
  try {

    bool more = self->ob_value->evokeCallback(approximateMinimumTimeInSeconds);

    PyEval_RestoreThread(_save);

    return PyBool_FromLong(more);
  }
  CATCH_C_EXCEPTION_THREADS(FederateNotExecutionMember)
  CATCH_C_EXCEPTION_THREADS(RTIinternalError)
}

static PyObject *
PyRTIambassador_evokeMultipleCallbacks(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0, *arg2 = 0;
  if (!PyArg_UnpackTuple(args, "evokeMultipleCallbacks", 2, 2, &arg1, &arg2))
    return 0;

  double approximateMinimumTimeInSeconds;
  if (!PyObject_GetDouble(approximateMinimumTimeInSeconds, arg1)) {
    PyErr_SetString(PyExc_TypeError, "First Argument needs to be a number!");
    return 0;
  }

  double approximateMaximumTimeInSeconds;
  if (!PyObject_GetDouble(approximateMaximumTimeInSeconds, arg2)) {
    PyErr_SetString(PyExc_TypeError, "Second Argument needs to be a number!");
    return 0;
  }

  PyThreadState *_save;
  _save = PyEval_SaveThread();
  try {

    bool more = self->ob_value->evokeMultipleCallbacks(approximateMinimumTimeInSeconds,
                                                       approximateMaximumTimeInSeconds);

    PyEval_RestoreThread(_save);

    return PyBool_FromLong(more);
  }
  CATCH_C_EXCEPTION_THREADS(FederateNotExecutionMember)
  CATCH_C_EXCEPTION_THREADS(RTIinternalError)
}

static PyObject *
PyRTIambassador_enableCallbacks(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "enableCallbacks", 0, 0))
    return 0;

  try {
    self->ob_value->enableCallbacks();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_disableCallbacks(PyRTIambassadorObject *self, PyObject *args)
{
  if (!PyArg_UnpackTuple(args, "disableCallbacks", 0, 0))
    return 0;

  try {
    self->ob_value->disableCallbacks();

    Py_IncRef(Py_None);
    return Py_None;
  }
  CATCH_C_EXCEPTION(FederateNotExecutionMember)
  CATCH_C_EXCEPTION(SaveInProgress)
  CATCH_C_EXCEPTION(RestoreInProgress)
  CATCH_C_EXCEPTION(RTIinternalError)
}

static PyObject *
PyRTIambassador_decodeFederateHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeFederateHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::FederateHandle federateHandle;
    federateHandle = self->ob_value->decodeFederateHandle(variableLengthData);
    return PyObject_NewFederateHandle(federateHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeObjectClassHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeObjectClassHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::ObjectClassHandle objectClassHandle;
    objectClassHandle = self->ob_value->decodeObjectClassHandle(variableLengthData);
    return PyObject_NewObjectClassHandle(objectClassHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeInteractionClassHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeInteractionClassHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::InteractionClassHandle interactionClassHandle;
    interactionClassHandle = self->ob_value->decodeInteractionClassHandle(variableLengthData);
    return PyObject_NewInteractionClassHandle(interactionClassHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeObjectInstanceHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeObjectInstanceHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::ObjectInstanceHandle objectInstanceHandle;
    objectInstanceHandle = self->ob_value->decodeObjectInstanceHandle(variableLengthData);
    return PyObject_NewObjectInstanceHandle(objectInstanceHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeAttributeHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeAttributeHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::AttributeHandle attributeHandle;
    attributeHandle = self->ob_value->decodeAttributeHandle(variableLengthData);
    return PyObject_NewAttributeHandle(attributeHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeParameterHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeParameterHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::ParameterHandle parameterHandle;
    parameterHandle = self->ob_value->decodeParameterHandle(variableLengthData);
    return PyObject_NewParameterHandle(parameterHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeDimensionHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeDimensionHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::DimensionHandle dimensionHandle;
    dimensionHandle = self->ob_value->decodeDimensionHandle(variableLengthData);
    return PyObject_NewDimensionHandle(dimensionHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeMessageRetractionHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeMessageRetractionHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::MessageRetractionHandle messageRetractionHandle;
    messageRetractionHandle = self->ob_value->decodeMessageRetractionHandle(variableLengthData);
    return PyObject_NewMessageRetractionHandle(messageRetractionHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyObject *
PyRTIambassador_decodeRegionHandle(PyRTIambassadorObject *self, PyObject *args)
{
  PyObject *arg1 = 0;
  if (!PyArg_UnpackTuple(args, "decodeRegionHandle", 1, 1, &arg1))
    return 0;

  rti1516::VariableLengthData variableLengthData;
  if (!PyObject_GetVariableLengthData(variableLengthData, arg1)) {
    PyErr_SetString(PyExc_TypeError, "encodedValue needs to be a bytearray!");
    return 0;
  }

  try {
    rti1516::RegionHandle regionHandle;
    regionHandle = self->ob_value->decodeRegionHandle(variableLengthData);
    return PyObject_NewRegionHandle(regionHandle);
  }
  CATCH_C_EXCEPTION(Exception)
}

static PyMethodDef PyRTIambassador_methods[] =
{
  // Two methods not mentioned in the standard, but to make automatic conversion to python values work
  {"setLogicalTimeFactory", (PyCFunction)PyRTIambassador_setLogicalTimeFactory, METH_VARARGS, ""},
  {"getLogicalTimeFactory", (PyCFunction)PyRTIambassador_getLogicalTimeFactory, METH_VARARGS, ""},

  // The set of methods as found in the rti1516 standard
  {"createFederationExecution", (PyCFunction)PyRTIambassador_createFederationExecution, METH_VARARGS, ""},
  {"destroyFederationExecution", (PyCFunction)PyRTIambassador_destroyFederationExecution, METH_VARARGS, ""},
  {"joinFederationExecution", (PyCFunction)PyRTIambassador_joinFederationExecution, METH_VARARGS, ""},
  {"resignFederationExecution", (PyCFunction)PyRTIambassador_resignFederationExecution, METH_VARARGS, ""},
  {"registerFederationSynchronizationPoint", (PyCFunction)PyRTIambassador_registerFederationSynchronizationPoint, METH_VARARGS, ""},
  {"synchronizationPointAchieved", (PyCFunction)PyRTIambassador_synchronizationPointAchieved, METH_VARARGS, ""},
  {"requestFederationSave", (PyCFunction)PyRTIambassador_requestFederationSave, METH_VARARGS, ""},
  {"federateSaveBegun", (PyCFunction)PyRTIambassador_federateSaveBegun, METH_VARARGS, ""},
  {"federateSaveComplete", (PyCFunction)PyRTIambassador_federateSaveComplete, METH_VARARGS, ""},
  {"federateSaveNotComplete", (PyCFunction)PyRTIambassador_federateSaveNotComplete, METH_VARARGS, ""},
  {"queryFederationSaveStatus", (PyCFunction)PyRTIambassador_queryFederationSaveStatus, METH_VARARGS, ""},
  {"requestFederationRestore", (PyCFunction)PyRTIambassador_requestFederationRestore, METH_VARARGS, ""},
  {"federateRestoreComplete", (PyCFunction)PyRTIambassador_federateRestoreComplete, METH_VARARGS, ""},
  {"federateRestoreNotComplete", (PyCFunction)PyRTIambassador_federateRestoreNotComplete, METH_VARARGS, ""},
  {"queryFederationRestoreStatus", (PyCFunction)PyRTIambassador_queryFederationRestoreStatus, METH_VARARGS, ""},
  {"publishObjectClassAttributes", (PyCFunction)PyRTIambassador_publishObjectClassAttributes, METH_VARARGS, ""},
  {"unpublishObjectClass", (PyCFunction)PyRTIambassador_unpublishObjectClass, METH_VARARGS, ""},
  {"unpublishObjectClassAttributes", (PyCFunction)PyRTIambassador_unpublishObjectClassAttributes, METH_VARARGS, ""},
  {"publishInteractionClass", (PyCFunction)PyRTIambassador_publishInteractionClass, METH_VARARGS, ""},
  {"unpublishInteractionClass", (PyCFunction)PyRTIambassador_unpublishInteractionClass, METH_VARARGS, ""},
  {"subscribeObjectClassAttributes", (PyCFunction)PyRTIambassador_subscribeObjectClassAttributes, METH_VARARGS, ""},
  {"unsubscribeObjectClass", (PyCFunction)PyRTIambassador_unsubscribeObjectClass, METH_VARARGS, ""},
  {"unsubscribeObjectClassAttributes", (PyCFunction)PyRTIambassador_unsubscribeObjectClassAttributes, METH_VARARGS, ""},
  {"subscribeInteractionClass", (PyCFunction)PyRTIambassador_subscribeInteractionClass, METH_VARARGS, ""},
  {"unsubscribeInteractionClass", (PyCFunction)PyRTIambassador_unsubscribeInteractionClass, METH_VARARGS, ""},
  {"reserveObjectInstanceName", (PyCFunction)PyRTIambassador_reserveObjectInstanceName, METH_VARARGS, ""},
  {"registerObjectInstance", (PyCFunction)PyRTIambassador_registerObjectInstance, METH_VARARGS, ""},
  {"updateAttributeValues", (PyCFunction)PyRTIambassador_updateAttributeValues, METH_VARARGS, ""},
  {"sendInteraction", (PyCFunction)PyRTIambassador_sendInteraction, METH_VARARGS, ""},
  {"deleteObjectInstance", (PyCFunction)PyRTIambassador_deleteObjectInstance, METH_VARARGS, ""},
  {"localDeleteObjectInstance", (PyCFunction)PyRTIambassador_localDeleteObjectInstance, METH_VARARGS, ""},
  {"changeAttributeTransportationType", (PyCFunction)PyRTIambassador_changeAttributeTransportationType, METH_VARARGS, ""},
  {"changeInteractionTransportationType", (PyCFunction)PyRTIambassador_changeInteractionTransportationType, METH_VARARGS, ""},
  {"requestAttributeValueUpdate", (PyCFunction)PyRTIambassador_requestAttributeValueUpdate, METH_VARARGS, ""},
  {"unconditionalAttributeOwnershipDivestiture", (PyCFunction)PyRTIambassador_unconditionalAttributeOwnershipDivestiture, METH_VARARGS, ""},
  {"negotiatedAttributeOwnershipDivestiture", (PyCFunction)PyRTIambassador_negotiatedAttributeOwnershipDivestiture, METH_VARARGS, ""},
  {"confirmDivestiture", (PyCFunction)PyRTIambassador_confirmDivestiture, METH_VARARGS, ""},
  {"attributeOwnershipAcquisition", (PyCFunction)PyRTIambassador_attributeOwnershipAcquisition, METH_VARARGS, ""},
  {"attributeOwnershipAcquisitionIfAvailable", (PyCFunction)PyRTIambassador_attributeOwnershipAcquisitionIfAvailable, METH_VARARGS, ""},
  {"attributeOwnershipDivestitureIfWanted", (PyCFunction)PyRTIambassador_attributeOwnershipDivestitureIfWanted, METH_VARARGS, ""},
  {"cancelNegotiatedAttributeOwnershipDivestiture", (PyCFunction)PyRTIambassador_cancelNegotiatedAttributeOwnershipDivestiture, METH_VARARGS, ""},
  {"cancelAttributeOwnershipAcquisition", (PyCFunction)PyRTIambassador_cancelAttributeOwnershipAcquisition, METH_VARARGS, ""},
  {"queryAttributeOwnership", (PyCFunction)PyRTIambassador_queryAttributeOwnership, METH_VARARGS, ""},
  {"isAttributeOwnedByFederate", (PyCFunction)PyRTIambassador_isAttributeOwnedByFederate, METH_VARARGS, ""},
  {"enableTimeRegulation", (PyCFunction)PyRTIambassador_enableTimeRegulation, METH_VARARGS, ""},
  {"disableTimeRegulation", (PyCFunction)PyRTIambassador_disableTimeRegulation, METH_VARARGS, ""},
  {"enableTimeConstrained", (PyCFunction)PyRTIambassador_enableTimeConstrained, METH_VARARGS, ""},
  {"disableTimeConstrained", (PyCFunction)PyRTIambassador_disableTimeConstrained, METH_VARARGS, ""},
  {"timeAdvanceRequest", (PyCFunction)PyRTIambassador_timeAdvanceRequest, METH_VARARGS, ""},
  {"timeAdvanceRequestAvailable", (PyCFunction)PyRTIambassador_timeAdvanceRequestAvailable, METH_VARARGS, ""},
  {"nextMessageRequest", (PyCFunction)PyRTIambassador_nextMessageRequest, METH_VARARGS, ""},
  {"nextMessageRequestAvailable", (PyCFunction)PyRTIambassador_nextMessageRequestAvailable, METH_VARARGS, ""},
  {"flushQueueRequest", (PyCFunction)PyRTIambassador_flushQueueRequest, METH_VARARGS, ""},
  {"enableAsynchronousDelivery", (PyCFunction)PyRTIambassador_enableAsynchronousDelivery, METH_VARARGS, ""},
  {"disableAsynchronousDelivery", (PyCFunction)PyRTIambassador_disableAsynchronousDelivery, METH_VARARGS, ""},
  {"queryGALT", (PyCFunction)PyRTIambassador_queryGALT, METH_VARARGS, ""},
  {"queryLogicalTime", (PyCFunction)PyRTIambassador_queryLogicalTime, METH_VARARGS, ""},
  {"queryLITS", (PyCFunction)PyRTIambassador_queryLITS, METH_VARARGS, ""},
  {"modifyLookahead", (PyCFunction)PyRTIambassador_modifyLookahead, METH_VARARGS, ""},
  {"queryLookahead", (PyCFunction)PyRTIambassador_queryLookahead, METH_VARARGS, ""},
  {"retract", (PyCFunction)PyRTIambassador_retract, METH_VARARGS, ""},
  {"changeAttributeOrderType", (PyCFunction)PyRTIambassador_changeAttributeOrderType, METH_VARARGS, ""},
  {"changeInteractionOrderType", (PyCFunction)PyRTIambassador_changeInteractionOrderType, METH_VARARGS, ""},
  {"createRegion", (PyCFunction)PyRTIambassador_createRegion, METH_VARARGS, ""},
  {"commitRegionModifications", (PyCFunction)PyRTIambassador_commitRegionModifications, METH_VARARGS, ""},
  {"deleteRegion", (PyCFunction)PyRTIambassador_deleteRegion, METH_VARARGS, ""},
  {"registerObjectInstanceWithRegions", (PyCFunction)PyRTIambassador_registerObjectInstanceWithRegions, METH_VARARGS, ""},
  {"associateRegionsForUpdates", (PyCFunction)PyRTIambassador_associateRegionsForUpdates, METH_VARARGS, ""},
  {"unassociateRegionsForUpdates", (PyCFunction)PyRTIambassador_unassociateRegionsForUpdates, METH_VARARGS, ""},
  {"subscribeObjectClassAttributesWithRegions", (PyCFunction)PyRTIambassador_subscribeObjectClassAttributesWithRegions, METH_VARARGS, ""},
  {"unsubscribeObjectClassAttributesWithRegions", (PyCFunction)PyRTIambassador_unsubscribeObjectClassAttributesWithRegions, METH_VARARGS, ""},
  {"subscribeInteractionClassWithRegions", (PyCFunction)PyRTIambassador_subscribeInteractionClassWithRegions, METH_VARARGS, ""},
  {"unsubscribeInteractionClassWithRegions", (PyCFunction)PyRTIambassador_unsubscribeInteractionClassWithRegions, METH_VARARGS, ""},
  {"sendInteractionWithRegions", (PyCFunction)PyRTIambassador_sendInteractionWithRegions, METH_VARARGS, ""},
  {"requestAttributeValueUpdateWithRegions", (PyCFunction)PyRTIambassador_requestAttributeValueUpdateWithRegions, METH_VARARGS, ""},
  {"getObjectClassHandle", (PyCFunction)PyRTIambassador_getObjectClassHandle, METH_VARARGS, ""},
  {"getObjectClassName", (PyCFunction)PyRTIambassador_getObjectClassName, METH_VARARGS, ""},
  {"getAttributeHandle", (PyCFunction)PyRTIambassador_getAttributeHandle, METH_VARARGS, ""},
  {"getAttributeName", (PyCFunction)PyRTIambassador_getAttributeName, METH_VARARGS, ""},
  {"getInteractionClassHandle", (PyCFunction)PyRTIambassador_getInteractionClassHandle, METH_VARARGS, ""},
  {"getInteractionClassName", (PyCFunction)PyRTIambassador_getInteractionClassName, METH_VARARGS, ""},
  {"getParameterHandle", (PyCFunction)PyRTIambassador_getParameterHandle, METH_VARARGS, ""},
  {"getParameterName", (PyCFunction)PyRTIambassador_getParameterName, METH_VARARGS, ""},
  {"getObjectInstanceHandle", (PyCFunction)PyRTIambassador_getObjectInstanceHandle, METH_VARARGS, ""},
  {"getObjectInstanceName", (PyCFunction)PyRTIambassador_getObjectInstanceName, METH_VARARGS, ""},
  {"getDimensionHandle", (PyCFunction)PyRTIambassador_getDimensionHandle, METH_VARARGS, ""},
  {"getDimensionName", (PyCFunction)PyRTIambassador_getDimensionName, METH_VARARGS, ""},
  {"getDimensionUpperBound", (PyCFunction)PyRTIambassador_getDimensionUpperBound, METH_VARARGS, ""},
  {"getAvailableDimensionsForClassAttribute", (PyCFunction)PyRTIambassador_getAvailableDimensionsForClassAttribute, METH_VARARGS, ""},
  {"getKnownObjectClassHandle", (PyCFunction)PyRTIambassador_getKnownObjectClassHandle, METH_VARARGS, ""},
  {"getAvailableDimensionsForInteractionClass", (PyCFunction)PyRTIambassador_getAvailableDimensionsForInteractionClass, METH_VARARGS, ""},
  {"getTransportationType", (PyCFunction)PyRTIambassador_getTransportationType, METH_VARARGS, ""},
  {"getTransportationName", (PyCFunction)PyRTIambassador_getTransportationName, METH_VARARGS, ""},
  {"getOrderType", (PyCFunction)PyRTIambassador_getOrderType, METH_VARARGS, ""},
  {"getOrderName", (PyCFunction)PyRTIambassador_getOrderName, METH_VARARGS, ""},
  {"enableObjectClassRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_enableObjectClassRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"disableObjectClassRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_disableObjectClassRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"enableAttributeRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_enableAttributeRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"disableAttributeRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_disableAttributeRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"enableAttributeScopeAdvisorySwitch", (PyCFunction)PyRTIambassador_enableAttributeScopeAdvisorySwitch, METH_VARARGS, ""},
  {"disableAttributeScopeAdvisorySwitch", (PyCFunction)PyRTIambassador_disableAttributeScopeAdvisorySwitch, METH_VARARGS, ""},
  {"enableInteractionRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_enableInteractionRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"disableInteractionRelevanceAdvisorySwitch", (PyCFunction)PyRTIambassador_disableInteractionRelevanceAdvisorySwitch, METH_VARARGS, ""},
  {"getDimensionHandleSet", (PyCFunction)PyRTIambassador_getDimensionHandleSet, METH_VARARGS, ""},
  {"getRangeBounds", (PyCFunction)PyRTIambassador_getRangeBounds, METH_VARARGS, ""},
  {"setRangeBounds", (PyCFunction)PyRTIambassador_setRangeBounds, METH_VARARGS, ""},
  {"normalizeFederateHandle", (PyCFunction)PyRTIambassador_normalizeFederateHandle, METH_VARARGS, ""},
  {"normalizeServiceGroup", (PyCFunction)PyRTIambassador_normalizeServiceGroup, METH_VARARGS, ""},
  {"evokeCallback", (PyCFunction)PyRTIambassador_evokeCallback, METH_VARARGS, ""},
  {"evokeMultipleCallbacks", (PyCFunction)PyRTIambassador_evokeMultipleCallbacks, METH_VARARGS, ""},
  {"enableCallbacks", (PyCFunction)PyRTIambassador_enableCallbacks, METH_VARARGS, ""},
  {"disableCallbacks", (PyCFunction)PyRTIambassador_disableCallbacks, METH_VARARGS, ""},
  {"decodeFederateHandle", (PyCFunction)PyRTIambassador_decodeFederateHandle, METH_VARARGS, ""},
  {"decodeObjectClassHandle", (PyCFunction)PyRTIambassador_decodeObjectClassHandle, METH_VARARGS, ""},
  {"decodeInteractionClassHandle", (PyCFunction)PyRTIambassador_decodeInteractionClassHandle, METH_VARARGS, ""},
  {"decodeObjectInstanceHandle", (PyCFunction)PyRTIambassador_decodeObjectInstanceHandle, METH_VARARGS, ""},
  {"decodeAttributeHandle", (PyCFunction)PyRTIambassador_decodeAttributeHandle, METH_VARARGS, ""},
  {"decodeParameterHandle", (PyCFunction)PyRTIambassador_decodeParameterHandle, METH_VARARGS, ""},
  {"decodeDimensionHandle", (PyCFunction)PyRTIambassador_decodeDimensionHandle, METH_VARARGS, ""},
  {"decodeMessageRetractionHandle", (PyCFunction)PyRTIambassador_decodeMessageRetractionHandle, METH_VARARGS, ""},
  {"decodeRegionHandle", (PyCFunction)PyRTIambassador_decodeRegionHandle, METH_VARARGS, ""},
  { 0, }
};

static PyObject*
PyObject_NewRTIambassador(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  std::vector<std::wstring> stringArgs;

  Py_ssize_t size = PySequence_Size(args);
  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject* o = PySequence_GetItem(args, i);
    std::wstring string;
    if (!PyObject_GetString(string, o)) {
      Py_DecRef(o);
      PyErr_SetString(PyExc_TypeError, "RTIambassador constructor just takes string arguments!");
      return 0;
    }
    Py_DecRef(o);
    stringArgs.push_back(string);
  }

  RTI_UNIQUE_PTR<rti1516::RTIambassador> ambassador;
  ambassador = rti1516::RTIambassadorFactory().createRTIambassador(stringArgs);
  if (!ambassador.get()) {
    PyErr_SetObject(PyRTI1516RTIinternalError.get(), PyUnicode_FromString("Cannot create RTIambassador!"));
    return 0;
  }

  PyRTIambassadorObject *self = PyObject_GC_New(PyRTIambassadorObject, type);
  if (!self)
    return 0;
  new (self) PyRTIambassadorObject;
  self->ob_value.reset(ambassador.release());

  PyObject_GC_Track(self);

  return (PyObject*)self;
}

static void
PyRTIambassadorObject_dealloc(PyRTIambassadorObject *o)
{
  PyObject_GC_UnTrack(o);

  o->PyRTIambassadorObject::~PyRTIambassadorObject();
  Py_TYPE(o)->tp_free(o);
}

static int
PyRTIambassadorObject_traverse(PyRTIambassadorObject *o, visitproc visit, void *arg)
{
  Py_VISIT(o->_federateAmbassador.ob_federateAmbassador);
  return 0;
}

static int
PyRTIambassadorObject_clear(PyRTIambassadorObject *o)
{
  Py_CLEAR(o->_federateAmbassador.ob_federateAmbassador);
  return 0;
}

static PyTypeObject PyRTIambassadorType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "RTIambassador",                  /* tp_name */
  sizeof(PyRTIambassadorObject),    /* tp_basicsize */
  0,                                /* tp_itemsize */
  (destructor)PyRTIambassadorObject_dealloc, /* tp_dealloc */
  0,                                /* tp_print */
  0,                                /* tp_getattr */
  0,                                /* tp_setattr */
  0,                                /* tp_compare */
  0,                                /* tp_repr */
  0,                                /* tp_as_number */
  0,                                /* tp_as_sequence */
  0,                                /* tp_as_mapping */
  0,                                /* tp_hash */
  0,                                /* tp_call */
  0,                                /* tp_str */
  0,                                /* tp_getattro */
  0,                                /* tp_setattro */
  0,                                /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE, /* tp_flags */
  "RTIambassador",                  /* tp_doc */
  (traverseproc)PyRTIambassadorObject_traverse, /* tp_traverse */
  (inquiry)PyRTIambassadorObject_clear, /* tp_clear */
  0,                                /* tp_richcompare */
  0,                                /* tp_weaklistoffset */
  0,                                /* tp_iter */
  0,                                /* tp_iternext */
  PyRTIambassador_methods,          /* tp_methods */
  0,                                /* tp_members */
  0,                                /* tp_getset */
  0,                                /* tp_base */
  0,                                /* tp_dict */
  0,                                /* tp_descr_get */
  0,                                /* tp_descr_set */
  0,                                /* tp_dictoffset */
  0,                                /* tp_init */
  0,                                /* tp_alloc */
  PyObject_NewRTIambassador,        /* tp_new */
};

///////////////////////////////////////////////////////////////////////////////

static PyMethodDef rti1516_methods[] = {
    {NULL, NULL}
};

#if 3 <= PY_MAJOR_VERSION

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "rti1516",
        NULL,
        -1,
        rti1516_methods,
        NULL,
        NULL,
        NULL,
        NULL
};

# define INITFUNCNAME PyInit_rti1516
# define INITERROR return NULL
#else
# define INITFUNCNAME initrti1516
# define INITERROR return
#endif

PyMODINIT_FUNC
INITFUNCNAME(void)
{
  if (PyType_Ready(&PyFederateHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyObjectClassHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyInteractionClassHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyObjectInstanceHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyAttributeHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyParameterHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyDimensionHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyRegionHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyMessageRetractionHandleType) < 0)
    INITERROR;
  if (PyType_Ready(&PyRTIambassadorType) < 0)
    INITERROR;

#if PY_MAJOR_VERSION >= 3
  PyObject* module = PyModule_Create(&moduledef);
#else
  PyObject* module = Py_InitModule3("rti1516", rti1516_methods, "rti1516 RTI/HLA backend implementation.");
#endif

  Py_IncRef((PyObject*)&PyFederateHandleType);
  PyModule_AddObject(module, "FederateHandle", (PyObject*)&PyFederateHandleType);
  Py_IncRef((PyObject*)&PyObjectClassHandleType);
  PyModule_AddObject(module, "ObjectClassHandle", (PyObject*)&PyObjectClassHandleType);
  Py_IncRef((PyObject*)&PyObjectInstanceHandleType);
  PyModule_AddObject(module, "ObjectInstanceHandle", (PyObject*)&PyObjectInstanceHandleType);
  Py_IncRef((PyObject*)&PyInteractionClassHandleType);
  PyModule_AddObject(module, "InteractionClassHandle", (PyObject*)&PyInteractionClassHandleType);
  Py_IncRef((PyObject*)&PyAttributeHandleType);
  PyModule_AddObject(module, "AttributeHandle", (PyObject*)&PyAttributeHandleType);
  Py_IncRef((PyObject*)&PyParameterHandleType);
  PyModule_AddObject(module, "ParameterHandle", (PyObject*)&PyParameterHandleType);
  Py_IncRef((PyObject*)&PyDimensionHandleType);
  PyModule_AddObject(module, "DimensionHandle", (PyObject*)&PyDimensionHandleType);
  Py_IncRef((PyObject*)&PyRegionHandleType);
  PyModule_AddObject(module, "RegionHandle", (PyObject*)&PyRegionHandleType);
  Py_IncRef((PyObject*)&PyMessageRetractionHandleType);
  PyModule_AddObject(module, "MessageRetractionHandle", (PyObject*)&PyMessageRetractionHandleType);

  Py_IncRef((PyObject*)&PyRTIambassadorType);
  PyModule_AddObject(module, "RTIambassador", (PyObject*)&PyRTIambassadorType);

  // enum OrderType
  PyModule_AddIntConstant(module, "RECEIVE", rti1516::RECEIVE);
  PyModule_AddIntConstant(module, "TIMESTAMP", rti1516::TIMESTAMP);

  // enum ResignAction
  PyModule_AddIntConstant(module, "UNCONDITIONALLY_DIVEST_ATTRIBUTES", rti1516::UNCONDITIONALLY_DIVEST_ATTRIBUTES);
  PyModule_AddIntConstant(module, "DELETE_OBJECTS", rti1516::DELETE_OBJECTS);
  PyModule_AddIntConstant(module, "CANCEL_PENDING_OWNERSHIP_ACQUISITIONS", rti1516::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS);
  PyModule_AddIntConstant(module, "DELETE_OBJECTS_THEN_DIVEST", rti1516::DELETE_OBJECTS_THEN_DIVEST);
  PyModule_AddIntConstant(module, "CANCEL_THEN_DELETE_THEN_DIVEST", rti1516::CANCEL_THEN_DELETE_THEN_DIVEST);
  PyModule_AddIntConstant(module, "NO_ACTION", rti1516::NO_ACTION);

  // enum RestoreFailureReason
  PyModule_AddIntConstant(module, "RTI_UNABLE_TO_RESTORE", rti1516::RTI_UNABLE_TO_RESTORE);
  PyModule_AddIntConstant(module, "FEDERATE_REPORTED_FAILURE_DURING_RESTORE", rti1516::FEDERATE_REPORTED_FAILURE_DURING_RESTORE);
  PyModule_AddIntConstant(module, "FEDERATE_RESIGNED_DURING_RESTORE", rti1516::FEDERATE_RESIGNED_DURING_RESTORE);
  PyModule_AddIntConstant(module, "RTI_DETECTED_FAILURE_DURING_RESTORE", rti1516::RTI_DETECTED_FAILURE_DURING_RESTORE);

  // enum RestoreStatus
  PyModule_AddIntConstant(module, "NO_RESTORE_IN_PROGRESS", rti1516::NO_RESTORE_IN_PROGRESS);
  PyModule_AddIntConstant(module, "FEDERATE_RESTORE_REQUEST_PENDING", rti1516::FEDERATE_RESTORE_REQUEST_PENDING);
  PyModule_AddIntConstant(module, "FEDERATE_WAITING_FOR_RESTORE_TO_BEGIN", rti1516::FEDERATE_WAITING_FOR_RESTORE_TO_BEGIN);
  PyModule_AddIntConstant(module, "FEDERATE_PREPARED_TO_RESTORE", rti1516::FEDERATE_PREPARED_TO_RESTORE);
  PyModule_AddIntConstant(module, "FEDERATE_RESTORING", rti1516::FEDERATE_RESTORING);
  PyModule_AddIntConstant(module, "FEDERATE_WAITING_FOR_FEDERATION_TO_RESTORE", rti1516::FEDERATE_WAITING_FOR_FEDERATION_TO_RESTORE);

  // enum SaveFailureReason
  PyModule_AddIntConstant(module, "RTI_UNABLE_TO_SAVE", rti1516::RTI_UNABLE_TO_SAVE);
  PyModule_AddIntConstant(module, "FEDERATE_REPORTED_FAILURE_DURING_SAVE", rti1516::FEDERATE_REPORTED_FAILURE_DURING_SAVE);
  PyModule_AddIntConstant(module, "FEDERATE_RESIGNED_DURING_SAVE", rti1516::FEDERATE_RESIGNED_DURING_SAVE);
  PyModule_AddIntConstant(module, "RTI_DETECTED_FAILURE_DURING_SAVE", rti1516::RTI_DETECTED_FAILURE_DURING_SAVE);
  PyModule_AddIntConstant(module, "SAVE_TIME_CANNOT_BE_HONORED", rti1516::SAVE_TIME_CANNOT_BE_HONORED);

  // enum SaveStatus
  PyModule_AddIntConstant(module, "NO_SAVE_IN_PROGRESS", rti1516::NO_SAVE_IN_PROGRESS);
  PyModule_AddIntConstant(module, "FEDERATE_INSTRUCTED_TO_SAVE", rti1516::FEDERATE_INSTRUCTED_TO_SAVE);
  PyModule_AddIntConstant(module, "FEDERATE_SAVING", rti1516::FEDERATE_SAVING);
  PyModule_AddIntConstant(module, "FEDERATE_WAITING_FOR_FEDERATION_TO_SAVE", rti1516::FEDERATE_WAITING_FOR_FEDERATION_TO_SAVE);

  // enum ServiceGroupIndicator
  PyModule_AddIntConstant(module, "FEDERATION_MANAGEMENT", rti1516::FEDERATION_MANAGEMENT);
  PyModule_AddIntConstant(module, "DECLARATION_MANAGEMENT", rti1516::DECLARATION_MANAGEMENT);
  PyModule_AddIntConstant(module, "OBJECT_MANAGEMENT", rti1516::OBJECT_MANAGEMENT);
  PyModule_AddIntConstant(module, "OWNERSHIP_MANAGEMENT", rti1516::OWNERSHIP_MANAGEMENT);
  PyModule_AddIntConstant(module, "TIME_MANAGEMENT", rti1516::TIME_MANAGEMENT);
  PyModule_AddIntConstant(module, "DATA_DISTRIBUTION_MANAGEMENT", rti1516::DATA_DISTRIBUTION_MANAGEMENT);
  PyModule_AddIntConstant(module, "SUPPORT_SERVICES", rti1516::SUPPORT_SERVICES);

  // enum SynchronizationFailureReason
  PyModule_AddIntConstant(module, "SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE", rti1516::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
  PyModule_AddIntConstant(module, "SYNCHRONIZATION_SET_MEMBER_NOT_JOINED", rti1516::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
  PyModule_AddIntConstant(module, "FEDERATE_RESIGNED_DURING_SYNCHRONIZATION", rti1516::FEDERATE_RESIGNED_DURING_SYNCHRONIZATION);
  PyModule_AddIntConstant(module, "RTI_DETECTED_FAILURE_DURING_SYNCHRONIZATION", rti1516::RTI_DETECTED_FAILURE_DURING_SYNCHRONIZATION);
  PyModule_AddIntConstant(module, "SYNCHRONIZATION_TIME_CANNOT_BE_HONORED", rti1516::SYNCHRONIZATION_TIME_CANNOT_BE_HONORED);

  // enum TransportationType
  PyModule_AddIntConstant(module, "RELIABLE", rti1516::RELIABLE);
  PyModule_AddIntConstant(module, "BEST_EFFORT", rti1516::BEST_EFFORT);

  PyRTI1516Exception.setBorrowedRef(PyErr_NewException((char*)"rti1516.Exception", NULL, NULL));
  PyModule_AddObject(module, "Exception", PyRTI1516Exception.get());

#define RTI_EXCEPTION(ExceptionKind)                                                                            \
  PyRTI1516 ## ExceptionKind.setBorrowedRef(PyErr_NewException((char*)"rti1516." # ExceptionKind, PyRTI1516Exception.get(), NULL)); \
  PyModule_AddObject(module, # ExceptionKind, PyRTI1516 ## ExceptionKind.get());

  RTI_EXCEPTION(AsynchronousDeliveryAlreadyDisabled)
  RTI_EXCEPTION(AsynchronousDeliveryAlreadyEnabled)
  RTI_EXCEPTION(AttributeAcquisitionWasNotCanceled)
  RTI_EXCEPTION(AttributeAcquisitionWasNotRequested)
  RTI_EXCEPTION(AttributeAlreadyBeingAcquired)
  RTI_EXCEPTION(AttributeAlreadyBeingDivested)
  RTI_EXCEPTION(AttributeAlreadyOwned)
  RTI_EXCEPTION(AttributeDivestitureWasNotRequested)
  RTI_EXCEPTION(AttributeNotDefined)
  RTI_EXCEPTION(AttributeNotOwned)
  RTI_EXCEPTION(AttributeNotPublished)
  RTI_EXCEPTION(AttributeNotRecognized)
  RTI_EXCEPTION(AttributeNotSubscribed)
  RTI_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOff)
  RTI_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOn)
  RTI_EXCEPTION(AttributeScopeAdvisorySwitchIsOff)
  RTI_EXCEPTION(AttributeScopeAdvisorySwitchIsOn)
  RTI_EXCEPTION(BadInitializationParameter)
  RTI_EXCEPTION(CouldNotCreateLogicalTimeFactory)
  RTI_EXCEPTION(CouldNotDecode)
  RTI_EXCEPTION(CouldNotDiscover)
  RTI_EXCEPTION(CouldNotEncode)
  RTI_EXCEPTION(CouldNotOpenFDD)
  RTI_EXCEPTION(CouldNotInitiateRestore)
  RTI_EXCEPTION(DeletePrivilegeNotHeld)
  RTI_EXCEPTION(RequestForTimeConstrainedPending)
  RTI_EXCEPTION(NoRequestToEnableTimeConstrainedWasPending)
  RTI_EXCEPTION(RequestForTimeRegulationPending)
  RTI_EXCEPTION(NoRequestToEnableTimeRegulationWasPending)
  RTI_EXCEPTION(ErrorReadingFDD)
  RTI_EXCEPTION(FederateAlreadyExecutionMember)
  RTI_EXCEPTION(FederateHasNotBegunSave)
  RTI_EXCEPTION(FederateInternalError)
  RTI_EXCEPTION(FederateNotExecutionMember)
  RTI_EXCEPTION(FederateOwnsAttributes)
  RTI_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM)
  RTI_EXCEPTION(FederateUnableToUseTime)
  RTI_EXCEPTION(FederatesCurrentlyJoined)
  RTI_EXCEPTION(FederationExecutionAlreadyExists)
  RTI_EXCEPTION(FederationExecutionDoesNotExist)
  RTI_EXCEPTION(IllegalName)
  RTI_EXCEPTION(IllegalTimeArithmetic)
  RTI_EXCEPTION(InteractionClassNotDefined)
  RTI_EXCEPTION(InteractionClassNotPublished)
  RTI_EXCEPTION(InteractionClassNotRecognized)
  RTI_EXCEPTION(InteractionClassNotSubscribed)
  RTI_EXCEPTION(InteractionParameterNotDefined)
  RTI_EXCEPTION(InteractionParameterNotRecognized)
  RTI_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOff)
  RTI_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOn)
  RTI_EXCEPTION(InTimeAdvancingState)
  RTI_EXCEPTION(InvalidAttributeHandle)
  RTI_EXCEPTION(InvalidDimensionHandle)
  RTI_EXCEPTION(InvalidFederateHandle)
  RTI_EXCEPTION(InvalidInteractionClassHandle)
  RTI_EXCEPTION(InvalidLogicalTime)
  RTI_EXCEPTION(InvalidLogicalTimeInterval)
  RTI_EXCEPTION(InvalidLookahead)
  RTI_EXCEPTION(InvalidObjectClassHandle)
  RTI_EXCEPTION(InvalidOrderName)
  RTI_EXCEPTION(InvalidOrderType)
  RTI_EXCEPTION(InvalidParameterHandle)
  RTI_EXCEPTION(InvalidRangeBound)
  RTI_EXCEPTION(InvalidRegion)
  RTI_EXCEPTION(InvalidRegionContext)
  RTI_EXCEPTION(InvalidRetractionHandle)
  RTI_EXCEPTION(InvalidServiceGroup)
  RTI_EXCEPTION(InvalidTransportationName)
  RTI_EXCEPTION(InvalidTransportationType)
  RTI_EXCEPTION(JoinedFederateIsNotInTimeAdvancingState)
  RTI_EXCEPTION(LogicalTimeAlreadyPassed)
  RTI_EXCEPTION(MessageCanNoLongerBeRetracted)
  RTI_EXCEPTION(NameNotFound)
  RTI_EXCEPTION(NoAcquisitionPending)
  RTI_EXCEPTION(ObjectClassNotDefined)
  RTI_EXCEPTION(ObjectClassNotKnown)
  RTI_EXCEPTION(ObjectClassNotPublished)
  RTI_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOff)
  RTI_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOn)
  RTI_EXCEPTION(ObjectInstanceNameInUse)
  RTI_EXCEPTION(ObjectInstanceNameNotReserved)
  RTI_EXCEPTION(ObjectInstanceNotKnown)
  RTI_EXCEPTION(OwnershipAcquisitionPending)
  RTI_EXCEPTION(RTIinternalError)
  RTI_EXCEPTION(RegionDoesNotContainSpecifiedDimension)
  RTI_EXCEPTION(RegionInUseForUpdateOrSubscription)
  RTI_EXCEPTION(RegionNotCreatedByThisFederate)
  RTI_EXCEPTION(RestoreInProgress)
  RTI_EXCEPTION(RestoreNotRequested)
  RTI_EXCEPTION(SaveInProgress)
  RTI_EXCEPTION(SaveNotInitiated)
  RTI_EXCEPTION(SpecifiedSaveLabelDoesNotExist)
  RTI_EXCEPTION(SynchronizationPointLabelNotAnnounced)
  RTI_EXCEPTION(TimeConstrainedAlreadyEnabled)
  RTI_EXCEPTION(TimeConstrainedIsNotEnabled)
  RTI_EXCEPTION(TimeRegulationAlreadyEnabled)
  RTI_EXCEPTION(TimeRegulationIsNotEnabled)
  RTI_EXCEPTION(UnableToPerformSave)
  RTI_EXCEPTION(UnknownName)
  RTI_EXCEPTION(InternalError)
#undef RTI_EXCEPTION

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}
