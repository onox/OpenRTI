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

#ifndef QHLADataType_h
#define QHLADataType_h

#include <Qt/QtCore>

class QHLADataElement;
class QHLADataTypeVisitor;

class QHLADataType : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(unsigned alignment READ alignment)
public:
  QHLADataType(QObject* parent);
  virtual ~QHLADataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const = 0;

  const QString& name() const;
  void setName(const QString& name);

  unsigned alignment() const;

protected:
  void setAlignment(unsigned alignment);

private:
  QString _name;
  unsigned _alignment;
};

#endif
