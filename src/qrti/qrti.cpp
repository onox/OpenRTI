/* -*-c++-*- OpenRTI - Copyright (C) 2010-2011 Mathias Froehlich
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

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <cstdlib>

#include <Qt/QtCore>
#include <Qt/QtGui>

#include "QRTI1516OMTHandler.h"
#include "QHLADataTypeVisitor.h"

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
#include "QRTIFederate.h"
#include "QRTI1516Federate.h"

/// Provides an iterface for various tree like structures while browsing the object model.
/// So consequently almost everything in the object model is derived from here.
class QTreeItemObject : public QObject {
public:
  QTreeItemObject(QObject* parent) :
    QObject(parent)
  { }
  virtual ~QTreeItemObject()
  { }

  enum ModelType {
    DataTypeModelType
  };

  // virtual bool validInModel(ModelType modelType) = 0;

  virtual int indexInParentItem(ModelType modelType) = 0;
  virtual QTreeItemObject* parentItem(ModelType modelType) = 0;

  virtual int numChildItems(ModelType modelType) = 0;
  virtual QTreeItemObject* childItem(ModelType modelType, int row) = 0;

  virtual int numColumns(ModelType modelType) = 0;
  virtual QVariant data(ModelType modelType, int column, int role) = 0;
  virtual Qt::ItemFlags flags(ModelType modelType) = 0;

// signals:
//   void dataChanged(int column);
//   void childInserted(int before);
};

class QTreeItemModel : public QAbstractItemModel {
public:
  QTreeItemModel(QObject* parent) :
    QAbstractItemModel(parent)
  {
  }
  virtual ~QTreeItemModel()
  {
  }

  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const
  {
    if (row < 0 || column < 0)
      return QModelIndex();
    if (!parent.isValid()) {
      // Handle the root object, child objects are the object classes
      if (!_rootItem)
        return QModelIndex();
      QTreeItemObject* item = _rootItem->childItem(_modelType, row);
      if (!item)
        return QModelIndex();
      return createIndex(row, column, item);
    } else {
      // A valid index, always contains a QObject
      QTreeItemObject* parentItem = static_cast<QTreeItemObject*>(parent.internalPointer());
      Q_ASSERT(parentItem);
      QTreeItemObject* childItem = parentItem->childItem(_modelType, row);
      if (!childItem)
        return QModelIndex();
      return createIndex(row, column, childItem);
    }
  }
  virtual QModelIndex parent(const QModelIndex& index) const
  {
    if (!index.isValid())
      return QModelIndex();

    // A valid index, always contains a QObject
    QTreeItemObject* item = static_cast<QTreeItemObject*>(index.internalPointer());
    Q_ASSERT(item);

    QTreeItemObject* parentItem = item->parentItem(_modelType);
    Q_ASSERT(parentItem);
    if (parentItem == _rootItem)
      return QModelIndex();

    return createIndex(parentItem->indexInParentItem(_modelType), 0, parentItem);
  }

  virtual int rowCount(const QModelIndex& parent) const
  {
    if (!_rootItem)
      return 0;
    // Handle the root item
    if (!parent.isValid())
      return _rootItem->numChildItems(_modelType);

    // A valid index, always contains a QObject
    QTreeItemObject* item = static_cast<QTreeItemObject*>(parent.internalPointer());
    Q_ASSERT(item);
    return item->numChildItems(_modelType);
  }
  virtual int columnCount(const QModelIndex& parent) const
  {
    if (!_rootItem)
      return 0;
    // Handle the root item
    if (!parent.isValid())
      return _rootItem->numColumns(_modelType);

    // A valid index, always contains a QObject
    QTreeItemObject* item = static_cast<QTreeItemObject*>(parent.internalPointer());
    Q_ASSERT(item);
    return item->numColumns(_modelType);
  }
  virtual QVariant data(const QModelIndex& index, int role) const
  {
    // A valid index, always contains a QObject
    QTreeItemObject* item = static_cast<QTreeItemObject*>(index.internalPointer());
    Q_ASSERT(item);
    return item->data(_modelType, index.column(), role);
  }
  virtual Qt::ItemFlags flags(const QModelIndex& index) const
  {
    // A valid index, always contains a QObject
    QTreeItemObject* item = static_cast<QTreeItemObject*>(index.internalPointer());
    Q_ASSERT(item);
    return item->flags(_modelType);
  }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (!_rootItem)
      return 0;
    return _rootItem->data(_modelType, section, role);
  }

  QTreeItemObject* _rootItem;
  QTreeItemObject::ModelType _modelType;
};

class HLABrowserMainWindow : public QMainWindow {
public:
  HLABrowserMainWindow() :
    _federate(0)
  {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Quit"), this, SLOT(close()));

    // QMenu* rtiMenu = menuBar()->addMenu("RTI");
    // QAction* joinFederation = rtiMenu->addAction("Join");

    _federate = new QRTI1516Federate(this);

    // {
    //   QDockWidget *dock = new QDockWidget(tr("DataTypes"), this);
    //   dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //   QTreeView* treeView = new QTreeView(dock);
    //   treeView->setModel(_federate->_dataTypeModel);
    //   dock->setWidget(treeView);
    //   addDockWidget(Qt::LeftDockWidgetArea, dock);
    // }

    // {
    //   QDockWidget *dock = new QDockWidget(tr("ObjectClasses"), this);
    //   dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //   QTreeView* treeView = new QTreeView(dock);
    //   treeView->setModel(_federate->getObjectClassListModel());
    //   dock->setWidget(treeView);
    //   addDockWidget(Qt::LeftDockWidgetArea, dock);
    // }

    // {
    //   QDockWidget *dock = new QDockWidget(tr("ObjectInstances"), this);
    //   dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //   QTreeView* treeView = new QTreeView(dock);
    //   treeView->setModel(_federate->getObjectInstanceListModel());
    //   dock->setWidget(treeView);
    //   addDockWidget(Qt::RightDockWidgetArea, dock);
    // }

    QTreeView* treeView = new QTreeView(this);
    treeView->setModel(_federate->getObjectInstanceListModel());
    setCentralWidget(treeView);
  }

  void joinFederation(const QString& federationExecution)
  {
    _federate->join(federationExecution);
  }

  QRTIFederate* getCurrentFederate()
  { return _federate; }

private:
  QRTIFederate* _federate;
};

int
main(int argc, char* argv[])
{
  // Q_INIT_RESOURCE(application);
  QApplication application(argc, argv);
  application.setApplicationName("HLA Browser");

  QString federationExecution;
  QString objectModel;

  QStringList arguments = QCoreApplication::arguments();
  for (int i = 1; i < arguments.size(); ++i) {
    if (arguments[i] == "-O" && i + 1 < arguments.size()) {
      objectModel = arguments[i + 1];
    } else if (arguments[i] == "-F" && i + 1 < arguments.size()) {
      federationExecution = arguments[i + 1];
    }
  }

  HLABrowserMainWindow mainWindow;
  if (!federationExecution.isEmpty())
    mainWindow.joinFederation(federationExecution);
  if (!objectModel.isEmpty() && mainWindow.getCurrentFederate())
    mainWindow.getCurrentFederate()->readObjectModel(objectModel);
  mainWindow.show();

  return application.exec();
}
