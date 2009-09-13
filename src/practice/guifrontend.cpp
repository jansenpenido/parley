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

#include "guifrontend.h"
#include "ui_practice_mainwindow.h"

#include <kxmlguiwindow.h>
#include <kdebug.h>
#include "writtenpracticewidget.h"
#include "flashcardmodewidget.h"

namespace Practice {
class MainWindow : public KXmlGuiWindow
{
public:
    MainWindow(QWidget* parent = 0, Qt::WindowFlags f = 0)
        : KXmlGuiWindow(parent, f) {};
};
}

using namespace Practice;

GuiFrontend::GuiFrontend(QObject* parent)
    : AbstractFrontend(parent), m_centralWidget(0)
{
    m_mainWindow = new MainWindow();
    QWidget* centralWidget = new QWidget(m_mainWindow);
    m_mainWindow->setCentralWidget(centralWidget);
    m_ui = new Ui::PracticeMainWindow();
    m_ui->setupUi(centralWidget);
    m_ui->centralPracticeWidget->setLayout(new QHBoxLayout(m_mainWindow));

}

QVariant GuiFrontend::userInput()
{
    return m_centralWidget->userInput();
}

KXmlGuiWindow* GuiFrontend::getWindow()
{
    return m_mainWindow;
}

void GuiFrontend::setMode(Mode mode)
{
    kDebug() << "setCentralWidget!";
    AbstractModeWidget *newWidget = 0;
    switch(mode) {
        case Written:
            if (/*m_centralWidget->metaObject()->className() == QLatin1String("WrittenPracticeWidget")*/false) {
                kDebug() << "Written practice widget is already the central widget";
                break;
            }
            newWidget = new WrittenPracticeWidget(m_mainWindow);
            break;
        case MultipleChoice:
            break;
        case FlashCard:
            newWidget = new FlashCardModeWidget(m_mainWindow);
            break;
        case MixedLetters:
            break;
    }
    if (newWidget) {
        m_ui->centralPracticeWidget->layout()->addWidget(newWidget);
        delete m_centralWidget;
        m_centralWidget = newWidget;
    }
}


void GuiFrontend::setLesson(const QString& lessonName)
{
    m_ui->lessonLabel->setText(i18nc("Display of the current lesson during practice", "Lesson: %1", lessonName));
}

void GuiFrontend::showQuestion()
{

    m_centralWidget->showQuestion();
/*
    // update progress bar
    m_ui->totalProgress->setMaximum(m_backend->totalEntryCount());
    m_ui->totalProgress->setValue(m_backend->practicedEntryCount());
    */
}


void GuiFrontend::showSolution()
{
    m_centralWidget->showSolution();
}


#include "guifrontend.moc"
