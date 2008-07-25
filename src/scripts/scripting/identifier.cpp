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
#include "identifier.h"

#include <QMap>

#include <KDebug>

namespace Scripting
{

    Identifier::Identifier()
            : QObject()
    {
        m_identifier = new KEduVocIdentifier();
    }

    Identifier::Identifier ( KEduVocIdentifier * identifier )
            : QObject(), m_identifier ( identifier )
    {
    }

    Identifier::Identifier ( KEduVocIdentifier & identifier )
            : QObject()
    {
        m_identifier = new KEduVocIdentifier ( identifier );
    }


    Identifier::~Identifier()
    {
    }

    QString Identifier::article ( KEduVocArticle::ArticleNumber number, KEduVocArticle::ArticleDefiniteness definite, KEduVocArticle::ArticleGender gender )
    {
        return m_identifier->article().article ( number,definite,gender );
    }


    void Identifier::setArticle ( const QString& article, KEduVocArticle::ArticleNumber number, KEduVocArticle::ArticleDefiniteness definite, KEduVocArticle::ArticleGender gender )
    {
        KEduVocArticle karticle = m_identifier->article();
        karticle.setArticle ( article, number, definite, gender );
        m_identifier->setArticle ( karticle );
    }

    QString Identifier::personalPronoun ( KEduVocConjugation::ConjugationNumber number, KEduVocConjugation::ConjugationPerson person ) const
    {
        return m_identifier->personalPronouns().personalPronoun ( person,number );
    }

    void Identifier::setPersonalPronoun ( const QString& conjugation,KEduVocConjugation::ConjugationNumber number, KEduVocConjugation::ConjugationPerson person )
    {
        KEduVocPersonalPronoun ppronouns = m_identifier->personalPronouns();
        ppronouns.setPersonalPronoun ( conjugation,person,number );
        m_identifier->setPersonalPronouns ( ppronouns );
    }

    QStringList Identifier::personalPronouns ()
    {
        KEduVocPersonalPronoun ppronouns = m_identifier->personalPronouns();

        QStringList list;

        for ( int n = 0; n <= KEduVocConjugation::Plural; n++ )
            for ( int p = 0; p <= KEduVocConjugation::ThirdNeutralCommon; p++ )
            {
                list << ppronouns.personalPronoun ( ( KEduVocConjugation::ConjugationPerson ) p, ( KEduVocConjugation::ConjugationNumber ) n );
            }

        return list;
    }

}