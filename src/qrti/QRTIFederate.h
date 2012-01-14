/* -*-c++-*- OpenRTI - Copyright (C) 2010-2012 Mathias Froehlich
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

#ifndef QRTIFederate_h
#define QRTIFederate_h

#include <Qt/QtCore>

#include "QHLAArrayDataType.h"
#include "QHLABasicDataType.h"
#include "QHLAEnumeratedDataType.h"
#include "QHLASimpleDataType.h"
#include "QHLAFixedRecordDataType.h"
#include "QHLAVariantRecordDataType.h"

#include "QHLAArrayDataElement.h"
#include "QHLAStringDataElement.h"
#include "QHLAEnumeratedDataElement.h"
#include "QHLAScalarDataElement.h"
#include "QHLAFixedRecordDataElement.h"
#include "QHLAVariantRecordDataElement.h"

#include "QRTIObjectClass.h"
#include "QRTIObjectClassAttribute.h"
#include "QRTIObjectInstance.h"
#include "QRTIObjectInstanceAttribute.h"

class QRTIFederate : public QObject {
public:
  QRTIFederate(QObject* parent = 0);
  virtual ~QRTIFederate();

  QHLADataType* getDataType(const QString& name)
  {
    if (QHLADataType* dataType = _basicDataTypeList->getDataType(name))
      return dataType;
    if (QHLADataType* dataType = _simpleDataTypeList->getDataType(name))
      return dataType;
    if (QHLADataType* dataType = _enumeratedDataTypeList->getDataType(name))
      return dataType;
    if (QHLADataType* dataType = _arrayDataTypeList->getDataType(name))
      return dataType;
    if (QHLADataType* dataType = _fixedRecordDataTypeList->getDataType(name))
      return dataType;
    if (QHLADataType* dataType = _variantRecordDataTypeList->getDataType(name))
      return dataType;
    return 0;
  }


  QHLABasicDataType* createBasicDataType()
  {
    return _basicDataTypeList->createDataType();
  }
  QHLABasicDataType* getBasicDataType(const QString& name)
  {
    return _basicDataTypeList->getDataType(name);
  }

  QHLASimpleDataType* createSimpleDataType()
  {
    return _simpleDataTypeList->createDataType();
  }

  QHLAEnumeratedDataType* createEnumeratedDataType()
  {
    return _enumeratedDataTypeList->createDataType();
  }
  QHLAEnumeratedDataType* getEnumeratedDataType(const QString& name)
  {
    return _enumeratedDataTypeList->getDataType(name);
  }

  QHLAArrayDataType* createArrayDataType()
  {
    return _arrayDataTypeList->createDataType();
  }
  QHLAArrayDataType* getArrayDataType(const QString& name)
  {
    return _arrayDataTypeList->getDataType(name);
  }

  QHLAFixedRecordDataType* createFixedRecordDataType()
  {
    return _fixedRecordDataTypeList->createDataType();
  }
  QHLAFixedRecordDataType* getFixedRecordDataType(const QString& name)
  {
    return _fixedRecordDataTypeList->getDataType(name);
  }

  QHLAVariantRecordDataType* createVariantRecordDataType()
  {
    return _variantRecordDataTypeList->createDataType();
  }
  QHLAVariantRecordDataType* getVariantRecordDataType(const QString& name)
  {
    return _variantRecordDataTypeList->getDataType(name);
  }


  // Not to confuse with QObject::connect ...
  // virtual void connectRTI() = 0;
  // virtual void disconnectRTI() = 0;

  virtual void join(const QString& federationExecutionName) = 0;
  // virtual void resign() = 0;

  virtual bool readObjectModel(const QString& objectModelFile) = 0;

  int getObjectClassIndex(const QString& name)
  { return _nameObjectClassIndexMap.value(name, -1); }
  int getNumObjectClasses() const
  { return _objectClassList.size(); }
  QRTIObjectClass* getObjectClass(int index)
  { return _objectClassList.value(index, 0); }
  QRTIObjectClass* getObjectClass(const QString& name)
  { return getObjectClass(getObjectClassIndex(name)); }

// protected:
  virtual QRTIObjectClass* _createObjectClass(const QString& name, QRTIObjectClass* parentObjectClass) = 0;
  int _insertObjectClass(QRTIObjectClass* objectClass)
  {
    int index = _objectClassList.size();
    _nameObjectClassIndexMap[objectClass->name()] = index;
    _objectClassList.push_back(objectClass);
    objectClass->_indexInFederate = index;
    return index;
  }
private:
  QMap<QString, int> _nameObjectClassIndexMap;
  QList<QRTIObjectClass*> _objectClassList;
public:

  int getNumObjectInstances() const
  { return _objectInstanceList.size(); }
  int getObjectInstanceIndex(const QString& name)
  { return _nameObjectInstanceIndexMap.value(name, -1); }
  QRTIObjectInstance* getObjectInstance(int index)
  { return _objectInstanceList.value(index, 0); }
  QRTIObjectInstance* getObjectInstance(const QString& name)
  { return getObjectInstance(getObjectInstanceIndex(name)); }

protected:
  // virtual QRTIObjectInstance* _registerObjectInstance(QRTIObjectClass* objectClass) = 0;
  // virtual QRTIObjectInstance* _registerObjectInstance(const QString& name, QRTIObjectClass* objectClass) = 0;
  int _insertObjectInstance(QRTIObjectInstance* objectInstance)
  {
    int index = _objectInstanceList.size();
    _nameObjectInstanceIndexMap[objectInstance->name()] = index;
    _objectInstanceList.push_back(objectInstance);
    objectInstance->_indexInFederate = index;
    /// FIXME way too crude
    _objectInstanceModel->reset();
    return index;
  }
  void _eraseObjectInstance(QRTIObjectInstance* objectInstance)
  {
    if (!objectInstance)
      return;
    int index = objectInstance->_indexInFederate;
    objectInstance->_indexInFederate = -1;
    _nameObjectInstanceIndexMap.remove(objectInstance->name());
    _objectInstanceList.removeAt(index);
    delete objectInstance;
    for (int i = index; i < _objectInstanceList.size(); ++i)
      _objectInstanceList[i]->_indexInFederate = i;
    /// FIXME way too crude
    _objectInstanceModel->reset();
  }
private:
  QMap<QString, int> _nameObjectInstanceIndexMap;
  QList<QRTIObjectInstance*> _objectInstanceList;
public:

  class ObjectClassModel : public QAbstractItemModel {
  public:
    ObjectClassModel(QRTIFederate* federate) :
      QAbstractItemModel(federate),
      _federate(federate)
    {
    }
    virtual ~ObjectClassModel()
    {
      _federate = 0;
    }

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const
    {
      if (row < 0 || column < 0)
        return QModelIndex();
      if (!parent.isValid()) {
        // Handle the root object, child objects are the object classes
        QRTIObjectClass* objectClass = _federate->getObjectClass(row);
        if (!objectClass)
          return QModelIndex();
        return createIndex(row, column, objectClass);
      } else {
        // A valid index, always contains a QObject
        QObject* object = static_cast<QObject*>(parent.internalPointer());
        // qAssert(object);
        if (QRTIObjectClass* objectClass = dynamic_cast<QRTIObjectClass*>(object)) {
          // Handle the object class objects, child objects are the object class attributes
          QRTIObjectClassAttribute* objectClassAttribute = objectClass->getObjectClassAttribute(row);
          if (!objectClassAttribute)
            return QModelIndex();
          return createIndex(row, column, objectClassAttribute);
        } else {
          return QModelIndex();
        }
      }
    }
    virtual QModelIndex parent(const QModelIndex& index) const
    {
      if (!index.isValid())
        return QModelIndex();
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(index.internalPointer());
      // qAssert(object);
      if (QRTIObjectClassAttribute* objectClassAttribute = dynamic_cast<QRTIObjectClassAttribute*>(object)) {
        // Parents of attributes are classes
        QRTIObjectClass* objectClass = objectClassAttribute->getObjectClass();
        return createIndex(objectClass->_indexInFederate, 0, objectClass);
      // } else if (QRTIObjectClass* objectClass = dynamic_cast<QRTIObjectClass*>(object)) {
      //   // parents of object classes are the root
      //   return QModelIndex();
      } else {
        return QModelIndex();
      }
    }

    virtual int rowCount(const QModelIndex& parent) const
    {
      // Handle the root item
      if (!parent.isValid())
        return _federate->getNumObjectClasses();
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(parent.internalPointer());
      // qAssert(object);
      if (QRTIObjectClass* objectClass = dynamic_cast<QRTIObjectClass*>(object)) {
        // object classes have the attributes as children
        return objectClass->getNumObjectClassAttributes();
      // } else if (QRTIObjectClassAttribute* objectClassAttribute = dynamic_cast<QRTIObjectClassAttribute*>(object)) {
      //   // ObjectClassAttributes have no children
      //   return 0;
      } else {
        return 0;
      }
    }
    virtual int columnCount(const QModelIndex& parent) const
    {
      // Handle the root item
      if (!parent.isValid())
        return 1;
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(parent.internalPointer());
      if (QRTIObjectClass* objectClass = dynamic_cast<QRTIObjectClass*>(object)) {
        return 1;
      } else if (QRTIObjectClassAttribute* objectClassAttribute = dynamic_cast<QRTIObjectClassAttribute*>(object)) {
        return 1;
      } else {
        return 0;
      }
    }
    virtual QVariant data(const QModelIndex& index, int role) const
    {
      if (role != Qt::DisplayRole)
        return QVariant();

      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(index.internalPointer());
      if (QRTIObjectClass* objectClass = dynamic_cast<QRTIObjectClass*>(object)) {
        // object classes have the attributes as children
        switch (index.column()) {
        case 0:
          return objectClass->name();
        default:
          return QVariant();
        }
      } else if (QRTIObjectClassAttribute* objectClassAttribute = dynamic_cast<QRTIObjectClassAttribute*>(object)) {
        // ObjectClassAttributes data
        switch (index.column()) {
        case 0:
          return objectClassAttribute->name();
        default:
          return QVariant();
        }
      } else {
        return QVariant();
      }
    }
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
      if (role != Qt::DisplayRole)
        return QVariant();
      switch (section) {
      case 0:
        return QString(tr("ObjectClasses"));
      default:
        return QVariant();
      }
    }
  private:
    QRTIFederate* _federate;
  };

  QAbstractItemModel* getObjectClassListModel()
  { return _objectClassModel; }

  class ObjectInstanceModel : public QAbstractItemModel {
  public:
    ObjectInstanceModel(QRTIFederate* federate) :
      QAbstractItemModel(federate),
      _federate(federate)
    {
    }
    virtual ~ObjectInstanceModel()
    {
      _federate = 0;
    }

    /// FIXME
    void reset()
    { QAbstractItemModel::reset(); }

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const
    {
      if (row < 0 || column < 0)
        return QModelIndex();
      if (!parent.isValid()) {
        // Handle the root object, child objects are the object instances
        QRTIObjectInstance* objectInstance = _federate->getObjectInstance(row);
        if (!objectInstance)
          return QModelIndex();
        return createIndex(row, column, objectInstance);
      } else {
        // A valid index, always contains a QObject
        QObject* object = static_cast<QObject*>(parent.internalPointer());
        // qAssert(object);
        if (QRTIObjectInstance* objectInstance = dynamic_cast<QRTIObjectInstance*>(object)) {
          // Handle the object instance objects, child objects are the object instance attributes
          QRTIObjectInstanceAttribute* objectInstanceAttribute = objectInstance->getObjectInstanceAttribute(row);
          if (!objectInstanceAttribute)
            return QModelIndex();
          return createIndex(row, column, objectInstanceAttribute);
        } else if (QRTIObjectInstanceAttribute* objectInstanceAttribute = dynamic_cast<QRTIObjectInstanceAttribute*>(object)) {
          if (row == 0)
            return createIndex(row, column, objectInstanceAttribute->_dataElement);
          return QModelIndex();

        } else if (QHLAArrayDataElement* dataElement = dynamic_cast<QHLAArrayDataElement*>(object)) {
          QHLADataElement* childElement = dataElement->dataElement(row);
          if (!childElement)
            return QModelIndex();
          return createIndex(row, column, childElement);
        } else if (QHLAFixedRecordDataElement* dataElement = dynamic_cast<QHLAFixedRecordDataElement*>(object)) {
          QHLADataElement* childElement = dataElement->field(row);
          if (!childElement)
            return QModelIndex();
          return createIndex(row, column, childElement);
        // } else if (QHLADataElement* dataElement = dynamic_cast<QHLADataElement*>(object)) {
      //   return createIndex(dataElement->indexInParent(), 0, dataElement);
        } else {
          return QModelIndex();
        }
      }
    }
    virtual QModelIndex parent(const QModelIndex& index) const
    {
      if (!index.isValid())
        return QModelIndex();
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(index.internalPointer());
      if (QRTIObjectInstanceAttribute* objectInstanceAttribute = dynamic_cast<QRTIObjectInstanceAttribute*>(object)) {
        // Parents of attributes are instances
        QRTIObjectInstance* objectInstance = objectInstanceAttribute->getObjectInstance();
        return createIndex(objectInstance->_indexInFederate, 0, objectInstance);
      } else if (QHLADataElement* dataElement = dynamic_cast<QHLADataElement*>(object)) {
        return createIndex(dataElement->indexInParent(), 0, dataElement->parent());
      // } else if (QRTIObjectInstance* objectInstance = dynamic_cast<QRTIObjectInstance*>(object)) {
      //   // parents of object instances are the root
      //   return QModelIndex();
      } else {
        return QModelIndex();
      }
    }

    virtual int rowCount(const QModelIndex& parent) const
    {
      // Handle the root item
      if (!parent.isValid())
        return _federate->getNumObjectInstances();
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(parent.internalPointer());
      if (QRTIObjectInstance* objectInstance = dynamic_cast<QRTIObjectInstance*>(object)) {
        // object instances have the attributes as children
        return objectInstance->getNumObjectInstanceAttributes();
      } else if (QRTIObjectInstanceAttribute* objectInstanceAttribute = dynamic_cast<QRTIObjectInstanceAttribute*>(object)) {
        if (!objectInstanceAttribute->_dataElement)
          return 0;
        return 1;
      } else if (QHLAArrayDataElement* dataElement = dynamic_cast<QHLAArrayDataElement*>(object)) {
        return dataElement->size();
      } else if (QHLAFixedRecordDataElement* dataElement = dynamic_cast<QHLAFixedRecordDataElement*>(object)) {
        return dataElement->numFields();
      } else if (QHLADataElement* dataElement = dynamic_cast<QHLADataElement*>(object)) {
        return 0;
      } else {
        return 0;
      }
    }
    virtual int columnCount(const QModelIndex& parent) const
    {
      // Handle the root item
      if (!parent.isValid())
        return 2;
      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(parent.internalPointer());
      if (QRTIObjectInstance* objectInstance = dynamic_cast<QRTIObjectInstance*>(object)) {
        return 1;
      } else if (QRTIObjectInstanceAttribute* objectInstanceAttribute = dynamic_cast<QRTIObjectInstanceAttribute*>(object)) {
        return 2;
      } else if (QHLAScalarDataElement* dataElement = dynamic_cast<QHLAScalarDataElement*>(object)) {
        return 2;
      } else if (QHLAStringDataElement* dataElement = dynamic_cast<QHLAStringDataElement*>(object)) {
        return 2;
      } else if (QHLADataElement* dataElement = dynamic_cast<QHLADataElement*>(object)) {
        return 1;
      } else {
        return 0;
      }
    }
    virtual QVariant data(const QModelIndex& index, int role) const
    {
      if (role != Qt::DisplayRole)
        return QVariant();

      // A valid index, always contains a QObject
      QObject* object = static_cast<QObject*>(index.internalPointer());
      if (QRTIObjectInstance* objectInstance = dynamic_cast<QRTIObjectInstance*>(object)) {
        // object instances have the attributes as children
        switch (index.column()) {
        case 0:
          return objectInstance->name();
        default:
          return QVariant();
        }
      } else if (QRTIObjectInstanceAttribute* objectInstanceAttribute = dynamic_cast<QRTIObjectInstanceAttribute*>(object)) {
        // ObjectInstanceAttributes data
        switch (index.column()) {
        case 0:
          return objectInstanceAttribute->name();
        // case 1:
        //   return objectInstanceAttribute->_rawData.toHex();
        default:
          return QVariant();
        }
      } else if (QHLAScalarDataElement* dataElement = dynamic_cast<QHLAScalarDataElement*>(object)) {
        switch (index.column()) {
        case 0:
          return dataElement->dataType()->name();
        case 1:
          return dataElement->value();
        default:
          return QVariant();
        }
      } else if (QHLAStringDataElement* dataElement = dynamic_cast<QHLAStringDataElement*>(object)) {
        switch (index.column()) {
        case 0:
          return dataElement->dataType()->name();
        case 1:
          return dataElement->value();
        default:
          return QVariant();
        }
      } else if (QHLADataElement* dataElement = dynamic_cast<QHLADataElement*>(object)) {
        switch (index.column()) {
        case 0:
          return dataElement->dataType()->name();
        case 1:
          // return objectInstanceAttribute->_rawData.toHex();
        default:
          return QVariant();
        }
      } else {
        return QVariant();
      }
    }
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
      if (role != Qt::DisplayRole)
        return QVariant();
      switch (section) {
      case 0:
        return QString(tr("ObjectInstances"));
      default:
        return QVariant();
      }
    }

    void objectAttributeChanged(QRTIObjectInstanceAttribute* objectInstanceAttribute)
    {
      QModelIndex index = createIndex(objectInstanceAttribute->getIndexInObjectInstance(), 1, objectInstanceAttribute);
      emit dataChanged(index, index);
    }

  private:
    QRTIFederate* _federate;
  };

  QAbstractItemModel* getObjectInstanceListModel()
  { return _objectInstanceModel; }

// private:
  /// All the data types we know about
  QHLABasicDataTypeList* _basicDataTypeList;
  QHLASimpleDataTypeList* _simpleDataTypeList;
  QHLAEnumeratedDataTypeList* _enumeratedDataTypeList;
  QHLAArrayDataTypeList* _arrayDataTypeList;
  QHLAFixedRecordDataTypeList* _fixedRecordDataTypeList;
  QHLAVariantRecordDataTypeList* _variantRecordDataTypeList;

  ObjectClassModel* _objectClassModel;
  ObjectInstanceModel* _objectInstanceModel;
};

#endif
