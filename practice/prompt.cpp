//
// C++ Implementation: prompt
//
// Description: Implementation of the prompt (or question) displaying class
//
//
// Author: David Capel <wot.narg@gmail.com>, (C) 2008
//

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "prompt.h"

#include <KDebug>
#include <kio/netaccess.h>

TextualPrompt::TextualPrompt ( KSvgRenderer * renderer, const QString& elementId ) :
        m_renderer ( renderer )
{
    m_backgroundRect = renderer->boundsOnElement ( elementId );
    setPos ( m_backgroundRect.x() + m_backgroundRect.width() / 20.0, m_backgroundRect.y() + m_backgroundRect.height() / 4.0 );
    adjustSize();
};

void TextualPrompt::slotSetText ( const QString& text ) {setPlainText ( text ); };

ImagePrompt::ImagePrompt ( KSvgRenderer * renderer, QGraphicsView * view, const QString& elementId, QWidget * parent ) :
        QLabel ( parent ),
        m_pic ( QPixmap() ),
        m_renderer ( renderer )
{
    QRect bounds = m_renderer->boundsOnElement ( elementId ).toRect();
    setGeometry ( view->mapToScene ( bounds ).boundingRect().toRect() );

    slotSetImage ( KUrl() );
};

SoundPrompt::SoundPrompt ( KSvgRenderer * renderer, QGraphicsView * view, const QString& elementId, QWidget * parent ) :
        QPushButton ( parent ),
        m_renderer ( renderer )
{
    if ( !m_media )
    {
        m_media = new Phonon::MediaObject ( this );
        Phonon::AudioOutput * audioOutput = new Phonon::AudioOutput ( Phonon::MusicCategory, this );
        createPath ( m_media, audioOutput );
    }

    setText ( "Play Sound" );
    QRect bounds = m_renderer->boundsOnElement ( elementId ).toRect();
    setGeometry ( view->mapToScene ( bounds ).boundingRect().toRect() );

    connect ( this, SIGNAL ( clicked() ), this, SLOT ( slotPlay() ) );

    slotSetSound ( KUrl() );
};

void SoundPrompt::slotSetSound ( const KUrl& sound )
{
    // remove the temp file from the last call
    // I'll make sure this function is called at destruction to remove any lingering temp files.
    KIO::NetAccess::removeTempFile ( m_tmpFile );

    //kDebug() << sound;
    m_sound = sound;
    if ( !sound.isEmpty() )
    {
        QString tmpFile;
        if ( KIO::NetAccess::download ( sound, m_tmpFile, this ) )
        {
            m_media->setCurrentSource ( tmpFile );
            setVisible ( true );
        }
        else
        {
            //KMessageBox::error(this, KIO::NetAccess::lastErrorString());
            kDebug() << KIO::NetAccess::lastErrorString();
        }
    }
        else
            setVisible ( false );
}

SoundPrompt::~SoundPrompt()
{
    slotSetSound(KUrl()); // removes any lingering tmp files.
}

void SoundPrompt::slotPlay()
{
    m_media->setCurrentSource ( m_sound );
    m_media->play();
}

void ImagePrompt::slotSetImage ( const KUrl& image )
{
    kDebug() << image;
    if ( !image.isEmpty() )
    {
        QString tmpFile;
        if ( KIO::NetAccess::download ( image, tmpFile, this ) )
        {
            m_pic.load ( tmpFile );
            KIO::NetAccess::removeTempFile ( tmpFile );
            setVisible ( true );
        }
        else
        {
            //KMessageBox::error(this, KIO::NetAccess::lastErrorString());
            kDebug() << KIO::NetAccess::lastErrorString();
        }
    }
    else
    {
        m_pic = QPixmap();
        setVisible ( false );
    }
    setPixmap ( m_pic );
}
