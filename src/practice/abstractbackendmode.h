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

#include "testentry.h"
#include "practiceoptions.h"
#include "abstractfrontend.h"

#include <QVariant>

namespace Practice {
    
class AbstractBackendMode :public QObject
{
    Q_OBJECT
    
public:
    AbstractBackendMode(const PracticeOptions& practiceOptions, AbstractFrontend *frontend, QObject *parent);
    virtual ~AbstractBackendMode() {}
    
    /** start practicing a new word. sets some default that can be overwritten by the modes.
      * m_frontend->showQuestion() should be called after the initialization. */
    virtual void setTestEntry(TestEntry* current);
    
    /** add a new synonym to the list of shown/answered synonyms depending on which mode we
      * are in. */
    virtual void addSynonym(const QString& entry) { if (!entry.isEmpty()) m_synonyms.append(entry); }
    
public Q_SLOTS:
    /** the frontend requested to continue */
    virtual void continueAction() = 0;
    /** the frontend requested a hint */
    virtual void hintAction() = 0;

Q_SIGNALS:
    /** the current word is done. Grade it and maybe remove it */
    void currentEntryFinished();
    
    /** ask for the next word to be practiced */
    void nextEntry();

    /** the word passed is done. Grade it and maybe remove it */
    void gradeEntry(TestEntry*);
    
protected:
    PracticeOptions m_practiceOptions;
    AbstractFrontend* m_frontend;
    TestEntry* m_current;
    QStringList m_synonyms;
};

}

#endif // ABSTRACTBACKENDMODE_H