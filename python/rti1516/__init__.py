# -*-python-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
#
# This file is part of OpenRTI.
#
# OpenRTI is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# OpenRTI is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
#

# Just import all so far
from _rti1516 import *

# A dynamic object system
import _fom

# FIXME move all this datatype python stuff into _fom for performance reasons

def alignTo(offset, octetBoundary):
    return ((offset + octetBoundary - 1) / octetBoundary) * octetBoundary

# Basic data is done with this implementation
class BasicData(object):
    def __init__(self, name, octetBoundary, encoder, decoder):
        self.__name = name
        self.__octetBoundary = octetBoundary
        self.__encoder = encoder
        self.__decoder = decoder

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        return self.__octetBoundary

    def resolveTypes(self, typemap):
        pass

    def resolveOctetBoundary(self):
        pass

    def referenceTypes(self, typeList):
        typeList.append(self)
        return typeList

    def encode(self, value, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        return self.__encoder(value, buffer, offset)

    def decode(self, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        return self.__decoder(buffer, offset)

# Simple data is a simple proxy to an other dataType
class SimpleData(object):
    def __init__(self, name, dataType):
        self.__name = name
        self.__dataType = dataType
        self.__type = None

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        t = self.__type()
        return t.getOctetBoundary()

    def resolveTypes(self, typemap):
        self.__type = weakref.ref(typemap[self.__dataType])

    def resolveOctetBoundary(self):
        pass

    def referenceTypes(self, typeList):
        typeList.append(self)
        t = self.__type()
        return t.referenceTypes(typeList)

    def encode(self, value, buffer, offset):
        t = self.__type()
        return t.encode(value, buffer, offset)

    def decode(self, buffer, offset):
        t = self.__type()
        return t.decode(buffer, offset)

# Most more complex datatypes need a backward reference to the
# typemap, since datatypes are also stored in the typemap, use a weakref
import weakref

class FixedRecordData(object):
    # Is the same implementation than simpleData - should we unify this?
    class Field(object):
        def __init__(self, name, dataType):
            self.__name = name
            self.__dataType = dataType
            self.__type = None

        def getName(self):
            return self.__name

        def getOctetBoundary(self):
            t = self.__type()
            return t.getOctetBoundary()

        def resolveTypes(self, typemap):
            self.__type = weakref.ref(typemap[self.__dataType])

        def resolveOctetBoundary(self):
            pass

        def referenceTypes(self, typeList):
            t = self.__type()
            return t.referenceTypes(typeList)

        def encode(self, value, buffer, offset):
            t = self.__type()
            return t.encode(value, buffer, offset)

        def decode(self, buffer, offset):
            t = self.__type()
            return t.decode(buffer, offset)

    def __init__(self, name):
        self.__name = name
        self.__octetBoundary = 0
        self.__fieldList = []

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        return self.__octetBoundary

    def addField(self, name, dataType):
        self.__fieldList.append(self.Field(name, dataType))

    def getFieldList(self):
        return self.__fieldList

    def resolveTypes(self, typemap):
        for f in self.__fieldList:
            f.resolveTypes(typemap)

    def resolveOctetBoundary(self):
        for f in self.__fieldList:
            f.resolveOctetBoundary()
            if self.__octetBoundary < f.getOctetBoundary():
                self.__octetBoundary = f.getOctetBoundary()

    def referenceTypes(self, typeList):
        if self in typeList:
            return typeList
        typeList.append(self)
        for f in self.__fieldList:
            typeList = f.referenceTypes(typeList)
        return typeList

    def encode(self, value, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        for f in self.__fieldList:
            buffer, offset = f.encode(value[f.getName()], buffer, offset)
        return buffer, offset

    def decode(self, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        value = FixedRecord()
        for f in self.__fieldList:
            value[f.getName()], offset = f.decode(buffer, offset)
        return value, offset


class FixedArrayData(object):
    def __init__(self, name, dataType, cardinality, isString):
        self.__name = name
        self.__dataType = dataType
        self.__cardinality = cardinality
        self.__isString = isString
        self.__type = None

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        t = self.__type()
        return t.getOctetBoundary()

    def resolveTypes(self, typemap):
        self.__type = weakref.ref(typemap[self.__dataType])

    def resolveOctetBoundary(self):
        pass

    def referenceTypes(self, typeList):
        if self in typeList:
            return typeList
        typeList.append(self)
        t = self.__type()
        return t.referenceTypes(typeList)

    def encode(self, value, buffer, offset):
        t = self.__type()
        count = self.__cardinality
        i = 0
        while i < count:
            i = i + 1
            buffer, offset = t.encode(value[i], buffer, offset)
        return buffer, offset

    def decode(self, buffer, offset):
        t = self.__type()
        value = []
        count = self.__cardinality
        i = 0
        if self.__isString:
            while i < count:
                i = i + 1
                v, offset = t.decode(buffer, offset)
                value.append(unichr(v))
            value = ''.join(value)
        else:
            while i < count:
                i = i + 1
                v, offset = t.decode(buffer, offset)
                value.append(v)
        return value, offset

class DynamicArrayData(object):
    def __init__(self, name, dataType, isString):
        self.__octetBoundary = 4
        self.__name = name
        self.__dataType = dataType
        self.__isString = isString
        self.__type = None
        self.__countType = None

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        return self.__octetBoundary

    def resolveTypes(self, typemap):
        self.__type = weakref.ref(typemap[self.__dataType])
        self.__countType = BasicData('HLAdynamicArraySize', 4, _fom.encodeUInt32BE, _fom.decodeUInt32BE)

    def resolveOctetBoundary(self):
        t = self.__type()
        if self.__octetBoundary < t.getOctetBoundary():
            self.__octetBoundary = t.getOctetBoundary()

    def referenceTypes(self, typeList):
        if self in typeList:
            return typeList
        typeList.append(self)
        t = self.__type()
        return t.referenceTypes(typeList)

    def encode(self, value, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        t = self.__type()
        count = len(value)
        buffer, offset = self.__countType.encode(count, buffer, offset)
        i = 0
        while i < count:
            i = i + 1
            buffer, offset = t.encode(value[i], buffer, offset)
        return buffer, offset

    def decode(self, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        t = self.__type()
        value = []
        count, offset = self.__countType.decode(buffer, offset)
        i = 0
        if self.__isString:
            while i < count:
                i = i + 1
                v, offset = t.decode(buffer, offset)
                value.append(unichr(v))
            value = ''.join(value)
        else:
            while i < count:
                i = i + 1
                v, offset = t.decode(buffer, offset)
                value.append(v)
        return value, offset

class EnumeratedData(object):
    def __init__(self, name, dataType):
        self.__name = name
        self.__enumeratorValueMap = {}
        self.__valueEnumeratorValueMap = {}
        self.__dataType = dataType
        self.__type = None

    def addEnumerator(self, name, values):
        # FIXME might be more than one, might not be able to do that before resolve?
        intValues = int(values)
        self.__enumeratorValueMap[name] = values
        self.__valueEnumeratorValueMap[intValues] = name

    def getEnumeratorValueMap(self):
        return self.__enumeratorValueMap

    def getValueEnumeratorMap(self):
        return self.__valueEnumeratorMap

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        t = self.__type()
        return t.getOctetBoundary()

    def resolveTypes(self, typemap):
        self.__type = weakref.ref(typemap[self.__dataType])

    def resolveOctetBoundary(self):
        pass

    def referenceTypes(self, typeList):
        typeList.append(self)
        t = self.__type()
        return t.referenceTypes(typeList)

    def encode(self, value, buffer, offset):
        t = self.__type()
        valueNumber = self.__enumeratorValueMap[value]
        buffer, offset = self.__type.encode(valueNumber, buffer, offset)
        return buffer, offset

    def decode(self, buffer, offset):
        t = self.__type()
        valueNumber, offset = t.decode(buffer, offset)
        value = self.__valueEnumeratorValueMap[valueNumber]
        return value, offset


class VariantRecordData(object):
    # Is the same implementation than simpleData - should we unify this?
    class Alternative(object):
        def __init__(self, name, enumerator, dataType):
            self.__name = name
            self.__enumerator = enumerator
            self.__dataType = dataType
            self.__type = None

        def getName(self):
            return self.__name

        def getEnumerator(self):
            return self.__enumerator

        def getOctetBoundary(self):
            t = self.__type()
            return t.getOctetBoundary()

        def resolveTypes(self, typemap):
            self.__type = weakref.ref(typemap[self.__dataType])

        def resolveOctetBoundary(self):
            pass

        def referenceTypes(self, typeList):
            t = self.__type()
            return t.referenceTypes(typeList)

        def encode(self, value, buffer, offset):
            t = self.__type()
            return t.encode(value, buffer, offset)

        def decode(self, buffer, offset):
            t = self.__type()
            return t.decode(buffer, offset)

    def __init__(self, name, dataType):
        self.__name = name
        self.__octetBoundary = 0
        self.__dataType = dataType
        self.__type = None
        self.__nameAlternativeMap = {}
        self.__enumeratorAlternativeMap = {}

    def getName(self):
        return self.__name

    def getOctetBoundary(self):
        return self.__octetBoundary

    def addAlternative(self, name, enumerator, dataType):
        alternative = self.Alternative(name, enumerator, dataType)
        self.__nameAlternativeMap[name] = alternative
        self.__enumeratorAlternativeMap[enumerator] = alternative

    def getNameAlternativeMap(self):
        return self.__nameAlternativeMap

    def getEnumeratorAlternativeMap(self):
        return self.__enumeratorAlternativeMap

    def resolveTypes(self, typemap):
        self.__type = weakref.ref(typemap[self.__dataType])
        for a in self.__nameAlternativeMap.values():
            a.resolveTypes(typemap)

    def resolveOctetBoundary(self):
        for a in self.__nameAlternativeMap.values():
            a.resolveOctetBoundary()
            if self.__octetBoundary < a.getOctetBoundary():
                self.__octetBoundary = a.getOctetBoundary()

    def referenceTypes(self, typeList):
        if self in typeList:
            return typeList
        typeList.append(self)
        t = self.__type()
        typeList = t.referenceTypes(typeList)
        for a in self.__nameAlternativeMap.values():
            typeList = a.referenceTypes(typeList)
        return typeList

    def encode(self, value, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        t = self.__type()
        for k in value.keys():
            if k in self.__nameAlternativeMap:
                alternative = self.__nameAlternativeMap[k]
                buffer, offset = t.encode(alternative.getEnumerator(), buffer, offset)
                return alternative.encode(value[key], buffer, offset)
        # FIXME default?? Throw??
        return buffer, offset

    def decode(self, buffer, offset):
        offset = alignTo(offset, self.__octetBoundary)
        t = self.__type()
        enumerator, offset = t.decode(buffer, offset)
        alternative = self.__enumeratorAlternativeMap[enumerator]
        value = {}
        value[alternative.getName()], offset = alternative.decode(buffer, offset)
        return value, offset

##################################################################
##################################################################
##################################################################

# We need to read an xml file
import xml.sax.handler

class FOMContentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        # Object class handling
        self.__objectClassList = []
        self.__objectClassStack = []

        # Interaction class handling
        self.__interactionClassList = []
        self.__interactionClassStack = []

        # Data type handling
        self.__typeMap = {}
        self.__currentEnumeratedData = None
        self.__currentFixedRecord = None
        self.__currentVariant = None

    def getTypeMap(self):
        for value in self.__typeMap.values():
            value.resolveTypes(self.__typeMap)
        for value in self.__typeMap.values():
            value.resolveOctetBoundary()
        return self.__typeMap

    def getObjectClassList(self):
        for objectClass in self.__objectClassList:
            parentObjectClass = objectClass["parentObjectClass"]
            while parentObjectClass:
                for attribute in parentObjectClass["attributeList"]:
                    objectClass["attributeList"].append(attribute)
                parentObjectClass = parentObjectClass["parentObjectClass"]
        return self.__objectClassList

    def getInteractionClassList(self):
        for interactionClass in self.__interactionClassList:
            parentInteractionClass = interactionClass["parentInteractionClass"]
            while parentInteractionClass:
                for parameter in parentInteractionClass["parameterList"]:
                    interactionClass["parameterList"].append(parameter)
                parentInteractionClass = parentInteractionClass["parentInteractionClass"]
        return self.__interactionClassList

    def startElement(self, elementName, attributes):
        if elementName == "objectClass":
            name = attributes["name"]
            objectClass = dict(attributes)
            objectClass["attributeList"] = []
            if 0 < len(self.__objectClassStack):
                objectClass["parentObjectClass"] = self.__objectClassStack[len(self.__objectClassStack) - 1]
            else:
                objectClass["parentObjectClass"] = None
            self.__objectClassStack.append(objectClass)
            self.__objectClassList.append(objectClass)

        elif elementName == "attribute":
            objectClass = self.__objectClassStack[len(self.__objectClassStack) - 1]
            objectClass["attributeList"].append(dict(attributes))

        elif elementName == "interactionClass":
            name = attributes["name"]
            interactionClass = dict(attributes)
            interactionClass["parameterList"] = []
            if 0 < len(self.__interactionClassStack):
                interactionClass["parentInteractionClass"] = self.__interactionClassStack[len(self.__interactionClassStack) - 1]
            else:
                interactionClass["parentInteractionClass"] = None
            self.__interactionClassStack.append(interactionClass)
            self.__interactionClassList.append(interactionClass)

        elif elementName == "parameter":
            interactionClass = self.__interactionClassStack[len(self.__interactionClassStack) - 1]
            interactionClass["parameterList"].append(dict(attributes))

        elif elementName == "arrayData":
            name = attributes["name"]
            dataType = attributes["dataType"]
            encoding = attributes["encoding"]
            cardinality = attributes["cardinality"]
            if name == 'HLAASCIIstring' or name == 'HLAunicodeString':
                isString = True
            else:
                isString = False
            if cardinality.lower() == "dynamic":
                self.__typeMap[name] = DynamicArrayData(name, dataType, isString)
            else:
                self.__typeMap[name] = FixedArrayData(name, dataType, int(cardinality), isString)

        elif elementName == "basicData":
            name = attributes["name"]
            if name == "HLAoctet":
                self.__typeMap[name] = BasicData(name, 1, _fom.encodeUInt8, _fom.decodeUInt8)
            elif name == "HLAoctetPairBE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeUInt16BE, _fom.decodeUInt16BE)
            elif name == "HLAoctetPairLE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeUInt16LE, _fom.decodeUInt16LE)
            elif name == "HLAinteger16BE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeInt16BE, _fom.decodeInt16BE)
            elif name == "HLAinteger16LE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeInt16LE, _fom.decodeInt16LE)
            elif name == "HLAinteger32BE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeInt32BE, _fom.decodeInt32BE)
            elif name == "HLAinteger32LE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeInt32LE, _fom.decodeInt32LE)
            elif name == "HLAinteger64BE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeInt64BE, _fom.decodeInt64BE)
            elif name == "HLAinteger64LE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeInt64LE, _fom.decodeInt64LE)
            elif name == "HLAfloat32BE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeFloat32BE, _fom.decodeFloat32BE)
            elif name == "HLAfloat32LE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeFloat32LE, _fom.decodeFloat32LE)
            elif name == "HLAfloat64BE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeFloat64BE, _fom.decodeFloat64BE)
            elif name == "HLAfloat64LE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeFloat64LE, _fom.decodeFloat64LE)
            elif name == "UnsignedShort":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeUInt16BE, _fom.decodeUInt16BE)
            elif name == "Unsignedinteger16BE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeUInt16BE, _fom.decodeUInt16BE)
            elif name == "Unsignedinteger16LE":
                self.__typeMap[name] = BasicData(name, 2, _fom.encodeUInt16LE, _fom.decodeUInt16LE)
            elif name == "UnsignedLong":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeUInt32BE, _fom.decodeUInt32BE)
            elif name == "Unsignedinteger32BE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeUInt32BE, _fom.decodeUInt32BE)
            elif name == "Unsignedinteger32LE":
                self.__typeMap[name] = BasicData(name, 4, _fom.encodeUInt32LE, _fom.decodeUInt32LE)
            elif name == "Unsignedinteger64BE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeUInt64BE, _fom.decodeUInt64BE)
            elif name == "Unsignedinteger64LE":
                self.__typeMap[name] = BasicData(name, 8, _fom.encodeUInt64LE, _fom.decodeUInt64LE)
            # else:

        elif elementName == "enumeratedData":
            name = attributes["name"]
            representation = attributes["representation"]
            self.__currentEnumeratedData = EnumeratedData(name, representation)
            self.__typeMap[name] = self.__currentEnumeratedData

        elif elementName == "enumerator":
            if not self.__currentEnumeratedData:
                return # FIXME error
            self.__currentEnumeratedData.addEnumerator(attributes["name"], attributes["values"])

        elif elementName == "fixedRecordData":
            name = attributes["name"]
            self.__currentFixedRecord = FixedRecordData(name)
            self.__typeMap[name] = self.__currentFixedRecord

        elif elementName == "field":
            if not self.__currentFixedRecord:
                return # FIXME error
            self.__currentFixedRecord.addField(attributes["name"], attributes["dataType"])

        elif elementName == "simpleData":
            name = attributes["name"]
            representation = attributes["representation"]
            self.__typeMap[name] = SimpleData(name, representation)

        elif elementName == "variantRecordData":
            name = attributes["name"]
            # encoding = attributes["encoding"]
            # discriminant = attributes["discriminant"]
            dataType = attributes["dataType"]
            self.__currentVariant = VariantRecordData(name, dataType)
            self.__typeMap[name] = self.__currentVariant

        elif elementName == "alternative":
            if not self.__currentVariant:
                return
            name = attributes["name"]
            enumerator = attributes["enumerator"]
            dataType = attributes["dataType"]
            self.__currentVariant.addAlternative(name, enumerator, dataType)

    def endElement(self, elementName):
        if elementName == "objectClass":
            self.__objectClassStack.pop()

        elif elementName == "interactionClass":
            self.__interactionClassStack.pop()

        elif elementName == "enumeratedData":
            self.__currentEnumeratedData = None

        elif elementName == "fixedRecordData":
            self.__currentFixedRecord = None

        elif elementName == "variantRecordData":
            self.__currentVariant = None

##################################################################
##################################################################
##################################################################

class FixedRecord(object):
    def __getitem__(self, key):
        return self.__dict__[key]
    def __setitem__(self, key, value):
        self.__dict__[key] = value
    def __delitem__(self, key):
        del self.__dict__[key]
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return repr(self.__dict__)

class NameHandlePair(object):
    def __init__(self, name, handle):
        self.__name = name
        self.__handle = handle

    def getName(self):
        return self.__name

    def getHandle(self):
        return self.__handle

class NameHandleMap(object):
    def __init__(self):
        self.__nameMap = {}
        self.__handleMap = {}

    def insert(self, nameHandlePair):
        self.__handleMap[nameHandlePair.getHandle()] = nameHandlePair
        self.__nameMap[nameHandlePair.getName()] = nameHandlePair

    def erase(self, nameHandlePair):
        del self.__handleMap[nameHandlePair.getHandle()]
        del self.__nameMap[nameHandlePair.getName()]

    def get(self, key):
        try:
            return self.__handleMap[key]
        except KeyError:
            pass
        return self.__nameMap[key]

    def __getitem__(self, key):
        return self.get(key)

    def items(self):
        return self.__handleMap.values()

class Container(NameHandlePair):
    def __init__(self, name, handle):
        NameHandlePair.__init__(self, name, handle)
        self.__dataType = None
        self.__encoder = None
        self.__encoderReferences = []

    def setEncoder(self, dataType, typeMap):
        try:
            encoder = typeMap[dataType]
            self.__dataType = dataType
            self.__encoder = encoder
            self.__encoderReferences = encoder.referenceTypes([])
        except KeyError:
            pass

    def encode(self, value):
        if not self.__encoder:
            return bytearray()
        encodedValue, off = self.__encoder.encode(value, bytearray(), 0)
        return encodedValue

    def decode(self, encodedValue):
        if not self.__encoder:
            return list()
        value, off = self.__encoder.decode(encodedValue, 0)
        return value


class Parameter(Container):
    def __init__(self, name, handle):
        Container.__init__(self, name, handle)

class InteractionClass(NameHandlePair):
    def __init__(self, name, handle):
        NameHandlePair.__init__(self, name, handle)
        self.__parameterMap = NameHandleMap()

class ObjectClassAttribute(Container):
    def __init__(self, name, handle):
        Container.__init__(self, name, handle)

class ObjectClass(NameHandlePair):
    def __init__(self, name, handle):
        NameHandlePair.__init__(self, name, handle)
        self.__attributeMap = NameHandleMap()

    def getAttribute(self, key):
        return self.__attributeMap.get(key)

    def insertAttribute(self, attribute):
        self.__attributeMap.insert(attribute)

    def createObjectInstance(self, name, handle):
        # This is the possible callback to overwrite
        objectInstance = ObjectInstance(name, handle, self)

        for attribute in self.__attributeMap.items():
            objectInstance.insertAttribute(self.createObjectInstanceAttribute(attribute))

        return objectInstance

    def createObjectInstanceAttribute(self, objectClassAttribute):
        return ObjectInstanceAttribute(objectClassAttribute)

class ObjectInstanceAttribute(object):
    def __init__(self, objectClassAttribute):
        self.__objectClassAttribute = objectClassAttribute

    def getName(self):
        return self.__objectClassAttribute.getName()

    def getHandle(self):
        return self.__objectClassAttribute.getHandle()

    def encode(self, value):
        return self.__objectClassAttribute.encode(value)

    def decode(self, encodedValue):
        return self.__objectClassAttribute.decode(encodedValue)

class ObjectInstance(NameHandlePair):
    def __init__(self, name, handle, objectClass):
        NameHandlePair.__init__(self, name, handle)
        self.__objectClass = objectClass
        self.__attributeMap = NameHandleMap()

    def insertAttribute(self, objectInstanceAttribute):
        name = objectInstanceAttribute.getName()
        handle = objectInstanceAttribute.getHandle()
        self.__attributeMap.insert(objectInstanceAttribute)
        self.__dict__[name] = []

    def reflectAttributeValues(self, attributeValues, tag, order, transport, d1, d2, d3, d4):
        for handle, encodedValue in attributeValues.items():
            objectInstanceAttribute = self.__attributeMap[handle]
            name = objectInstanceAttribute.getName()
            self.__dict__[name] = objectInstanceAttribute.decode(encodedValue)

class Federate(NameHandlePair):
    def __init__(self, name, handle, rtiAmbassador, federateAmbassador):
        NameHandlePair.__init__(self, name, handle)
        self.__rtiAmbassador = rtiAmbassador
        self.__federateAmbassador = federateAmbassador

    def resignFederationExecution(self, resignAction):
        self.__rtiAmbassador.resignFederationExecution(resignAction)

    def getInteractionClass(self, key):
        return self.__federateAmbassador.getInteractionClass(key)

    def getInteractionClasses(self):
        return self.__federateAmbassador.getInteractionClasses()

    def createInteractionClass(self, name, handle):
        # This is the possible callback to overwrite
        return InteractionClass(name, handle)

    def getObjectClass(self, key):
        return self.__federateAmbassador.getObjectClass(key)

    def getObjectClasses(self):
        return self.__federateAmbassador.getObjectClasses()

    def createObjectClass(self, name, handle):
        # This is the possible callback to overwrite
        return ObjectClass(name, handle)

    def getObjectInstance(self, key):
        return self.__federateAmbassador.getObjectInstance(key)

    def getObjectInstances(self):
        return self.__federateAmbassador.getObjectInstances()

    def readFOM(self, filename):
        contentHandler = FOMContentHandler()
        parser = xml.sax.make_parser()
        parser.setContentHandler(contentHandler)
        parser.parse(filename)
        typeMap = contentHandler.getTypeMap()

        interactionClassList = contentHandler.getInteractionClassList()
        for interactionClassData in interactionClassList:
            interactionClassName = interactionClassData["name"]
            interactionClassHandle = self.__rtiAmbassador.getInteractionClassHandle(interactionClassName)

            interactionClass = self.createInteractionClass(interactionClassName, interactionClassHandle)
            if not interactionClass:
                continue

            # for parameterData in interactionClass["parameterList"]:
            #     parameterName = parameterData["name"]
            #     parameterHandle = self.__rtiAmbassador.getParameterHandle(interactionClassHandle, parameterName)

            self.__federateAmbassador.insertInteractionClass(interactionClass)
            self.__rtiAmbassador.publishInteractionClass(interactionClassHandle)
            self.__rtiAmbassador.subscribeInteractionClass(interactionClassHandle)

        objectClassList = contentHandler.getObjectClassList()
        for objectClassData in objectClassList:
            objectClassName = objectClassData["name"]
            objectClassHandle = self.__rtiAmbassador.getObjectClassHandle(objectClassName)

            objectClass = self.createObjectClass(objectClassName, objectClassHandle)
            if not objectClass:
                continue

            attributeHandleSet = set()
            for attributeData in objectClassData["attributeList"]:
                attributeName = attributeData["name"]
                attributeHandle = self.__rtiAmbassador.getAttributeHandle(objectClassHandle, attributeName)
                attributeHandleSet.add(attributeHandle)

                attribute = ObjectClassAttribute(attributeName, attributeHandle)
                attribute.setEncoder(attributeData["dataType"], typeMap)

                objectClass.insertAttribute(attribute)

            self.__federateAmbassador.insertObjectClass(objectClass)
            self.__rtiAmbassador.publishObjectClassAttributes(objectClassHandle, attributeHandleSet)
            self.__rtiAmbassador.subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet)



# make this private
class FederateAmbassador(object):
    def __init__(self):
        # Map object instances by handle
        self.__interactionClassMap = NameHandleMap()
        self.__objectClassMap = NameHandleMap()
        self.__objectInstanceMap = NameHandleMap()

    def getInteractionClass(self, key):
        return self.__interactionClassMap[key]

    def getInteractionClasses(self):
        return self.__interactionClassMap.items()

    def insertInteractionClass(self, interactionClass):
        self.__interactionClassMap.insert(interactionClass)

    def getObjectClass(self, key):
        return self.__objectClassMap[key]

    def getObjectClasses(self):
        return self.__objectClassMap.items()

    def insertObjectClass(self, objectClass):
        self.__objectClassMap.insert(objectClass)

    def getObjectInstance(self, key):
        return self.__objectInstanceMap[key]

    def getObjectInstances(self):
        return self.__objectInstanceMap.items()

    def insertObjectInstance(self, objectInstance):
        self.__objectInstanceMap.insert(objectInstance)

    def eraseObjectInstance(self, objectInstance):
        self.__objectInstanceMap.erase(objectInstance)

    def discoverObjectInstance(self, objectInstanceHandle, objectClassHandle, name):
        objectClass = self.__objectClassMap[objectClassHandle]
        objectInstance = objectClass.createObjectInstance(name, objectInstanceHandle)
        self.insertObjectInstance(objectInstance)

    def removeObjectInstance(self, objectInstanceHandle, tag, sentOrder, logicalTime, recievedOrder, retractionHandle):
        objectInstance = self.getObjectInstance(objectInstanceHandle)
        self.eraseObjectInstance(objectInstance)

    def reflectAttributeValues(self, objectInstanceHandle, attributeValues, tag, order, transport, d1, d2, d3, d4):
        objectInstance = self.getObjectInstance(objectInstanceHandle)
        objectInstance.reflectAttributeValues(attributeValues, tag, order, transport, d1, d2, d3, d4)

class Ambassador(object):
    def __init__(self):
        self.__rtiAmbassador = None
        self.__federate = None

    def connect(self, *args):
        self.__rtiAmbassador = RTIambassador(*args)

    def disconnect(self):
        self.__rtiAmbassador = None
        self.__federate = None

    def createFederationExecution(self, federationExecution, fom):
        self.__rtiAmbassador.createFederationExecution(federationExecution, fom)

    def destroyFederationExecution(self, federationExecution):
        self.__rtiAmbassador.destroyFederationExecution(federationExecution)

    def joinFederationExecution(self, federateType, federationExecution):
        if self.__federate:
            raise "Ambassador already joined"
        federateAmbassador = FederateAmbassador()
        federateHandle = self.__rtiAmbassador.joinFederationExecution(federateType, federationExecution, federateAmbassador)
        self.__federate = self.createFederate(federateType, federateHandle, self.__rtiAmbassador, federateAmbassador)
        return self.__federate.getHandle()

    def createFederate(self, federateType, federateHandle, rtiAmbassador, federateAmbassador):
        # This is the possible callback on join
        return Federate(federateType, federateHandle, rtiAmbassador, federateAmbassador)

    def resignFederationExecution(self, resignAction = NO_ACTION):
        self.__federate.resignFederationExecution(resignAction)

    def readFOM(self, filename):
        self.__federate.readFOM(filename)

    def getInteractionClass(self, key):
        return self.__federate.getInteractionClass(key)

    def getInteractionClasses(self):
        return self.__federate.getInteractionClasses()

    def insertInteractionClass(self, interactionClass):
        self.__federate.insertInteractionClass(interactionClass)


    def getObjectClass(self, key):
        return self.__federate.getObjectClass(key)

    def getObjectClasses(self):
        return self.__federate.getObjectClasses()

    def insertObjectClass(self, objectClass):
        self.__federate.insertObjectClass(objectClass)


    def getObjectInstance(self, key):
        return self.__federate.getObjectInstance(key)

    def getObjectInstances(self):
        return self.__federate.getObjectInstances()


    def evokeCallback(self, *args):
        self.__rtiAmbassador.evokeCallback(*args)

    def evokeMultipleCallbacks(self, *args):
        self.__rtiAmbassador.evokeMultipleCallbacks(*args)
