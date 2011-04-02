/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#include "ServerObjectModel.h"

namespace OpenRTI {

void
ObjectClass::insertObjectInstance(ObjectInstance* objectInstance)
{
  OpenRTIAssert(objectInstance);
  objectInstance->insertToObjectClassList(_objectInstanceList);
}

void
ObjectClass::eraseObjectInstance(ObjectInstance* objectInstance)
{
  OpenRTIAssert(objectInstance);
  objectInstance->eraseFromObjectClassList(_objectInstanceList);
}

void
ServerObjectModel::insert(const FOMModuleList& moduleList)
{
  for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
    insert(*i);
  }

  _fomModuleSet.insertModuleList(moduleList);
}

void
ServerObjectModel::insert(const FOMModule& module)
{
  for (FOMInteractionClassList::const_iterator i = module.getInteractionClassList().begin();
       i != module.getInteractionClassList().end(); ++i) {
    insertInteractionClass(*i, i->getParentInteractionClassHandle());
  }
  for (FOMObjectClassList::const_iterator i = module.getObjectClassList().begin();
       i != module.getObjectClassList().end(); ++i) {
    insertObjectClass(*i, i->getParentObjectClassHandle());
  }
}

void
ServerObjectModel::insertInteractionClass(const FOMInteractionClass& module, const InteractionClassHandle& parentHandle)
{
  if (getInteractionClass(module.getInteractionClassHandle())) {
    OpenRTIAssert(module.getParameterList().empty());
  } else {
    InteractionClass* parentInteractionClass = getInteractionClass(parentHandle);
    SharedPtr<InteractionClass> interactionClass;
    interactionClass = new InteractionClass(module.getName(), module.getInteractionClassHandle(), parentInteractionClass);
    // FIXME, this???
    // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
    insertInteractionClass(interactionClass);
  }
}

void
ServerObjectModel::insertObjectClass(const FOMObjectClass& module, const ObjectClassHandle& parentHandle)
{
  if (getObjectClass(module.getObjectClassHandle())) {
    OpenRTIAssert(module.getAttributeList().empty());
  } else {
    ObjectClass* parentObjectClass = getObjectClass(parentHandle);
    SharedPtr<ObjectClass> objectClass;
    objectClass = new ObjectClass(module.getName(), module.getObjectClassHandle(), parentObjectClass);

    if (parentObjectClass) {
      for (ObjectClassAttributeVector::const_iterator i = parentObjectClass->getObjectClassAttributeVector().begin();
           i != parentObjectClass->getObjectClassAttributeVector().end(); ++i) {
        // FIXME share these among object classes???
        SharedPtr<ObjectClassAttribute> attribute;
        attribute = new ObjectClassAttribute((*i)->getName(), (*i)->getHandle());
        // FIXME, this???
        // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
        objectClass->insertObjectClassAttribute(attribute);
      }
    }

    for (FOMAttributeList::const_iterator i = module.getAttributeList().begin();
         i != module.getAttributeList().end(); ++i) {
      // FIXME share these among object classes???
      SharedPtr<ObjectClassAttribute> attribute;
      attribute = new ObjectClassAttribute(i->getName(), i->getAttributeHandle());
      // FIXME, this???
      // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
      objectClass->insertObjectClassAttribute(attribute);
    }
    insertObjectClass(objectClass);
  }
}

}
