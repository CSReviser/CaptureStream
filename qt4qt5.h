#ifndef QT4QT5_H
#define QT4QT5_H

#if (QT_VERSION < 0x050000 && defined(Q_WS_WIN)||(QT_VERSION >= 0x050000 && defined(Q_OS_WIN)))
#define QT4_QT5_WIN
#endif

#if (QT_VERSION < 0x050000 && defined(Q_WS_MAC)||(QT_VERSION >= 0x050000 && defined(Q_OS_MACX)))
#define QT4_QT5_MAC
#endif

#if (QT_VERSION < 0x060000)
#define QT5
#endif

#if (QT_VERSION >= 0x060000)
#define QT6
#endif

#endif // QT4QT5_H
