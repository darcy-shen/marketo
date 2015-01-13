#include "kmarkpad.h"
#include "markdown.h"

#include <string>
#include <iostream>
#include <sstream>

#include <KDE/KLocale>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KMessageBox>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/Cursor>

#include <QTimer>
#include <QList>
#include <QWebFrame>

KMarkPad::KMarkPad(QWidget *parent)
    : QWidget(parent)
{
    hl = new QHBoxLayout(this);
    hs = new QSplitter(this);
    previewer = new KWebView(this);
    m_livePreview = false;
    
    previewer->setTextSizeMultiplier(0.8);
    
    // TODO: new_editor should be carefully handled, may memory leak
    KTextEditor::Editor* new_editor = KTextEditor::EditorChooser::editor();
    if (!new_editor) {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                       "please check your KDE installation."));
        note = 0;
    } else {
        note = new_editor->createDocument(0);
        editor = qobject_cast<KTextEditor::View*>(note->createView(this));
    }
    hs->addWidget(previewer);
    hs->addWidget(editor);
    hl->addWidget(hs);
    
    QList<int> sizeList;
    sizeList << 400 << 400;
    hs->setSizes(sizeList);
    
    connect(note, SIGNAL(textChanged(KTextEditor::Document *)), 
        this, SLOT(updatePreviewer()));
    connect(editor, SIGNAL(cursorPositionChanged(KTextEditor::View *,const KTextEditor::Cursor&)),
        this, SLOT(updatePreviewerByCursor(KTextEditor::View *, const KTextEditor::Cursor&)));
}

void KMarkPad::preview(bool livePreview)
{
    std::stringstream html_ss;
    std::string html;
    markdown::Document processer;
    
    // Markdown rendering
    processer.read(std::string(note->text().toUtf8().constData()));
    processer.write(html_ss);
    html = html_ss.str();
    
    // Preview it
    m_livePreview = livePreview;
    previewer->setHtml(QString::fromUtf8(html.c_str()), QUrl());
    
    // Scroll to the correct position
    updatePreviewerByCursor(0, editor->cursorPosition());
    
    if (livePreview) {
        editor->setHidden(false);
        previewer->setHidden(false);
    } else {
        editor->setHidden(true);
        previewer->setHidden(false);
    }
}

void KMarkPad::unpreview()
{
    editor->setHidden(false);
    previewer->setHidden(true);
}

void KMarkPad::updatePreviewer()
{
    if (m_livePreview)
        QTimer::singleShot(1000, this, SLOT(preview()));
}

void KMarkPad::updatePreviewerByCursor(KTextEditor::View *a_editor, const KTextEditor::Cursor& a_cursor)
{
    int sourceTotal = note->lines();
    int sourceCur = a_cursor.line();
    int targetTotal = previewer->page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
    int targetCur = sourceCur * targetTotal / sourceTotal;
    
    previewer->page()->mainFrame()->setScrollPosition(QPoint(0, targetCur - 100));
}

KMarkPad::~KMarkPad()
{

}

#include "kmarkpad.moc"