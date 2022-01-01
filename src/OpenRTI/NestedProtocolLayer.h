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

#ifndef OpenRTI_NestedProtocolLayer_h
#define OpenRTI_NestedProtocolLayer_h

#include "AbstractProtocolLayer.h"

namespace OpenRTI {

/// base class for all protocol layers that have a non leaf protocol layer behaviour
class OPENRTI_API NestedProtocolLayer : public AbstractProtocolLayer {
public:
  NestedProtocolLayer();
  virtual ~NestedProtocolLayer();

  // Is called from the parent protocol layer when there is data to read
  virtual void read(AbstractProtocolSocket&) = 0;
  virtual bool getEnableRead() const = 0;
  // Is called from the parent protocol layer when there is space to write something
  virtual void write(AbstractProtocolSocket&) = 0;
  virtual bool getEnableWrite() const = 0;

  virtual void error(const Exception& e) = 0;

  void setProtocolLayer(const SharedPtr<AbstractProtocolLayer>& protocolLayer)
  { _protocolLayer = protocolLayer; }
  const SharedPtr<AbstractProtocolLayer>& getProtocolLayer() const
  { return _protocolLayer; }

private:
  SharedPtr<AbstractProtocolLayer> _protocolLayer;
};

} // namespace OpenRTI

#endif
