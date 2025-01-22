/* POLEView - Graphical utility to view structure storage 
   Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, US
*/

#ifndef POLEVIEW
#define POLEVIEW

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QTreeWidget>
#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QTreeWidget>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QVBoxLayout>

#include "pole.h"

class PoleView : public QMainWindow
{
  Q_OBJECT
  
  public:
    PoleView();
    
  private slots:
  
    void newWindow();
    void choose();
    void openFile( const QString &fileName );
    void closeFile();
    void viewStream();
    void exportStream();
    void about();
    void aboutQt();

  private:
    PoleView( const PoleView& ); 
    PoleView& operator=( const PoleView& );
    POLE::Storage* storage;
    QTreeWidget* view;
};


class StreamView: public QDialog
{
  Q_OBJECT
  
  public:
    StreamView( POLE::Stream* stream );
    
  private slots:
    void loadStream();
    void goTop();
  
  private:
    StreamView( const StreamView& );
    StreamView& operator=( const StreamView& );
    void appendData( unsigned char* data, unsigned length );
    POLE::Stream* stream;
    QLabel* infoLabel;
    QTextEdit* log;  
};

#endif // POLEVIEW
