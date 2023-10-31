// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef COPILOT_H
#define COPILOT_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace CodeGeeX {
class Copilot : public QObject
{
    Q_OBJECT

public:
    Copilot();

    void postGenerate(const QString &url, const QString &apiKey, const QString &prompt, const QString &suffix);

    void postComment(const QString &url,
                     const QString &apiKey,
                     const QString &prompt,
                     const QString &lang,
                     const QString &locale,
                     const QString &apisecret = "");

    void postTranslate(const QString &url,
                       const QString &apiKey,
                       const QString &prompt,
                       const QString &src_lang,
                       const QString &dst_lang,
                       const QString &apisecret = "");

    void postFixBug(const QString &url,
                    const QString &apiKey,
                    const QString &prompt,
                    const QString &lang,
                    const QString &apisecret = "");

signals:
    void response(const QString &response);

private:
    QNetworkReply *postMessage(const QString &url, const QByteArray &body);

    QByteArray assembleGenerateBody(const QString &prompt,
                             const QString &suffix,
                             const QString &apikey,
                             int n = 1,
                             const QString &apisecret = "");

    QByteArray assembleCommentBody(const QString &prompt,
                             const QString &lang,
                             const QString &locale,
                             const QString &apikey,
                             const QString &apisecret = "");

    QByteArray assembleTranslateBody(const QString &prompt,
                             const QString &src_lang,
                             const QString &dst_lang,
                             const QString &apikey,
                             const QString &apisecret = "");

    QByteArray assembleBugfixBody(const QString &prompt,
                             const QString &lang,
                             const QString &apikey,
                             const QString &apisecret = "");

    void processResponse(QNetworkReply *reply);

    QNetworkAccessManager *manager = nullptr;
};

}// end namespace

#endif // COPILOT_H
