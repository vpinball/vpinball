/* POLEView - Graphical utility to view structure storage 
   Copyright (C) 2005 Ariya Hidayat <ariya@kde.org>

   Redistribution and use in source and binary forms, with or without 
   modification, are permitted provided that the following conditions 
   are met:
   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors may be 
     used to endorse or promote products derived from this software without 
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
   THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "poleview.h"
#include "pole.h"

PoleView::PoleView()
{
  storage = 0;
  
  view = new QTreeWidget( this );
  view->setColumnCount( 2 ); 
  view->setHeaderLabels( QStringList() << tr("Name" ) << tr("Size" ) );
  
  view->setAlternatingRowColors ( true );
  setCentralWidget( view );
  
  // create actions

  // file
  QAction *openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(choose()));

  QAction *newWindowAct = new QAction(tr("&New Window"), this);
  newWindowAct->setShortcut(tr("Ctrl+N"));
  connect(newWindowAct, SIGNAL(triggered()), this, SLOT(newWindow()));

  QAction *closeAct = new QAction(tr("&Close"), this);
  closeAct->setShortcut(tr("Ctrl+C"));
  connect(closeAct, SIGNAL(triggered()), this, SLOT(closeFile()));

  QAction *exitAct = new QAction(tr("&Quit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // stream
  QAction *exportAct = new QAction(tr("&Export..."), this);
  connect(exportAct, SIGNAL(triggered()), this, SLOT(exportStream()));

  QAction *viewAct = new QAction(tr("&View..."), this);
  connect(viewAct, SIGNAL(triggered()), this, SLOT(viewStream()));

  // help
  QAction *aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  QAction *aboutQtAct = new QAction(tr("About Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));


  // create menus
  QMenu *fileMenu = new QMenu(tr("&File"), this);
  fileMenu->addAction(openAct);
  fileMenu->addAction(newWindowAct);
  fileMenu->addAction(closeAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  QMenu *streamMenu = new QMenu(tr("&Stream"), this);
  streamMenu->addAction(exportAct);
  streamMenu->addAction(viewAct);

  QMenu *helpMenu = new QMenu(tr("&Help"), this);
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);


  menuBar()->addMenu(fileMenu);
  menuBar()->addMenu(streamMenu);
  menuBar()->addMenu(helpMenu);

  resize( 400, 300 );
  setWindowTitle( tr("POLEView" ) );
  statusBar()->showMessage( tr("Ready"), 5000 );
}

void PoleView::newWindow()
{
  PoleView* v = new PoleView();
  v->show();
}

void PoleView::choose()
{
  QString form("%1 (%2)" );
  QString allTypes( "*.doc *.xls *.xla *.ppt *.dot *.xlt *.pps" );
  QString filter1 = QString(form).arg( tr("Microsoft Office Files") ).arg( allTypes );
  QString filter2a = QString(form).arg( tr("Microsoft Word Document") ).arg( "*.doc" );
  QString filter2b = QString(form).arg( tr("Microsoft Word Template") ).arg( "*.dot" );
  QString filter3a = QString(form).arg( tr("Microsoft Excel Workbook") ).arg( "*.xls" );
  QString filter3b = QString(form).arg( tr("Microsoft Excel Template") ).arg( "*.xlt" );
  QString filter3c = QString(form).arg( tr("Microsoft Excel Add-In") ).arg( "*.xla" );
  QString filter4a = QString(form).arg( tr("Microsoft PowerPoint Presentation") ).arg( "*.ppt" );
  QString filter4b = QString(form).arg( tr("Microsoft PowerPoint Template") ).arg( "*.pps" );
  QString filter5 = QString(form).arg( tr("All Files") ).arg( "*" );
  
  QString filter = filter5.append( ";;" ).append( filter1 );
  filter = filter.append( ";;" ).append( filter2a );
  filter = filter.append( ";;" ).append( filter2b );
  filter = filter.append( ";;" ).append( filter3a );
  filter = filter.append( ";;" ).append( filter3b );
  filter = filter.append( ";;" ).append( filter3c );
  filter = filter.append( ";;" ).append( filter4a );
  filter = filter.append( ";;" ).append( filter4b );
  
  QString fn = QFileDialog::getOpenFileName( 0, "", "", filter );
  
  if ( !fn.isEmpty() ) openFile( fn );
  else
    statusBar()->showMessage( tr("Loading aborted"), 2000 );
}

class StreamItem: public QTreeWidgetItem
{
public:
  StreamItem( QTreeWidgetItem* parent, const QString& name, POLE::Stream* stream = 0 );
  StreamItem( QTreeWidget* parent, const QString& name  );
  QString name;
  POLE::Stream* stream;  
};

StreamItem::StreamItem( QTreeWidgetItem* parent, const QString& n, POLE::Stream* s ): 
QTreeWidgetItem( parent )
{
  name = n;
  setText(0, name);
  stream = s;
  if( stream )
    setText( 1, QString::number( stream->size() ) );
  //setOpen( true );
}

StreamItem::StreamItem( QTreeWidget* parent, const QString& n ):
QTreeWidgetItem( parent )
{
  name = n;
  setText(0, name);

  stream = 0;
  //setOpen( true );
}

void visit( QTreeWidgetItem* parent, POLE::Storage* storage, const std::string path  )
{
  std::list<std::string> entries;
  entries = storage->entries( path );

  std::list<std::string>::iterator it;
  for( it = entries.begin(); it != entries.end(); ++it )
  {
    std::string name = *it;
    std::string fullname = path + name;
    
    if( storage->isDirectory( fullname ) )
    {
      StreamItem* item = new StreamItem( parent, name.c_str() );
      visit( item, storage, fullname + "/" );
    }
    else
      new StreamItem( parent, name.c_str(), new POLE::Stream( storage, fullname.c_str() ) );
  }
}

void PoleView::openFile( const QString &fileName )
{
  if( storage ) closeFile();
  
  QTime t; t.start();  
  storage = new POLE::Storage( fileName.toLatin1().data() );
  storage->open();

  if( storage->result() != POLE::Storage::Ok )
  {
    QString msg = QString( tr("Unable to open file %1\n") ).arg(fileName);
    QMessageBox::critical( 0, tr("Error"), msg );
    closeFile();
    return;
  }
  
  QString msg = QString( tr("Loading %1 (%2 ms)") ).arg( fileName ).arg( t.elapsed() );
  statusBar()->showMessage( msg, 2000 );
  
  view->clear();
  StreamItem* root = new StreamItem( view, tr("Root") );
  visit( root, storage, "/" );

  setWindowTitle( QString( tr("%1 - POLEView" ).arg( fileName ) ) );
}

void PoleView::closeFile()
{
  if( storage )
  {
    storage->close();
    delete storage;
  }
  
  storage = 0;
  view->clear();
  setWindowTitle( tr("POLEView" ) );
}

void PoleView::viewStream() {
/*
  QModelIndex idx = view->currentIndex();
  for (QModelIndexList::iterator it=idxList.begin(); it<idxList.end(); ++it)
    if ( it->column() == 0 )
      files << model->filePath( *it );
*/



  StreamItem* item = (StreamItem*) view->currentItem ();
  if( !item )
  {
    QMessageBox::warning( 0, tr("View Stream"),
      tr("Nothing is selected"), 
      QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }
  
  QString name = item->text( 0 );
  if( !item->stream )
  {
    QMessageBox::warning( 0, tr("View Stream"),
      tr("'%1' is not a stream").arg( name ), 
      QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }
  
  StreamView* sv = new StreamView( item->stream );
  
  sv->setWindowTitle( name );
  QTimer::singleShot( 200, sv, SLOT( show() ) ); 
}

void PoleView::exportStream()
{
  StreamItem* item = (StreamItem*) view->currentItem();
  if( !item )
  {
    QMessageBox::warning( 0, tr("View Stream"),
      tr("Nothing is selected"), 
      QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }
  
  QString name = item->text( 0 );
  if( !item->stream )
  {
    QMessageBox::warning( 0, tr("View Stream"),
      tr("'%1' is not a stream").arg( name ), 
      QMessageBox::Ok, QMessageBox::NoButton );
    return;
  }
  
  QString fn = QFileDialog::getSaveFileName( 0, tr("Export Stream As"), name );
  if ( fn.isEmpty() )
  {
    statusBar()->showMessage( tr("Export aborted"), 2000 );
    return;
  }
  
  unsigned char buffer[16];
  QFile outf( fn );
  if( !outf.open( QIODevice::WriteOnly ) )
  {
    QString msg = QString( tr("Unable to write to file %1\n") ).arg(fn);
    QMessageBox::critical( 0, tr("Error"), msg );
    return;
  }
    
  statusBar()->showMessage( tr("Exporting... Please wait") );
  for( ;; )
  {
      unsigned read = item->stream->read( buffer, sizeof( buffer ) );
      outf.write( (const char*)buffer, read  );
      if( read < sizeof( buffer ) ) break;
  }
  outf.close();
  statusBar()->showMessage( tr("Stream is exported."), 2000 );
}

void PoleView::about()
{
  QMessageBox::about( this, tr("About POLEView"),
    tr("Simple structured storage viewer\n"
    "Copyright (C) 2004 Ariya Hidayat (ariya@kde.org)"));
}

void PoleView::aboutQt()
{
  QMessageBox::aboutQt( this, tr("POLEView") );
}

#define STREAM_MAX_SIZE 32  // in KB

StreamView::StreamView( POLE::Stream* s ): QDialog( 0 )
{
  stream = s;
  setModal( false );
  
  QVBoxLayout* layout = new QVBoxLayout( this );
  //layout->setAutoAdd( true );
  layout->setMargin( 10 );
  layout->setSpacing( 5 );
  
  infoLabel = new QLabel( this );
  
  log = new QTextEdit( this );
  //log->setTextFormat( Qt::LogText );
  //log->setFont( QFont("Courier") );
  //log->setMinimumSize( 500, 300 );

  layout->addWidget( infoLabel );
  layout->addWidget( log );  
  QTimer::singleShot( 0, this, SLOT( loadStream() ) ); 
}

void StreamView::loadStream()
{
  unsigned size = stream->size();
  
  if( size > STREAM_MAX_SIZE*1024 )
  {
    infoLabel->setText( tr("This stream is too large. "
      "Only the first %1 KB is shown.").arg( STREAM_MAX_SIZE ) );
    size = STREAM_MAX_SIZE*1024;
  }
  
  unsigned char buffer[16];
  stream->seek( 0 );
  for( unsigned j = 0; j < size; j+= 16 )
  {
    unsigned read = stream->read( buffer, 16 );  
    appendData( buffer, read );
    if( read < sizeof( buffer ) ) break;
  }
  
  QTimer::singleShot( 100, this, SLOT( goTop() ) );  
}

void StreamView::goTop()
{
  //log->ensureVisible( 0, 0 );
}

void StreamView::appendData( unsigned char* data, unsigned length )
{
  QString msg;
  for( unsigned i = 0; i < length; i++ )
  {
    QString s = QString::number( data[i], 16 );
    while( s.length() < 2 ) s.prepend( '0' );
    msg.append( s );
    msg.append( ' ' );
  }
  msg.append( "   " );
  for( unsigned i = 0; i < length; i++ )
  {
    if( (data[i]>31) && (data[i]<128) )
    {
      if( data[i] == '<' ) msg.append( "&lt;" );
      else if( data[i] == '>' ) msg.append( "&gt;" );
      else if( data[i] == '&' ) msg.append( "&amp;" );
      else msg.append( data[i] );
    }
    else msg.append( '.' );
  }
  
  log->append( msg );
}

// --- main program

#include <qapplication.h>

int main( int argc, char ** argv )
{
  QApplication a( argc, argv );

  PoleView* v = new PoleView();
  v->show();
  
  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
