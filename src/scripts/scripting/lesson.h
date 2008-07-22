/***************************************************************************

    Copyright 2008 Avgoustinos Kadis <avgoustinos.kadis@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SCRIPTINGSCRIPTOBJECTLESSON_H
#define SCRIPTINGSCRIPTOBJECTLESSON_H

#include <keduvoclesson.h>
#include "container.h"

#include <QObject>
#include <KSharedPtr>

namespace Scripting
{

    class Expression;

    /**
    Lesson script object class

        @author Avgoustinos Kadis <avgoustinos.kadis@kdemail.net>
    */
    class Lesson : public Container
    {
            Q_OBJECT
        public:

            Lesson ( KEduVocLesson * lesson );

            Lesson ( KEduVocContainer * container );

            Lesson ( const QString& name );

            ~Lesson();

        public slots:
            QVariantList entries ( bool recursive = false ) const;
            QObject * entry ( int row, bool recursive = false );
            int entryCount ( bool recursive = false );
            void appendEntry ( Expression * entry );

            /**
             * Creates and appends a new entry with the given @p translations
             * @code
             * #how to add a new entry with appendNewEntry()
             * import Parley
             * lesson = Parley.doc.rootLesson
             * lesson.appendNewEntry(["good morning","bonjour"])
             * lesson.appendNewEntry(["play","jouer"])
             * @endcode
             * @param translations A string list with the translations (in same order as their identifiers)
             */
            void appendNewEntry ( QStringList translations );

            /**
             * Insert an entry on a specific @p index
             * @param index Index where the entry will be added
             * @param entry The entry to add (can be created by newEntry() function)
             */
            void insertEntry ( int index, Expression * entry );

            /**
             * Removes an entry from this lesson
             * @code
             * #how to remove all the entries with the word "play" (from all lessons)
             * import Parley
             * for lesson in Parley.doc.allLessons():
             *     for entry in lesson.entries():
             *         for tr in entry.translations():
             *             if tr.text == "play":
             *                 lesson.remove(entry)
             * @endcode
             * @param entry A reference to the entry to remove
             */
            void removeEntry ( QObject * entry );

            /**
             * Creates and returns a new Expression Object
             * @return A new Expression object
             */
            QObject* newEntry();

            /**
             * Creates and returns a new Expression Object
             * @param translations
             * @return A new Expression object
             */
            QObject* newEntry ( QStringList translations );


            // @note this one doesn't work with the previous one (python doesn't know which one to call)
            /*
             * Creates and returns a new Expression Object
             * @param expression
             * @return A new Expression object
             */
//             QObject* newEntry ( const QString & expression );


            //child lesson public functions (just to change the names from "Container" to "Lesson")
            void appendChildLesson ( Lesson *child ) { appendChildContainer ( child ); }
            void insertChildLesson ( int row, Lesson *child ) { insertChildContainer ( row, child ); }
            void deleteChildLesson ( int row ) { deleteChildContainer ( row ); }
            void removeChildLesson ( int row ) { removeChildContainer ( row ); }
            QObject *childLesson ( int row ) { return new Lesson ( m_lesson->childContainer ( row ) ); }

            /**
             * Retrieve a child container by its name
             * Returns 0 if no container is found
             * @param name lesson name
             * @return the child lesson
             */
            QObject *childLesson ( const QString& name ) { return new Lesson ( m_lesson->childContainer ( name ) ); }

            QVariantList childLessons ( bool recursive = false );

            /**
             * Searches through all the child lessons (recursively) and returns the first lesson the specified @p name
             * @code
             * #how to search for a lesson
             * import Parley
             * lesson = Parley.doc.rootLesson.findChildLesson("Lesson 5")
             * if lesson != None:
             *     print "found"
             *     print lesson.name
             * else:
             *     print "not found"
             * @endcode
             * @param name Name of the lesson to look for
             * @return A reference to a lesson if found. 0 otherwise
             */
            QObject * findChildLesson ( const QString& name );

            int childLessonCount() const { return childContainerCount(); }

        private:
            KEduVocLesson* m_lesson;
    };

}

#endif
