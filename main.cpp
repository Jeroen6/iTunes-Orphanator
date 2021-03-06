/*
MIT License

Copyright (c) 2016 Jeroen Lodder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include <QtXml>
#include <QUrl>
#include <QElapsedTimer>

#define USE_DBG_FILE false

#ifdef Q_OS_OSX
#warning Program not designed nor tested for OSX
#endif

#ifdef Q_OS_UNIX
#warning Program not designed nor tested for UNIX
#endif

#ifdef Q_OS_WIN

#endif

/// List of supported file types
char *iTunesFileTypes[] = {
    "mp3",
    "mp3",
    "m4a",
    "m4p",
    "m4v",
    "mov",
    "aac",
    "aiff"
};

// Scan directory
void RecurseDirectory(const QString& sDir, QList<QString> &dirFiles)
{
    QDir dir(sDir);
    QFileInfoList list = dir.entryInfoList();
    for (int iList=0;iList<list.count();iList++)
    {
        QFileInfo info = list[iList];
        QString sFilePath = info.filePath();
        if (info.isDir())
        {
            // recursive
            if (info.fileName()!=".." && info.fileName()!=".")
            {
                RecurseDirectory(sFilePath, dirFiles);
            }
        }
        else
        {
            dirFiles += sFilePath.toLower();
        }
    }
}

// Main application
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream cout(stdout);   /// Console output
    QElapsedTimer timer;        /// Performance measurement
    QList<QString> libFiles;    /// Files referenced by iTunes
    QList<QString> dirFiles;    /// Files found in directory
    cout << "Scanning " << endl;
    timer.start();

    // Stage 1 : scan exported XML for all Location keys
    QFile xmlFile;
    xmlFile.setFileName(QDir::homePath()+"\\Music\\iTunes\\Library.xml");
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Load XML File Problem.";
    }

    QXmlStreamReader xmlReader(&xmlFile);
    while(!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if(token == QXmlStreamReader::StartElement) {
            if(xmlReader.name() == "key") {
                QString NodeName="";
                QString NodeValue="";
                NodeName=(xmlReader.readElementText());
                xmlReader.readNext();
                if(xmlReader.name() == "string") {
                    NodeValue=(xmlReader.readElementText());
                }
                if(NodeName=="Location")
                {
                    libFiles += NodeValue;
                }
            }
        }
    }
    xmlFile.close();
    xmlReader.clear();

    // Stage 2 : Prepare extracted xml data
#if USE_DBG_FILE
    QFile fileDebug("Orphanator_xmlparsed.txt");      // in lib
    QTextStream logDebug(&fileDebug);
    fileDebug.open(QIODevice::WriteOnly | QIODevice::Truncate);
#endif
    // Remove html
    for(int i=0; i < libFiles.count(); i++)
    {
        // create local working copy
        QString w = libFiles.at(i);
#if USE_DBG_FILE
        logDebug << w << endl;
#endif
        // remove "file://localhost/"
        w.remove(0, 17);

        // unescape url % encodings
        QByteArray u;
        u.append(w);
        w = QUrl::fromPercentEncoding(u);

        // Since windows doesn't care about cases
        w = w.toLower();

        // replace extracted string with cleaned string
        libFiles.replace(i, w);
    }

    // Stage 3 : scan directory
    QString dir(QDir::homePath()+"\\Music\\iTunes\\iTunes Media\\Music");
    RecurseDirectory(dir, dirFiles);
    // Clean data
    for(int i=0; i < dirFiles.count(); i++){
        if(dirFiles.at(i).contains("AlbumArt")){
                dirFiles.removeAt(i); i--;
        }
        if(dirFiles.at(i).contains(".jpg")){
                dirFiles.removeAt(i); i--;
        }
    }

    // Stage 4 : Checking
    // Create a bunch of files
    QFile fileListed("Orphanator_listed.txt");      // in lib
    QFile fileExisting("Orphanator_existing.txt");  // in folder
    QFile fileMissing("Orphanator_unref.txt");      // missing from folder
    QFile fileOrphans("Orphanator_orhpans.txt");    // missing from lib

    // Expecting creation of write only files is allowed
    fileListed.open(QIODevice::WriteOnly | QIODevice::Truncate);
    fileExisting.open(QIODevice::WriteOnly | QIODevice::Truncate);
    fileMissing.open(QIODevice::WriteOnly | QIODevice::Truncate);
    fileOrphans.open(QIODevice::WriteOnly | QIODevice::Truncate);

    // Using stream to write to file
    QTextStream logListed(&fileListed);
    QTextStream logExisting(&fileExisting);
    QTextStream logMissing(&fileMissing);
    QTextStream logOrphans(&fileOrphans);

    // Check for dead links
    foreach (const QString &str, libFiles){
        logListed << str << endl;
        if(! QFile(str).exists()){
            logMissing << str << endl;
        }
    }

    // Check for orphan files
    foreach (const QString &str, dirFiles){
        logExisting << str << endl;
        bool found = false;
        foreach (const QString &substr, libFiles){
            if(substr == str){
                found = true;
                break;
            }
        }
        if(!found){
            logOrphans << str << endl;
        }
    }

    cout << "Done in " << timer.elapsed() << " ms" << endl;
    return a.exec();
}

