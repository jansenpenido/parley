/***************************************************************************
    Copyright 2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ABSTRACTBACKENDMODE_H
#define ABSTRACTBACKENDMODE_H

#include "practiceold/testentry.h"
#include "abstractfrontend.h"

#include <QVariant>

namespace Practice {
    
class AbstractBackendMode :public QObject
{
    Q_OBJECT
    
public:
    AbstractBackendMode(AbstractFrontend *frontend, QObject *parent) ;
    virtual ~AbstractBackendMode() {}
    virtual void setTestEntry(TestEntry* current) { m_current = current; }

    virtual qreal verifyAnswer(const QVariant& answer) = 0;
    void continueAction();
    
Q_SIGNALS:
    void nextEntry();
    
protected:
    AbstractFrontend* m_frontend;
    TestEntry* m_current;
};

}

#endif // ABSTRACTBACKENDMODE_H
