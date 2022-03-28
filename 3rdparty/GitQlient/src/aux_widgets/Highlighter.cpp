/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Copyright (C) 2020 Francesc Martinez
** LinkedIn: www.linkedin.com/in/cescmm/
** Web: www.francescmm.com
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "Highlighter.h"

namespace
{

void createHighlightningRules(QVector<Highlighter::HighlightingRule> &highlightingRules)
{
   if (highlightingRules.empty())
   {
      Highlighter::HighlightingRule rule;

      QTextCharFormat format;
      format.setForeground(QColor(255, 184, 108));
      rule.pattern = QRegularExpression(QStringLiteral("::[A-Za-z0-9_]+"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(219, 219, 168));
      rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(80, 200, 175));
      rule.pattern = QRegularExpression(QStringLiteral("new \\b[A-Za-z0-9_]+(?=\\()"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(87, 155, 213));
      const QString keywordPatterns[]
          = { QStringLiteral("\\bchar\\b"),     QStringLiteral("\\bclass\\b"),     QStringLiteral("\\bconst\\b"),
              QStringLiteral("\\bdouble\\b"),   QStringLiteral("\\benum\\b"),      QStringLiteral("\\bexplicit\\b"),
              QStringLiteral("\\bfriend\\b"),   QStringLiteral("\\binline\\b"),    QStringLiteral("\\bint\\b"),
              QStringLiteral("\\blong\\b"),     QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
              QStringLiteral("\\bprivate\\b"),  QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
              QStringLiteral("\\bshort\\b"),    QStringLiteral("\\bsignals\\b"),   QStringLiteral("\\bsigned\\b"),
              QStringLiteral("\\bslots\\b"),    QStringLiteral("\\bstatic\\b"),    QStringLiteral("\\bstruct\\b"),
              QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"),   QStringLiteral("\\btypename\\b"),
              QStringLiteral("\\bunion\\b"),    QStringLiteral("\\bunsigned\\b"),  QStringLiteral("\\bvirtual\\b"),
              QStringLiteral("\\bauto\\b"),     QStringLiteral("\\bexplicit\\b"),  QStringLiteral("\\bfinal\\b"),
              QStringLiteral("\\bnullptr\\b"),  QStringLiteral("\\boverride\\b"),  QStringLiteral("\\busing\\b"),
              QStringLiteral("\\bvoid\\b"),     QStringLiteral("\\bvolatile\\b"),  QStringLiteral("\\bbool\\b"),
              QStringLiteral("\\btrue\\b"),     QStringLiteral("\\bfalse\\b"),     QStringLiteral("\\bdelete\\b"),
              QStringLiteral("\\bnew\\b"),      QStringLiteral("\\bthis\\b") };

      for (const QString &pattern : keywordPatterns)
      {
         rule.pattern = QRegularExpression(pattern);
         rule.format = format;
         highlightingRules.append(rule);
      }

      format.setForeground(QColor(80, 200, 175));
      rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(98, 114, 164));
      rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(205, 144, 119));
      rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(219, 219, 168));
      rule.pattern = QRegularExpression(QStringLiteral("\\&[A-Za-z0-9_]+::[A-Za-z0-9_]+"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(80, 200, 175));
      rule.pattern = QRegularExpression(QStringLiteral("\\&?\\b[A-Za-z0-9_]+::"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(205, 144, 119));
      rule.pattern = QRegularExpression(QStringLiteral("<[A-Za-z0-9_\\.]+>"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(80, 200, 175));
      rule.pattern = QRegularExpression(QStringLiteral("[A-Za-z0-9_\\.]+<[A-Za-z0-9_\\.]+>"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(QColor(195, 133, 191));
      rule.pattern = QRegularExpression(QStringLiteral("#include"));
      rule.format = format;
      highlightingRules.append(rule);

      format.setForeground(Qt::white);
      rule.pattern = QRegularExpression(QStringLiteral("::"));
      rule.format = format;
      highlightingRules.append(rule);
   }
}

}

QVector<Highlighter::HighlightingRule> Highlighter::highlightingRules;

Highlighter::Highlighter(QTextDocument *parent)
   : QSyntaxHighlighter(parent)
{
   createHighlightningRules(highlightingRules);
   multiLineCommentFormat.setForeground(QColor(98, 114, 164));
   commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
   commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void Highlighter::highlightBlock(const QString &text)
{
   for (const HighlightingRule &rule : qAsConst(highlightingRules))
   {
      QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
      while (matchIterator.hasNext())
      {
         QRegularExpressionMatch match = matchIterator.next();
         setFormat(match.capturedStart(), match.capturedLength(), rule.format);
      }
   }
   setCurrentBlockState(0);

   int startIndex = 0;
   if (previousBlockState() != 1)
      startIndex = text.indexOf(commentStartExpression);

   while (startIndex >= 0)
   {
      QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
      int endIndex = match.capturedStart();
      int commentLength = 0;

      if (endIndex == -1)
      {
         setCurrentBlockState(1);
         commentLength = text.length() - startIndex;
      }
      else
      {
         commentLength = endIndex - startIndex + match.capturedLength();
      }
      setFormat(startIndex, commentLength, multiLineCommentFormat);
      startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
   }
}
