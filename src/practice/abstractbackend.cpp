/***************************************************************************
    Copyright 2009 Daniel Laidig <d.laidig@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "abstractbackend.h"

using namespace Practice;

AbstractBackend::AbstractBackend(AbstractFrontend *frontend, QObject* parent)
    : QObject(parent)
{
    m_frontend = frontend;
}

#include "abstractbackend.moc"
