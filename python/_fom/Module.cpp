/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#include <limits>

static void
rawEncodeUInt8BE(uint8_t* data, uint8_t value)
{
  data[0] = uint8_t(value);
}

static uint8_t
rawDecodeUInt8BE(const uint8_t* data)
{
  return data[0];
}

static void
rawEncodeInt8BE(uint8_t* data, int8_t value)
{ rawEncodeUInt8BE(data, value); }

static uint8_t
rawDecodeInt8BE(const uint8_t* data)
{ return rawDecodeUInt8BE(data); }


static void
rawEncodeUInt16LE(uint8_t* data, uint16_t value)
{
  data[0] = uint8_t(value);
  data[1] = uint8_t(value >> 8);
}

static uint16_t
rawDecodeUInt16LE(const uint8_t* data)
{
  uint16_t value = uint16_t(data[0]);
  value |= uint16_t(data[1]) << 8;
  return value;
}

static void
rawEncodeUInt16BE(uint8_t* data, uint16_t value)
{
  data[0] = uint8_t(value >> 8);
  data[1] = uint8_t(value);
}

static uint16_t
rawDecodeUInt16BE(const uint8_t* data)
{
  uint16_t value = uint16_t(data[0]) << 8;
  value |= uint16_t(data[1]);
  return value;
}

static void
rawEncodeInt16LE(uint8_t* data, int16_t value)
{ rawEncodeUInt16LE(data, value); }

static int16_t
rawDecodeInt16LE(const uint8_t* data)
{ return rawDecodeUInt16LE(data); }

static void
rawEncodeInt16BE(uint8_t* data, int16_t value)
{ rawEncodeUInt16BE(data, value); }

static int16_t
rawDecodeInt16BE(const uint8_t* data)
{ return rawDecodeUInt16BE(data); }


static void
rawEncodeUInt32LE(uint8_t* data, uint32_t value)
{
  data[0] = uint8_t(value);
  data[1] = uint8_t(value >> 8);
  data[2] = uint8_t(value >> 16);
  data[3] = uint8_t(value >> 24);
}

static uint32_t
rawDecodeUInt32LE(const uint8_t* data)
{
  uint32_t value = uint32_t(data[0]);
  value |= uint32_t(data[1]) << 8;
  value |= uint32_t(data[2]) << 16;
  value |= uint32_t(data[3]) << 24;
  return value;
}

static void
rawEncodeUInt32BE(uint8_t* data, uint32_t value)
{
  data[0] = uint8_t(value >> 24);
  data[1] = uint8_t(value >> 16);
  data[2] = uint8_t(value >> 8);
  data[3] = uint8_t(value);
}

static uint32_t
rawDecodeUInt32BE(const uint8_t* data)
{
  uint32_t value = uint32_t(data[0]) << 24;
  value |= uint32_t(data[1]) << 16;
  value |= uint32_t(data[2]) << 8;
  value |= uint32_t(data[3]);
  return value;
}

static void
rawEncodeInt32LE(uint8_t* data, int32_t value)
{ rawEncodeUInt32LE(data, value); }

static int32_t
rawDecodeInt32LE(const uint8_t* data)
{ return rawDecodeUInt32LE(data); }

static void
rawEncodeInt32BE(uint8_t* data, int32_t value)
{ rawEncodeUInt32BE(data, value); }

static int32_t
rawDecodeInt32BE(const uint8_t* data)
{ return rawDecodeUInt32BE(data); }


static void
rawEncodeUInt64LE(uint8_t* data, uint64_t value)
{
  data[0] = uint8_t(value);
  data[1] = uint8_t(value >> 8);
  data[2] = uint8_t(value >> 16);
  data[3] = uint8_t(value >> 24);
  data[4] = uint8_t(value >> 32);
  data[5] = uint8_t(value >> 40);
  data[6] = uint8_t(value >> 48);
  data[7] = uint8_t(value >> 56);
}

static uint64_t
rawDecodeUInt64LE(const uint8_t* data)
{
  uint64_t value = uint64_t(data[0]);
  value |= uint64_t(data[1]) << 8;
  value |= uint64_t(data[2]) << 16;
  value |= uint64_t(data[3]) << 24;
  value |= uint64_t(data[4]) << 32;
  value |= uint64_t(data[5]) << 40;
  value |= uint64_t(data[6]) << 48;
  value |= uint64_t(data[7]) << 56;
  return value;
}

static void
rawEncodeUInt64BE(uint8_t* data, uint64_t value)
{
  data[0] = uint8_t(value >> 56);
  data[1] = uint8_t(value >> 48);
  data[2] = uint8_t(value >> 40);
  data[3] = uint8_t(value >> 32);
  data[4] = uint8_t(value >> 24);
  data[5] = uint8_t(value >> 16);
  data[6] = uint8_t(value >> 8);
  data[7] = uint8_t(value);
}

static uint64_t
rawDecodeUInt64BE(const uint8_t* data)
{
  uint64_t value = uint64_t(data[0]) << 56;
  value |= uint64_t(data[1]) << 48;
  value |= uint64_t(data[2]) << 40;
  value |= uint64_t(data[3]) << 32;
  value |= uint64_t(data[4]) << 24;
  value |= uint64_t(data[5]) << 16;
  value |= uint64_t(data[6]) << 8;
  value |= uint64_t(data[7]);
  return value;
}

static void
rawEncodeInt64LE(uint8_t* data, int64_t value)
{ rawEncodeUInt64LE(data, value); }

static int64_t
rawDecodeInt64LE(const uint8_t* data)
{ return rawDecodeUInt64LE(data); }

static void
rawEncodeInt64BE(uint8_t* data, int64_t value)
{ rawEncodeUInt64BE(data, value); }

static int64_t
rawDecodeInt64BE(const uint8_t* data)
{ return rawDecodeUInt64BE(data); }


static void
rawEncodeFloat32LE(uint8_t* data, float value)
{
  union { float d; uint32_t i; } u;
  u.d = value;
  rawEncodeUInt32LE(data, u.i);
}

static float
rawDecodeFloat32LE(const uint8_t* data)
{
  union { float d; uint32_t i; } u;
  u.i = rawDecodeUInt32LE(data);
  return u.d;
}

static void
rawEncodeFloat32BE(uint8_t* data, float value)
{
  union { float d; uint32_t i; } u;
  u.d = value;
  rawEncodeUInt32BE(data, u.i);
}

static float
rawDecodeFloat32BE(const uint8_t* data)
{
  union { float d; uint32_t i; } u;
  u.i = rawDecodeUInt32BE(data);
  return u.d;
}


static void
rawEncodeFloat64LE(uint8_t* data, double value)
{
  union { double d; uint64_t i; } u;
  u.d = value;
  rawEncodeUInt64LE(data, u.i);
}

static double
rawDecodeFloat64LE(const uint8_t* data)
{
  union { double d; uint64_t i; } u;
  u.i = rawDecodeUInt64LE(data);
  return u.d;
}

static void
rawEncodeFloat64BE(uint8_t* data, double value)
{
  union { double d; uint64_t i; } u;
  u.d = value;
  rawEncodeUInt64BE(data, u.i);
}

static double
rawDecodeFloat64BE(const uint8_t* data)
{
  union { double d; uint64_t i; } u;
  u.i = rawDecodeUInt64BE(data);
  return u.d;
}


////////////////////////////////////////

// FIXME these could be improoved

static int
getUInt8(uint8_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<uint8_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint8_t."));
      return 0;
    } else if (std::numeric_limits<uint8_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint8_t."));
      return 0;
    } else {
      value = uint8_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    // long l = PyLong_AsUnsignedLong(o);
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<uint8_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint8_t."));
      return 0;
    } else if (std::numeric_limits<uint8_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint8_t."));
      return 0;
    } else {
      value = uint8_t(l);
      return 1;
    }
    // FIXME long long and float can still
  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getUInt8(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an uint8_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newUInt8(uint8_t v)
{
  return PyInt_FromLong(v);
}

static int
getInt8(int8_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<int8_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int8_t."));
      return 0;
    } else if (std::numeric_limits<int8_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int8_t."));
      return 0;
    } else {
      value = int8_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<int8_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int8_t."));
      return 0;
    } else if (std::numeric_limits<int8_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int8_t."));
      return 0;
    } else {
      value = int8_t(l);
      return 1;
    }

  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getInt8(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an int8_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newInt8(int8_t v)
{
  return PyInt_FromLong(v);
}


static int
getUInt16(uint16_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<uint16_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint16_t."));
      return 0;
    } else if (std::numeric_limits<uint16_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint16_t."));
      return 0;
    } else {
      value = uint16_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    // long l = PyLong_AsUnsignedLong(o);
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<uint16_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint16_t."));
      return 0;
    } else if (std::numeric_limits<uint16_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint16_t."));
      return 0;
    } else {
      value = uint16_t(l);
      return 1;
    }
    // FIXME long long and float can still
  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getUInt16(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an uint16_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newUInt16(uint16_t v)
{
  return PyInt_FromLong(v);
}

static int
getInt16(int16_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<int16_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int16_t."));
      return 0;
    } else if (std::numeric_limits<int16_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int16_t."));
      return 0;
    } else {
      value = int16_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<int16_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int16_t."));
      return 0;
    } else if (std::numeric_limits<int16_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int16_t."));
      return 0;
    } else {
      value = int16_t(l);
      return 1;
    }

  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getInt16(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an int16_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newInt16(int16_t v)
{
  return PyInt_FromLong(v);
}


static int
getUInt32(uint32_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<uint32_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint32_t."));
      return 0;
    } else if (std::numeric_limits<uint32_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint32_t."));
      return 0;
    } else {
      value = uint32_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    // long l = PyLong_AsUnsignedLong(o);
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<uint32_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint32_t."));
      return 0;
    } else if (std::numeric_limits<uint32_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint32_t."));
      return 0;
    } else {
      value = uint32_t(l);
      return 1;
    }
    // FIXME long long and float can still
  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getUInt32(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an uint32_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newUInt32(uint32_t v)
{
  return PyInt_FromLong(v);
}

static int
getInt32(int32_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<int32_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int32_t."));
      return 0;
    } else if (std::numeric_limits<int32_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int32_t."));
      return 0;
    } else {
      value = int32_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<int32_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int32_t."));
      return 0;
    } else if (std::numeric_limits<int32_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int32_t."));
      return 0;
    } else {
      value = int32_t(l);
      return 1;
    }

  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getInt32(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an int32_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newInt32(int32_t v)
{
  return PyInt_FromLong(v);
}


static int
getUInt64(uint64_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<uint64_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint64_t."));
      return 0;
    } else if (std::numeric_limits<uint64_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint64_t."));
      return 0;
    } else {
      value = uint64_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    // long l = PyLong_AsUnsignedLong(o);
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<uint64_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an uint64_t."));
      return 0;
    } else if (std::numeric_limits<uint64_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an uint64_t."));
      return 0;
    } else {
      value = uint64_t(l);
      return 1;
    }
    // FIXME long long and float can still
  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getUInt64(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an uint64_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newUInt64(uint64_t v)
{
  return PyLong_FromUnsignedLongLong(v);
}

static int
getInt64(int64_t& value, PyObject* o)
{
  if (PyInt_Check(o)) {
    long l = PyInt_AsLong(o);
    if (l < std::numeric_limits<int64_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int64_t."));
      return 0;
    } else if (std::numeric_limits<int64_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int64_t."));
      return 0;
    } else {
      value = int64_t(l);
      return 1;
    }
  } else if (PyLong_Check(o)) {
    long l = PyLong_AsLong(o);
    if (l < std::numeric_limits<int64_t>::min()) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to small to encode into an int64_t."));
      return 0;
    } else if (std::numeric_limits<int64_t>::max() < l) {
      PyErr_SetObject(PyExc_OverflowError, PyString_FromString("Numeric value to huge to encode into an int64_t."));
      return 0;
    } else {
      value = int64_t(l);
      return 1;
    }

  } else {
    // returns int or long if convertible
    PyObject* i = PyNumber_Int(o);
    if (i) {
      int ret = getInt64(value, i);
      Py_DecRef(i);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into an int64_t for encoding."));
      return 0;
    }
  }
}

static PyObject*
newInt64(int64_t v)
{
  return PyLong_FromLongLong(v);
}


static int
getFloat32(float& value, PyObject* o)
{
  if (PyFloat_Check(o)) {
    value = PyFloat_AsDouble(o);
    return 1;

  } else {
    // returns float if convertible
    PyObject* f = PyNumber_Float(o);
    if (f) {
      int ret = getFloat32(value, f);
      Py_DecRef(f);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into a float for encoding."));
      return 0;
    }
  }
}

static PyObject*
newFloat32(double v)
{
  return PyFloat_FromDouble(v);
}


static int
getFloat64(double& value, PyObject* o)
{
  if (PyFloat_Check(o)) {
    value = PyFloat_AsDouble(o);
    return 1;

  } else {
    // returns float if convertible
    PyObject* f = PyNumber_Float(o);
    if (f) {
      int ret = getFloat64(value, f);
      Py_DecRef(f);
      return ret;

    } else {
      PyErr_SetObject(PyExc_TypeError, PyString_FromString("Cannot convert numeric value into a float for encoding."));
      return 0;
    }
  }
}

static PyObject*
newFloat64(double v)
{
  return PyFloat_FromDouble(v);
}

#define ENCODER_PAIR(TYPE, CTYPE, SIZE, ENDIAN)                         \
                                                                        \
static PyObject*                                                        \
encode ## TYPE ## ENDIAN(PyObject *self, PyObject *args)                \
{                                                                       \
  PyObject *v = 0, *b = 0, *o = 0;                                      \
  if (!PyArg_UnpackTuple(args, "encode" #TYPE #ENDIAN, 3, 3, &v, &b, &o)) \
    return 0;                                                           \
                                                                        \
  /* v is the value to encode */                                        \
  CTYPE value;                                                          \
  if (!get ## TYPE(value, v)) {                                         \
    return 0;                                                           \
  }                                                                     \
                                                                        \
  /* b is the buffer to encode into */                                  \
  if (!PyByteArray_Check(b)) {                                          \
    PyErr_SetObject(PyExc_TypeError,                                    \
                    PyString_FromString("Buffer needs to by a byte array.")); \
    return 0;                                                           \
  }                                                                     \
                                                                        \
  /* o is the offset into the buffer to encode */                       \
  Py_ssize_t offset = PyNumber_AsSsize_t(o, 0);                         \
  if (PyByteArray_Resize(b, offset + SIZE)) {                             \
    PyErr_SetObject(PyExc_TypeError,                                    \
                    PyString_FromString("Cannot resize to required size.")); \
    return 0;                                                           \
  }                                                                     \
                                                                        \
  /* The buffer to encode into */                                       \
  uint8_t* buffer = reinterpret_cast<uint8_t*>(PyByteArray_AsString(b)); \
  rawEncode ## TYPE ## ENDIAN(buffer + offset, value);                  \
                                                                        \
  /* Return the may be resized buffer and the new offset */             \
  PyObject* second = PyNumber_Add(o, PyInt_FromLong(SIZE));             \
                                                                        \
  Py_IncRef(b);                                                         \
  PyObject *tuple = PyTuple_Pack(2, b, second);                         \
                                                                        \
  return tuple;                                                         \
}                                                                       \
                                                                        \
static PyObject *                                                       \
decode ## TYPE ## ENDIAN(PyObject *self, PyObject *args)                \
{                                                                       \
  PyObject *b = 0, *o = 0;                                              \
  if (!PyArg_UnpackTuple(args, "decode" #TYPE #ENDIAN, 2, 2, &b, &o))   \
    return 0;                                                           \
                                                                        \
  /* b is the buffer to encode into */                                  \
  if (!PyByteArray_Check(b)) {                                          \
    return 0;                                                           \
  }                                                                     \
                                                                        \
  /* o is the offset into the buffer to encode */                       \
  Py_ssize_t offset = PyNumber_AsSsize_t(o, 0);                         \
  if (PyByteArray_Size(b) < offset + SIZE) {                            \
    PyErr_SetObject(PyExc_OverflowError,                                \
                    PyString_FromString("Insufficient buffer size to decode value.")); \
    return 0;                                                           \
  }                                                                     \
                                                                        \
  const uint8_t* buffer = reinterpret_cast<const uint8_t*>(PyByteArray_AsString(b)); \
  PyObject* v = new ## TYPE(rawDecode ## TYPE ## ENDIAN(buffer + offset));  \
                                                                        \
  /* Return the may be resized buffer and the new offset */             \
  PyObject *second = PyNumber_Add(o, PyInt_FromLong(SIZE));             \
  return PyTuple_Pack(2, v, second);                                    \
}

ENCODER_PAIR(UInt8, uint8_t, 1, BE)
ENCODER_PAIR(Int8, int8_t, 1, BE)

ENCODER_PAIR(UInt16, uint16_t, 2, BE)
ENCODER_PAIR(Int16, int16_t, 2, BE)
ENCODER_PAIR(UInt16, uint16_t, 2, LE)
ENCODER_PAIR(Int16, int16_t, 2, LE)

ENCODER_PAIR(UInt32, uint32_t, 4, BE)
ENCODER_PAIR(Int32, int32_t, 4, BE)
ENCODER_PAIR(UInt32, uint32_t, 4, LE)
ENCODER_PAIR(Int32, int32_t, 4, LE)

ENCODER_PAIR(UInt64, uint64_t, 8, BE)
ENCODER_PAIR(Int64, int64_t, 8, BE)
ENCODER_PAIR(UInt64, uint64_t, 8, LE)
ENCODER_PAIR(Int64, int64_t, 8, LE)

ENCODER_PAIR(Float32, float, 4, BE)
ENCODER_PAIR(Float32, float, 4, LE)
ENCODER_PAIR(Float64, double, 8, BE)
ENCODER_PAIR(Float64, double, 8, LE)


static PyMethodDef methods[] =
{
  {"encodeUInt8", (PyCFunction)encodeUInt8BE, METH_VARARGS, ""},
  {"decodeUInt8", (PyCFunction)decodeUInt8BE, METH_VARARGS, ""},
  {"encodeInt8", (PyCFunction)encodeInt8BE, METH_VARARGS, ""},
  {"decodeInt8", (PyCFunction)decodeInt8BE, METH_VARARGS, ""},

  {"encodeUInt16BE", (PyCFunction)encodeUInt16BE, METH_VARARGS, ""},
  {"decodeUInt16BE", (PyCFunction)decodeUInt16BE, METH_VARARGS, ""},
  {"encodeUInt16LE", (PyCFunction)encodeUInt16LE, METH_VARARGS, ""},
  {"decodeUInt16LE", (PyCFunction)decodeUInt16LE, METH_VARARGS, ""},
  {"encodeInt16BE", (PyCFunction)encodeInt16BE, METH_VARARGS, ""},
  {"decodeInt16BE", (PyCFunction)decodeInt16BE, METH_VARARGS, ""},
  {"encodeInt16LE", (PyCFunction)encodeInt16LE, METH_VARARGS, ""},
  {"decodeInt16LE", (PyCFunction)decodeInt16LE, METH_VARARGS, ""},

  {"encodeUInt32BE", (PyCFunction)encodeUInt32BE, METH_VARARGS, ""},
  {"decodeUInt32BE", (PyCFunction)decodeUInt32BE, METH_VARARGS, ""},
  {"encodeUInt32LE", (PyCFunction)encodeUInt32LE, METH_VARARGS, ""},
  {"decodeUInt32LE", (PyCFunction)decodeUInt32LE, METH_VARARGS, ""},
  {"encodeInt32BE", (PyCFunction)encodeInt32BE, METH_VARARGS, ""},
  {"decodeInt32BE", (PyCFunction)decodeInt32BE, METH_VARARGS, ""},
  {"encodeInt32LE", (PyCFunction)encodeInt32LE, METH_VARARGS, ""},
  {"decodeInt32LE", (PyCFunction)decodeInt32LE, METH_VARARGS, ""},

  {"encodeUInt64BE", (PyCFunction)encodeUInt64BE, METH_VARARGS, ""},
  {"decodeUInt64BE", (PyCFunction)decodeUInt64BE, METH_VARARGS, ""},
  {"encodeUInt64LE", (PyCFunction)encodeUInt64LE, METH_VARARGS, ""},
  {"decodeUInt64LE", (PyCFunction)decodeUInt64LE, METH_VARARGS, ""},
  {"encodeInt64BE", (PyCFunction)encodeInt64BE, METH_VARARGS, ""},
  {"decodeInt64BE", (PyCFunction)decodeInt64BE, METH_VARARGS, ""},
  {"encodeInt64LE", (PyCFunction)encodeInt64LE, METH_VARARGS, ""},
  {"decodeInt64LE", (PyCFunction)decodeInt64LE, METH_VARARGS, ""},

  {"encodeFloat32BE", (PyCFunction)encodeFloat32BE, METH_VARARGS, ""},
  {"decodeFloat32BE", (PyCFunction)decodeFloat32BE, METH_VARARGS, ""},
  {"encodeFloat32LE", (PyCFunction)encodeFloat32LE, METH_VARARGS, ""},
  {"decodeFloat32LE", (PyCFunction)decodeFloat32LE, METH_VARARGS, ""},

  {"encodeFloat64BE", (PyCFunction)encodeFloat64BE, METH_VARARGS, ""},
  {"decodeFloat64BE", (PyCFunction)decodeFloat64BE, METH_VARARGS, ""},
  {"encodeFloat64LE", (PyCFunction)encodeFloat64LE, METH_VARARGS, ""},
  {"decodeFloat64LE", (PyCFunction)decodeFloat64LE, METH_VARARGS, ""},

  {0,}
};

PyMODINIT_FUNC
init_fom(void)
{
  Py_InitModule3("_fom", methods, "FOM encoding helper functions.");
}
