/***************************************************************************
    Copyright 2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DECLENSIONWIDGET_H
#define DECLENSIONWIDGET_H

#include "ui_declensionwidget.h"

#include <keduvocdeclension.h>

class KEduVocExpression;
class KEduVocDocument;

namespace Editor
{

class DeclensionWidget : public QWidget, public Ui::DeclensionWidget
{
    Q_OBJECT

public:
    explicit DeclensionWidget(QWidget *parent = 0);

public slots:
    void setDocument(KEduVocDocument* doc);
    void setTranslation(KEduVocExpression* entry, int translation);

    /**
     * Fill the line edits
     */
    void updateEntries();

    void nextNumber();

signals:
    void sigModified();

private slots:
    void textChanged(const QString&);

private:
    int currentAdditionalWordFlag();
    void setupLineEdits();

    int m_identifier;
    KEduVocExpression* m_entry;
    KEduVocDocument* m_doc;

    /**
     * All line edits and labels, index corresponding to KEduVocWordFlag::indexOf
     */
    QMap< int, QLineEdit* > m_DeclensionLineEdits;
};

}

#endif
